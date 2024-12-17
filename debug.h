#ifndef DEBUG_H
#define DEBUG_H

#include <string>

	void init(int output, const char* name = (const char*)"gsb");
	void stop();
	void set_flag(bool flag);
	//	void set_output(int output);
	void dprintf(int level, std::string fmt, ...);
	void printMsg();
	void set_debug_level(int level);
	void dprintf_c(int level, std::string fmt, ...);

#endif
