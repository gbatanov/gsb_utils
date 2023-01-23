#ifndef GSB_TTIMER_H
#define GSB_TTIMER_H

// Класс потоко-безопасного неблокирующего таймера с секундной дискретностью
// TTimer - однократный запуск командой run
// CycleTimer - командой run запускается циклический таймер, последующие вызовы игнорируются

typedef void (*cb_timer)();

class TTimer
{
public:
    TTimer(uint64_t period, cb_timer cb_func) : period_(period), cb_func_(cb_func)
    {
        if (period > 0)
            periodDefault_ = period;
        init();
    }
    ~TTimer()
    {
        Flag.store(false);

        cv_timer.notify_all();
        reset();
        cb_func_ = nullptr;
        if (t.joinable())
            t.join();
    }

    // Индикатор состояния - счетчик таймера активен
    bool is_active() { return active_.load(); }
    // Индикатор состояния - выполняется коллбэк-функция или идет отсчет (таймер запущен)
    bool is_process() { return process_.load() || active_.load(); }
    bool set_new_period(uint64_t period)
    {
        std::lock_guard<std::mutex> lg(period_mtx_);
        period_ = period + 1;
        return true;
    }

    // Сброс таймера без вызова callback-функции
    // Если коллбэк-функция уже запущена, функция сброса не сделает ничего.
    void reset()
    {
        std::lock_guard<std::mutex> lg(period_mtx_);
        active_.store(false);
        period_ = 0;
    }

    // Запуск таймера с указанным периодом.
    // Если таймер уже запущен, аналогичен добавлению нового периода к уже исполненному,
    // если нет - запускает таймер с заданным периодом.
    void run(uint64_t period)
    {
        if (is_active())
        {
            set_new_period(period);
        }
        else
        {
            std::lock_guard<std::mutex> lg(period_mtx_);
            period_ = period + 1;
            active_.store(true);
            cv_timer.notify_one();
        }
    }

    // Запуск таймера c дефолтным периодом.
    // Повторный запуск при запущенном таймере игнорируется.
    void run()
    {
        if (is_process())
            return;

        std::lock_guard<std::mutex> lg(period_mtx_);
        period_ = periodDefault_;

        active_.store(true);
        cv_timer.notify_one();
    }

protected:
    // Создаю  рабочий поток
    void init()
    {
        t = std::thread([this]()
                        {
                            while (this->Flag.load())
                            {
                                std::unique_lock<std::mutex> ul(cv_timer_mutex);
                                cv_timer.wait(ul, [this]()
                                              { return this->active_.load() || !this->Flag.load(); });

                                do
                                {
                                    if (!this->Flag.load())
                                        break;
                                    std::this_thread::sleep_for(std::chrono::seconds(1));
                                } while (this->dec_period());
                                if (0 == this->dec_period() && this->Flag.load())
                                {
                                    this->active_.store(false);
                                    this->process_.store(true);
                                    // Исключения в коллбэк-функции перехватываем, чтобы не влияли на сам таймер
                                    try
                                    {
                                        this->cb_func_();
                                    }
                                    catch (...)
                                    {
                                    }
                                    this->process_.store(false);
                                    ul.unlock();
                                    if (this->isCycle)
                                    {
                                        this->period_ = this->periodDefault_;
                                        this->active_.store(true);
                                        cv_timer.notify_one();
                                    }
                                }
                            } // while Flag
                        });
    }

    // Уменьшает счетчик таймера на единицу до достижения нуля.
    uint64_t dec_period()
    {
        std::lock_guard<std::mutex> lg(period_mtx_);
        if (period_ > 1)
            return --period_;
        else
            return 0;
    }

    uint64_t period_{0};                // Рабочий счетчик, изначально содержит рабочий период
    uint64_t periodDefault_{0};         // Дефолтный период, задается в конструкторе
    cb_timer cb_func_;                  // Коллбэк-функция, задается в конструкторе
    std::mutex period_mtx_;             // Мьютекс на изменения периода
    std::atomic<bool> active_{false};   // Состояние процесса изменения периода
    std::atomic<bool> process_{false};  // Состояние процесса выполнения коллбэк-функции
    std::thread t;                      // Рабочие потоки
    std::condition_variable cv_timer{}; // Условная переменная для таймера
    std::mutex cv_timer_mutex{};        // Мьютекс на условную переменную для таймера
    std::atomic<bool> Flag{true};       // Флаг разрешения работы внутренним потокам
    bool isCycle{false};                // Признак циклического самозапуска
};

class CycleTimer : public TTimer
{
public:
    CycleTimer(uint64_t period, cb_timer cb_func) : TTimer(period, cb_func)
    {
        isCycle = true;
    }
};

#endif
