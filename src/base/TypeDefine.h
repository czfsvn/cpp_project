#pragma once

#include <algorithm>
#include <sstream>

namespace cncpp
{
    template <typename T>
    std::string toStr(const T& t)
    {
        return to_string(t);
    }

    template <typename T>
    T cast(const std::string& src)
    {
        T                 t;
        std::stringstream stream;
        stream << src;
        stream >> t;
        return t;
    }

    /*  example
    const std::list<std::string>& result = strSplit<std::list<std::string>>(line, " ");
    */
    template <typename Cont>
    inline Cont strSplit(const std::string& src, const std::string& sep)
    {
        Cont        cont = {};
        std::string s;
        for (std::string::const_iterator i = src.begin(); i != src.end(); i++)
        {
            if (sep.find(*i) != std::string::npos)
            {
                if (s.length())
                    cont.emplace_back(s);
                s = "";
            }
            else
            {
                s += *i;
            }
        }

        if (!s.empty())
        {
            cont.emplace_back(s);
        }

        return cont;
    }

    /* example
    const std::list<std::string>&   res1 = str_split<std::list>(line, " ");
    const std::vector<std::string>& res2 = str_split<std::vector>(line, " ");
    const std::deque<std::string>&  res3 = str_split<std::deque>(line, " ");
    */
    template <template <typename T, typename = std::allocator<T>> class Cont>
    inline Cont<std::string> str_split(const std::string& src, const std::string& sep)
    {
        Cont<std::string> cont = {};
        std::string       s;
        for (std::string::const_iterator i = src.begin(); i != src.end(); i++)
        {
            if (sep.find(*i) != std::string::npos)
            {
                if (s.length())
                    cont.emplace_back(s);
                s = "";
            }
            else
            {
                s += *i;
            }
        }

        if (!s.empty())
        {
            cont.emplace_back(s);
        }

        return cont;
    }

    inline std::string& replace_all(std::string& src, const std::string& old_str, const std::string& new_str)
    {
        while (true)
        {
            std::string::size_type pos = src.find(old_str);
            if (pos == std::string::npos)
                break;

            src.replace(pos, old_str.length(), new_str);
            /*
            if ((pos = str.find(old_str)) != std::string::npos)
                str.replace(pos, old_str.length(), new_str);
            else
                break;
                */
        }

        return src;
    }

    inline void to_upper(std::string& src)
    {
        std::transform(src.begin(), src.end(), src.begin(),
            [](unsigned char c)
            {
                return std::toupper(c);
            });
    }

    inline void to_lower(std::string& src)
    {
        std::transform(src.begin(), src.end(), src.begin(),
            [](unsigned char c)
            {
                return std::tolower(c);
            });
    }

    std::string formatStr(const char* format, ...)
    {
        char    str[1024] = {};
        va_list ap;
        va_start(ap, format);
        vsnprintf(str, sizeof str, format, ap);
        va_end(ap);
        return str;
    }

}  // namespace cncpp
