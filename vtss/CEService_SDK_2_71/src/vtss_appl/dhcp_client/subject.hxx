/*

 Vitesse API software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.

*/

#ifndef __SUBJECT_H__
#define __SUBJECT_H__

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_DHCP_CLIENT

extern "C" {
#include "vtss_trace_api.h"
}

#include "time.hxx"
#include "intrusive_list.hxx"
#include "vtss_mutex.hxx"

namespace VTSS {

template<typename Time>
struct BasicTimer;
typedef BasicTimer<cyg_tick_count_t> Timer;

struct eCosSubjectThread;
typedef eCosSubjectThread SubjectThread;

template<typename ThreadCtx, typename Clock>
class BasicSubjectRunner;
typedef BasicSubjectRunner<SubjectThread, eCosClock> SubjectRunner;

struct AbstractTriggerHandler;

struct Trigger :
    public ListNode_impl<Trigger>
{
    typedef enum {
        success = 0,
        could_not_attach = 1,
        hangup = 2,
    } subject_status_t;

    Trigger();
    Trigger(AbstractTriggerHandler *handler );
    ~Trigger();

    void signal(subject_status_t status, SubjectRunner * sr);
    void execute();

protected:
    AbstractTriggerHandler * handler_;
    subject_status_t status_;

private:
    inline Trigger(const Trigger&); // No copies
}; // Trigger

struct AbstraceTimeHandler
{
    virtual void handle(Timer *t) = 0;
};

struct AbstractTriggerHandler
{
    virtual void execute(Trigger & t, Trigger::subject_status_t status) = 0;
}; // struct AbstractTriggerHandler

template<typename Time>
struct BasicTimer : public VTSS::ListNode_impl<BasicTimer<Time> >
{
    template<typename T0, typename T1> friend class BasicSubjectRunner;

    BasicTimer(AbstraceTimeHandler * cb) : cb_(cb) { }

    Time timeout() const { return timeout_; }

    bool operator<(const BasicTimer& rhs) {
        return timeout_ < rhs.timeout();
    }

    void valid(bool c) { valid_ = c; }
    bool valid() const { return valid_; }

private:
    void timeout(Time to) {
        timeout_ = to;
    }

    void invoke_cb() {
        if (cb_) {
            cb_->handle(this);
        }
    }

    AbstraceTimeHandler * cb_;
    Time timeout_;
    bool valid_;
};


template<typename ThreadCtx, typename Clock>
class BasicSubjectRunner
{
public:
    BasicSubjectRunner() {
        lock_guard<> lock_(lock_get());
        ticks = Clock::to_time_t(seconds(1));
    }

    bool timer_del(Timer& t) {
        lock_guard<> lock_(lock_get());

        if (!t.is_linked()) {
            t.valid(false);
            return true;
        }

        typedef typename VTSS::List<Timer>::iterator I;
        for (I i = timers.begin(); i != timers.end(); ++i) {
            if (&(*i) == &t) {
                timers.unlink(t);
                t.valid(false);
                request_service();
                return true;
            }
        }

        return false;
    }

    void timer_add(Timer& t, nanoseconds timeout) {
        if (t.is_linked()) {
            VTSS_ASSERT(timer_del(t));
        }

        lock_guard<> lock_(lock_get());
        T_N("Timer add: %p ns=%llu ticks=%llu now=%llu", (void *)&t,
                timeout.raw(), Clock::to_time_t(timeout), Clock::now());
        t.valid(false);
        t.timeout(Clock::now() + Clock::to_time_t(timeout));
        timers.insert_sorted(t);
        request_service();
    }

    bool trigger_del(Trigger& t) {
        lock_guard<> lock_(lock_get());

        typedef typename VTSS::List<Trigger>::iterator I;
        for (I i = triggers.begin(); i != triggers.end(); ++i) {
            if (&(*i) == &t) {
                triggers.unlink(t);
                request_service();
                return true;
            }
        }
        return false;
    }

    void trigger_add(Trigger&  t) {
        lock_guard<> lock_(lock_get());

        VTSS_ASSERT(!t.is_linked());
        T_N("Triigger add: %p", (void *)&t);
        triggers.push_back(t);
        request_service();
    }

