#ifdef WIN32
// Устанавливает CodePage в консоли Windows
// Примечание: проверялось только на Windows 11
#include "Windows.h"

class ConsoleCP
{

public:
	ConsoleCP(uint32_t cp)
	{
#ifdef UNICODE
		std::setlocale(LC_ALL, ".UTF8"); // Или ".65001" для UTF-8
#endif
		oldin = GetConsoleCP();
		oldout = GetConsoleOutputCP();
		SetConsoleCP(cp);
		SetConsoleOutputCP(cp);
	}

	~ConsoleCP()
	{
		SetConsoleCP(oldin);
		SetConsoleOutputCP(oldout);
	}
private:
	uint32_t oldin;
	uint32_t oldout;

};
#else
class ConsoleCP
{

public:
	ConsoleCP(int cp)
	{
	}

	~ConsoleCP()
	{
	}
};
#endif
