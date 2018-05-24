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

The "co-operation" is best achieved by creating small, short running tasks (functions).
Basically wherever you'd need to include the infamous `delay()` call in your Arduino program
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
* version 2.0 brings two new great features: now you can **modify scheduled tasks** and also **cancel them**.
  - to modify task setup (change their timing or priority) simply call the `setTimeout()`/`setInterval()`/`setRepeated()` functions again.
  - to stop/cancel a scheduled task and remove from Tasker's queue call the new function `cancel()`.
  - if familiar with Javascript you can call `clearTimeout()` and `clearInterval()` (identical with `cancel()`).
  - to find out when a given task will be called use the new `scheduledIn()` function.
  - another important change is making the optional `int` parameter passed into your functions truly optional, so if you don't want to use it you don't need to declare your function with it. I.e. the `void myFunction(int /*unused*/)` is a history now - use simple and clean `void myFunction()`.
  - Please read the *Upgrading from v1.2 to v2.0* paragraph below for further details.

* version 1.4 changes the default priority value when **Tasker** is instantiated without the optional parameter. In previous versions the priority was enabled by default, now it is disabled. Users of **Tasker** found the prioritized handling of tasks rather counter-intuitive because it could happen almost randomly that some tasks were sometimes not executed at all (when a higher priority task ran for too long). Whoever wants to keep the original behaviour please instantiate **Tasker** like this: `Tasker tasker(TRUE);`. There are also two new functions that help to query or set the priority value: `isPrioritized()` and `setPrioritized(bool)`.

* version 1.3 removes the `run()` function - please call `tasker.loop()` in your Arduino `loop()` function instead. This makes **Tasker** much more Arduino friendly and compatible with far more platforms where the Arduino 'kernel' does some housekeeping behind the scenes and needs the `loop()` to be running for it. It also allowed me to remove the `yield()` call that didn't really bring anything but issues on some platforms.

* version 1.2 adds optional priorities when defining tasks

* version 1.1 adds clear example of DS18B20 handling


Upgrading from v1.2 to v2.0
---------------------------
Versions 1.3-2.0 released in May 2018 introduced some small API changes that were not backward compatible so you may need to update your source code (see below for details). Changing library API is always better avoided but the collected user feedback in last year led me to simplify the API and made it more intuitive to use, which is so good thing that it was worth changing the API a bit. These are the things you might need to update in your application when using Tasker:

### default value of Tasker constructor has changed
If you rely on the prioritized task execution (most users don't!) then enable it explicitly by adding (TRUE) as the ctor parameter because it's no longer enabled by default:

  | old code             | new code              |
  |----------------------|-----------------------|
  | `Tasker tasker;`     | `Tasker tasker(TRUE);`|
 
  Let me repeat that the prioritized task execution may cause that some tasks with lower priority are sometimes not executed if the tasks with higher priority spent too much time. This might lead to some head scratching when you're missing some function calls randomly. So most users will be happier with the default constructor without any parameter: `Tasker tasker;`

### change run() to loop()
If you were using the `tasker.run()` function please change it for calling `tasker.loop()` in your Arduino loop():

  old code:
  ```cpp
  // originally Tasker suggested to call run() as the last thing in setup()
  void setup() {
      ...
      tasker.run();
  }
  // Arduino loop() was then unused
  void loop() { }
  ```

  new code:
  ```cpp
  // now Tasker needs to have tasker.loop() called in Arduino loop()
  void setup() {
      ...
  }
  void loop() {
      tasker.loop();
      // you can add your code here, too
  }
  ```

### remove unused parameter from task declaration
If you don't use the additional parameter in your task/function then simply remove it:

| old code                             | new code                     |
|--------------------------------------|------------------------------|
| `void myFunction(int /* unused */) {`| `void myFunction() {`        |

### optional int parameter must be nonnegative
Functions/tasks can be called with an optional `int` parameter. Since v2.0 its value (specified in ``setTimeout()`` etc) must be nonnegative, i.e. 0 or greater.

How to use
----------

1. install **Tasker** from **Arduino Library Manager** or create new *Tasker* folder under your Arduino projects' libraries folder and place Tasker files there so the header file ends in *./libraries/Tasker/Tasker.h*
2. in Arduino IDE load File -> Examples -> Tasker -> MultiBlink (or other examples found there)
3. see how easy it is to add three tasks and run them all at once (or how to read the DS18B20 without waiting)
4. use that example as a basis for your own code

Tasker API
----------

* `Tasker([bool prioritized])`. The class constructor takes
  an optional bool flag (that is set to false if omitted). If this flag
  is TRUE then the Tasker prioritizes the scheduled tasks. If the flag
  is FALSE then the Tasker considers all scheduled tasks equal. More about priorities later.

```cpp
  Tasker tasker;        // creates non-prioritizing tasker
  Tasker tasker(TRUE);  // creates prioritizing tasker
```

* `setTimeout(function, time_in_milliseconds [, optional_int [, optional_priority]])`
  Tasker will call the *function* in *time_in_milliseconds* from now.
  It will run the function only once. May pass the *optional_int* parameter (nonnegative) into the called function.
  When the task finishes its Tasker slot is made available for new tasks (more about slots later).

* `setInterval(function, time_in_milliseconds [, optional_int [, optional_priority]])`
  Tasker will call the *function* repeatedly and forever, every
  *time_in_milliseconds* from now on.
  May pass the *optional_int* parameter (nonnegative) into the called function.

* `setRepeated(function, time, number_of_repeats [, optional_int [, optional_priority]])`
  Tasker will call the *function* repeatedly for *number_of_repeats*,
  every *time* (in_milliseconds) from now on.
  May pass the *optional_int* parameter (nonnegative) into the called function.
  When the task finishes (after its last iteration) its Tasker slot is made available for new tasks.

* `cancel(function [, optional_int ])`
  If Tasker has the *function* in its scheduler queue (added there by either of those three functions above)
  it will cancel any further execution of the function and will remove it from its scheduler queue instantly.
  Its Tasker slot is made available for new tasks, of course.
  If you happened to add certain *function* to Tasker several times with different optional int parameters
  then you need to use the same optional int parameter when calling the `cancel()` so that Tasker
  knows which of the several task slots with the same *function* to remove.

* `clearTimeout(function [, optional_int ])` is identical to `cancel()`, it just
  uses the well known Javascript API.

* `clearInterval(function [, optional_int ])` is identical to `cancel()`, it just
  uses the well known Javascript API.
  
* `scheduledIn(function [, optional_int ])` returns number of milliseconds till calling the given *function*. Returned 0 means that *function* (with *optional_int*) is not in Tasker's queue so it will never be called.

* `loop()` when called it runs the Tasker scheduler and process all waiting tasks, then ends.
  It's best to let your program call this Tasker function as often as possible, ideally in the Arduino's `loop()` function:

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
