## Различные вспомогательные утилиты для моих проектов

### gsbutils::(функции отладочного вывода)
В основном коде не забываем устанавливать флаг, разрешать работу потока вывода и открывать нужный лог. 
```
std::atomic<bool> Flag{true};

openlog("zhub2", LOG_PID, LOG_LOCAL7); //(local7.log)
std::thread msgt = std::thread(&gsbutils::printMsg);

```
По окончании закрываем лог и поток.
```
msgt.join();
closelog();
```