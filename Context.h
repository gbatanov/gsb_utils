#ifndef GSB_CONTEXT_H
#define GSB_CONTEXT_H

#include <stdio.h>
#include <cstdint>
#include <vector>
#include <thread>
#include <algorithm>


class Context
{
public:
	// ����������� �� ���������
	Context() {
		timeout_ = 0;
		done_.store(false);
	}
	// ����������� �����������
	Context(Context& cont) {
		timeout_ = cont.timeout_;
		done_.store(cont.done_.load());
		parent = &cont;
		parent->Childrens.push_back(this);
	}
	// ��������� ������������
	Context& operator=(Context& cont) {
		if (this == &cont)
			return *this;
		timeout_ = cont.timeout_;
		done_.store(cont.done_.load());
		parent = &cont;
		parent->Childrens.push_back(this);
		return *this;
	}
	// ����������
	~Context() {
#ifdef DEBUG
		std::cout << "Context destructor\n";
#endif
		done_.store(true);
		cv_timer.notify_one();
		if (tmr.joinable()) {
			tmr.join();
		}
		if (err_code_ == -1)
			err_code_ = 0;
		for (Context* a : this->Childrens) {
			delete a;
		}
		if (parent) {
			parent->remove_children(this);
		}
	}
	// �������� ��������� �������� �� ������
	void remove_children(Context* ctx) {
		typedef std::vector<Context*>::iterator CtxIterator;
		CtxIterator elem = std::find(Childrens.begin(), Childrens.end(), ctx);
		if (elem != Childrens.end()) {
			Childrens.erase(elem);
		}
	}
	// ������ ��������
//	static Context* create() {
//		Context* ctx = new Context;
//		return ctx;
//
//	}
	// ����������� ���������
	static Context copy(Context ctx_) {
		Context ctx = ctx_;
		return ctx;

	}
	// ����������� ��������� � ���������
	static Context create_with_timeout(Context& ctx_, uint64_t timeout) {
		Context ctx = ctx_;
		ctx.set_timeout(timeout);
		if (timeout > 0)
			ctx.run_timer();
		return ctx;
	}
	// ��������� ��������
	void set_timeout(uint64_t timeout) {
		timeout_ = timeout;
	}

	// ������ ���������, ������ ���������������� �� ������� �� ���� ��������,
	// ������� ��������� � �������� � ���������
	void Cancel() {
		done_.store(true);
		err_code_ = 1;
		for (Context* a : this->Childrens) {
			a->Cancel();
		}
	}

	// ������� ���������� ����������, ���� true, ��� ������� � ���� ���������� � �� ������� ������ �����������
	bool Done() {
		if (parent)
			done_.store(done_.load() || parent->done_.load());
		return done_;
	}

	int8_t Error() {
		return err_code_;
	}

	Context* get_parent() {
		return parent;
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
	// ������� ������� �������
	void process()
	{
		std::unique_lock<std::mutex> ul(cv_timer_mutex);
		cv_timer.wait(ul, [this]()
			{ return active_.load() || Done(); });
		while (dec_period() && !Done())
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		if (Done()) {
			active_.store(false);
			return;
		}
		active_.store(false);
		done_.store(true);
		err_code_ = 2;

		ul.unlock();
	}


	Context* parent = nullptr;
	// -1 - �� ����������
	// 0 - ��� ������, �������� ���������� �� ���������� ������ (�������)
	// 1 - �������� ���������� �� ������ (������� ������� ������ Cancel())
	// 2 - �������� ���������� �� ����-����
	int8_t err_code_ = -1;
	std::vector<Context*>Childrens = {};
	std::atomic<bool> done_{ false };
	uint64_t timeout_ = 0;
	uint64_t period_{ 0 };              // ������� �������, �������� ������� ���������
	std::atomic<bool> active_{ false }; // ��������� �������� ��������� �������
	std::thread tmr;                    // ������� �����
	std::condition_variable cv_timer{}; // �������� ���������� ��� �������
	std::mutex cv_timer_mutex{};        // ������� �� �������� ���������� ��� �������
};

#endif