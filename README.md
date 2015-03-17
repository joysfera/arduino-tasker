Tasker for Arduino
==================

This is a cooperative scheduler for running multiple tasks on Arduino. The tasks are
called automatically at specified times for specified number of times. This frees your
program from timing logic and makes your Arduino look like it's doing several things at once.

*Cooperative* means that the tasks you create need to behave nicely -
to co-operate together by running for a short time only, otherwise it will not work well.
*Scheduler* means that each task has its own schedule so Tasker knows when the tasks are to be
started (in what time since now) and how many times they should be invoked
(only once, X times or forever).

The "co-operation" is best achieved by creating small, short running tasks.
Basically wherever you'd need to include the infamous *delay()* call in your Arduino program
that's the place where you actually want to break the code flow, split
the source code into separate functions and let Tasker run them as separate tasks.

There are many similar libraries for the same purpose available on the Internet
but they are either buggy (don't handle timer overflow) or too complicated, cumbersome to use,
unnecessary object-oriented or otherwise hard to understand and follow.

This Tasker library is carefully designed to be extremely simple
yet very powerful. It has short, clean API and clear implementation that fits
on a single page and can be reviewed easily.
Best of all, its API is intentionally similar to JavaScript's timer
functions so HTML/JavaScript programmers will feel right at home.
For the unfamiliar with JavaScript there's a short example code included that
illustrates the whole API and its best usage.

How to use
----------

1. create new **Tasker** folder under your Arduino projects' libraries folder and place Tasker files there so the header file ends in **./libraries/Tasker/Tasker.h**
2. in Arduino IDE load File -> Examples -> Tasker -> MultiBlink
3. see how easy it is to add three tasks and run them all at once
4. use that example as a basis for your own code

Tasker API
----------

* <code>Tasker([bool prioritized])</code>. The class constructor takes
  an optional bool flag (that is set to true if omitted). If this flag
  is TRUE then the Tasker prioritizes the scheduled tasks. If the flag
  is FALSE then the Tasker considers all scheduled tasks equal. More about priorities later.

```cpp
	Tasker tasker;        // creates prioritizing tasker
	Tasker tasker(FALSE); // creates non-prioritizing tasker
```

* <code>setTimeout(function_name, time_in_milliseconds [, optional_int])</code>
  Will call the *function_name* in *time_in_milliseconds* from now.
  Will run it only once. May pass the *optional_int* parameter into the called function.
  When the task finishes its Tasker slot is made available for new tasks (more about slots later).

* <code>setInterval(function_name, time_in_milliseconds [, optional_int])</code>
  Will call the *function_name* repeatedly and forever, every
  *time_in_milliseconds* from now on.
  May pass the *optional_int* parameter into the called function.
  When the task finishes (after its last iteration) its Tasker slot is made available for new tasks.

* <code>setRepeated(function_name, time, number_of_repeats [, optional_int])</code>
  Will call the *function_name* repeatedly for *number_of_repeats*,
  every *time* (in_milliseconds) from now on.
  May pass the <code>optional_int</code> parameter into the called function.

* <code>run()</code> when called it starts the Tasker scheduler and will never return.
  Best to be called as the very last command of the Arduino's <code>setup()</code> function:

```cpp
	void setup() {
		tasker.setInterval(...);
		tasker.run();	// will not return
	}

	void loop() {
		// unused, never called
	}
```

* optional: if you, for whatever reason, don't want to let the <code>Tasker.run()</code>
  govern all of your running code and wish to run Tasker together with some
  existing code you can omit the <code>run()</code> and call the <code>Tasker.loop()</code>
  repeatedly instead.

```cpp
	void setup() {
		tasker.setInterval(...);
	}

	void loop() {
		tasker.loop();	// needs to be called as often as possible
		// legacy code here
	}
```

Task priorities
---------------
If the Tasker constructor was not called with a FALSE flag then the internal
scheduler will prioritize the tasks in its queue. Tasks added later have lower
priority than those added earlier. Thus you'll want to add your more
important tasks first and the less important ones add afterwards.

```cpp
	Tasker tasker;
	tasker.setInterval(most_important_fn, ..);
	tasker.setInterval(less_important_fn, ..);
	tasker.setInterval(least_important_fn, ..);
```

Normally, when there is enough time for calling each of the scheduled task
at the right time the priorities don't play any role but when a previous task takes
longer time and the scheduler detects that certain tasks are delayed
(are behind their schedule) it needs to decide which task will get run of those
that should have been run already. And that's where the tasks' priorities step
in: the task added earlier (= with a higher priority) will be chosen.
If the priorities were disabled then the scheduler would simply run the next task
in its queue. If all your tasks are equally important you might want to disable
the priorities by passing FALSE into the constructor:

```cpp
	Tasker tasker(false);
	tasker.setInterval(fn, ..);
	tasker.setInterval(equally_important_fn, ..);
	tasker.setInterval(order_doesnt_matter_fn, ..);
```

Caveats
-------
Make sure you define enough task "slots" in the Tasker, before you include its
header file. By default there are 10 slots defined for up to ten tasks running
simultaneously but you might want to increase this number - or even decrease, to
save the precious RAM (each slot takes 14 bytes of RAM).

```cpp
	#define TASKER_MAX_TASKS 32
	#include "Tasker.h"
```

Good news is that Tasker automatically releases slots of finished tasks (those
that were invoked by *setTimeout* or those that were run by *setRepeated* for their last time).
That's why one can chain the tasks with *setTimeout* calls from within
task function or even call one task using *setTimeout* recursively and the slots don't run out.

Also, please be aware that the scheduled tasks cannot be stopped. This is
intentional, well balanced design decision for this simple Tasker library.
You can always work around this limitation by adding the following condition
as the first instruction in your task function:

```cpp
	void a_task(int) {
		if (should_be_stopped_flag) return;
		...
	}
```

I consider this library finished and stable for everyday use. Adding more features
is not expected, the library will stay short, simple and fast.

Enjoy

Petr Stehlík

http://joysfera.blogspot.com/ with link to my G+ profile and contact information.
