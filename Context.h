#ifndef GSB_CONTEXT_H
#define GSB_CONTEXT_H

#include <stdio.h>
#include <cstdint>
#include "Channel.h"
#include "TTimer.h"

class Context
{
public:

	Context(Context&) = delete;
	~Context(){
		tmr->stop();
		done_ = false;
	}

	static Context* create_with_timeout(uint64_t timeout) {
		Context *ctx = new Context();
		ctx->timeout_ = timeout;
		ctx->done_ = false;
		return ctx;
	}

	void start() {
		if (timeout_ > 0) {
			tmr = new gsbutils::TTimer(timeout_);
			tmr->run();
		}
	}

	void cancel(){
		done_ = true;
	}

	bool Done() {
		return done_ |= tmr->done.load();
	}

private:
	Context() {
	}
	bool done_;
	uint64_t timeout_ = 0;
	gsbutils::TTimer *tmr;
};

#endif