    void run() {
        typedef typename ThreadCtx::clock_t clock_t;
        typedef typename ThreadCtx::timer_t timer_t;
        while(1) {
            typename Clock::time_t now = Clock::now();

            uint32_t trigger_cnt = 0;
            uint32_t timer_trigger_cnt = 0;

            lock();

            // evaluate trigger queue
            while (!triggers.empty()) {
                typedef typename VTSS::List<Trigger>::iterator I;
                I t = triggers.begin();
                triggers.pop_front();

                T_N("trigger-execute %p", &(*t));
                unlock();
                t->execute();
                lock();

                trigger_cnt++;
            }

            // evaluate timers
            while ((!timers.empty()) &&
                   now >= timers.begin()->timeout()) {
                typedef typename VTSS::List<timer_t>::iterator I;
                I t = timers.begin();
                timers.pop_front();
                t->valid(true);

                T_N("timer-trigger-execute %p", &(*t));
                unlock();
                t->invoke_cb();
                lock();

                timer_trigger_cnt++;
            }

            typename Clock::time_t sleep_time = 0;
            if (!timers.empty()) {
                sleep_time = timers.begin()->timeout();
            }


            T_N("TriggerCnt=%u TimerTriggerCnt=%u SleepTime=%llu now=%llu",
                    trigger_cnt, timer_trigger_cnt, sleep_time, now);

            unlock();

            //sleep(ticks);
            if (sleep_time == 0) {
                sleep();
            } else {
                sleep(sleep_time);
            }
        }
    }

private:
    void request_service() {
        static_cast<ThreadCtx*>(this)->request_service();
    }

    void sleep() {
        static_cast<ThreadCtx*>(this)->sleep();
    }

    void sleep(typename Clock::time_t time) {
        static_cast<ThreadCtx*>(this)->sleep(time);
    }

    mutex& lock_get() {
        return static_cast<ThreadCtx*>(this)->lock_get();
    }

    void lock() {
        lock_get().lock();
    }

    void unlock() {
        lock_get().unlock();
    }

