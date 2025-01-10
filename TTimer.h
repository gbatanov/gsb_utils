#ifndef GSB_TTIMER_H
#define GSB_TTIMER_H

#include "version.h"
#include "Context.h"

// Класс потоко-безопасного неблокирующего таймера с секундной дискретностью
// TTimer - однократный запуск командой run
// CycleTimer - командой run запускается циклический таймер, последующие вызовы игнорируются

typedef void (*cb_timer)();
typedef void (*cb_timer_arg)(void*);

class TTimer
{
public:
	TTimer() = delete;
	TTimer(TTimer&) = delete;

	/// @brief Конструктор таймера
	/// @param period В секундах
	/// @param cb_func  Коллбэк-функция, вызываемая при срабатывании таймера
	TTimer(gsbutils::Context* ctx, uint64_t period, cb_timer cb_func) : period_(period), cb_func_(cb_func)
	{
		if (ctx)
			tctx = *ctx;
		//		else
		//			gsbutils::Context tctx ;
		active_.store(false);
		if (period > 0)
			periodDefault_ = period;
		cb_arg = NULL;
		init();
	}

	/// @brief Конструктор таймера
	/// @param period В секундах
	/// @param cb_func  Коллбэк-функция, вызываемая при срабатывании таймера
	TTimer(gsbutils::Context* ctx, uint64_t period, cb_timer_arg cb_func, void* cb_args) : period_(period), cb_func_arg_(cb_func), cb_arg(cb_args)
	{
		if (ctx)
			tctx = *ctx;
		//		else
		//			gsbutils::Context tctx ;
		active_.store(false);
		if (period > 0)
			periodDefault_ = period;
		init();
	}
	/// @brief Конструктор таймера без коллбэк-функции
	 /// @param period В секундах
	TTimer(gsbutils::Context* ctx, uint64_t period) : period_(period)
	{
		if (ctx)
			tctx = *ctx;
		//		else
		//			gsbutils::Context tctx;
		cb_func_ = nullptr;
		active_.store(false);
		if (period > 0)
			periodDefault_ = period;
		init();
	}

	~TTimer()
	{
		tctx.Cancel();
		stop();
	}

	/// @brief Останавливает все процессы таймера
	void stop()
	{
		tctx.Cancel();
		cv_timer.notify_all();
		reset();
		cb_func_ = nullptr;
		if (tmr.joinable())
			tmr.join();
	}

	/// Индикатор состояния - счетчик таймера активен
	bool is_active() { return active_.load(); }

	/// Индикатор состояния - выполняется коллбэк-функция или идет отсчет (таймер запущен)
	bool is_process() { return process_.load() || active_.load(); }

	/// @brief Установка нового периода
	/// @param period
	/// @return
	bool set_new_period(uint64_t period)
	{
		std::lock_guard<std::mutex> lg(period_mtx_);
		period_ = period;
		return true;
	}

	/// Сброс таймера без вызова callback-функции
	/// Если коллбэк-функция уже запущена, функция сброса не сделает ничего.
	void reset()
	{
		std::lock_guard<std::mutex> lg(period_mtx_);
		active_.store(false);
		period_ = 0;
	}

	/// Запуск таймера с указанным периодом.
	/// Если таймер уже запущен, аналогичен добавлению нового периода к уже исполненному,
	/// если нет - запускает таймер с заданным периодом.
	void run(uint64_t period)
	{
		if (tctx.Done())
			return;

		done_.store(false);

		if (is_active())
		{
			set_new_period(period);
		}
		else
		{
			std::lock_guard<std::mutex> lg(period_mtx_);
			period_ = period;
			active_.store(true);
			cv_timer.notify_one();
		}
	}

	/// Запуск таймера c дефолтным периодом.
	/// Повторный запуск при запущенном таймере игнорируется.
	void run()
	{
		if (tctx.Done())
			return;
		if (is_process())
			return;

		std::lock_guard<std::mutex> lg(period_mtx_);
		period_ = periodDefault_;
		done_.store(false);
		active_.store(true);
		cv_timer.notify_one();
	}

	bool Done() {
		done_.store(done_.load() || tctx.Done());

		return done_.load();
	}

	std::string version()
	{
		return std::string(Project_VERSION_MAJOR) + "." + Project_VERSION_MINOR + "." + Project_VERSION_PATCH;
	}


protected:
	/// Создаю  рабочий поток
	void init()
	{
		if (inited)
			return;

		inited = true;

		tmr = std::thread(&TTimer::process, this);
	}

	void process()
	{
		while (!tctx.Done())
		{
			std::unique_lock<std::mutex> ul(cv_timer_mutex);
			cv_timer.wait(ul, [this]()
				{ return active_.load() || tctx.Done(); });
			while (dec_period() && !tctx.Done())
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			if (0 == dec_period() && !tctx.Done())
			{
				active_.store(false);
				if (cb_func_ || cb_func_arg_) {
					process_.store(true);
					// Исключения в коллбэк-функции перехватываем, чтобы не влияли на сам таймер
					// коллбэк-функция должна быть максимально короткой, если таймер работает циклически
					// TODO: отмена коллбэк-функции при отмене контекста
					try
					{
						if (cb_func_)
							cb_func_();
						else if (cb_func_arg_)
							cb_func_arg_(cb_arg);
					}
					catch (...)
					{
					}
					process_.store(false);
				}
				ul.unlock();
				if (isCycle && !tctx.Done())
				{
					period_ = periodDefault_;
					active_.store(true);
					cv_timer.notify_one();
				}
				else {
					done_.store(true);
				}
			}
		} // while
		active_.store(false);
	}

	/// Уменьшает счетчик таймера на единицу до достижения нуля.
	uint64_t dec_period()
	{
		if (tctx.Done())
			return 0;
		std::lock_guard<std::mutex> lg(period_mtx_);
		if (period_ > 0)
		{
			uint64_t ret = period_--;
			return ret;
		}
		else
			return 0;
	}

	std::atomic<bool> done_{ false };      // включаем при срабатывании таймера
	uint64_t period_{ 0 };                // Рабочий счетчик, изначально содержит рабочий период
	uint64_t periodDefault_{ 0 };         // Дефолтный период, задается в конструкторе
	cb_timer cb_func_ = NULL;                    // Коллбэк-функция, задается в конструкторе
	cb_timer_arg cb_func_arg_ = NULL;                    // Коллбэк-функция, задается в конструкторе
	void* cb_arg = NULL;				  // Аргумент коллбэк-функции
	std::mutex period_mtx_;               // Мьютекс на изменения периода
	std::atomic<bool> active_{ false };   // Состояние процесса изменения периода
	std::atomic<bool> process_{ false };  // Состояние процесса выполнения коллбэк-функции
	std::thread tmr;                      // Рабочие потоки
	std::condition_variable cv_timer{};   // Условная переменная для таймера
	std::mutex cv_timer_mutex{};          // Мьютекс на условную переменную для таймера
	bool isCycle{ false };                // Признак циклического самозапуска
	bool inited = false;
	gsbutils::Context tctx;
};

class CycleTimer : public gsbutils::TTimer
{
public:
	CycleTimer(gsbutils::Context* ctx, uint64_t period, cb_timer cb_func) : gsbutils::TTimer(ctx, period, cb_func)
	{
		isCycle = true;
	}
	CycleTimer(gsbutils::Context* ctx, uint64_t period, cb_timer_arg cb_func, void* cb_args = NULL) : gsbutils::TTimer(ctx, period, cb_func, cb_args)
	{
		isCycle = true;
	}

};

#endif
