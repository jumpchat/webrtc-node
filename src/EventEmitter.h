#ifndef WEBRTC_EVENTEMITTER_H
#define WEBRTC_EVENTEMITTER_H

#include "Common.h"

namespace WebRTC {
template <class T>
class EventWrapper;

class Event : public rtc::RefCountInterface {
    template <class T>
    friend class EventWrapper;
    friend class rtc::RefCountedObject<Event>;
    friend class EventEmitter;

public:
    inline bool HasWrap() const
    {
        RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

        return _wrap;
    }

    template <class T>
    inline T Type() const
    {
        RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

        return static_cast<T>(_event);
    }

    template <class T>
    const T& Unwrap() const
    {
        RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

        static T nowrap;

        if (_wrap) {
            const EventWrapper<T>* ptr = static_cast<const EventWrapper<T>*>(this);
            return ptr->_content;
        }

        nowrap = T();
        return nowrap;
    }

private:
    explicit Event(int event = 0)
        : _event(event)
        , _wrap(false)
    {
        RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    }

    virtual ~Event()
    {
        RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    }

protected:
    int _event;
    bool _wrap;
};

template <class T>
class EventWrapper : public Event {
    friend class rtc::RefCountedObject<EventWrapper<T>>;
    friend class Event;
    friend class EventEmitter;

private:
    explicit EventWrapper(int event, const T& content)
        : Event(event)
        , _content(content)
    {
        _wrap = true;
    }

    virtual ~EventWrapper() {}

protected:
    T _content;
};

class EventEmitter {
    friend class NotifyEmitter;

public:
    explicit EventEmitter(uv_loop_t* loop = 0, bool notify = false);
    virtual ~EventEmitter();

    void AddListener(EventEmitter* listener = 0);
    void RemoveListener(EventEmitter* listener = 0);
    void RemoveAllListeners();

    void SetReference(bool alive = true);

    void Emit(int event = 0);
    void Emit(rtc::scoped_refptr<Event> event);

    template <class T>
    inline void Emit(int event, const T& content)
    {
        RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
        EventEmitter::Emit(new rtc::RefCountedObject<EventWrapper<T>>(event, content));
    }

    virtual void On(Event* event) = 0;

private:
    static void onAsync(uv_async_t* handle, int status);
    static void onEnded(uv_handle_t* handle);

    void Dispose();
    void DispatchEvents();

    void AddParent(EventEmitter* listener = 0);
    void RemoveParent(EventEmitter* listener = 0);

protected:
    bool _notify;
    uv_mutex_t _lock;
    uv_mutex_t _list;
    uv_async_t* _async;
    std::queue<rtc::scoped_refptr<Event>> _events;
    std::vector<EventEmitter*> _listeners;
    std::vector<EventEmitter*> _parents;
};

class NotifyEmitter : public EventEmitter {
public:
    NotifyEmitter(EventEmitter* listener = 0);

    virtual void On(Event* event);
};
};

#endif
