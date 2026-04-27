# Embedded Assignments Run Guide

This guide explains how to run each assignment in Keil step by step, what to open, what to watch, where to place breakpoints, and what result you should expect.

## Important Note
- Assignment 03, Assignment 05, and Assignment 07 were already runtime-checked in Keil and their steps below are verified.
- Assignment 06, Assignment 04, and Assignment 02 are based on the current code in the workspace and are prepared as the next debug flow to follow.

## Recommended Order
1. Assignment 03
2. Assignment 05
3. Assignment 07
4. Assignment 06
5. Assignment 04
6. Assignment 02

## Standard Debug Windows
For all assignments, keep these windows open unless noted otherwise:
- source window showing `main.c`
- `Watch 1`
- `Call Stack + Locals`
- `Registers`

`Registers` is useful, but the most important windows are `main.c` and `Watch 1`.

---

## Assignment 03
Project file:
- `Hassabou-Assignments/Assignment-03/Assignment-03.uvprojx`

### Build
1. Open the project in Keil.
2. Click `Rebuild`.
3. Expected result: `0 Error(s), 0 Warning(s)`.

### Watch Variables
Add these to `Watch 1`:
- `g_interruptCount`
- `g_taskWakeCount`
- `g_lastNotificationTick`
- `g_actionTaskHandle`

### Breakpoints
Set these two breakpoints in `main.c`:
- the line `g_interruptCount++;` inside `GPIOF_Handler()`
- the line `g_taskWakeCount += pendingNotifications;` inside `vActionTask()`

### Run Steps
1. Start a debug session.
2. Press `Run`.
3. Use the LaunchPad / Port F simulator window.
4. Press `SW1` once.
5. The first stop should be in `GPIOF_Handler()` at `g_interruptCount++;`.
6. Press `F10` once.
7. Expected: `g_interruptCount` becomes `1`.
8. Press `F5`.
9. The next stop should be in `vActionTask()` at `g_taskWakeCount += pendingNotifications;`.
10. Press `F10` once.
11. Expected: `g_taskWakeCount` becomes `1` and `g_lastNotificationTick` changes from `0`.

### What Success Looks Like
- interrupt fires first
- ISR runs
- task wakes after the ISR
- both counters increment in order

### Screenshot Set
1. ISR stop before the interrupt counter update.
2. Task stop before the task counter update.
3. Updated watch values after stepping on the task counter line.

---

## Assignment 05
Project file:
- `Hassabou-Assignments/Assignment-05/Assignment-05.uvprojx`

### Build
1. Open the project in Keil.
2. Click `Rebuild`.
3. Expected result: `0 Error(s), 0 Warning(s)`.

### Watch Variables
Add these to `Watch 1`:
- `gInterruptsFired`
- `gSemaphoreAccepted`
- `gDroppedInterrupts`
- `gTasksProcessed`
- `gTriggerBursts`
- `gHandlerBusy`

### Breakpoints
Set these two breakpoints in `main.c`:
- the line `gInterruptsFired++;` inside `GPIOF_Handler()`
- the line `gTasksProcessed++;` inside `vHandlerTask()`

### Run Steps
1. Start a debug session.
2. Press `Run`.
3. Do not click anything in the simulator. This project triggers interrupts automatically in software.
4. The first stop should be in `GPIOF_Handler()` at `gInterruptsFired++;`.
5. Press `F10` once.
6. Expected: `gInterruptsFired` becomes `1`.
7. Press `F5`.
8. The next stop should be in `vHandlerTask()` at `gTasksProcessed++;`.
9. Press `F10` once.
10. Expected: `gTasksProcessed` becomes `1` and `gHandlerBusy` stays `1`.
11. Press `F5` again.
12. The next stop should return to the ISR while the handler is still busy.
13. Step through the ISR until after the `xSemaphoreGiveFromISR()` decision.
14. Keep watching `gSemaphoreAccepted` and `gDroppedInterrupts`.
15. Final expected proof: `gDroppedInterrupts` becomes `1` or greater.

### What Success Looks Like
- multiple interrupts fire while the handler is still busy
- at least one interrupt is dropped
- final evidence looks like this pattern:
  - `gInterruptsFired = 3`
  - `gSemaphoreAccepted = 2`
  - `gDroppedInterrupts = 1`
  - `gTasksProcessed = 1`
  - `gHandlerBusy = 1`

### Screenshot Set
1. first ISR stop
2. first handler-task stop
3. final ISR stop where `gDroppedInterrupts > 0`

---

## Assignment 07
Project file:
- `Hassabou-Assignments/Assignment-07/Assignment-07.uvprojx`

### Build
1. Open the project in Keil.
2. Click `Rebuild`.
3. Expected result: `0 Error(s), 0 Warning(s)`.

### Watch Variables
Add these to `Watch 1`:
- `gInterruptsFired`
- `gTokensQueued`
- `gTokensProcessed`
- `gBurstCounter`
- `gHandlerActive`

