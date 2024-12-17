
#include <stdio.h>
#include <stdarg.h>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <iostream>
#include <mutex>
#include <queue>
#include <atomic>
#include <thread>
#include <chrono>
#ifndef Win32
#include <syslog.h>
#endif
#include <condition_variable>
#include "version.h"
#include "gsbutils.h"
#include "debug.h"

#ifdef Win32
constexpr auto LOG_DEBUG = 1;
#endif

// в маке пишет в сислог, но посмотреть можно только командой в консоли: (gsblog - имя программы)
// log show --info --debug --predicate "process == 'gsb'"
// log show --info --debug  --process gsb --last 10m

// НЕ забывать при старте программы в дебаговом режиме запускать поток
//   std::thread msgT = std::thread(gsbutils::printMsg);
// а в релизном режиме открывать сислог
//    openlog("gsb", LOG_PID, LOG_LOCAL7); //(local7.log), на маке все пишет в общий лог

// до версии 0.8.27 вся инициализация на стороне вызывающей программы
// с версии 0.8.27 в вызывающей программе достаточно вызывать gsbutils::init(output),
// а при завершении программы - gsbutils::stop()
// output - направление вывода - консоль(0) или сислог (1)

#define MSG_BUFF_SIZE (4096)

namespace gsbutils
{
	std::condition_variable cv;
	std::mutex log_mutex, cv_m;
	std::queue<std::string> msg_queue;
	int debug_level = 1;
	std::atomic<bool> Flag{ true };
	int output = 0; // 0 - console, default 1 - syslog
	std::thread msgt;

	std::string version()
	{
		return std::string(Project_VERSION_MAJOR) + "." + Project_VERSION_MINOR + "." + Project_VERSION_PATCH;
	}

	void printMsg()
	{
		while (Flag.load())
		{
			std::string msg;
			std::unique_lock<std::mutex> lk(cv_m);
			while (Flag.load())
			{
				if (std::cv_status::timeout == cv.wait_for(lk, std::chrono::seconds(2)))
				{
					continue;
				}
				else if (!msg_queue.empty())
				{
					break;
				}
			}

			lk.unlock();
			{
				std::lock_guard<std::mutex> lg(log_mutex);
				while (!msg_queue.empty() && Flag.load())
				{
					msg = msg_queue.front();
					msg_queue.pop();
					std::cout << msg.c_str();
				}
				std::cout.flush();
			}
		}
	}


	void init(int output_, const char* name)
	{
		output = output_ == 0 ? 0 : 1;
		Flag.store(true);
#ifndef Win32
		if (output)
			openlog(name, LOG_PID, LOG_LOCAL7); //(local7.log)
		else
#endif
			msgt = std::thread(printMsg);
	}

	void stop()
	{
		Flag.store(false);
#ifndef Win32
		if (output)
			closelog();
		else if (msgt.joinable())
#endif
			msgt.join();
	}

	void set_flag(bool flag)
	{
		Flag.store(flag);
	}
	/// @brief
	/// @param level 0 отключает всякий отладочный вывод куда-либо
	void set_debug_level(int level)
	{
		debug_level = level;
	}

	void dprintf_c(int level, std::string fmt, ...)
	{
		if (output != 0)
			return;
		if (level > LOG_DEBUG)
			level = LOG_DEBUG;
		if (level <= debug_level)
		{
			char buf[MSG_BUFF_SIZE]{ 0 };

			va_list ap;
			va_start(ap, fmt);
			int len = vsnprintf(buf, MSG_BUFF_SIZE, ("  " + fmt).c_str(), ap);
			if (len < 0)
				return;
			buf[len] = 0;
			va_end(ap);
			std::lock_guard<std::mutex> lg(log_mutex);
			msg_queue.push(std::string(buf));
			cv.notify_one();
		}
	}

	void dprintf(int level, std::string fmt, ...)
	{

		if (level <= debug_level)
		{
			va_list ap;
			char buf[MSG_BUFF_SIZE]{ 0 };
			char buf1[MSG_BUFF_SIZE - 32]{ 0 };
			size_t len = 0, len1 = 0;

			if (output == 0)
			{
				std::time_t t1 = std::time(nullptr); // текущее время timestamp
				std::tm tm = *std::localtime(&t1);   // structure
				len = std::strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S ", &tm);
				if (len > -1 && len < MSG_BUFF_SIZE)
					buf[len] = 0;
			}
			va_start(ap, fmt);
			len1 = vsnprintf(buf1, MSG_BUFF_SIZE - 32, fmt.c_str(), ap);
			if (len1 < 1 || len1 >(MSG_BUFF_SIZE - 33))
				return;
			buf1[len1] = 0;
			va_end(ap);

			if (output == 0)
			{
				strncat(buf, buf1, len1);
				std::lock_guard<std::mutex> lg(log_mutex);
				msg_queue.push(std::string(buf));
				cv.notify_one();
			}
#ifndef Win32
			else
				syslog(level, "%s", buf1);
#endif
		}
	}
}