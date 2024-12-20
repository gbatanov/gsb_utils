#ifndef GSB_CONTEXT_H
#define GSB_CONTEXT_H

#include <stdio.h>
#include <cstdint>
#include <vector>
#include "Channel.h"
#include "TTimer.h"

class Context
{
public:
	Context() {
		std::cout << "default constructor\n";
		timeout_ = 0;
		done_ = false;
		tmr = nullptr;
	}
	Context(Context& cont) {
		std::cout << "copy constructor\n";
		timeout_ = cont.timeout_;
		done_ = false;
		parent = &cont;
		parent->Childrens.push_back(this);
		tmr = nullptr;

	}
	~Context() {
		std::cout << "destructor\n";
		if (tmr)
			tmr->stop();
		done_ = true;
		for (Context* a : this->Childrens) {
			delete a;
		}
	}
	static Context* create() {
		Context* ctx = new Context;
		return ctx;

	}
	static Context* create_with_timeout(Context* ctx_, uint64_t timeout) {
		Context* ctx = new Context(*ctx_);
		ctx->set_timeout(timeout);
		return ctx;
	}

	void set_timeout(uint64_t timeout) {
		timeout_ = timeout;
	}
	void start() {
		if (timeout_ > 0) {
			tmr = new gsbutils::TTimer(timeout_);
			tmr->run();
		}
	}

	void Cancel() {
		std::cout << "Cancel\n";
		done_ = true;
		for (Context* a : this->Childrens) {
			a->Cancel();
		}

	}

	bool Done() {
		if (timeout_ > 0 && !tmr)
			start();
		if (parent)
			done_ |= parent->done_;
		if (timeout_ > 0)
			done_ |= tmr->done.load();
		return done_;
	}

private:

	Context* parent = nullptr;
	std::vector<Context*>Childrens = {};
	bool done_ = false;
	uint64_t timeout_ = 0;
	gsbutils::TTimer* tmr = nullptr;
};

#endif