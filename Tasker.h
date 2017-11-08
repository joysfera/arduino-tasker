/*
 * Tasker for Arduino - cooperative task scheduler with Javascript like API
 * Copyleft (c) 2015-2017  Petr Stehlik  petr@pstehlik.cz
 * Distributed under the GNU LGPL http://www.gnu.org/licenses/lgpl.txt
 */

#ifndef _tasker_h
#define _tasker_h

#ifndef TASKER_MAX_TASKS
#define TASKER_MAX_TASKS 10 // max 254 entries, one occupies 14 bytes of RAM
#endif

#include "Arduino.h"

typedef void (*TaskCallback)(int);

class Tasker
{
public:
	Tasker(bool prioritized = true);
	bool setTimeout(TaskCallback func, unsigned long interval, int param = 0, byte prio = TASKER_MAX_TASKS);
	bool setInterval(TaskCallback func, unsigned long interval, int param = 0, byte prio = TASKER_MAX_TASKS);
	bool setRepeated(TaskCallback func, unsigned long interval, unsigned int repeat, int param = 0, byte prio = TASKER_MAX_TASKS);
	void loop(void);
	void run(void) { while(true) { loop(); yield(); } }

  void changeInterval(TaskCallback func, unsigned long interval);
  void resetCountdown(TaskCallback func);
  void cancel(TaskCallback func);
  uint8_t isScheduled(TaskCallback func);

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

bool Tasker::setTimeout(TaskCallback func, unsigned long interval, int param, byte prio)
{
	return setRepeated(func, interval, 1, param, prio);
}

bool Tasker::setInterval(TaskCallback func, unsigned long interval, int param, byte prio)
{
	return setRepeated(func, interval, 0, param, prio);
}

bool Tasker::setRepeated(TaskCallback func, unsigned long interval, unsigned int repeat, int param, byte prio)
{
	if (t_count >= TASKER_MAX_TASKS || interval == 0)
		return false;
	byte pos = (prio < t_count) ? prio : t_count;
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
			if (t_prioritized)
				break;
			yield();
			now = millis();
		}
		if (inc)
			t_idx++;
	}
}

void Tasker::changeInterval(TaskCallback func, unsigned long interval) {
  for (uint8_t i = 0; i < t_count; i++) {
    TASK *t = &tasks[i];
    if (t->call == func) {
      t->interval = interval;
      return;
    }
  }
}

void Tasker::resetCountdown(TaskCallback func) {
  for (uint8_t i = 0; i < t_count; i++) {
    TASK *t = &tasks[i];
    if (t->call == func) {
      t->lastRun = millis();
      return;
    }
  }
}

void Tasker::cancel(TaskCallback func) {
  for (uint8_t i = 0; i < t_count; i++) {
    TASK *t = &tasks[i];

    if (t->call == func) {
      // drop the this task by removing its slot
      memmove(tasks + i, tasks + i + 1, sizeof(TASK) * (t_count - i - 1));
      return;
    }
  }
}

uint8_t Tasker::isScheduled(TaskCallback func) {
  for (uint8_t i = 0; i < t_count; i++) {
      TASK *t = &tasks[i];

      if (t->call == func) {
        return 1;
      }
    }

  return 0;
}

#endif // _tasker_h
