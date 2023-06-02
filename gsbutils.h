#ifndef GSBUTILS_H
#define GSBUTILS_H

#include <stdio.h>
#include <string>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <ctime>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <syslog.h>
#include <vector>


/// @brief Набор полезных мне утилит
namespace gsbutils
{
#include "debug.h"
#include "SStrings.h"
#include "TTimer.h"
#include "DDate.h"
#include "Channel.h"
}

#ifdef DEBUG
#define ERRLOG(fmt, args...) gsbutils::dprintf(1, "[ERROR][%s:%d]" fmt, __func__, __LINE__, ##args)
#define DBGLOG(fmt, args...) gsbutils::dprintf(3, "[DEBUG][%s:%d]" fmt, __func__, __LINE__, ##args)
#define INFOLOG(fmt, args...) gsbutils::dprintf(2, "[Info]" fmt, ##args)
#else
#define ERRLOG(fmt, args...) gsbutils::dprintf(1, "[ERROR]" fmt, ##args)
#define INFOLOG(fmt, args...) gsbutils::dprintf(2, "[Info]" fmt, ##args)
#define DBGLOG(x, ...)
#endif

#endif
