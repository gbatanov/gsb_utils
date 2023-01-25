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
    // Возвращает дату/время по таймстампу в виде строки yyyy-mm-dd hh:mm:ss
    static std::string timestamp_to_string(time_t timestamp)
    {
        std::string result;
        char buf[256]{0};
        std::tm tm = *std::localtime(&timestamp); // structure
        size_t len = std::strftime(buf, sizeof(buf) / sizeof(buf[0]), " %Y-%m-%d %H:%M:%S", &tm);
        buf[len] = 0;
        result = std::string(buf);
        return result;
    }
    // Возвращает текущие дату/время в виде строки yyyy-mm-dd hh:mm:ss
    static std::string current_time()
    {
        return timestamp_to_string(std::time(nullptr));
    }
};

#endif