# Assignment 04 Report

## Course
CSE323: Advanced Embedded Systems Design  
CSE411/CSE411s: Real-Time and Embedded Systems Design

## Assignment
Assignment 04: Forced Context Switching with `portYIELD_FROM_ISR()`

## Objective
The objective of this assignment is to demonstrate the effect of forced context switching from inside an interrupt service routine in FreeRTOS. The required comparison is between:
- a version that calls `portYIELD_FROM_ISR()`
- a version that does not call it

The expected difference is that when the macro is used, the awakened high-priority task should run immediately after the ISR. Without the macro, the task should be delayed until the next scheduling opportunity.

## System Design
The implemented system contains:

1. `vTriggerTask()`
This task periodically triggers the interrupt in software using `NVIC_SW_TRIG_R = 30`.

2. `GPIOF_Handler()`
This ISR:
- clears the interrupt flag
- increments `gInterruptCount`
- records the current tick in `gLastIsrTick`
- sends a direct notification to the high-priority task
- increments `gPendSvRequests` if a higher-priority task was woken
- optionally calls `portYIELD_FROM_ISR()` depending on the value of `USE_ISR_YIELD`

3. `vHighPriorityTask()`
This task waits for the notification and then:
- increments `gHighTaskRuns`
- records `gLastHighTaskTick`
- computes `gObservedLatencyTicks = gLastHighTaskTick - gLastIsrTick`

4. `vLowPriorityTask()`
This task runs in the background and increments `gLowTaskIterations`, proving that the scheduler is active.

## Variables Used for Verification
The following watch variables were used:
- `gInterruptCount`
- `gPendSvRequests`
- `gHighTaskRuns`
- `gLowTaskIterations`
- `gLastIsrTick`
- `gLastHighTaskTick`
- `gObservedLatencyTicks`

## Comparison Strategy
Two runs were performed:

### Case A: With `portYIELD_FROM_ISR()`
The macro was enabled using:
`#define USE_ISR_YIELD 1U`

Expected behavior:
- the high-priority task runs immediately after the ISR
- `gObservedLatencyTicks = 0`

### Case B: Without `portYIELD_FROM_ISR()`
The macro was disabled using:
`#define USE_ISR_YIELD 0U`

Expected behavior:
- the high-priority task wakes later
- `gObservedLatencyTicks` becomes greater than zero

## Screenshot Verification and Analysis
The screenshot set now provides a valid comparison between the two cases.

### Screenshot 1: ISR breakpoint with scheduler running
File: `Screenshot 2026-04-27 180448.png`

This screenshot shows the system stopped in `GPIOF_Handler()` at the ISR breakpoint. `gLowTaskIterations` is already increasing, which proves the scheduler is active and tasks are running.

![ISR breakpoint with scheduler running](Screenshot%202026-04-27%20180448.png)

### Screenshot 2: ISR entry during the enabled-yield run
File: `Screenshot 2026-04-27 180457.png`

This screenshot shows the ISR entry with the original enabled-yield behavior. It is a valid setup screenshot for the `USE_ISR_YIELD = 1U` case and shows the interrupt path is being hit correctly.

![ISR entry during enabled-yield run](Screenshot%202026-04-27%20180457.png)

### Screenshot 3: Delayed task execution with `USE_ISR_YIELD = 0U`
File: `Screenshot 2026-04-27 184036.png`

This screenshot shows the delayed case after disabling `portYIELD_FROM_ISR()`. The watch values show:
- `gInterruptCount = 1`
- `gHighTaskRuns = 1`
- `gLastIsrTick = 0x000003E8`
- `gLastHighTaskTick = 0x000003E9`
- `gObservedLatencyTicks = 1`

This is the key proof that the high-priority task did not run immediately after the ISR. It ran one tick later.

![Delayed task execution with USE_ISR_YIELD = 0U](Screenshot%202026-04-27%20184036.png)

### Screenshot 4: Repeated delayed behavior with `USE_ISR_YIELD = 0U`
File: `Screenshot 2026-04-27 184045.png`

This screenshot confirms the same delayed behavior on the next interrupt cycle. The watch values show:
- `gInterruptCount = 2`
- `gHighTaskRuns = 2`
- `gLastIsrTick = 0x000008C0`
- `gLastHighTaskTick = 0x000008C1`
- `gObservedLatencyTicks = 1`

This proves the delay is not accidental. It repeats consistently when `portYIELD_FROM_ISR()` is disabled.

![Repeated delayed behavior with USE_ISR_YIELD = 0U](Screenshot%202026-04-27%20184045.png)

### Screenshot 5: Immediate task execution with `USE_ISR_YIELD = 1U`
File: `Screenshot 2026-04-27 184419.png`

This screenshot shows the enabled-yield case again after returning `USE_ISR_YIELD` to `1U`. The watch values show:
- `gInterruptCount = 1`
- `gHighTaskRuns = 1`
- `gLastIsrTick = 0x000003E8`
- `gLastHighTaskTick = 0x000003E8`
- `gObservedLatencyTicks = 0`

This is the direct contrast to the `0U` case. Here the high-priority task ran immediately in the same tick as the ISR.

![Immediate task execution with USE_ISR_YIELD = 1U](Screenshot%202026-04-27%20184419.png)

## Experimental Result Summary
The comparison between the two cases is clear:

### With `portYIELD_FROM_ISR()` enabled
- `gObservedLatencyTicks = 0`
- the high-priority task runs in the same tick as the ISR
- task wake-up is immediate

### With `portYIELD_FROM_ISR()` disabled
- `gObservedLatencyTicks = 1`
- the high-priority task runs one tick later
- task wake-up is delayed

## Interpretation
The experiment confirms the purpose of `portYIELD_FROM_ISR()`. When an ISR wakes a higher-priority task, the macro forces the scheduler to switch context immediately instead of waiting for the next normal scheduling point. Without the macro, the task is still awakened logically, but execution is delayed until later. This is why the no-yield case shows a one-tick latency while the yield-enabled case shows zero latency.

## Conclusion
The assignment objective was achieved successfully. The implemented FreeRTOS application demonstrates the practical effect of `portYIELD_FROM_ISR()` on task wake-up timing. The debugger evidence proves that enabling the macro causes immediate context switching, while disabling it delays execution of the higher-priority task. This confirms the macro’s role in forced context switching from inside an ISR.
