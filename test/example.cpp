#include <memory>
#include <string>
#include "../gsbutils.h"

std::atomic<bool> Flag{true};

typedef void (*tfunc)();
// std::shared_ptr<gsbutils::Channel<std::string>> chan;
gsbutils::Channel<std::string> chan(1);

static void cbfunc()
{
    INFOLOG("Timer1 done\n","");
    chan.write("chan_msg");
}
static void cbfunc2()
{
   gsbutils::dprintf(1, "Timer2 cycle\n","");
}
int main(int argc, char **argv)
{
    //    chan = std::make_shared<gsbutils::Channel<std::string>>(1);
    gsbutils::init(0, (const char *)"gsb");
    gsbutils::set_debug_level(3);
    std::cout << (gsbutils::DDate::current_time()).c_str() << std::endl;

    std::string ts1 = "iuyoyoiu";
    std::string ts2 = gsbutils::SString::remove_after(ts1, "oi");
    std::string ts3 = gsbutils::SString::remove_before(ts1, "oi");
    printf("%s %s %s\n", ts1.c_str(), ts2.c_str(), ts3.c_str());
    std::string ts4 = ts1;
    std::string ts5 = ts1;
    std::string ts6 = ts1;
    gsbutils::SString::replace_first(ts4, "yo", "ХАХА");
    gsbutils::SString::replace_all(ts5, "yo", "ХиХи");
    gsbutils::SString::remove_all(ts6, "yo");
    printf("%s  %s %s %s\n", ts1.c_str(), ts4.c_str(), ts5.c_str(), ts6.c_str());

    std::string primTrim = "\t   test string trim  \r";
    std::string afterTrim = gsbutils::SString::trim(primTrim);
    gsbutils::dprintf(1, "%s\n", afterTrim.c_str());

    std::string splitString = "eeg, egrwe,etwettut";
    std::vector<std::string> splittedString = gsbutils::SString::split(splitString, ",");

    for (auto & str : splittedString){
        gsbutils::dprintf(1, "%s\n", str.c_str());
    }

    gsbutils::dprintf(1, "Час дня: %d \n", gsbutils::DDate::get_hour_of_day());

    gsbutils::TTimer t(10, cbfunc);
    t.run();
    INFOLOG("Timer1 started\n","");
    std::string inMsg = "";

    // Здесь будет задержка на 10 секунд, пока коллбэк функция таймера не запишет в канал
    inMsg = chan.read(NULL);
    INFOLOG("after Timer1 inMsg= %s \n", inMsg.c_str());
    gsbutils::CycleTimer tc(1, cbfunc2);
    tc.run();

    std::string url = "qwewr=kjhkjh&tetyeye&uio=333";
    printf("Source string:  %s \n", url.c_str());
    auto resultParse = gsbutils::SString::parse_string_with_delimiter(url, "&", "=");

    for (auto &it : resultParse)
    {
        if (it.second.empty())
            printf("%s \n", it.first.c_str());
        else
            printf("%s=%s \n", it.first.c_str(), it.second.c_str());
    }
    t.run();
    // Делаем паузу, чтобы увидеть срабатывание таймера через 10 секунд
    // и вывод циклического таймера каждую секунду
    std::this_thread::sleep_for(std::chrono::seconds(20));

    gsbutils::stop();
    return 0;
}