#ifndef __SAFE_CONTAINER_H__
#define __SAFE_CONTAINER_H__

#include <unordered_map>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/thread/thread.hpp>

namespace cncpp
{
    class Noncopyable
    {
    protected:
        Noncopyable()  = default;
        ~Noncopyable() = default;

    private:
        Noncopyable(const Noncopyable &) = delete;
        const Noncopyable &operator=(const Noncopyable &) = delete;
    };

    template<typename T>
    class SafeQueue
    {
    private:
        struct Node
        {
            std::shared_ptr<T>    data;
            std::unique_ptr<Node> next;
        };

        std::mutex              head_mutex;
        std::mutex              tail_mutex;
        std::unique_ptr<Node>   head;
        Node *                  tail;
        std::condition_variable data_cond;

    private:
        Node *get_tail()
        {
            std::lock_guard<std::mutex> tail_lk(tail_mutex);
            return tail;
        }

        std::unique_ptr<Node> pop_head()
        {
            std::unique_ptr<Node> old_head = std::move(head);
            head                           = std::move(old_head->next);
            return old_head;
        }

        std::unique_lock<std::mutex> wait_for_data()
        {
            std::unique_lock<std::mutex> head_lock(head_mutex);
            data_cond.wait(head_lock, [&] { head.get() != get_tail(); });
            return std::move(head_lock);
        }

        std::unique_ptr<Node> wait_pop_head()
        {
            std::unique_ptr<std::mutex> head_lock(wait_for_data());
            return pop_head();
        }

        std::unique_ptr<Node> wait_pop_head(T &value)
        {
            std::unique_ptr<std::mutex> head_lock(wait_for_data());
            value = std::move(*head->data);
            return pop_head();
        }

    private:
        std::unique_ptr<Node> try_pop_head()
        {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            if (head.get() == get_tail())
            {
                return std::unique_ptr<Node>();
            }
            return pop_head();
        }

        std::unique_ptr<Node> try_pop_head(T &value)
        {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            if (head.get() == get_tail())
            {
                return std::unique_ptr<Node>();
            }
            value = std::move(*head->data);
            return pop_head();
        }

    public:
        SafeQueue()
            : head(new Node)
            , tail(head.get())
        {}

        SafeQueue(const SafeQueue &other) = delete;
        SafeQueue &operator=(const SafeQueue &other) = delete;

        void emplace_back(T &&new_val)
        {
            std::shared_ptr<T>    new_data(std::make_shared<T>(std::move(new_val)));
            std::unique_ptr<Node> p(new Node);
            {
                std::lock_guard<std::mutex> tail_lock(tail_mutex);
                tail->data           = new_data;
                const Node *new_tail = p.get();
                tail->next           = std::move(p);
                tail                 = new_tail;
            }
            data_cond.notify_one();
        }

        void emplace_back(const T &new_val)
        {
            std::shared_ptr<T>    new_data(std::make_shared<T>(std::move(new_val)));
            std::unique_ptr<Node> p(new Node);
            {
                std::lock_guard<std::mutex> tail_lock(tail_mutex);
                tail->data           = new_data;
                const Node *new_tail = p.get();
                tail->next           = std::move(p);
                tail                 = new_tail;
            }
            data_cond.notify_one();
        }

        void push_back(T &&new_val)
        {
            emplace_back(new_val);
        }

        void push_back(const T &new_val)
        {
            emplace_back(new_val);
        }

        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_ptr<Node> const old_head = wait_pop_head();
            return old_head->data;
        }

        void wait_and_pop(T &value)
        {
            std::unique_ptr<Node> const old_head = wait_pop_head(value);
        }

        std::shared_ptr<T> try_pop()
        {
            std::unique_ptr<Node> old_head = try_pop_head();
            return old_head ? old_head->data : std::shared_ptr<T>();
        }

        bool try_pop(T &value)
        {
            std::unique_ptr<Node> const old_head = try_pop_head(value);
            return old_head;
        }

        bool empty()
        {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            return (head.get() == get_tail());
        }
    };

    template<typename Key, typename Value>
    class SafeMap : Noncopyable
    {
        typedef std::unordered_map<Key, Value> map_type;
        typedef boost::shared_mutex            rw_mutex;
        typedef boost::shared_lock<rw_mutex>   read_lock;
        typedef std::unique_lock<rw_mutex>     write_lock;

    public:
        typedef typename map_type::key_type    key_type;
        typedef typename map_type::mapped_type mapped_type;
        typedef typename map_type::value_type  value_type;

        typedef typename map_type::pointer       pointer;
        typedef typename map_type::const_pointer const_pointer;

        typedef typename map_type::size_type       size_type;
        typedef typename map_type::difference_type different_type;
        typedef typename map_type::reference       reference;
        typedef typename map_type::const_reference const_reference;
        typedef typename map_type::iterator        iterator;
        typedef typename map_type::const_iterator  const_iterator;

    private:
        map_type         m_map = {};
        mutable rw_mutex m_mutex;

    public:
        bool empty() const
        {
            read_lock lock(m_mutex);
            return m_map.empty();
        }

        size_type size() const
        {
            read_lock lock(m_mutex);
            return m_map.size();
        }

        size_type max_size() const
        {
            read_lock lock(m_mutex);
            return m_map.max_size();
        }

        iterator begin() const
        {
            read_lock lock(m_mutex);
            return m_map.begin();
        }

        const_iterator begin()
        {
            read_lock lock(m_mutex);
            return m_map.begin();
        }

        iterator end() const
        {
            read_lock lock(m_mutex);
            return m_map.end();
        }

        const_iterator end()
        {
            read_lock lock(m_mutex);
            return m_map.end();
        }

        bool insert(const key_type &k, const mapped_type &v)
        {
            write_lock lock(m_mutex);
            return m_map.insert(value_type(k, v)).second;
        }

        bool emplace(const key_type &k, const mapped_type &v)
        {
            write_lock lock(m_mutex);
            return m_map.emplace(value_type(k, v)).second;
        }

        bool find(const key_type &k)
        {
            read_lock lock(m_mutex);
            return m_map.find(k) != m_map.end();
        }

        size_type erase(const key_type &k)
        {
            write_lock lock(m_mutex);
            return m_map.erase(k);
        }

        void clear()
        {
            write_lock lock(m_mutex);
            m_map.clear();
        }

        typedef boost::optional<mapped_type> optional_mapped_type;
        optional_mapped_type                 at(const key_type &k)
        {
            read_lock lock(m_mutex);
            if (m_map.find(k) != m_map.end())
                return optional_mapped_type(m_map[k]);

            return optional_mapped_type();
        }

        const mapped_type &operator[](const key_type &k)
        {
            read_lock lock(m_mutex);
            BOOST_ASSERT(m_map.find(k) != m_map.end());
            return m_map[k];
        }

        void set(const key_type &k, const mapped_type &v)
        {
            write_lock lock(m_mutex);
            m_map[k] = v;
        }

        template<typename Func>
        void for_each(Func func)
        {
            read_lock lock(m_mutex);
            std::for_each(m_map.begin(), m_map.end(), func);
        }
    };
} // namespace cncpp

#endif