/*
 * Tasker for Arduino - cooperative task scheduler with Javascript like API
 * Copyleft (c) 2015-2026  Petr Stehlik  petr@pstehlik.cz
 * Distributed under the GNU LGPL http://www.gnu.org/licenses/lgpl.txt
 *
 * Number of tasks configurable at compile time to save precious RAM
 *
 * Memory usage:
 *   - 15 bytes per scheduled task on AVR (Uno, Nano, Mega...)
 *   - 24 bytes per task on 32-bit platforms (ESP32, Teensy, RP2040, etc.)
 */

#ifndef _tasker_h
#define _tasker_h

#ifndef TASKER_MAX_TASKS
#define TASKER_MAX_TASKS 10 // max 255 entries on AVR, up to 65535 entries elsewhere
#endif

#include <Arduino.h>

#if defined(__AVR__)
    using tasker_count_t = uint8_t;
#else
    using tasker_count_t = uint16_t;
#endif

// Pack the TASK struct on AVR (8-bit) to consume as little RAM as possible.
#if defined(__AVR__)
    #define TASKER_PACKED __attribute__((packed))
#else
    #define TASKER_PACKED
#endif

typedef void (*TaskCallback)(void);
typedef void (*TaskCallbackInt)(int);
typedef void (*TaskCallbackPtr)(void*);

// Backward compatibility aliases
typedef TaskCallback    TaskCallback0;
typedef TaskCallbackInt TaskCallback1;

template <size_t MAX_TASKS = TASKER_MAX_TASKS>
class TaskerT
{
public:
    TaskerT(bool prioritized = false);

    // No parameter
    bool setImmediate(TaskCallback func, tasker_count_t prio = MAX_TASKS);
    bool setTimeout(TaskCallback func, unsigned long interval, tasker_count_t prio = MAX_TASKS);
    bool setInterval(TaskCallback func, unsigned long interval, tasker_count_t prio = MAX_TASKS);
    bool setRepeated(TaskCallback func, unsigned long interval, unsigned int repeat, tasker_count_t prio = MAX_TASKS);
    bool cancel(TaskCallback func);
    bool clearTimeout(TaskCallback func) { return cancel(func); }
    bool clearInterval(TaskCallback func) { return cancel(func); }
    unsigned long scheduledIn(TaskCallback func);

    // int parameter
    bool setImmediate(TaskCallbackInt func, int value, tasker_count_t prio = MAX_TASKS);
    bool setTimeout(TaskCallbackInt func, unsigned long interval, int value, tasker_count_t prio = MAX_TASKS);
    bool setInterval(TaskCallbackInt func, unsigned long interval, int value, tasker_count_t prio = MAX_TASKS);
    bool setRepeated(TaskCallbackInt func, unsigned long interval, unsigned int repeat, int value, tasker_count_t prio = MAX_TASKS);
    bool cancel(TaskCallbackInt func, int value);
    bool clearTimeout(TaskCallbackInt func, int value) { return cancel(func, value); }
    bool clearInterval(TaskCallbackInt func, int value) { return cancel(func, value); }
    unsigned long scheduledIn(TaskCallbackInt func, int value);

    // void* parameter
    bool setImmediate(TaskCallbackPtr func, void* value, tasker_count_t prio = MAX_TASKS);
    bool setTimeout(TaskCallbackPtr func, unsigned long interval, void* value, tasker_count_t prio = MAX_TASKS);
    bool setInterval(TaskCallbackPtr func, unsigned long interval, void* value, tasker_count_t prio = MAX_TASKS);
    bool setRepeated(TaskCallbackPtr func, unsigned long interval, unsigned int repeat, void* value, tasker_count_t prio = MAX_TASKS);
    bool cancel(TaskCallbackPtr func, void* value);
    bool clearTimeout(TaskCallbackPtr func, void* value) { return cancel(func, value); }
    bool clearInterval(TaskCallbackPtr func, void* value) { return cancel(func, value); }
    unsigned long scheduledIn(TaskCallbackPtr func, void* value);

    void loop(void);

    bool isPrioritized() { return t_prioritized; }
    void setPrioritized(bool prioritized) { t_prioritized = prioritized; }

    // Capacity helpers
    bool            isFull()    const { return t_count >= MAX_TASKS; }
    tasker_count_t  count()     const { return t_count; }
    tasker_count_t  available() const { return static_cast<tasker_count_t>(MAX_TASKS - t_count); }

    // Small helper for global user data (e.g. context)
    void*  getUserData() const { return userData; }
    void   setUserData(void* data) { userData = data; }

    static constexpr size_t capacity = MAX_TASKS;

private:
    // TASK struct size:
    //   - 15 bytes on AVR (packed, unsigned int repeat = 2 bytes)
    //   - Larger on 32-bit platforms (unsigned int repeat = 4 bytes + alignment)
    struct TASKER_PACKED TASK {
        void*           call;
        void*           param;
        unsigned long   interval;
        unsigned long   lastRun;
        unsigned int    repeat;
        uint8_t         flags;
    };

