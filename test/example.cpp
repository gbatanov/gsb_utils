#include <memory>
#include <string>
#include <thread>
#include <gsbutils.h>



typedef void (*tfunc)();

using namespace gsbutils;

Channel<std::string> chan(1);

std::condition_variable DDebug::cv;
std::mutex  DDebug::log_mutex, DDebug::cv_m;
std::queue<std::string>  DDebug::msg_queue;
int  DDebug::debug_level;
int  DDebug::output; // 0 - console, default 1 - syslog
std::thread  DDebug::msgt;
gsbutils::Context * DDebug::dctx;

static void cbfunc()
{
	DDebug::dprintf(3, "chan.write: chan_msg\n");
	chan.write("chan_msg");
}
static void cbfunc2()
{
	DDebug::dprintf(3, "Timer2 cycle\n");
}

int main(int argc, char** argv)
{
	Context* ctx = Context::create();
	DDebug::init(ctx, 0, NULL);
	DDebug::set_debug_level(7);

	DDebug::dprintf(3, "Start: %s\n", (DDate::current_time()).c_str());

	// Работа со строками
	std::string ts1 = "iuyoyoiu";
	std::string ts2 = SString::remove_after(ts1, "oi");
	std::string ts3 = SString::remove_before(ts1, "oi");
	DDebug::dprintf(3, "%s %s %s\n", ts1.c_str(), ts2.c_str(), ts3.c_str());
	std::string ts4 = ts1;
	std::string ts5 = ts1;
	std::string ts6 = ts1;
	SString::replace_first(ts4, "yo", "ХАХА");
	SString::replace_all(ts5, "yo", "ХиХи");
	SString::remove_all(ts6, "yo");
	DDebug::dprintf(3, "%s  %s %s %s\n", ts1.c_str(), ts4.c_str(), ts5.c_str(), ts6.c_str());

	std::string primTrim = "\t   test string trim  \r";
	std::string afterTrim = SString::trim(primTrim);
	DDebug::dprintf(3, "%s\n", afterTrim.c_str());

	std::string splitString = "eeg, egrwe,etwettut";
	std::vector<std::string> splittedString = SString::split(splitString, ",");

	for (auto& str : splittedString) {
		DDebug::dprintf(3, "%s\n", str.c_str());
	}


	DDebug::dprintf(3, "Hour of day: %d\n", DDate::get_hour_of_day());

	TTimer t(6, cbfunc);
	t.run();
	DDebug::dprintf(3, "Timer1 started\n");
	std::string inMsg = "";

	// Здесь будет задержка, пока коллбэк функция таймера не запишет в канал
	inMsg = chan.read(NULL);
	DDebug::dprintf(3, "after Timer1 inMsg=%s \n", inMsg.c_str());
	CycleTimer tc(1, cbfunc2);
	tc.run();

	std::string url = "qwewr=kjhkjh&tetyeye&uio=333";
	DDebug::dprintf(3, "Source string:  %s \n", url.c_str());
	auto resultParse = SString::parse_string_with_delimiter(url, "&", "=");

	for (auto& it : resultParse)
	{
		if (it.second.empty())
			DDebug::dprintf(3, "%s \n", it.first.c_str());
		else
			DDebug::dprintf(3, "%s=%s \n", it.first.c_str(), it.second.c_str());
	}
	// Повторяем запуск таймера 1
	t.run();
	// Делаем паузу, чтобы увидеть срабатывание таймера 
	// и вывод циклического таймера каждую секунду
	std::this_thread::sleep_for(std::chrono::seconds(10));
	DDebug::stop();
	return 0;
}