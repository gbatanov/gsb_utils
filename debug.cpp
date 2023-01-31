#include "version.h"
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
#include <syslog.h>
#include "gsbutils.h"
#include "debug.h"

// в маке пишет в сислог, но посмотреть можно только командой в консоли: (gsblog - имя программы)
// log show --info --debug --predicate "process == 'gsb'"
// log show --info --debug  --process gsb --last 10m

// НЕ забывать при старте программы в дебаговом режиме запускать поток
//   std::thread msgT = std::thread(gsbutils::printMsg);
// а в релизном режиме открывать сислог
//    openlog("gsb", LOG_PID, LOG_LOCAL7); //(local7.log), на маке все пишет в общий лог

#define MSG_BUFF_SIZE (4096)

namespace gsbutils
{
    std::atomic<bool> Flag{true};
    std::mutex log_mutex;
    std::queue<std::string> msg_queue;
    int debug_level = 1;

    void printMsg()
    {
        while (Flag.load())
        {
            if (!msg_queue.empty())
            {
                std::string msg;
                {
                    std::lock_guard<std::mutex> lg(log_mutex);
                    msg = msg_queue.front();
                    msg_queue.pop();
                    std::cout << msg.c_str();
                }
            }
            else
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1s);
            }
        }
    }

    void set_debug_level(int level)
    {
        debug_level = level;
    }

    void dprintf_c(int level, std::string fmt, ...)
    {

        if (level > LOG_DEBUG)
            level = LOG_DEBUG;
        if (level <= debug_level)
        {
            char buf[MSG_BUFF_SIZE]{0};

            va_list ap;
            va_start(ap, fmt);
            int len = vsnprintf(buf, MSG_BUFF_SIZE, ("  " + fmt).c_str(), ap);
            buf[len] = 0;
            va_end(ap);
            std::lock_guard<std::mutex> lg(log_mutex);
            msg_queue.push(std::string(buf));
        }
    }

    // Отрицательный level выводит в консоль
    void dprintf(int level, std::string fmt, ...)
    {

        if (level <= debug_level)
        {
            va_list ap;
            char buf[MSG_BUFF_SIZE]{0};
            char buf1[MSG_BUFF_SIZE - 32]{0};
            size_t len = 0, len1 = 0;

            if (level < 0)
            {
                std::time_t t1 = std::time(nullptr); // текущее время timestamp
                std::tm tm = *std::localtime(&t1);   // structure
                len = std::strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S ", &tm);
                buf[len] = 0;
            }
            va_start(ap, fmt);
            len1 = vsnprintf(buf1, MSG_BUFF_SIZE - 32, fmt.c_str(), ap);
            if (len1 == 0)
                return;
            buf1[len1] = 0;
            va_end(ap);

            if (level < 0)
            {
                strncat(buf, buf1, len1);
                std::lock_guard<std::mutex> lg(log_mutex);
                msg_queue.push(std::string(buf));
            }
            else
                syslog(level, "%s", buf1);
        }
    }
}