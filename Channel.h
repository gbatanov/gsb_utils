#ifndef GSB_CHANNEL_H
#define GSB_CHANNEL_H

#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>


template <class T>
class Channel
{
public:
	// дефолтный конструктор и конструктор копировани€ запрещены
	Channel() = delete;
	Channel(Channel&) = delete;
	Channel operator=(Channel&) = delete;
	Channel(uint64_t size)
	{
		size_ = size;
		Msg_.reserve(size + 1);
		closed.store(false);
		unbuffered_flag.clear();
	}
	~Channel()
	{
		if (!is_closed())
			close();
	}
	void close()
	{
		unbuffered_flag.test_and_set();
		closed.store(true);
		cv_r.notify_all();
		cv_w.notify_all();
	}
	bool is_closed() {
		return closed.load();
	}

	// «апись в канал
	// при размере буфера 0 функци€ записи ожидает запрос на чтение (поток блокируетс€,
	// если нет готового читател€).
	void write(T msg)
	{
		if (size_ == 0) { // Ќебуферизированный канал
			if (!unbuffered_flag.test())
				unbuffered_flag.wait(false);

			std::unique_lock<std::mutex> ul(mtxW);
			cv_w.wait(ul, [this] {return this->is_closed() || Msg_.size() == 0; });

			if (!is_closed())
				Msg_.push_back(msg);
			ul.unlock();
			unbuffered_flag.clear();
		}
		else {
			// ≈сли очередь заполнена, ждем освобождени€ как минимум одного элемента.
			std::unique_lock<std::mutex> ul(mtxW);
			cv_w.wait(ul, [this] {return this->is_closed() || Msg_.size() < size_; });

			if (!is_closed())
				Msg_.push_back(msg);

			ul.unlock();
		}
		cv_r.notify_all(); // уведомл€ем все потоки, ожидающие возможности прочитать из канала
		// уведомление посылаем в любом случае, даже если канал закрыт, чтобы ожидающие потоки
		// получили сообщение о закрытии канала и завершили работу с ним.
	}

	// „тение из канала
	T read(bool* ok)
	{
		T msg{};

		if (size_ == 0) { // Ќебуферизированный канал
			unbuffered_flag.test_and_set();
			unbuffered_flag.notify_one();
		}
		std::unique_lock<std::mutex> ul(mtxR);
		cv_r.wait(ul, [this] {return this->is_closed() || this->Msg_.size() > 0; });

		if (Msg_.size() > 0) {
			msg = Msg_.front();
			Msg_.erase(Msg_.begin());
			cv_w.notify_all();
		}

		if (ok != nullptr)
			*ok = !is_closed();

		ul.unlock();
		return msg;
	}

private:
	std::vector<T> Msg_{};
	uint64_t size_ = 0; // размер буфера, 0 - небуферизированный (потоки с чтением и записью блокируютс€
	// до готовности другой стороны) 
	std::mutex mtxR, mtxW, mtxMsg;
	std::condition_variable cv_r, cv_w;
	std::atomic_flag unbuffered_flag{};
	std::atomic<bool> closed; // channel closed
};

#endif