    time_t ticks;
    List<Timer> timers;
    List<Trigger> triggers;
};
//typedef BasicTimerService<eCosClock> TimeService;

struct eCosSubjectThread :
    public BasicSubjectRunner<eCosSubjectThread, eCosClock>
{
    friend class BasicSubjectRunner<eCosSubjectThread, eCosClock>;

    typedef eCosClock clock_t;
    typedef BasicTimer<eCosClock::time_t> timer_t;

    eCosSubjectThread ( const char * n ) : name(n) { }
    const static unsigned wake_flag= 1;

    const char * name;
    cyg_flag_t   event_flag;
    cyg_thread   thread_block;
    cyg_handle_t thread_handle;
    char         thread_stack[THREAD_DEFAULT_STACK_SIZE];

private:
    void request_service() {
        T_N("Requesting service");
        cyg_flag_setbits(&event_flag, wake_flag);
    }

    void sleep() {
        T_N("Sleeping");
        cyg_flag_wait(&event_flag,
                      0xFFFFFFFF,
                      CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
        T_N("Wakeup");
    }

    void sleep(eCosClock::time_t time) {
        cyg_flag_value_t res;
        T_N("Timmed sleeping %llu", time);
        res = cyg_flag_timed_wait(&event_flag,
                                  0xFFFFFFFF,
                                  CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR,
                                  time);
        T_N("Wake up res=%08x", res);
    }

    mutex& lock_get() {
        return mutex_;
    }

    mutex mutex_;
    eCosSubjectThread (const eCosSubjectThread&); // no copies
}; // struct SubjectThread

typedef eCosSubjectThread SubjectThread;

template<typename T>
struct ReadOnlySubject
{
    explicit ReadOnlySubject(SubjectRunner &);
    ReadOnlySubject(SubjectRunner &, const T&);

    ~ReadOnlySubject();

    T get() const;
    T get( Trigger & t );

    void signal(Trigger::subject_status_t status);
    void attach(Trigger & trigger);
    bool detach(Trigger & trigger);

protected:
    void attach_(Trigger & trigger);

    T value_;
    SubjectRunner & subject_runner_;
    List<Trigger> trigger_list;

private:
    ReadOnlySubject(const ReadOnlySubject<T>&); // no copy constructor
}; // struct ReadOnlySubject<T>

template<typename T>
struct Subject :
    public ReadOnlySubject<T>
{
    struct Proxy {
        explicit Proxy (Subject* parent) : p(parent) { }
        ~Proxy() { p->signal(); }
        T*operator->() { return &p->_data; }
        T& operator*() { return p->_data; }
    private:
        Subject* p;
    };

    Subject(SubjectRunner &);
    explicit Subject(SubjectRunner &, T value);
    void set(const T& value, bool force = false);
    Proxy proxy_rw() { return Proxy(this); }

private:
    Subject(const Subject<T>&); // no copy constructor
}; // struct Subject<T>


////////////////////////////////////////////////////////////////////////////
// Trigger implementation
////////////////////////////////////////////////////////////////////////////
Trigger::Trigger( ) :
    handler_(0),
    status_(success)
{
}

Trigger::Trigger( AbstractTriggerHandler *handler ) :
    handler_(handler),
    status_(success)
{
}

Trigger::~Trigger( )
{
    // There is no thread safe way to unlink, so this must be done in the
    // destructor of the owning object
    VTSS_ASSERT(!is_linked());
}

void Trigger::signal(subject_status_t status, SubjectRunner * sr)
{
    status_ = status;
    sr->trigger_add(*this);
}

void Trigger::execute()
{
    if (!handler_)
        return;

    handler_->execute(*this, status_);
}

////////////////////////////////////////////////////////////////////////////
// ReadOnlySubject implementation
////////////////////////////////////////////////////////////////////////////
template<typename T>
ReadOnlySubject<T>::ReadOnlySubject( SubjectRunner & sr ) :
    subject_runner_(sr)
{
}

template<typename T>
ReadOnlySubject<T>::ReadOnlySubject( SubjectRunner & sr, const T& v ) :
    value_(v),
    subject_runner_(sr)
{
}

template<typename T>
ReadOnlySubject<T>::~ReadOnlySubject()
{
    // TODO Send a hang-up to all attached triggers
    signal(Trigger::hangup);
}

template<typename T>
T ReadOnlySubject<T>::get() const
{
    return value_;
}

template<typename T>
T ReadOnlySubject<T>::get(Trigger & t)
{
    attach_(t);
    return value_;
}

template<typename T>
void ReadOnlySubject<T>::attach(Trigger & trigger)
{
    attach_(trigger);
}

template<typename T>
bool ReadOnlySubject<T>::detach(Trigger & trigger)
{
    trigger_list.unlink(trigger);
    return true;
}

template<typename T>
void ReadOnlySubject<T>::signal(Trigger::subject_status_t status)
{
    while (!trigger_list.empty()) {
        Trigger & t = trigger_list.front();
        trigger_list.pop_front();
        t.signal(status, &subject_runner_);
    }
}

template<typename T>
void ReadOnlySubject<T>::attach_(Trigger & trigger)
{
    trigger_list.push_back(trigger);
}

////////////////////////////////////////////////////////////////////////////
// Subject implementation
////////////////////////////////////////////////////////////////////////////
template<typename T>
Subject<T>::Subject(SubjectRunner & sr) :
    ReadOnlySubject<T>(sr)
{
}

template<typename T>
Subject<T>::Subject(  SubjectRunner & sr, T v ) :
    ReadOnlySubject<T>(sr, v)
{
}

template<typename T>
void Subject<T>::set( const T& value, bool force )
{
    if (force || value != ReadOnlySubject<T>::value_) {
        ReadOnlySubject<T>::value_ = value;
        ReadOnlySubject<T>::signal(Trigger::success);
    }
}
} /* VTSS */

extern "C" void subject_thread_run( cyg_addrword_t _st_ptr )
{
    using namespace VTSS;
    SubjectThread * st_ptr = (SubjectThread *)_st_ptr;
    st_ptr->run();
}

extern "C" void subject_thread_start(VTSS::SubjectThread * st)
{
    using namespace VTSS;
    cyg_flag_init( &st->event_flag );
    cyg_thread_create(THREAD_DEFAULT_PRIO, subject_thread_run,
                      (cyg_addrword_t)st,  const_cast<char*>(st->name),
                      st->thread_stack,    sizeof(st->thread_stack),
                      &st->thread_handle,  &st->thread_block);
    cyg_thread_resume(st->thread_handle);
}

#endif /* __SUBJECT_H__ */

