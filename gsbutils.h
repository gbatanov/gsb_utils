#ifndef GSBUTILS_H
#define GSBUTILS_H

#include <stdio.h>
#include <string>
#include <cstring>
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
#ifndef Win32
#include <syslog.h>
#endif
#include <queue>
#include <algorithm>
#include <stdarg.h>
#include <fstream>
#include <iomanip>
#include "version.h"

/// @brief Набор полезных мне утилит
namespace gsbutils
{
#include "SStrings.h"
#include "TTimer.h"
#include "DDate.h"
#include "Channel.h"
#include "ThreadPool.h"
#include "Context.h"
#include "Debug.h"
}

#endif