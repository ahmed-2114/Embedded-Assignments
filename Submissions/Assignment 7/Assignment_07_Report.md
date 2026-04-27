# Assignment 07 Report: Counting Semaphore Event Queuing Under Interrupt Overload

## Course Designations
- CSE323: Advanced Embedded Systems Design
- CSE411/CSE411s: Real-Time and Embedded Systems Design

## 1.0 Executive Objective

The primary objective of this report is to empirically demonstrate the architectural advantages of counting semaphores within a FreeRTOS environment, specifically as a solution to high-frequency interrupt bursts. Unlike a binary semaphore, which is strictly a dual-state mechanism and susceptible to event loss, a counting semaphore possesses an internal maximum count. This assignment proves that by utilizing a counting semaphore, multiple distinct asynchronous hardware events can be successfully queued and preserved while the deferred processing task is occupied.

## 2.0 System Architecture & Execution Flow

To reliably demonstrate the queuing capabilities of the counting semaphore, the system architecture mirrors the overload scenario from Assignment 05, substituting the synchronization primitive.

### 2.1 Task & ISR Hierarchy

`vTriggerTask()` (The Stimulus): Engineered to generate rapid, automated bursts of three software-triggered interrupts using `NVIC_SW_TRIG_R`. This guarantees a deterministic and repeatable overload condition.

`GPIOF_Handler()` (The Interceptor): Represents the asynchronous interrupt source. Upon execution, it:

- clears the active interrupt flag
- increments the `gInterruptsFired` and `gTokensQueued` diagnostic counters
- gives a token to the counting semaphore via `xSemaphoreGiveFromISR()`

`vHandlerTask()` (The Deferred Processor): Acts as the primary computational task. Upon acquiring a semaphore token via `xSemaphoreTake()`, it:

- asserts a `gHandlerActive = 1` flag
- increments `gTokensProcessed`
- intentionally stalls execution using a simulated computational delay to guarantee the task remains busy while the subsequent interrupts from the burst arrive

## 3.0 Screenshot Verification & Sequential Analysis

To capture undeniable proof of the queued interrupts, the Keil IDE debugging environment was utilized. Breakpoints were established at `gInterruptsFired++;` in the ISR and `gTokensProcessed++;` in the handler task. Furthermore, the Keil Logic Analyzer was deployed to visually track the execution sequence over time. All 9 captured states were verified against the expected RTOS execution model.

### Screenshot 1: Baseline System State

This capture illustrates the initial debug state prior to the execution of the primary interrupt burst. It is a valid setup image showing the workspace before the key event sequence begins.

![Baseline System State](Screenshot%202026-04-27%20164120.png)

*Caption: Initial debug setup before the primary interrupt burst is processed.*

### Screenshot 2: Interrupt Overload Registration Begins

Execution halts within `GPIOF_Handler()` as the software triggers the first interrupt of the burst. The system is stopped at the ISR counter line before the increment completes, which is the correct first-breakpoint behavior.

At this point:

- `gInterruptsFired = 0`
- `gTokensQueued = 0`
- `gBurstCounter = 1`

This matches the expected pre-increment ISR state.

![Interrupt Overload Registration Begins](Screenshot%202026-04-27%20164143.png)

*Caption: First ISR hit before the interrupt and queue counters are updated.*

### Screenshot 3: Initial Semaphore Assignment

Following the first ISR step, the interrupt and queue counters update as expected:

- `gInterruptsFired = 1`
- `gTokensQueued = 1`

This proves the first interrupt was successfully registered and one token was placed into the counting semaphore.

![Initial Semaphore Assignment](Screenshot%202026-04-27%20164210.png)

*Caption: First interrupt registered and first semaphore token queued successfully.*

### Screenshot 4: Deferred Processing Activation

The scheduler awakens `vHandlerTask()`. At the handler breakpoint before `gTokensProcessed++;`, the system state shows:

- `gInterruptsFired = 3`
- `gTokensQueued = 3`
- `gTokensProcessed = 0`
- `gHandlerActive = 1`

