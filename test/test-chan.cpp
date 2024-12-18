// Пример использования самописного канала
#include <iostream>
#include <thread>
#include <format>
#include <chrono>
#include <Channel.h>

using namespace std;
Channel<std::string> chan(2);

static void thread1() {
	for (int i = 0; i < 20; i++) {
		std::cout << std::format("Send: {}", "nechto") << i << std::endl;
		chan.write(std::format("chan_msg {}", i));
		this_thread::sleep_for(500ms);
	}
	chan.stop();
}

static void thread2() {
	while (!chan.is_stopped()) {
		bool ok = true;
		std::string str = chan.read(&ok);
		if (!ok)
			break;
		std::string str2 = chan.read(&ok);
		if (!ok)
			break;
		std::cout << std::format("Result2: {} {}\n", str,str2);
		this_thread::sleep_for(1200ms);
	}
}
static void thread3() {
	while (!chan.is_stopped()) {
		bool ok = true;
		std::string str = chan.read(&ok);
		if (!ok)
			break;
		std::cout << std::format("Result3: {}\n", str);
		this_thread::sleep_for(1300ms);
	}
}
static void thread4() {
	while (!chan.is_stopped()) {
		bool ok = true;
		std::string str = chan.read(&ok);
		if (!ok)
			break;
		std::cout << std::format("Result4: {}\n", str);
		this_thread::sleep_for(1400ms);
	}
}

int main()
{
	cout << "Hello CMake." << endl;

	std::thread t1(thread1);
	std::thread t2(thread2);
	std::thread t3(thread3);
	std::thread t4(thread4);

	if (t1.joinable())
		t1.join();
	if (t2.joinable())
		t2.join();
	if (t3.joinable())
		t3.join();
	if (t4.joinable())
		t4.join();

	return 0;
}