For the required Logic Analyzer evidence, also add this signal to the Logic Analyzer window:
- `gTokensProcessed`

### Breakpoints
Set these two breakpoints in `main.c`:
- the line `gInterruptsFired++;` inside `GPIOF_Handler()`
- the line `gTokensProcessed++;` inside `vHandlerTask()`

### Run Steps
1. Start a debug session.
2. Press `Run`.
3. Do not click anything manually. The trigger task fires the interrupts in software.
4. First expected stop: ISR at `gInterruptsFired++;`.
5. Press `F10` once.
6. Expected: `gInterruptsFired = 1` and `gTokensQueued = 1`.
7. Press `F5`.
8. Next expected stop: handler task at `gTokensProcessed++;`.
9. Press `F10` once.
10. Expected: `gTokensProcessed = 1` and `gHandlerActive = 1`.
11. Press `F5` again.
12. The ISR should be hit again while the handler is still active.
13. Continue until the three interrupts in the burst have happened.
14. Expected final behavior:
- `gInterruptsFired = 3`
- `gTokensQueued = 3`
- `gTokensProcessed` increases one by one as the handler drains the queue
- `gHandlerActive = 1` during processing
15. Open `View` -> `Analysis Windows` -> `Logic Analyzer`.
16. Click `Setup...` in the Logic Analyzer window.
17. Add the signal `gTokensProcessed`.
18. Click `OK`, then click `Clear` in the Logic Analyzer window.
19. Press `Run` again and let the burst complete.
20. Expected Logic Analyzer result: `gTokensProcessed` rises in a staircase pattern from `0` to `1`, then `2`, then `3`.
21. If you leave the program running for a second burst, the same staircase repeats and the count continues to rise.

### What Success Looks Like
- three distinct interrupts occur
- no events are dropped
- the counting semaphore stores all of them
- `gTokensProcessed` eventually catches up to the queued count
- the Logic Analyzer shows `gTokensProcessed` stepping upward one token at a time rather than one single jump

### Screenshot Set
1. first ISR stop with queue counters at the start of the burst
2. handler task processing the first token
3. later state showing queued tokens still preserved while the handler is busy
4. Logic Analyzer screenshot when `gTokensProcessed = 1`
5. Logic Analyzer screenshot when `gTokensProcessed = 2`
6. Logic Analyzer screenshot when `gTokensProcessed = 3`

---

## Assignment 06
Project file:
- `Hassabou-Assignments/Assignment-06/Assignment-06.uvprojx`

### Build
1. Open the project in Keil.
2. Click `Rebuild`.
3. Expected result: `0 Error(s), 0 Warning(s)`.

### Watch Variables
Add these to `Watch 1`:
- `gActiveResources`
- `gMaxObservedResources`
- `gAcquireCount[0]`
- `gAcquireCount[1]`
- `gAcquireCount[2]`
- `gAcquireCount[3]`
- `gAcquireCount[4]`
- `gReleaseCount[0]`
- `gReleaseCount[1]`
- `gReleaseCount[2]`
- `gReleaseCount[3]`
- `gReleaseCount[4]`
- `gTaskUsingResource[0]`
- `gTaskUsingResource[1]`
- `gTaskUsingResource[2]`
- `gTaskUsingResource[3]`
- `gTaskUsingResource[4]`

### Breakpoints
Set these two breakpoints in `main.c`:
- the line `gActiveResources++;`
- the line `gActiveResources--;`

### Run Steps
1. Start a debug session.
2. Press `Run`.
3. The worker tasks should begin competing automatically.
4. The debugger should stop when a worker acquires a resource at `gActiveResources++;`.
5. Press `F10` once.
6. Expected: `gActiveResources` increases, and one of the `gTaskUsingResource[]` entries becomes `1`.
7. Press `F5`.
8. Stop again when another worker acquires a resource.
9. Expected: `gActiveResources` can become `2`, but it should never exceed `2`.
10. Continue running and observe releases at `gActiveResources--;`.
11. Expected: tasks outside the resource pool remain waiting until one token is released.
12. Keep watching `gMaxObservedResources`.
13. Final expected proof: `gMaxObservedResources` never becomes greater than `2`.

### What Success Looks Like
- at most two tasks are active inside the resource section at any time
- `gTaskUsingResource[]` shows which two are active
- `gAcquireCount[]` and `gReleaseCount[]` keep increasing over time
- `gMaxObservedResources = 2`

### Screenshot Set
1. state where one worker holds a resource
2. state where two workers hold resources at the same time
3. watch evidence that no third worker enters the resource section
4. later state showing resources released and reassigned

---

## Assignment 04
Project file:
- `Hassabou-Assignments/Assignment-04/Assignment-03.uvprojx`

Important note:
- the folder is `Assignment-04`, but the copied Keil project file still has the old name `Assignment-03.uvprojx`

