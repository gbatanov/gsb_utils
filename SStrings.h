#ifndef GSB_SSTRING_H
#define GSB_SSTRING_H

/// @brief Строковые функции
class SString
{
public:
    /// @brief Меняем одну часть строки на заданную (первое вхождение)
    /// @param inout Входная/выходная строка
    /// @param what Часть строки, которую заменяем
    /// @param with Строка, на которую меняем
    /// @return Количество замен
    static std::size_t replace_first(std::string &inout, std::string_view what, std::string_view with)
    {
        std::size_t count = 0;
        std::string::size_type pos{};
        if (inout.npos != (pos = inout.find(what.data(), 0, what.length())))
        {
            inout.replace(pos, what.length(), with.data(), with.length());
            count = 1;
        }
        return count;
    }

    /// @brief Меняем одну часть строки на заданную (все вхождения)
    /// @param inout Входная/выходная строка
    /// @param what Часть строки, которую заменяем
    /// @param with Строка, на которую меняем
    /// @return Количество замен
    static std::size_t replace_all(std::string &inout, std::string_view what, std::string_view with)
    {
        std::size_t count{};
        for (std::string::size_type pos{};
             inout.npos != (pos = inout.find(what.data(), pos, what.length()));
             pos += with.length(), ++count)
        {
            inout.replace(pos, what.length(), with.data(), with.length());
        }
        return count;
    }

    /// @brief Удаляем из строки все вхождения
    /// @param inout Входная/выходная строка
    /// @param what Удаляемая часть
    /// @return  Количество удаленных вхождений
    static std::size_t remove_all(std::string &inout, std::string_view what)
    {
        return replace_all(inout, what, "");
    }
    /// @brief Удаляем до первого вхождения, включая само вхождение
    /// @param inout Входная строка
    /// @param delimiter Строка-вхождение
    /// @return Результирующая строка
    static std::string remove_before(std::string inout, std::string_view delimiter)
    {
        std::string result = (inout);
        std::string::size_type pos{};
        pos = result.find(delimiter.data(), 0, delimiter.length());
        if (pos == inout.npos)
            return result;
        result.replace(0, pos + delimiter.length(), "", 0);
        return result;
    }

    /// @brief Удаляем после первого вхождения, включая само вхождение
    /// @param inout Входная строка
    /// @param delimiter Строка-вхождение
    /// @return Результирующая строка
    static std::string remove_after(std::string inout, std::string_view delimiter)
    {
        std::string result = (inout);
        std::string::size_type pos{};
        pos = result.find(delimiter.data(), 0, delimiter.length());
        if (pos == inout.npos)
            return result;
        result.replace(pos, result.length() - pos, "");
        return result;
    }

    /// @brief Разбиваем входную строку по парам ключ/значение
    /// @param instring  Входная строка
    /// @param delimiter Разделитель пар
    /// @param second_delimiter Разделитель внутри пары
    /// @return
    static std::vector<std::pair<std::string, std::string>> parse_string_with_delimiter(std::string instring, std::string delimiter = "&", std::string second_delimiter = "=")
    {
        std::vector<std::pair<std::string, std::string>> result{};
        while (instring.npos != instring.find(delimiter))
        {
            // на каждом шагу работаем с temp
            std::string tmp = remove_after(instring, delimiter);
            instring = remove_before(instring, delimiter);
            result.push_back(split_string_with_delimiter(tmp, second_delimiter));
        }
        // на последнем обрабатываем instring, если еще что-то осталось
        if (instring.size())
        {
            result.push_back(split_string_with_delimiter(instring, second_delimiter));
        }
        return result;
    }

    /// @brief Создаем пару ключ/значение из входной строки
    /// @param instring Входная строка
    /// @param delimiter Разделитель между ключом и значением
    /// @return
    static std::pair<std::string, std::string> split_string_with_delimiter(std::string instring, std::string delimiter)
    {
        std::string key = remove_after(instring, delimiter);
        std::string value = ""; // TODO: nullptr or empty string???
        if (instring.npos != instring.find(delimiter))
        {
            value = remove_before(instring, delimiter);
        }
        std::pair<std::string, std::string> result{key, value};

        return result;
    }
    /// @brief Обрезка строки слева
    /// @param instring Входная строка
    /// @return Результирующая строка
    static inline std::string ltrim(std::string instring)
    {
        std::string line = instring;
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch)
                                              { return !std::isspace(ch); }));
        return line;
    }

    /// @brief Обрезка строки справа
    /// @param instring Входная строка
    /// @return Результирующая строка
    static inline std::string rtrim(std::string instring)
    {
        std::string line = instring;
        line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch)
                                { return !std::isspace(ch); })
                       .base(),
                   line.end());
        return line;
    }

    /// @brief Обрезка строки слева и справа
    /// @param instring Входная строка
    /// @return Результирующая строка
    static inline std::string trim(std::string instring)
    {
        std::string line = ltrim(instring);
        return rtrim(line);
    }

    static inline std::vector<std::string> split(std::string instring, std::string delimiter)
    {
        std::vector<std::string> result{};
        while (instring.npos != instring.find(delimiter))
        {
            // на каждом шагу работаем с temp
            std::string tmp = remove_after(instring, delimiter);
            instring = remove_before(instring, delimiter);
            result.push_back(tmp);
        }
        if (instring.size())
        {
            result.push_back(instring);
        }
        return result;
    }
    static void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
    {
        hex1 = c / 16;
        hex2 = c % 16;
        hex1 += hex1 <= 9 ? '0' : 'a' - 10;
        hex2 += hex2 <= 9 ? '0' : 'a' - 10;
    }
    /// @brief Функция urlencode
    /// @param s Входная строка
    /// @return Выходная строка
    static std::string urlencode(std::string s)
    {
        const char *str = s.c_str();
        std::vector<char> v(s.size());
        v.clear();
        for (size_t i = 0, l = s.size(); i < l; i++)
        {
            char c = str[i];
            if ((c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
                c == '*' || c == '\'' || c == '(' || c == ')')
            {
                v.push_back(c);
            }
            else if (c == ' ')
            {
                v.push_back('+');
            }
            else
            {
                v.push_back('%');
                unsigned char d1, d2;
                hexchar(c, d1, d2);
                v.push_back(d1);
                v.push_back(d2);
            }
        }

        return std::string(v.cbegin(), v.cend());
    }
};

#endif
