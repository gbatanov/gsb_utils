#include <memory>
#include <string>
#include <thread>
#include <gsbutils.h>



typedef void (*tfunc)();
typedef void (*tfunc_arg)(void*);

gsbutils::Channel<std::string> chan(1);

std::condition_variable gsbutils::DDebug::cv;
std::mutex  gsbutils::DDebug::log_mutex, gsbutils::DDebug::cv_m;
std::queue<std::string>  gsbutils::DDebug::msg_queue;
int  gsbutils::DDebug::debug_level;
int  gsbutils::DDebug::output; // 0 - console, default 1 - syslog -1 - stop debug output, close DDebug
std::thread  gsbutils::DDebug::msgt;

static void cbfunc()
{
	gsbutils::DDebug::dprintf(3, "chan.write: chan_msg\n");
	chan.write("chan_msg");
}
static void cbfunc2()
{
	gsbutils::DDebug::dprintf(3, "Timer2 cycle\n");
}

int main(int argc, char** argv)
{
	gsbutils::Context ctx;
	gsbutils::DDebug::init(&ctx, 0, NULL);
	gsbutils::DDebug::set_debug_level(7);

	gsbutils::DDebug::dprintf(3, "Start: %s\n", (gsbutils::DDate::current_time()).c_str());

	// Работа со строками
	std::string ts1 = "iuyoyoiu";
	std::string ts2 = gsbutils::SString::remove_after(ts1, "oi");
	std::string ts3 = gsbutils::SString::remove_before(ts1, "oi");
	gsbutils::DDebug::dprintf(3, "%s %s %s\n", ts1.c_str(), ts2.c_str(), ts3.c_str());
	std::string ts4 = ts1;
	std::string ts5 = ts1;
	std::string ts6 = ts1;
	gsbutils::SString::replace_first(ts4, "yo", "ХАХА");
	gsbutils::SString::replace_all(ts5, "yo", "ХиХи");
	gsbutils::SString::remove_all(ts6, "yo");
	gsbutils::DDebug::dprintf(3, "%s  %s %s %s\n", ts1.c_str(), ts4.c_str(), ts5.c_str(), ts6.c_str());

	std::string primTrim = "\t   test string trim  \r";
	std::string afterTrim = gsbutils::SString::trim(primTrim);
	gsbutils::DDebug::dprintf(3, "%s\n", afterTrim.c_str());

	std::string splitString = "eeg, egrwe,etwettut";
	std::vector<std::string> splittedString = gsbutils::SString::split(splitString, ",");

	for (auto& str : splittedString) {
		gsbutils::DDebug::dprintf(3, "%s\n", str.c_str());
	}


	gsbutils::DDebug::dprintf(3, "Hour of day: %d\n", gsbutils::DDate::get_hour_of_day());

	gsbutils::TTimer t(&ctx,(uint64_t)6, (tfunc)cbfunc);
	t.run();
	gsbutils::DDebug::dprintf(3, "Timer1 started\n");
	std::string inMsg = "";

	// Здесь будет задержка, пока коллбэк функция таймера не запишет в канал
	inMsg = chan.read(NULL);
	gsbutils::DDebug::dprintf(3, "after Timer1 inMsg=%s \n", inMsg.c_str());
	gsbutils::CycleTimer tc(&ctx, (uint64_t)1, (tfunc)cbfunc2);
	tc.run();

	std::string url = "qwewr=kjhkjh&tetyeye&uio=333";
	gsbutils::DDebug::dprintf(3, "Source string:  %s \n", url.c_str());
	auto resultParse = gsbutils::SString::parse_string_with_delimiter(url, "&", "=");

	for (auto& it : resultParse)
	{
		if (it.second.empty())
			gsbutils::DDebug::dprintf(3, "%s \n", it.first.c_str());
		else
			gsbutils::DDebug::dprintf(3, "%s=%s \n", it.first.c_str(), it.second.c_str());
	}
	// Повторяем запуск таймера 1
	t.run();
	// Делаем паузу, чтобы увидеть срабатывание таймера 
	// и вывод циклического таймера каждую секунду
	std::this_thread::sleep_for(std::chrono::seconds(10));

	tc.stop();

	gsbutils::DDebug::stop();
	return 0;
}