#ifndef DEBUG_H
#define DEBUG_H

#include <string>

void set_flag(bool flag);
void set_output(int where);
void dprintf(int level, std::string fmt, ...);
void printMsg();
void set_debug_level(int level);
void dprintf_c(int level, std::string fmt, ...);

#endif
