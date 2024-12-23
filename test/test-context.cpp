#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <gsbutils.h>

using namespace std;

gsbutils::Context* ctxmain = gsbutils::Context::create();

static void thread1(gsbutils::Context* ctx, bool* res) {
	gsbutils::TTimer t(ctx,6);
	t.run();
	int i = 0;
	while (!ctx->Done() && !t.Done()) {
		this_thread::sleep_for(1000ms);
		cout << format("Work1 {}\n", i);
		i++;
	}
	if (t.Done()) {
		if (res)
			*res = true;
		return;
	}
	if (ctx->Done()) {
		if (res)
			*res = false;
		return;
	}
	if (res)
		*res = false;
}

static void thread2(gsbutils::Context* ctx, bool* res) {
	int i = 0;
	while (!ctx->Done()) {
		this_thread::sleep_for(1000ms);
		cout << format("Work2 {}\n", i);
		i++;
	}
	if (res)
		*res = false;
}

static void ctxcancel() {
	ctxmain->Cancel();
}

int main(int argc, char** argv)
{
	bool res1 = false;
	bool res2 = false;

	gsbutils::Context* ctx = gsbutils::Context::create_with_timeout(ctxmain, 12);

	gsbutils::TTimer t(ctx,8, ctxcancel);
	t.run();

	thread t1(thread1, ctx, &res1); // отменяется по таймауту через 12 секунд (выходит сам через 6)
	thread t2(thread2, ctxmain, &res2);// глобальная отмена через 8 секунд
	if (t1.joinable())
		t1.join();
	if (t2.joinable())
		t2.join();

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
