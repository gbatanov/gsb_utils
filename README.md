## Различные вспомогательные утилиты для проекта Zhub2

### gsbutils::Channel (аналог каналов из Go, в разработке)
### gsbutils::Context (аналог контекста из Go, в разаработке)
### gsbutils::TTimer (потокобезопасный таймер с коллбэк-функцией)
### gsbutils::SString (функции работы со строками)
### gsbutils::ВВфеу (функции работы с датой/временем)

### gsbutils::DDebug (функции отладочного вывода)
В основном коде при старте вызываем
```
    gsbutils::DDebug::init(0, NULL); // или gsbutils::DDebug::init(1, (const char*)"logname");
    gsbutils::DDebug::set_debug_level(7);
    gsbutils::DDebug::set_flag(1);
```
где 0 - вывод в консоль, 1 - вывод в syslog (local7.log). 


Если вывод в syslog, "logname" - имя процесса в логе. При выводе в консоль параметр игнорируется.


По окончании закрываем лог и поток.
```
gsbutils::DDebug::stop();
```