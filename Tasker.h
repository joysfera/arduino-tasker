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
	bool setTimeout(TaskCallback1 func, unsigned long interval, int param, byte prio = TASKER_MAX_TASKS);

	bool setInterval(TaskCallback0 func, unsigned long interval, byte prio = TASKER_MAX_TASKS);
	bool setInterval(TaskCallback1 func, unsigned long interval, int param, byte prio = TASKER_MAX_TASKS);

	bool setRepeated(TaskCallback0 func, unsigned long interval, unsigned int repeat, byte prio = TASKER_MAX_TASKS);
	bool setRepeated(TaskCallback1 func, unsigned long interval, unsigned int repeat, int param, byte prio = TASKER_MAX_TASKS);

	bool cancel(TaskCallback0 func);
	bool cancel(TaskCallback1 func, int param);

	bool clearTimeout(TaskCallback0 func) { cancel(func); };
	bool clearTimeout(TaskCallback1 func, int param) { cancel(func, param); };

	bool clearInterval(TaskCallback0 func) { cancel(func); };
	bool clearInterval(TaskCallback1 func, int param) { cancel(func, param); };

	unsigned long scheduledIn(TaskCallback0 func);
	unsigned long scheduledIn(TaskCallback1 func, int param);

	void loop(void);

	bool isPrioritized() { return t_prioritized; }
	void setPrioritized(bool prioritized) { t_prioritized = prioritized; }

private:
	struct TASK {
		TaskCallback1 call;
		int param;
		unsigned long interval;
		unsigned long lastRun;
		unsigned int repeat;
	};

	int findTask(TaskCallback0 func);
	int findTask(TaskCallback1 func, int param);
	bool addTask(TaskCallback1 func, unsigned long interval, unsigned int repeat, int param, byte prio);
	bool removeTask(int t_idx);

	TASK tasks[TASKER_MAX_TASKS];
	byte t_count;
	bool t_prioritized;
	static const int NO_PARAMETER = -1;
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

bool Tasker::setTimeout(TaskCallback1 func, unsigned long interval, int param, byte prio)
{
	return setRepeated(func, interval, 1, param, prio);
}

bool Tasker::setInterval(TaskCallback1 func, unsigned long interval, int param, byte prio)
{
	return setRepeated(func, interval, 0, param, prio);
}

bool Tasker::setInterval(TaskCallback0 func, unsigned long interval, byte prio)
{
	return setRepeated(func, interval, 0, prio);
}

bool Tasker::setRepeated(TaskCallback0 func, unsigned long interval, unsigned int repeat, byte prio)
{
	return addTask((TaskCallback1)func, interval, repeat, NO_PARAMETER, prio);
}

bool Tasker::setRepeated(TaskCallback1 func, unsigned long interval, unsigned int repeat, int param, byte prio)
{
	if (param < 0) param = 0; // param can be nonnegative only
	return addTask(func, interval, repeat, param, prio);
}

bool Tasker::cancel(TaskCallback0 func)
{
	return removeTask(findTask(func));
}

bool Tasker::cancel(TaskCallback1 func, int param)
{
	return removeTask(findTask(func, param));
}

unsigned long Tasker::scheduledIn(TaskCallback0 func)
{
	return scheduledIn((TaskCallback1)func, NO_PARAMETER);
}

// how long before the given task will be called
// return 0 = task is not scheduled
// return 1 = scheduled to run as soon as possible
// return X = time period in milliseconds
unsigned long Tasker::scheduledIn(TaskCallback1 func, int param)
{
	int t_idx = findTask(func, param);
	if (t_idx >= 0) {
		TASK &t = tasks[t_idx];
		unsigned long now = millis();
		if (now - t.lastRun >= t.interval)
			return 1; // scheduled to run as soon as possible
		else
			return t.lastRun + t.interval - now;
	}
	return 0;
}

void Tasker::loop(void)
{
	byte t_idx = 0;
	unsigned long now = millis();
	while(t_idx < t_count) {
		bool inc = true;
		TASK &t = tasks[t_idx];
		if (now - t.lastRun >= t.interval) {
			int param = t.param;
			TaskCallback1 call = t.call;

			t.lastRun += t.interval;
			if (t.repeat > 0 && --t.repeat == 0) {
				// drop the finished task by removing its slot
				removeTask(t_idx);
				inc = false;
			}

			if (param >= 0)   // param can be nonnegative only
				(*(call))(param);
			else
				(*(TaskCallback0)(call))();

			if (t_prioritized)
				break;
			now = millis();
		}
		if (inc)
			t_idx++;
	}
}

int Tasker::findTask(TaskCallback0 func)
{
	return findTask((TaskCallback1)func, NO_PARAMETER);
}

int Tasker::findTask(TaskCallback1 func, int param)
{
	for(byte t_idx = 0; t_idx < t_count; t_idx++) {
		TASK &t = tasks[t_idx];
		if (t.call == func && t.param == param)
			return t_idx;
	}
	return -1;
}

bool Tasker::addTask(TaskCallback1 func, unsigned long interval, unsigned int repeat, int param, byte prio)
{
	byte pos = (prio < t_count) ? prio : t_count;  // position of newly added task is based on priority

	int idx = findTask(func, param);
	if (idx >= 0) {
		removeTask(idx);       // if there's a matching task then remove it first
		pos = idx;             // new task will replace the original one
	}

	if (t_count >= TASKER_MAX_TASKS || interval == 0)
		return false;

	if (pos < t_count)
		memmove(tasks+pos+1, tasks+pos, sizeof(TASK)*(t_count-pos));
	TASK &t = tasks[pos];
	t.call = func;
	t.interval = interval;
	t.param = param;
	t.lastRun = millis();
	t.repeat = repeat;
	t_count++;
	return true;
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
// vim: tabstop=4 shiftwidth=4 noexpandtab cindent
