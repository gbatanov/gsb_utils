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
	Channel() = delete;
	Channel(Channel&) = delete;
	Channel(uint64_t size)
	{
		size_ = size;
		Msg_.reserve(size+1);
		stopped.store(false);
	}
	~Channel()
	{
		if (!stopped)
			stop();
	}
	void stop()
	{
		stopped.store(true);
		cv_r.notify_all();
		cv_w.notify_all();
	}
	bool is_stopped() {
		return stopped;
	}

	// «апись в канал
	void write(T msg)
	{
		// ≈сли очередь заполнена, ждем освобождени€ как минимум одного элемента.
		if (Msg_.size() !=0 && Msg_.size() >= size_)
		{
			std::unique_lock<std::mutex> ul(mtxW);

			while (Msg_.size() != 0 && Msg_.size() >= size_ && !stopped.load())
			{
				if (std::cv_status::timeout == cv_w.wait_for(ul, std::chrono::milliseconds(200)))
				{
					if (stopped.load())
						break;
				}
			}

			ul.unlock();
		}
		if (!stopped.load())
		{
			std::lock_guard<std::mutex> lg(mtxMsg);
			Msg_.push_back(msg);
		}
		cv_r.notify_all(); // уведомл€ем все потоки, ожидающие возможности прочитать из канала
	}

	// „тение из канала
	T read(bool* ok)
	{
		T msg{};

		if (Msg_.size() == 0)
		{
			std::unique_lock<std::mutex> ul(mtxR);
			while (Msg_.size() == 0 && !stopped.load())
			{
				if (std::cv_status::timeout == cv_r.wait_for(ul, std::chrono::milliseconds(200)))
				{
					if (stopped.load())
						break;
				}
			}
			ul.unlock();
		}
		if (!stopped.load())
		{
			std::lock_guard<std::mutex> lg(mtxR);
			if (Msg_.size() > 0) {
				msg = Msg_.front();
				Msg_.erase(Msg_.begin());
				if (ok != nullptr)
					*ok = true;
				return msg;
			}
		}
		else {
			if (ok != nullptr)
				*ok = false;
			msg = T{};
			return msg;
		}
	}

private:
	std::vector<T> Msg_{};
	uint64_t size_ = 0;
	std::mutex mtxR, mtxW, mtxMsg;
	std::condition_variable cv_r, cv_w;
	std::atomic<bool> stopped; // channel closed
};

#endif