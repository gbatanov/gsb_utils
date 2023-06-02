#ifndef GSB_CHANNEL_H
#define GSB_CHANNEL_H

#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "version.h"

template <class T>
class Channel
{
public:
    Channel() = delete;
    Channel(Channel &) = delete;
    Channel(uint64_t size)
    {
        size_ = size;
        Msg_.reserve(size);
    }
    ~Channel()
    {
    }
    void stop()
    {
        stopped = true;
        cv_r.notify_all();
        cv_w.notify_all();
    }
    void write(T msg)
    {
        if (Msg_.size() >= size_)
        {
            std::unique_lock<std::mutex> ul(mtx_w);
            cv_w.wait(ul, [this]()
                      { return stopped || Msg_.size() < size_; });
            ul.unlock();
        }
        if (stopped)
            return;
        Msg_.push_back(msg);
        cv_r.notify_one();
    }

    T read()
    {
        if (Msg_.size() == 0)
        {
            std::unique_lock<std::mutex> ul(mtx_r);
            while (Msg_.size() == 0 && !stopped)
            {
                if (std::cv_status::timeout == cv_r.wait_for(ul, std::chrono::milliseconds(100)))
                {
                    if (stopped)
                    {
                        ul.unlock();
                        return T{};
                    }
                }
                if (Msg_.size() > 0)
                    ul.unlock();
            }
        }
        if (stopped)
            return T{};
        T msg = Msg_.front();
        Msg_.erase(Msg_.begin());
        cv_w.notify_one();
        return msg;
    }

private:
    std::vector<T> Msg_{};
    uint64_t size_ = 0;
    std::mutex mtx_r, mtx_w;
    std::condition_variable cv_r, cv_w;
    bool stopped = false;
};

#endif