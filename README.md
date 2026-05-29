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
* version 3.0 is a major modernization (full backward compatibility preserved via `using Tasker = TaskerT<...>` alias):
  - New `TaskerT<N>` template lets you set max tasks per instance at compile time (`TaskerT<64> tasker;`), while bare `Tasker tasker;` keeps working and honors `#define TASKER_MAX_TASKS`.
  - Added `setImmediate()` (all 3 callback variants) for one-shot "run as soon as possible" tasks.
  - Full `void*` parameter support across the entire API (new `TaskCallbackPtr` type) in addition to the classic no-param and `int` variants.
  - New capacity helpers: `isFull()`, `count()`, `available()`, plus `static constexpr size_t capacity`.
  - `getUserData()` / `setUserData()` for attaching context (especially useful with `void*` callbacks).
  - `int` parameter values can now be negative (v2 artificially clamped them >=0); `void*` supports any pointer value.
  - Better platform support: on 32-bit boards (ESP32, STM32, Teensy, RP2040...) you get up to 65535 tasks, ~4 billion repeats (unsigned int), full 32/64-bit pointers and larger `int`; on 8-bit AVR (Uno etc.) max 255 tasks, 16-bit repeat count, packed 15-byte tasks for RAM savings.
  - Internal cleanups (unified findTask/scheduledIn logic, no more magic sentinels) while keeping identical behavior for old code.

* version 2.0 brings two new great features: now you can **modify scheduled tasks** and also **cancel them**.
  - to modify task setup (change their timing or priority) simply call the `setTimeout()`/`setInterval()`/`setRepeated()` functions again.
  - to stop/cancel a scheduled task and remove from Tasker's queue call the new function `cancel()`.
  - if familiar with JavaScript you can call `clearTimeout()` and `clearInterval()` (identical with `cancel()`).
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
(Note: in v3 this restriction was lifted — negative values are now fully supported for `int` params, and `void*` accepts any pointer.)

Upgrading from v2 to v3
-----------------------
Version 3 (2026) is a major internal modernization but **maintains 100% backward compatibility**. Old code using `Tasker tasker;` compiles and behaves identically with zero changes.

### Template for max tasks (recommended for new code)
The classic global `#define` still works and controls the default size:
```cpp
#define TASKER_MAX_TASKS 20
#include <Tasker.h>
Tasker tasker;           // uses the #define (or 10 if omitted)
```
For per-instance sizing (new in v3) use the template instead:
```cpp
#include <Tasker.h>
TaskerT<64> bigTasker;   // 64 slots, independent of any #define
TaskerT<8>  tiny;        // good for AVR with tight RAM
Tasker tasker;           // still works, respects TASKER_MAX_TASKS
```
`Tasker` is simply `using Tasker = TaskerT<TASKER_MAX_TASKS>;`

### Major improvements in v3
The main goals were to modernize the internals while preserving 100% backward compatibility:

- **Cleaner callback types**: `TaskCallback`, `TaskCallbackInt`, `TaskCallbackPtr` (old `TaskCallback0`/`1` aliases still work).
- **Removed fragile hacks**: No more magic `NO_PARAMETER` sentinel or forcing `int` parameters to be nonnegative. Parameters are now cleanly stored as `void*` with explicit flags.
- **Better safety**: `addTask()` now safely returns `false` when full. `findTask()` negative results are properly handled.
- **setImmediate()**: New one-shot "run as soon as possible" tasks. Internally uses `interval=0` + `IS_IMMEDIATE` flag. `scheduledIn()` returns 1 for them. (No `int` convenience overload for `void*` handlers — use `TaskCallbackInt` if you need an integer.)
- **Internal cleanup**: Uses `tasker_count_t` (uint8_t on AVR, uint16_t elsewhere) instead of `byte`. No more magic sentinels.

### New in v3 (see full list in the ChangeLog above)
- `setImmediate(...)` for one-shot immediate tasks (3 variants)
- Full `void*` callback support + `setUserData()`/`getUserData()`
- `isFull()`, `count()`, `available()`, `capacity`
- Negative `int` parameter values now allowed
- `TaskerT<N>` for per-instance sizing (see template section above)

