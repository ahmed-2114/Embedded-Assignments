# Assignment 05 Report

## Course
CSE323: Advanced Embedded Systems Design  
CSE411/CSE411s: Real-Time and Embedded Systems Design

## Assignment
Assignment 05: Binary Semaphore Behavior Under Multiple Pending Interrupts

## Objective
The objective of this assignment is to demonstrate the limitation of a binary semaphore in FreeRTOS when multiple interrupts occur while the handler task is still busy. A binary semaphore can only hold one pending token, so if additional interrupts happen before the task returns to wait on the semaphore again, some of those interrupt events are lost.

## System Setup
The implemented application contains two FreeRTOS tasks and one ISR:

1. `vTriggerTask()`
This task generates a burst of three software-triggered interrupts using `NVIC_SW_TRIG_R = 30`. The burst is generated automatically, so no manual button interaction is required.

2. `GPIOF_Handler()`
This ISR represents the interrupt source. It:
- clears the interrupt flag
- increments `gInterruptsFired`
- toggles an LED indicator
- gives a binary semaphore using `xSemaphoreGiveFromISR()`
- increments either `gSemaphoreAccepted` or `gDroppedInterrupts` depending on whether the semaphore could accept the signal

3. `vHandlerTask()`
This task waits on the binary semaphore using `xSemaphoreTake()`. When it receives a token, it:
- sets `gHandlerBusy = 1`
- increments `gTasksProcessed`
- simulates long processing using `vTaskDelay(1200)`
- clears the busy flag when processing ends

## Variables Used for Verification
The following variables were monitored in the Keil Watch window:
- `gInterruptsFired`
- `gSemaphoreAccepted`
- `gDroppedInterrupts`
- `gTasksProcessed`
- `gTriggerBursts`
- `gHandlerBusy`

These variables were used to verify whether interrupts occurred, whether the semaphore accepted or dropped them, and whether the handler task was still busy when later interrupts arrived.

## Debug Procedure
The project built successfully in Keil with no errors or warnings. After entering debug mode, the following breakpoints were used:

- ISR breakpoint at the line `gInterruptsFired++;`
- handler-task breakpoint at the line `gTasksProcessed++;`

The program was then allowed to run. Since the trigger task generates interrupts automatically, the debugger stopped at the breakpoints without any manual interaction.

## Screenshot Verification and Analysis
All screenshots in the submission folder were checked against the expected execution sequence. The captured sequence is correct.

### Screenshot 1: Initial debug setup
File: `Screenshot 2026-04-27 160102.png`

This screenshot shows the initial debug state before the first interrupt burst is processed. Both breakpoints are visible and all watch variables are still zero. This is a valid starting-state capture.

![Initial debug setup](Screenshot%202026-04-27%20160102.png)

### Screenshot 2: First ISR hit before increment
File: `Screenshot 2026-04-27 160131.png`

Execution stopped in `GPIOF_Handler()` at the breakpoint line `gInterruptsFired++;`. At this moment:
- `gInterruptsFired = 0`
- `gTriggerBursts = 1`

This matches the expected state before the first ISR counter increment executes.

![First ISR hit before increment](Screenshot%202026-04-27%20160131.png)

### Screenshot 3: First ISR after stepping over increment
File: `Screenshot 2026-04-27 160144.png`

After stepping once, the ISR counter increased:
- `gInterruptsFired = 1`

This confirms that the first interrupt was generated and entered the ISR successfully.

![First ISR after increment](Screenshot%202026-04-27%20160144.png)

### Screenshot 4: Handler task awakened, before processing count update
File: `Screenshot 2026-04-27 160155.png`

Execution stopped at the handler-task breakpoint on `gTasksProcessed++;`. At this point:
- `gInterruptsFired = 1`
- `gSemaphoreAccepted = 1`
- `gTasksProcessed = 0`
- `gHandlerBusy = 1`

This matches the expected state: the first interrupt has already been accepted by the binary semaphore, and the handler task has woken up and is now busy.

![Handler task awakened before count update](Screenshot%202026-04-27%20160155.png)

### Screenshot 5: Handler task after processing count update
File: `Screenshot 2026-04-27 160207.png`

After stepping once in the handler task:
- `gTasksProcessed = 1`
- `gHandlerBusy = 1`

This proves that the first interrupt event was actually processed by the handler task.

![Handler task after count update](Screenshot%202026-04-27%20160207.png)

### Screenshot 6: Later ISR hit while handler is still busy
File: `Screenshot 2026-04-27 160220.png`

Execution returned to `GPIOF_Handler()` while:
- `gTasksProcessed = 1`
- `gHandlerBusy = 1`
- `gSemaphoreAccepted = 1`
- `gDroppedInterrupts = 0`

This is an important intermediate state. It proves that another interrupt arrived while the handler task was still processing the first event.

![Later ISR hit while handler busy](Screenshot%202026-04-27%20160220.png)

### Screenshot 7: Second ISR after increment
File: `Screenshot 2026-04-27 160233.png`

After stepping over the ISR increment again:
- `gInterruptsFired = 2`
- `gTasksProcessed = 1`
- `gHandlerBusy = 1`

This confirms the second interrupt occurred while the handler was still busy, which is the exact overload condition required by the assignment.

![Second ISR after increment](Screenshot%202026-04-27%20160233.png)

### Screenshot 8: Final proof of dropped interrupt
File: `Screenshot 2026-04-27 160254.png`

This is the key proof screenshot. At this point:
- `gInterruptsFired = 3`
- `gSemaphoreAccepted = 2`
- `gDroppedInterrupts = 1`
- `gTasksProcessed = 1`
- `gHandlerBusy = 1`

This means three interrupts occurred, but only two semaphore signals were accepted and one interrupt event was dropped. Since the task had only processed one event so far and was still busy, this directly demonstrates the limitation of a binary semaphore under multiple pending interrupts.

![Final proof of dropped interrupt](Screenshot%202026-04-27%20160254.png)

## Experimental Result Summary
The observed sequence proves the required binary semaphore behavior:

1. The first interrupt was accepted and woke the handler task.
2. The handler task became busy processing that event.
3. Additional interrupts arrived during the busy period.
4. The binary semaphore could not queue every incoming interrupt.
5. At least one interrupt was dropped, as shown by `gDroppedInterrupts = 1`.

## Interpretation
A binary semaphore only stores one pending state. It does not count every interrupt occurrence separately. Therefore, when multiple interrupts happen before the handler task can return to `xSemaphoreTake()`, the extra events cannot all be preserved. This is why one of the later interrupts was lost in the experiment.

## Challenges Faced
The main challenge was proving not only that interrupts were firing, but also that one of them was actually lost. This was solved by using separate watch variables for:
- total interrupts fired
- accepted semaphore signals
- dropped interrupt signals
- processed task events
- handler busy state

This made it possible to show the exact mismatch between interrupt generation and successful event processing.

## Conclusion
The assignment objective was achieved successfully. The implemented FreeRTOS application demonstrates that when multiple interrupts occur while the handler task is still busy, a binary semaphore cannot queue them all. The debugger evidence shows that three interrupts occurred, but one of them was dropped. This confirms the expected limitation of binary semaphores in overload scenarios.