    int findTask(TaskCallback func);
    int findTask(TaskCallbackInt func, int value);
    int findTask(TaskCallbackPtr func, void* value);

    bool addTask(void* func, unsigned long interval, unsigned int repeat, void* value, uint8_t flags, tasker_count_t prio);

    bool removeTask(int t_idx);

    TASK tasks[MAX_TASKS];
    tasker_count_t t_count;
    bool t_prioritized;
    void* userData = nullptr;

    static constexpr uint8_t HAS_PARAM     = 0b00000001;
    static constexpr uint8_t PARAM_IS_INT  = 0b00000010;
    static constexpr uint8_t IS_IMMEDIATE  = 0b00000100;

    int findTaskImpl(void* func, void* param, uint8_t flags, uint8_t mask);
    unsigned long scheduledInImpl(int t_idx);
};

// ====================== Implementation ======================

template <size_t MAX_TASKS>
TaskerT<MAX_TASKS>::TaskerT(bool prioritized)
{
    t_count = 0;
    t_prioritized = prioritized;
}

// === No parameter versions ===

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setImmediate(TaskCallback func, tasker_count_t prio)
{
    return addTask((void*)func, 0, 0, nullptr, IS_IMMEDIATE, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setTimeout(TaskCallback func, unsigned long interval, tasker_count_t prio)
{
    return addTask((void*)func, interval, 1, nullptr, 0, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setInterval(TaskCallback func, unsigned long interval, tasker_count_t prio)
{
    return addTask((void*)func, interval, 0, nullptr, 0, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setRepeated(TaskCallback func, unsigned long interval, unsigned int repeat, tasker_count_t prio)
{
    return addTask((void*)func, interval, repeat, nullptr, 0, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::cancel(TaskCallback func)
{
    return removeTask(findTask(func));
}

template <size_t MAX_TASKS>
unsigned long TaskerT<MAX_TASKS>::scheduledIn(TaskCallback func)
{
    return scheduledInImpl(findTask(func));
}

// === int parameter versions ===

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setImmediate(TaskCallbackInt func, int value, tasker_count_t prio)
{
    return addTask((void*)func, 0, 0, reinterpret_cast<void*>(value), IS_IMMEDIATE | HAS_PARAM | PARAM_IS_INT, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setTimeout(TaskCallbackInt func, unsigned long interval, int value, tasker_count_t prio)
{
    return addTask((void*)func, interval, 1, reinterpret_cast<void*>(value), HAS_PARAM | PARAM_IS_INT, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setInterval(TaskCallbackInt func, unsigned long interval, int value, tasker_count_t prio)
{
    return addTask((void*)func, interval, 0, reinterpret_cast<void*>(value), HAS_PARAM | PARAM_IS_INT, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setRepeated(TaskCallbackInt func, unsigned long interval, unsigned int repeat, int value, tasker_count_t prio)
{
    return addTask((void*)func, interval, repeat, reinterpret_cast<void*>(value), HAS_PARAM | PARAM_IS_INT, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::cancel(TaskCallbackInt func, int value)
{
    return removeTask(findTask(func, value));
}

template <size_t MAX_TASKS>
unsigned long TaskerT<MAX_TASKS>::scheduledIn(TaskCallbackInt func, int value)
{
    return scheduledInImpl(findTask(func, value));
}

// === void* parameter versions ===

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setImmediate(TaskCallbackPtr func, void* value, tasker_count_t prio)
{
    return addTask((void*)func, 0, 0, value, IS_IMMEDIATE | HAS_PARAM, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setTimeout(TaskCallbackPtr func, unsigned long interval, void* value, tasker_count_t prio)
{
    return addTask((void*)func, interval, 1, value, HAS_PARAM, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setInterval(TaskCallbackPtr func, unsigned long interval, void* value, tasker_count_t prio)
{
    return addTask((void*)func, interval, 0, value, HAS_PARAM, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::setRepeated(TaskCallbackPtr func, unsigned long interval, unsigned int repeat, void* value, tasker_count_t prio)
{
    return addTask((void*)func, interval, repeat, value, HAS_PARAM, prio);
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::cancel(TaskCallbackPtr func, void* value)
{
    return removeTask(findTask(func, value));
}

template <size_t MAX_TASKS>
unsigned long TaskerT<MAX_TASKS>::scheduledIn(TaskCallbackPtr func, void* value)
{
    return scheduledInImpl(findTask(func, value));
}

// Returns milliseconds until the task is next due to run:
//   0 = not scheduled
//   1 = due immediately (IS_IMMEDIATE or overdue)
//   X = due in X ms
template <size_t MAX_TASKS>
unsigned long TaskerT<MAX_TASKS>::scheduledInImpl(int t_idx)
{
    if (t_idx < 0) return 0;

    TASK &t = tasks[t_idx];
    if (t.flags & IS_IMMEDIATE) return 1;

    unsigned long now = millis();
    return (now - t.lastRun >= t.interval) ? 1 : (t.lastRun + t.interval - now);
}

// === Core implementation ===

template <size_t MAX_TASKS>
void TaskerT<MAX_TASKS>::loop(void)
{
    tasker_count_t t_idx = 0;
    unsigned long now = millis();

    while (t_idx < t_count) {
        bool inc = true;
        TASK &t = tasks[t_idx];

        if ((t.flags & IS_IMMEDIATE) || (now - t.lastRun >= t.interval)) {
            void* the_call = t.call;
            void* the_param = t.param;
            uint8_t the_flags = t.flags;

            t.lastRun += t.interval;

            if ((the_flags & IS_IMMEDIATE) || (t.repeat > 0 && --t.repeat == 0)) {
                // drop the finished task by removing its slot
                removeTask(t_idx);
                inc = false;
            }

            if (the_flags & HAS_PARAM) {
                if (the_flags & PARAM_IS_INT) {
                    int value = static_cast<int>(reinterpret_cast<uintptr_t>(the_param));
                    reinterpret_cast<TaskCallbackInt>(the_call)(value);
                } else {
                    reinterpret_cast<TaskCallbackPtr>(the_call)(the_param);
                }
            } else {
                reinterpret_cast<TaskCallback>(the_call)();
            }

            if (t_prioritized)
                break;

            now = millis();
        }
        if (inc)
            t_idx++;
    }
}

template <size_t MAX_TASKS>
int TaskerT<MAX_TASKS>::findTask(TaskCallback func)
{
    return findTaskImpl((void*)func, nullptr, 0, HAS_PARAM);
}

template <size_t MAX_TASKS>
int TaskerT<MAX_TASKS>::findTask(TaskCallbackInt func, int value)
{
    void* v = reinterpret_cast<void*>(value);
    return findTaskImpl((void*)func, v, HAS_PARAM | PARAM_IS_INT, HAS_PARAM | PARAM_IS_INT);
}

template <size_t MAX_TASKS>
int TaskerT<MAX_TASKS>::findTask(TaskCallbackPtr func, void* value)
{
    return findTaskImpl((void*)func, value, HAS_PARAM, HAS_PARAM);
}

template <size_t MAX_TASKS>
int TaskerT<MAX_TASKS>::findTaskImpl(void* func, void* param, uint8_t flags, uint8_t mask)
{
    for (tasker_count_t i = 0; i < t_count; i++) {
        TASK &t = tasks[i];
        if (t.call == func &&
            (t.flags & mask) == flags &&
            ((flags & HAS_PARAM) == 0 || t.param == param))
            return i;
    }
    return -1;
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::addTask(void* func, unsigned long interval, unsigned int repeat,
                                void* value, uint8_t flags, tasker_count_t prio)
{
    tasker_count_t pos = (prio < t_count) ? prio : t_count;  // position of newly added task is based on priority

    uint8_t f = flags & (HAS_PARAM | PARAM_IS_INT);
    uint8_t m = f ? f : HAS_PARAM;
    int idx = findTaskImpl(func, f ? value : nullptr, f, m);
    if (idx >= 0) {
        removeTask(idx);       // if there's a matching task then remove it first
        pos = idx;             // new task will replace the original one
    }

    if (t_count >= MAX_TASKS || (interval == 0 && (flags & IS_IMMEDIATE) == 0))
        return false;

    if (pos < t_count)
        memmove(tasks + pos + 1, tasks + pos, sizeof(TASK) * (t_count - pos));

    TASK &t = tasks[pos];
    t.call     = func;
    t.param    = value;
    t.interval = interval;
    t.lastRun  = (flags & IS_IMMEDIATE) ? 0 : millis();
    t.repeat   = repeat;
    t.flags    = flags;

    t_count++;
    return true;
}

template <size_t MAX_TASKS>
bool TaskerT<MAX_TASKS>::removeTask(int t_idx)
{
    if (t_idx >= 0 && static_cast<tasker_count_t>(t_idx) < t_count) {
        tasker_count_t idx = static_cast<tasker_count_t>(t_idx);
        memmove(tasks + idx, tasks + idx + 1, sizeof(TASK) * (t_count - idx - 1));
        t_count--;
        return true;
    }
    return false;
}

// Backward-compatibility alias for 100% source compatibility with old code.
// Bare "Tasker tasker;" continues to work exactly as before.
// Use TaskerT<N> when you want to specify a custom number of tasks.
using Tasker = TaskerT<TASKER_MAX_TASKS>;

#endif // _tasker_h
