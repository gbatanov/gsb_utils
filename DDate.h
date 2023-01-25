#ifndef GSB_DDATE_H
#define GSB_DDATE_H

// функции даты и времени
class DDate
{
public:
    // часы суток
    static uint8_t get_hour_of_day()
    {
        std::tm *tm{};
        std::time_t t = std::time(nullptr);
        tm = std::localtime(&t);
        return (uint8_t)tm->tm_hour;
    }
    // Возвращает текущие дату/время в виде строки yyyy-mm-dd hh:mm:ss
    static std::string current_time()
    {
        std::string result;
        char buf[128]{0};
        std::time_t t1 = std::time(nullptr); // текущее время timestamp
        std::tm tm = *std::localtime(&t1);   // structure
        size_t len = std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", &tm);
        buf[len] = 0;
        result = std::string(buf);
        return result;
    }
};

#endif