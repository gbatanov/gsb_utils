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
};

#endif