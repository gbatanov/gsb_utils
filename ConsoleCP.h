#include "Windows.h"

class ConsoleCP
{

public:
	ConsoleCP(int cp)
	{
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
	int oldin;
	int oldout;

};