All changes are fully backward compatible.

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
  Tasker tasker;        // creates non-prioritizing tasker (uses default size)
  Tasker tasker(TRUE);  // creates prioritizing tasker
  TaskerT<20> small;    // v3: explicit size (recommended for new code)
```

* `setTimeout(function, time_in_milliseconds [, optional_int_or_voidptr [, optional_priority]])`
  Tasker will call the *function* in *time_in_milliseconds* from now.
  It will run the function only once. May pass an optional int or void* parameter into the called function.
  When the task finishes its Tasker slot is made available for new tasks (more about slots later).

* `setInterval(function, time_in_milliseconds [, optional_int_or_voidptr [, optional_priority]])`
  Tasker will call the *function* repeatedly and forever, every
  *time_in_milliseconds* from now on.
  May pass an optional int or void* parameter into the called function.

* `setRepeated(function, time, number_of_repeats [, optional_int_or_voidptr [, optional_priority]])`
  Tasker will call the *function* repeatedly for *number_of_repeats*,
  every *time* (in_milliseconds) from now on.
  May pass an optional int or void* parameter into the called function.
  When the task finishes (after its last iteration) its Tasker slot is made available for new tasks.

* `setImmediate(function [, optional_int_or_voidptr [, optional_priority]])` (new in v3)
  Schedules a one-shot task that will be called as soon as possible (higher priority than normal timers). Three variants exist for no-param, int, and void* callbacks. Always one-shot; `scheduledIn()` returns 1 for them.

* `cancel(function [, optional_int_or_voidptr ])`
  If Tasker has the *function* in its scheduler queue (added there by either of those three functions above)
  it will cancel any further execution of the function and will remove it from its scheduler queue instantly.
  Its Tasker slot is made available for new tasks, of course.
  If you added the same *function* multiple times with different parameters you must supply the exact same parameter value when cancelling.

* `clearTimeout(function [, optional_int_or_voidptr ])` is identical to `cancel()`, it just
  uses the well known JavaScript API.

* `clearInterval(function [, optional_int_or_voidptr ])` is identical to `cancel()`, it just
  uses the well known JavaScript API.
   
* `scheduledIn(function [, optional_int_or_voidptr ])` returns number of milliseconds till calling the given *function*. Returned 0 means that *function* (with parameter) is not in Tasker's queue so it will never be called. (Returns 1 for immediate tasks.)

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
If the priorities were disabled (by default they are) then the scheduler would simply run the next task
in its queue. If all your tasks are equally important (they most probably are) you might simply ignore the whole idea of priorities and their implementation.

```cpp
Tasker tasker;
tasker.setInterval(fn, ..);
tasker.setInterval(equally_important_fn, ..);
tasker.setInterval(order_doesnt_matter_fn, ..);
```

Caveats
-------
**Configuring the maximum number of tasks (slots):**

The classic way still works:
```cpp
#define TASKER_MAX_TASKS 32
#include <Tasker.h>
Tasker tasker;   // gets 32 slots
```
For new code the recommended way is the template (gives per-instance sizing and works even without any #define):
```cpp
#include <Tasker.h>
TaskerT<32> tasker;     // 32 slots for this instance
TaskerT<200> big;       // up to 65535 on 32-bit boards, 255 on AVR
Tasker plain;           // uses TASKER_MAX_TASKS or default 10
```

Memory usage:
- 15 bytes/task on AVR (packed)
- ~24 bytes/task on 32-bit (larger repeat counts + pointers)

Tasker automatically frees slots of finished tasks, so recursive/chained `setTimeout()` is safe.

I consider this library finished and stable for everyday use. Adding more features
is not expected, the library will stay short, simple and fast.

Author
------
Petr Stehlík

E-mail: petr@pstehlik.cz

Web: https://www.pstehlik.cz/

Active on X as https://x.com/joysfera

Longer articles are published at blog: http://joysfera.blogspot.com/
