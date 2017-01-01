/*
 * Tasker for Arduino - cooperative task scheduler
 * Copyright (c) 2015  Petr Stehlik  petr@pstehlik.cz
 * Distributed under the GNU LGPL http://www.gnu.org/licenses/lgpl.txt
 */

#ifndef TASKER_MAX_TASKS
#define TASKER_MAX_TASKS	10	/* max 254 entries, one occupies 14 bytes of RAM */
#endif

#include "Arduino.h"

typedef void (*TaskCallback)(int);

class Tasker
{
public:
	Tasker(bool prioritized = true);
	bool setTimeout(TaskCallback func, unsigned long interval, int param = 0);
	bool setInterval(TaskCallback func, unsigned long interval, int param = 0);
	bool setRepeated(TaskCallback func, unsigned long interval, unsigned int repeat, int param = 0);
	void loop(void);
	void run(void) { while(true) loop(); }
private:
	struct TASK {
		TaskCallback call;
		int param;
		unsigned long interval;
		unsigned long lastRun;
		unsigned int repeat;
	};

	TASK tasks[TASKER_MAX_TASKS];
	byte t_count;
	bool t_prioritized;
};


Tasker::Tasker(bool prioritized)
{
	t_count = 0;
	t_prioritized = prioritized;
}

bool Tasker::setTimeout(TaskCallback func, unsigned long interval, int param)
{
	return setRepeated(func, interval, 1, param);
}

bool Tasker::setInterval(TaskCallback func, unsigned long interval, int param)
{
	return setRepeated(func, interval, 0, param);
}

bool Tasker::setRepeated(TaskCallback func, unsigned long interval, unsigned int repeat, int param)
{
	if (t_count >= TASKER_MAX_TASKS || interval == 0)
		return false;
	TASK &t = tasks[t_count];
	t.call = func;
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
			(*(t.call))(t.param);
			if (t.repeat > 0 && --t.repeat == 0) {
				// drop the finished task by removing its slot
				memmove(tasks+t_idx, tasks+t_idx+1, sizeof(TASK)*(t_count-t_idx-1));
				t_count--;
				inc = false;
			}
			yield();
			if (t_prioritized)
				break;
			now = millis();
		}
		if (inc)
			t_idx++;
	}
}

