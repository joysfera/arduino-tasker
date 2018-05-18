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
Basically wherever you'd need to include the infamous <code>delay()</code> call in your Arduino program
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

ChangeLog
---------
* version 1.4 changes the default priority value when <code>Tasker</code> is instantiated without the optional parameter. In previous versions the priority was enabled, now it is disabled. I (and also other users of **Tasker**) found the prioritized handling of tasks rather counter-intuitive because it could happen almost randomly that some tasks were sometimes not executed at all (when a higher priority task ran for too long). Whoever wants to keep the original behaviour please instantiate Tasker like this: <code>Tasker tasker(TRUE);</code>. There are also two new functions that help to query or set the priority value: <code>isPrioritized()</code> and <code>setPrioritized(bool)</code>.

* version 1.3 removes the <code>run()</code> function - please call <code>tasker.loop()</code> in your Arduino <code>loop()</code> function instead. This makes **Tasker** much more Arduino friendly and compatible with far more platforms where the Arduino 'kernel' does some housekeeping behind the scenes and needs the <code>loop()</code> to be running for it. It also allowed me to remove the <code>yield()</code> call that didn't really bring anything but issues in compiling on some platforms.

* version 1.2 adds optional priorities when defining tasks

* version 1.1 adds clear example of DS18B20 handling


How to use
----------

1. create new **Tasker** folder under your Arduino projects' libraries folder and place Tasker files there so the header file ends in **./libraries/Tasker/Tasker.h** or **install from Arduino Library Manager**
2. in Arduino IDE load File -> Examples -> Tasker -> MultiBlink (or other examples found there)
3. see how easy it is to add three tasks and run them all at once (or read the DS18B20 without waiting)
4. use that example as a basis for your own code

Tasker API
----------

* <code>Tasker([bool prioritized])</code>. The class constructor takes
  an optional bool flag (that is set to false if omitted). If this flag
  is TRUE then the Tasker prioritizes the scheduled tasks. If the flag
  is FALSE then the Tasker considers all scheduled tasks equal. More about priorities later.

```cpp
	Tasker tasker;        // creates non-prioritizing tasker
	Tasker tasker(TRUE);  // creates prioritizing tasker
```

* <code>setTimeout(function_name, time_in_milliseconds [, optional_int [, optional_priority]])</code>
  Tasker will call the *function_name* in *time_in_milliseconds* from now.
  It will run the function only once. May pass the *optional_int* parameter into the called function.
  When the task finishes its Tasker slot is made available for new tasks (more about slots later).

* <code>setInterval(function_name, time_in_milliseconds [, optional_int [, optional_priority]])</code>
  Tasker will call the *function_name* repeatedly and forever, every
  *time_in_milliseconds* from now on.
  May pass the *optional_int* parameter into the called function.

* <code>setRepeated(function_name, time, number_of_repeats [, optional_int [, optional_priority]])</code>
  Tasker will call the *function_name* repeatedly for *number_of_repeats*,
  every *time* (in_milliseconds) from now on.
  May pass the <code>optional_int</code> parameter into the called function.
  When the task finishes (after its last iteration) its Tasker slot is made available for new tasks.

* <code>loop()</code> when called it runs the Tasker scheduler and process all waiting tasks, then ends.
  Best to be called as often as possible, ideally in the Arduino's <code>loop()</code> function:

```cpp
	void loop() {
		tasker.loop();
	}
```

Task priorities (optional)
--------------------------
If the Tasker constructor was called with a parameter (TRUE) then the internal
scheduler will prioritize the tasks in its queue. Tasks added later have lower
priority than those added earlier, unless you specify their priority with
optional parameter: the lower its value the higher priority, 0 = highest priority.

```cpp
	Tasker tasker(TRUE);
	tasker.setInterval(most_important_fn, ..); // formerly added calls have automatically higher priority
	tasker.setInterval(less_important_fn, ..); // the later added calls the lower priority they have
	tasker.setInterval(highest_priority_fn, .., .., 0); // unless you specify the priority explicitly by the last parameter
```

Normally, when there is enough time for calling each of the scheduled task
at the right time the priorities don't play any role but when a previous task takes
longer time and the scheduler detects that certain tasks are delayed
(are behind their schedule) it needs to decide which task will be run of those
that should have been run already. And that's where the tasks' priorities step
in: the task added earlier or with a higher priority will be chosen.
If the priorities were disabled (by default the are) then the scheduler would simply run the next task
in its queue. If all your tasks are equally important (they most probably are) you might simply ignore the whole idea of priorities and their implementation.

```cpp
	Tasker tasker;
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

Author
------
Petr Stehlík

E-mail: petr@pstehlik.cz

Web: https://www.pstehlik.cz/

Daily active on Google+: https://plus.google.com/+PetrStehl%C3%ADk

Longer articles are published at blog: http://joysfera.blogspot.com/

Sometimes tweets as https://twitter.com/joysfera
