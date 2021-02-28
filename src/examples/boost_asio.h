#ifndef __BOOST_ASIO_20201125_H__
#define __BOOST_ASIO_20201125_H__

#include <iostream>
#include <boost/asio.hpp>

using namespace std;

namespace ns_boost_timer
{
	void sync_wait()
	{
		boost::asio::io_service ios;
		boost::asio::deadline_timer timer(ios, boost::posix_time::seconds(2));
		//std::cout << (uint32_t)timer.expires_at() << std::end;
		timer.wait();
		std::cout << "hello,asio deadline_timer\n";
	}

	void print()
	{
		std::cout << "hello,asio print, now£º" << time(nullptr) << std::endl;
	}

	void print2()
	{
		std::cout << "hello, boost print, now: " << time(nullptr) << std::endl;
	}

	void async_wait()
	{
		boost::asio::io_service ios;
		boost::asio::deadline_timer timer(ios, boost::posix_time::seconds(2));
	}

	class Timer
	{
	private:
		uint32_t inteval = 0;
		boost::function<void()> func;
		boost::asio::deadline_timer timer;
		boost::asio::io_service& io_service;

	public:
		template<typename FUNC>
		Timer(boost::asio::io_service& ios, uint32_t ti, FUNC&& fun):io_service(ios), 
			inteval(ti), func(std::move(fun)), timer(ios, boost::posix_time::milliseconds(ti))
		{
			timer.async_wait(boost::bind(&Timer::callBack, this, boost::asio::placeholders::error));
		}

		void callBack(const boost::system::error_code& err)
		{
			func();
			//timer.expires_at(timer.expires_at() + boost::posix_time::millisec(inteval));
			timer.expires_from_now(boost::asio::deadline_timer::duration_type(0, 0, 0, inteval * 1000));
			timer.async_wait(boost::bind(&Timer::callBack, this, boost::asio::placeholders::error));
		}
	};

	void testTimer()
	{
		boost::asio::io_service ios;
		Timer timer1(ios, 2000, print);
		Timer timer2(ios, 4000, print2);

		ios.run();
	}

	void main()
	{
		//sync_wait();
		testTimer();
	}
}





#endif
