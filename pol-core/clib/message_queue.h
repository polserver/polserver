#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <boost/noncopyable.hpp>

/*
TODO:
- this is an blocking queue, consider after updating to a newer boost version
the lockfree queues.

Idea for lockfree pop_wait:
keep the conditional and lock members
void push(Message const& msg)
{
   _queue.push_back(msg);
   _notifier.notify_one(); // no lock only signal the state
}

void pop_wait(Message& msg)
{
 if (try_pop(msg)) // fast lookup
  return;
 // queue is empty part
 boost::mutex::scoped_lock lock(_mutex);
 while (!try_pop(msg)) 
 {
  t= some time (a few ms)
  _notifier.timed_wait(lock, t); // timed wait just to be sure that no deadlocks
can accur
                                 // since between the try_pop a new entry can be
inserted
 }
}
*/
template <typename Message>
class message_queue : boost::noncopyable
{
   public:
    message_queue() : _queue(), _mutex(), _notifier(), _cancel(false)
    {
    }
    ~message_queue()
    {
        cancel();
    }

    // push new message into queue and notify possible wait_pop
    void push(Message const& msg)
    {
        list<Message> tmp;
        tmp.push_back(msg);  // costly pushback outside the lock
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.splice(_queue.end(), tmp);  // fast splice inside
            _notifier.notify_one();
        }
    }

    // check if empty (unsafe aka senseless)
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }

    // tries to get a message true on success false otherwise
    bool try_pop(Message& msg)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty()) return false;
        msg = _queue.front();
        _queue.pop_front();
        return true;
    }

    // waits till queue is non empty
    void pop_wait(Message& msg)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (_queue.empty() && !_cancel)
            _notifier.wait(lock);  // will unlock mutex during wait
        if (_cancel) throw Canceled();
        msg = _queue.front();
        _queue.pop_front();
    }

    void cancel()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _cancel = true;
        _notifier.notify_all();
    }

    struct Canceled
    {
    };

   private:
    list<Message> _queue;
    mutable std::mutex _mutex;
    std::condition_variable _notifier;
    bool _cancel;
};

#endif