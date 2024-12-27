#ifndef THREAD_POOL_GSB
#define THREAD_POOL_GSB

// Пул потоков для проекта Zhub2

typedef void (*thread_func)(void*);

template <class T, uint8_t count>
class ThreadPool
{
public:
	ThreadPool(ThreadPool&) = delete;
	ThreadPool() {
		tc = count;
		std::cout << tc << std::endl;
	};

	~ThreadPool()
	{
		stop_threads();
	}

	void init_threads(gsbutils::Context* cntx, thread_func handle)
	{
		ctx = cntx;
		handle_ = handle;
		//       uint8_t tc = count;
		threadVec.reserve(tc);
		while (tc--)
			threadVec.push_back(new std::thread(&ThreadPool::on_command, this));
	}

	void stop_threads()
	{
		ctx->Cancel();
		cv_queue.notify_all();
		for (std::thread* t : threadVec)
			if (t->joinable())
				t->join();
	}
	// добавление команды в очередь
	void add_command(T cmd)
	{
		if (ctx->Done())
			return;
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
			{ return !taskQueue.empty() || ctx->Done(); });

		if (ctx->Done())
			return cmd;

		if (!taskQueue.empty())
		{
			cmd = taskQueue.front();
			taskQueue.pop();
		}
		ul.unlock();
		return cmd;
	}


protected:
	std::vector<std::thread*> threadVec{}; // вектор указателей на поток
	std::queue<T> taskQueue{};              // очередь команд
	std::mutex tqMtx{};                     // мьютех на запись/чтение очереди комманд
	std::condition_variable cv_queue;       // cv на очередь команд
private:
	gsbutils::Context* ctx = nullptr;

	uint8_t tc{ count }; // количество потоков в пуле

	/// @brief Функция потока
	void on_command()
	{
		while (!ctx->Done())
		{
			T cmd = get_command();
			if (!ctx->Done())
				handle_((void*)&cmd); // вызов функции-обработчика
		}
	}
	thread_func handle_ = nullptr; // реальная функция-обработчик команды
};

#endif