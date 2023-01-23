#ifndef DEBUG_H
#define DEBUG_H

#include <string>

void dprintf(unsigned int level, std::string fmt, ...);
void printMsg();
void set_debug_level(unsigned int level);
void dprintf_c(unsigned int level, std::string fmt, ...);

#endif
