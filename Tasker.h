/*
 * Tasker for Arduino - cooperative task scheduler with Javascript like API
 * Copyleft (c) 2015-2018  Petr Stehlik  petr@pstehlik.cz
 * Distributed under the GNU LGPL http://www.gnu.org/licenses/lgpl.txt
 */

#ifndef _tasker_h
#define _tasker_h

#ifndef TASKER_MAX_TASKS
#define TASKER_MAX_TASKS 10 // max 254 entries, one occupies 14 bytes of RAM
#endif

#include "Arduino.h"

typedef void (*TaskCallback0)(void);
typedef void (*TaskCallback1)(int);

class Tasker
{
public:
	Tasker(bool prioritized = false);

	bool setTimeout(TaskCallback0 func, unsigned long interval, byte prio = TASKER_MAX_TASKS);
	bool setInterval(TaskCallback0 func, unsigned long interval, byte prio = TASKER_MAX_TASKS);
	bool setRepeated(TaskCallback0 func, unsigned long interval, unsigned int repeat, byte prio = TASKER_MAX_TASKS);
	bool cancel(TaskCallback0 func);
	bool clearTimeout(TaskCallback0 func) { cancel(func); };
	bool clearInterval(TaskCallback0 func) { cancel(func); };

	bool setTimeout(TaskCallback1 func, unsigned long interval, int param, byte prio = TASKER_MAX_TASKS);
	bool setInterval(TaskCallback1 func, unsigned long interval, int param, byte prio = TASKER_MAX_TASKS);
	bool setRepeated(TaskCallback1 func, unsigned long interval, unsigned int repeat, int param, byte prio = TASKER_MAX_TASKS);
	bool cancel(TaskCallback1 func, int param);
	bool clearTimeout(TaskCallback1 func, int param) { cancel(func, param); };
	bool clearInterval(TaskCallback1 func, int param) { cancel(func, param); };

	void loop(void);
	bool isPrioritized() { return t_prioritized; }
	void setPrioritized(bool prioritized) { t_prioritized = prioritized; }

private:
	struct TASK {
		union {
			TaskCallback0 call0;
			TaskCallback1 call1;
		};
		int param;
		unsigned long interval;
		unsigned long lastRun;
		unsigned int repeat;
	};

	int findTask(TaskCallback0 func);
	int findTask(TaskCallback1 func, int param);
	bool removeTask(int t_idx);

	TASK tasks[TASKER_MAX_TASKS];
	byte t_count;
	bool t_prioritized;
};


Tasker::Tasker(bool prioritized)
{
	t_count = 0;
	t_prioritized = prioritized;
}

bool Tasker::setTimeout(TaskCallback0 func, unsigned long interval, byte prio)
{
	return setRepeated(func, interval, 1, prio);
}

bool Tasker::setInterval(TaskCallback0 func, unsigned long interval, byte prio)
{
	return setRepeated(func, interval, 0, prio);
}

bool Tasker::setRepeated(TaskCallback0 func, unsigned long interval, unsigned int repeat, byte prio)
{
	if (t_count >= TASKER_MAX_TASKS || interval == 0)
		return false;
	byte pos = (prio < t_count) ? prio : t_count;
	if (pos < t_count)
		memmove(tasks+pos+1, tasks+pos, sizeof(TASK)*(t_count-pos));
	TASK &t = tasks[pos];
	t.call0 = func;
	t.interval = interval;
	t.param = -1;
	t.lastRun = millis();
	t.repeat = repeat;
	t_count++;
	return true;
}

bool Tasker::setTimeout(TaskCallback1 func, unsigned long interval, int param, byte prio)
{
	return setRepeated(func, interval, 1, param, prio);
}

bool Tasker::setInterval(TaskCallback1 func, unsigned long interval, int param, byte prio)
{
	return setRepeated(func, interval, 0, param, prio);
}

bool Tasker::setRepeated(TaskCallback1 func, unsigned long interval, unsigned int repeat, int param, byte prio)
{
	if (t_count >= TASKER_MAX_TASKS || interval == 0)
		return false;
	byte pos = (prio < t_count) ? prio : t_count;
	if (pos < t_count)
		memmove(tasks+pos+1, tasks+pos, sizeof(TASK)*(t_count-pos));
	TASK &t = tasks[pos];
	t.call1 = func;
	t.interval = interval;
	t.param = param;
	t.lastRun = millis();
	t.repeat = repeat;
	t_count++;
	return true;
}

void Tasker::loop(void)
{
	byte t_idx = 0;
	unsigned long now = millis();
	while(t_idx < t_count) {
		bool inc = true;
		TASK &t = tasks[t_idx];
		if (now - t.lastRun >= t.interval) {
			t.lastRun += t.interval;
			if (t.param >= 0)
				(*(t.call1))(t.param);
			else
				(*(t.call0))();
			if (t.repeat > 0 && --t.repeat == 0) {
				// drop the finished task by removing its slot
				removeTask(t_idx);
				inc = false;
			}
			if (t_prioritized)
				break;
			now = millis();
		}
		if (inc)
			t_idx++;
	}
}

bool Tasker::cancel(TaskCallback0 func)
{
	return removeTask(findTask(func));
}

int Tasker::findTask(TaskCallback0 func)
{
	for(byte t_idx = 0; t_idx < t_count; t_idx++) {
		TASK &t = tasks[t_idx];
		if (t.call0 == func)
			return t_idx;
	}
	return -1;
}

bool Tasker::cancel(TaskCallback1 func, int param)
{
	return removeTask(findTask(func, param));
}

int Tasker::findTask(TaskCallback1 func, int param)
{
	for(byte t_idx = 0; t_idx < t_count; t_idx++) {
		TASK &t = tasks[t_idx];
		if (t.call1 == func && t.param == param)
			return t_idx;
	}
	return -1;
}

bool Tasker::removeTask(int t_idx)
{
	if (t_idx >= 0 && t_idx < t_count) {
		memmove(tasks+t_idx, tasks+t_idx+1, sizeof(TASK)*(t_count-t_idx-1));
		t_count--;
		return true;
	}
	return false;
}

#endif // _tasker_h
