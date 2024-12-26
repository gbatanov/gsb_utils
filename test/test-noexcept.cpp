#include <iostream>

// Эта функция бросает исключение, которое будет перехвачено в main
static void func1(int b) {
	throw std::overflow_error("too big");
}

// Эта функция бросает исключение, которое остановит приложение
// и не будет перехвачено в main
static void func2(int b) noexcept {
	throw std::overflow_error("too small");
}

int main() {
	std::cout << "start \n";
	try
	{
		std::cout << "A \n";
		func1(0);
		std::cout << "AA \n";
	}
	catch (const std::exception& e)
	{
		std::cout << "func1: " << e.what() << "\n";
	}
	try
	{
		std::cout << "B \n";
		func2(0);
		std::cout << "BB \n";
	}
	catch (const std::exception& e)
	{
		std::cout << "func2: " << e.what() << "\n";
	}

	return 0;
}