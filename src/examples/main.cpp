#include <iostream>
#include <boost/thread.hpp>
#include "boost/noncopyable.hpp"

#include "boost_asio.h"

using namespace std;

template <typename T>
class basic_atom : public boost::noncopyable
{
private:
	T n;
	mutex mtx;

public:
	basic_atom(T x = T()) : n(x) {}
	T operator++()
	{
		//boost::mutex::scoped_lock lock(mtx);
		return ++n;
	}
	operator T() { return n; }
};

namespace ns_boost_thread
{
	boost::mutex io_mu;
	typedef basic_atom<uint32_t> atom_u32;
	void print(atom_u32 &x, const std::string &str)
	{
		for (uint32_t i = 0; i < 5; i++)
		{
			boost::mutex::scoped_lock lock(io_mu);
			std::cout << str << ++x << std::endl;
		}
	}

	void main()
	{
		atom_u32 x;
		boost::thread(print, boost::ref(x), "hello");
		boost::thread(print, boost::ref(x), "boost");
		boost::thread(print, boost::ref(x), "thread");
		boost::this_thread::sleep(boost::posix_time::seconds(2));
	}
} // namespace ns_boost_thread

int32_t main()
{
	std::cout << "Hello, main\n";
	//ns_boost_thread::main();
	ns_boost_timer::main();
	return 0;
}