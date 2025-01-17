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
	// ��������� ����������� � ����������� ����������� ���������
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

	// ������ � �����
	// ��� ������� ������ 0 ������� ������ ������� ������ �� ������ (����� �����������,
	// ���� ��� �������� ��������).
	void write(T msg)
	{
		if (size_ == 0) { // ������������������ �����
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
			// ���� ������� ���������, ���� ������������ ��� ������� ������ ��������.
			std::unique_lock<std::mutex> ul(mtxW);
			cv_w.wait(ul, [this] {return this->is_closed() || Msg_.size() < size_; });

			if (!is_closed())
				Msg_.push_back(msg);

			ul.unlock();
		}
		cv_r.notify_all(); // ���������� ��� ������, ��������� ����������� ��������� �� ������
		// ����������� �������� � ����� ������, ���� ���� ����� ������, ����� ��������� ������
		// �������� ��������� � �������� ������ � ��������� ������ � ���.
	}

	// ������ �� ������
	T read(bool* ok)
	{
		T msg{};

		if (size_ == 0) { // ������������������ �����
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
	uint64_t size_ = 0; // ������ ������, 0 - ������������������ (������ � ������� � ������� �����������
	// �� ���������� ������ �������) 
	std::mutex mtxR, mtxW, mtxMsg;
	std::condition_variable cv_r, cv_w;
	std::atomic_flag unbuffered_flag{};
	std::atomic<bool> closed; // channel closed
};

#endif