#ifndef THREAD_POOL_GSB
#define THREAD_POOL_GSB

typedef void (*thread_func)(void *);

template <class T>
class ThreadPool
{
public:
    ThreadPool()
    {
    }
    ~ThreadPool()
    {
        if (flag.load())
            stop_threads();
    }

    void init_threads(thread_func handle, uint8_t count)
    {
        handle_ = handle;
        uint8_t tc = count;
        threadVec.reserve(tc);
        while (tc--)
            threadVec.push_back(new std::thread(&ThreadPool::on_command, this));
    }

    void stop_threads()
    {
        flag.store(false);
        cv_queue.notify_all();
        for (std::thread *t : threadVec)
            if (t->joinable())
                t->join();
    }
    // добавление команды в очередь
    void add_command(T cmd)
    {
        std::lock_guard<std::mutex> lg(tqMtx);
        taskQueue.push(cmd);
        cv_queue.notify_all();
    }

    // получение команды из очереди
    T get_command()
    {
        T cmd{};
        {
            std::lock_guard<std::mutex> lg(tqMtx);
            if (!taskQueue.empty())
            {
                cmd = taskQueue.front();
                taskQueue.pop();
                return cmd;
            }
        }

        std::unique_lock<std::mutex> ul(tqMtx);
        cv_queue.wait(ul, [this]()
                      { return !taskQueue.empty() || !flag.load(); });

        if (flag.load() && !taskQueue.empty())
        {
            cmd = taskQueue.front();
            taskQueue.pop();
        }
        ul.unlock();
        return cmd;
    }

    std::atomic<bool> flag{true}; // флаг разрешения работы потоков

protected:
    std::vector<std::thread *> threadVec{}; // вектор указателей на поток
    std::queue<T> taskQueue{};              // очередь команд
    std::mutex tqMtx{};                     // мьютех на запись/чтение очереди комманд
    std::condition_variable cv_queue;       // cv на очередь команд
private:
    /// @brief Функция потока
    void on_command()
    {
        while (flag.load())
        {
            T cmd = get_command();
            if (flag.load())
                handle_((void *)&cmd); // вызов функции-обработчика
        }
    }
    thread_func handle_; // реальная функция-обработчик команды
};

#endif