This is a critical proof point because all three interrupts from the burst have already been preserved before the handler finishes its first processing cycle.

![Deferred Processing Activation](Screenshot%202026-04-27%20164439.png)

*Caption: Handler task awakened while all three interrupt-generated tokens are already queued.*

### Screenshot 5: Initial Token Processed

Stepping forward inside the handler task increments the processing counter:

- `gTokensProcessed = 1`
- `gInterruptsFired = 3`
- `gTokensQueued = 3`

This confirms the handler has begun draining the queued tokens one at a time while the interrupt backlog remains preserved.

![Initial Token Processed](Screenshot%202026-04-27%20164444.png)

*Caption: First queued token processed by the handler task while the remaining tokens stay buffered.*

### Screenshot 6: Third Interrupt Queued During Delay

This later queued-state screenshot remains consistent with the expected behavior. The queue count and processing count are distinct, showing that preserved interrupt events remain available while the handler continues its delayed processing path.

![Third Interrupt Queued During Delay](Screenshot%202026-04-27%20164449.png)

*Caption: Queued events remain preserved while the handler is still active.*

### Screenshot 7: Logic Analyzer Staircase, Step 1

The Keil Logic Analyzer shows `gTokensProcessed` rising from `0` to `1`. The watch values at the same time show:

- `gInterruptsFired = 3`
- `gTokensQueued = 3`
- `gTokensProcessed = 1`
- `gHandlerActive = 1`

This is the correct beginning of the staircase effect.

![Logic Analyzer Step 1](Screenshot%202026-04-27%20165741.png)

*Caption: Logic Analyzer shows the first processed token as the staircase begins.*

### Screenshot 8: Logic Analyzer Staircase, Step 2

The Logic Analyzer now shows the second step. The watch values show:

- `gInterruptsFired = 3`
- `gTokensQueued = 3`
- `gTokensProcessed = 2`
- `gHandlerActive = 1`

This confirms the second queued token was preserved and processed later rather than being lost.

![Logic Analyzer Step 2](Screenshot%202026-04-27%20165936.png)

*Caption: Logic Analyzer confirms the second queued token is processed in sequence.*

### Screenshot 9: Logic Analyzer Staircase, Step 3

The Logic Analyzer shows the third step upward. The watch values show:

- `gInterruptsFired = 6`
- `gTokensQueued = 6`
- `gTokensProcessed = 3`
- `gBurstCounter = 2`
- `gHandlerActive = 1`

This screenshot is still valid even though the second burst has already begun. The important evidence is that `gTokensProcessed` continues to rise one level at a time, producing the staircase pattern required by the assignment.

![Logic Analyzer Step 3](Screenshot%202026-04-27%20165951.png)

*Caption: Logic Analyzer shows the third sequential processing step, confirming preserved event queuing.*

### 3.1 Experimental Result Summary

The observed execution sequence confirms the theoretical advantages of the counting semaphore through these distinct phases:

1. Three distinct interrupts occurred in a rapid burst.
2. The ISR successfully fed all three events into the counting semaphore.
3. The RTOS queued all three tokens without data loss.
4. The handler task processed the events sequentially at a slower rate.
5. The Logic Analyzer confirmed this sequential, preserved processing via the staircase growth of `gTokensProcessed`.

## 4.0 Interpretation & Engineering Trade-offs

The data clearly illustrates the fundamental advantage of a counting semaphore in high-frequency real-time systems. By maintaining an internal count, it decouples the strict timing of the hardware interrupt from the execution speed of the processing task. While this requires slightly more memory overhead than a binary semaphore, it is a mandatory architectural choice for systems where dropping a physical event such as a sensor tick or a communication packet is unacceptable.

## 5.0 Conclusion

The assignment objectives were successfully achieved. The implementation definitively proves that under overload conditions, a counting semaphore successfully queues multiple consecutive interrupts while the handler task is occupied. The debugger evidence and Logic Analyzer staircase graph confirm that zero events were lost, resolving the critical failure mode previously observed with binary semaphores.
