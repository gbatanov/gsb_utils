#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <gsbutils.h>

using namespace std;
typedef void (*cb_timer)(void*);

static void thread1(gsbutils::Context* ctx, bool& res) {
	gsbutils::Context ctx1 = gsbutils::Context::create_with_timeout(*ctx, 6);
	int i = 0;
	while (!ctx1.Done()) {
		this_thread::sleep_for(1000ms);
		cout << format("Work1 {}\n", i);
		i++;
	}
	if (ctx->Done())
		res = false;
	else if (ctx1.Done())
		res = true;
	else
		res = false;
}

static void thread2(gsbutils::Context* ctx, bool& res) {
	int i = 0;
	while (!ctx->Done()) {
		this_thread::sleep_for(1000ms);
		cout << format("Work2 {}\n", i);
		i++;
	}
	res = false;
}

// коллбэк-функция таймера с параметром
static void ctxcancel(void* ctx) {

	cout << "ctxancel\n";
	static_cast<gsbutils::Context*>(ctx)->Cancel();
}

int main(int argc, char** argv)
{
	gsbutils::ConsoleCP cp(1251);

	bool res1 = false;
	bool res2 = false;

	gsbutils::Context ctxmain;
	gsbutils::Context ctx = gsbutils::Context::create_with_timeout(ctxmain, 5); // 5 - bad1, 7 - good1

	gsbutils::TTimer t(&ctxmain, (uint64_t)8, (cb_timer)ctxcancel, (void*)&ctxmain);
	t.run();

	thread t1(thread1, &ctx, std::ref(res1)); // отменяется по таймауту из ctx через 5 секунд (выходит сам через 6)
	thread t2(thread2, &ctxmain, std::ref(res2));// глобальная отмена через 8 секунд
	if (t1.joinable())
		t1.join();
	if (t2.joinable())
		t2.join();

	cout << (uint16_t)ctxmain.Error() << endl;
	cout << (uint16_t)ctx.Error() << endl;

	if (res1) {
		cout << "good1\n";
	}
	else {
		cout << "\nbad1\n";
	}
	if (res2) {
		cout << "good2\n";
	}
	else {
		cout << "\nbad2\n";
	}
}
