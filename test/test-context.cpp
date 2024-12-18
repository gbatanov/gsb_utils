#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <gsbutils.h>

using namespace std;

static void thread1(bool* res) {
	gsbutils::TTimer t(6);
	t.run();
	int i = 0;
	while (!t.done) {
		this_thread::sleep_for(1000ms);
		cout << format("Work {}", i) << endl;
		i++;
	}
	if (res)
		*res = true;
}

static bool long_func(gsbutils::Context* ctx) {
	ctx->start();
	bool tres = false;
	thread t1(thread1, &tres);
	while (!ctx->Done()) {
		if (tres) {
			// функция завешилась до таймаута
			// несмотря на это, надо явно ждать завершения потока,
			// иначеен ошибка!
			if (t1.joinable())
				t1.join();
			return true;
		}
	}

	// таймаут сработал раньше
	t1.detach();
	return false;

}

int main(int argc, char** argv)
{
	gsbutils::Context* ctx = gsbutils::Context::create_with_timeout(10);
	bool res = long_func(ctx);

	if (res) {
		cout << "good\n";
	}
	else {
		cout << "\nbad\n";
	}

}
