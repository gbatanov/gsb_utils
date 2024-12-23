#ifndef GSB_CONTEXT_H
#define GSB_CONTEXT_H

#include <stdio.h>
#include <cstdint>
#include <vector>
#include <thread>

class Context
{
public:
	// ����������� �� ���������
	Context() {
		timeout_ = 0;
		done_ = false;
	}
	// ����������� �����������
	Context(Context& cont) {
		timeout_ = cont.timeout_;
		done_ = false;
		parent = &cont;
		parent->Childrens.push_back(this);
		//		tmr = nullptr;

	}
	// ����������
	~Context() {
		//		if (tmr)
		//			tmr->stop();
		done_ = true;
		for (Context* a : this->Childrens) {
			delete a;
		}
	}
	// ������ ��������
	static Context* create() {
		Context* ctx = new Context;
		return ctx;

	}
	// ����������� ���������
	static Context* copy(Context* ctx_) {
		Context* ctx = new Context(*ctx_);
		return ctx;

	}
	// ����������� ��������� � ���������
	static Context* create_with_timeout(Context* ctx_, uint64_t timeout) {
		Context* ctx = new Context(*ctx_);
		ctx->set_timeout(timeout);
		if (timeout > 0)
			ctx->run_timer();
		return ctx;
	}
	// ��������� ��������
	void set_timeout(uint64_t timeout) {
		timeout_ = timeout;
	}

	// ������ ���������, ������ ���������������� �� ������� �� ���� ��������,
	// ������� ��������� � �������� � ���������
	void Cancel() {
		done_ = true;
		for (Context* a : this->Childrens) {
			a->Cancel();
		}
	}

	// ������� ���������� ����������, ��� ������� � ���� ���������� � �� ������� ������ �����������
	bool Done() {
		if (parent)
			done_.store(done_.load() || parent->done_.load());
		return done_;
	}

	void run_timer()
	{
		tmr = std::thread(&Context::process, this);
		period_ = timeout_;
		active_.store(true);
		cv_timer.notify_one();
	}
protected:
	/// ��������� ������� ������� �� ������� �� ���������� ����.
	uint64_t dec_period()
	{
		if (Done())
			return 0;
		if (period_ > 0)
		{
			uint64_t ret = period_--;
			return ret;
		}
		else
			return 0;
	}
	void process()
	{
		std::unique_lock<std::mutex> ul(cv_timer_mutex);
		cv_timer.wait(ul, [this]()
			{ return active_.load() || Done(); });
		while (dec_period() && !Done())
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		if (0 == dec_period() && !Done())
		{
			active_.store(false);
			ul.unlock();
			done_.store(true);
		}

	}


	Context* parent = nullptr;
	std::vector<Context*>Childrens = {};
	std::atomic<bool> done_{ false };
	uint64_t timeout_ = 0;
	uint64_t period_{ 0 };                // ������� �������, �������� ������� ���������
//	std::mutex period_mtx_;             // ������� �� ��������� �������
	std::atomic<bool> active_{ false };   // ��������� �������� ��������� �������
	std::thread tmr;                    // ������� �����
	std::condition_variable cv_timer{}; // �������� ���������� ��� �������
	std::mutex cv_timer_mutex{};        // ������� �� �������� ���������� ��� �������
	bool inited = false;
};

#endif