### Build
1. Open the project file from the `Assignment-04` folder.
2. Click `Rebuild`.
3. Expected result: `0 Error(s), 0 Warning(s)`.

### Watch Variables
Add these to `Watch 1`:
- `gInterruptCount`
- `gPendSvRequests`
- `gHighTaskRuns`
- `gLowTaskIterations`
- `gLastIsrTick`
- `gLastHighTaskTick`
- `gObservedLatencyTicks`

### Breakpoints
Set these two breakpoints in `main.c`:
- the line `gInterruptCount++;` inside `GPIOF_Handler()`
- the line `gHighTaskRuns++;` inside `vHighPriorityTask()`

### Run Steps
1. Start a debug session.
2. Press `Run`.
3. Do not click anything manually. The trigger task generates interrupts in software.
4. First expected stop: ISR at `gInterruptCount++;`.
5. Press `F10` once.
6. Expected: `gInterruptCount` increases.
7. Press `F5`.
8. Next expected stop: high-priority task at `gHighTaskRuns++;`.
9. Press `F10` once.
10. Expected: `gHighTaskRuns` increases and `gObservedLatencyTicks` is updated.
11. Keep note of `gObservedLatencyTicks` when `USE_ISR_YIELD` is `1`.
12. For the comparison case, change `#define USE_ISR_YIELD 1U` to `#define USE_ISR_YIELD 0U`, rebuild, and repeat the same steps.
13. Compare the latency values and the delay in task wake-up between the two runs.

### What Success Looks Like
With yield enabled:
- `gPendSvRequests` increases
- `gHighTaskRuns` follows quickly after the ISR
- `gObservedLatencyTicks` is small

Without yield enabled:
- high-priority task wake-up is delayed
- `gObservedLatencyTicks` is larger or visibly delayed until the next scheduling point

### Screenshot Set
1. ISR stop with yield-enabled build
2. high-priority task wake-up with yield-enabled build
3. corresponding high-priority task wake-up with yield-disabled build
4. comparison screenshot or watch values showing the latency difference

---

## Assignment 02
Project file:
- `Hassabou-Assignments/Assignment-02/Assignment-02.uvprojx`

### Build
1. Open the project in Keil.
2. Click `Rebuild`.
3. Expected result: project builds successfully.

### Debug Windows
For this assignment, in addition to the standard windows, also open:
- `Memory 1`
- disassembly only if needed

### Watch Variables
Add these to `Watch 1`:
- `g_ui32SysTickCount`
- `g_restoreAttemptCount`
- `g_currentTaskId`
- `g_nextTaskId`
- `g_contextCorruptionDetected`
- `g_expectedPc`
- `g_observedPc`
- `g_faultSignature`
- `g_currentSp`
- `g_restoredSp`
- `g_faultyRestoreSp`
- `g_lastDecodedFrame.pc`
- `g_lastDecodedFrame.xpsr`

### Breakpoints
Set these two breakpoints in `main.c`:
- the line `g_restoreAttemptCount++;` inside `AttemptContextRestore()`
- the `if` condition that checks whether the decoded frame is corrupted

### Run Steps
1. Start a debug session.
2. Press `Run`.
3. Wait until SysTick has advanced enough for a restore attempt.
4. First expected stop: `AttemptContextRestore()` at `g_restoreAttemptCount++;`.
5. Press `F10` and watch the restore variables update.
6. Continue stepping until after `g_lastDecodedFrame = DecodeFrame(candidateSp);`.
7. Compare:
- `g_expectedPc`
- `g_observedPc`
- `g_lastDecodedFrame.xpsr`
8. Because `g_useBrokenRestore = 1`, the broken restore path for Task 2 should use `g_faultyRestoreSp = g_restoredSp + 1`.
9. When the corruption check executes, expected result:
- `g_contextCorruptionDetected = 1`
- `g_observedPc` does not match `g_expectedPc`, or the xPSR thumb bit is wrong
- `g_faultSignature` becomes nonzero
10. Use `Memory 1` to inspect the stack contents around `g_restoredSp` and `g_faultyRestoreSp`.

### What Success Looks Like
- synthetic stack frames are built for both tasks
- the restore attempt for Task 2 intentionally uses the wrong stack pointer
- decoded frame values become inconsistent
- the corruption flag is raised

### Screenshot Set
1. breakpoint at the start of `AttemptContextRestore()`
2. watch values after decoding the frame
3. proof that `g_faultyRestoreSp` differs from `g_restoredSp`
4. corruption-detection state showing mismatched PC or bad xPSR

---

## Final Submission Pattern
For each assignment, use this order in the submission folder:
1. screenshots
2. report markdown or pdf
3. optional build log screenshot if you want extra proof

A clean report should always contain:
1. objective
2. short system setup
3. debug method
4. screenshot evidence
5. interpretation of the evidence
6. conclusion
