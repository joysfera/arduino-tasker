Tasker for Arduino
==================

This is a cooperative scheduler for running multiple tasks on Arduino "at once".

*Cooperative* means that the tasks you define need to behave well -
to co-operate together by running for a short time only, otherwise it will not work.
*Scheduler* means that when creating a task you schedule when it is to be
started (in what time since now) and how many times it should be run
(once, X times or forever).

The "co-operation" is best achieved by creating small, short running tasks.
Basically wherever you'd like include a <code>delay()</code> in your Arduino program
that's the place where you actually want to break the code flow, split
the source code into separate functions and let them run as separate tasks.

There are many similar libraries for the same purpose but they are either
buggy (don't handle timer overflow) or too complicated, cumbersome to use,
unnecessary object-oriented or otherwise hard to understand and follow.

This Tasker library is intentionally designed to be extremely simple
yet very powerful. It has a short, clean API and clear implementation that fits
on a single page and can be reviewed easily.
Best of all, its API is intentionally similar to JavaScript's timer
functions so HTML/JavaScript programmers will feel at home immediately.
For the unfamiliar with JavaScript there's a short example code included that
illustrates the whole API and its best usage.

How to use
----------

1. unpack Tasker into your Arduino libraries folder
2. in Arduino IDE load File -> Examples -> Tasker -> MultiBlink
3. see how easy it is to define three tasks and run them "at once"
4. use that example as a base for your own code

Tasker API
----------

* <code>Tasker([bool prioritized])</code>. The class constructor takes
  an optional bool flag (that is set to true if omitted). If this flag
  is TRUE then the Tasker prioritizes the scheduled tasks. If the flag
  is FALSE then the Tasker considers all scheduled tasks equal.

* <code>setTimeout(function_name, time_in_milliseconds [, optional_int])</code>
  Will call the <code>function_name</code> in <code>time_in_milliseconds</code> from now.
  Will run it only once. May pass the optional int parameter into the called function.

* <code>setInterval(function_name, time_in_milliseconds [, optional_int])</code>
  Will call the <code>function_name</code> repeatedly and forever, every
  <code>time_in_milliseconds</code> from now on.
  May pass the optional int parameter into the called function.

* <code>setRepeated(function_name, time_in_milliseconds, number_of_repeats [, optional_int])</code>
  Will call the <code>function_name</code> repeatedly for <code>number_of_repeats</code>,
  every <code>time_in_milliseconds</code> from now on.
  May pass the optional int parameter into the called function.

* <code>run()</code> when called it starts the Tasker scheduler and will never return.
  Best to be called as the very last command of the Arduino's <code>setup()</code> function.

* optional: if you, for whatever reason, don't want to let the <code>Tasker.run()</code>
  govern all of your running code and wish to run Tasker in parallel with some
  existing code you can omit the <code>run()</code> and call the <code>Tasker.loop()</code>
  repeatedly instead.

Task priorities
---------------
If the Tasker constructor was not called with a FALSE flag then the internal
scheduler will prioritize the tasks in its queue. Tasks defined later have lower
priority than those defined earlier. Thus you'll want to define your more
important tasks first and add the less important afterwards.

	Tasker tasker;
	tasker.setInterval(most_important_fn, ..);
	tasker.setInterval(less_important_fn, ..);
	tasker.setInterval(least_important_fn, ..);

Normally, when there is enough time for calling each of the scheduled task
at the right time the priorities don't play any role but when a previous task takes
longer time and the scheduler detects that certain tasks are delayed
(are behind their schedule) it needs to decide which task will get run of those
that should have been run already. And that's where the tasks' priorities step
in: the task defined earlier (= with a higher priority) will be chosen.
If the priorities were disabled then the scheduler would simply run the next task
in its queue. If all your tasks are equally important you might want to disable
the priorities by passing FALSE into the constructor:

	Tasker tasker(false);
	tasker.setInterval(fn, ..);
	tasker.setInterval(equally_important_fn, ..);
	tasker.setInterval(order_doesnt_matter_fn, ..);

Caveats
-------
Make sure you define enough task "slots" in the Tasker, before you include its
header file. By default there are 10 slots defined for up to ten tasks running
simultaneously but you might want to increase this number - or even decrease, to
save the precious RAM (each slot takes 14 bytes of RAM).

	#define TASKER_MAX_TASKS 32
	#include "Tasker.h"

The scheduled tasks cannot be stopped. This is intentional design decision of
this simple Tasker library. I've been weighting the possibility of stopping
tasks against the simplicity of implementation and its usage and the automatic
queue updating feature (that helps implementing chained setTimeout).
Even though it would be possible to add names/IDs to scheduled tasks
I don't want to do that - there are other libraries availabe that do just that
and are far more complicated.

Enjoy

Petr Stehlik
joysfera@gmail.com
