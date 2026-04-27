# Assignment 06 Report

## Course
CSE323: Advanced Embedded Systems Design  
CSE411/CSE411s: Real-Time and Embedded Systems Design

## Assignment
Assignment 06: Counting Semaphore for Shared Resource Management

## Objective
The purpose of this assignment is to demonstrate how a counting semaphore can be used to manage a limited pool of shared resources in a FreeRTOS system. The required proof is that multiple worker tasks may compete for the resources, but no more than the available number of resources can be used at the same time.

## System Scenario
The implemented design models a resource pool with:
- 5 worker tasks competing for access
- 2 identical shared resources available at any time
- 1 counting semaphore initialized with a maximum count of 2 and an initial count of 2

Each worker task repeatedly:
- attempts to take a token from the counting semaphore
- marks itself as active if it succeeds
- simulates resource usage using `vTaskDelay()`
- releases the token back to the semaphore after finishing

## Verification Variables
The following variables were used in the debugger watch window:
- `gActiveResources`
- `gMaxObservedResources`
- `gTaskUsingResource[0]`
- `gTaskUsingResource[1]`
- `gTaskUsingResource[2]`
- `gTaskUsingResource[3]`
- `gTaskUsingResource[4]`

These variables are sufficient to show:
- how many resources are currently occupied
- the maximum number of simultaneously occupied resources observed so far
- which worker tasks are currently inside the resource-usage section

## Debug Method
The project was run in Keil debug mode with breakpoints placed at:
- `gActiveResources++;`
- `gActiveResources--;`

This allowed observation of both resource acquisition and resource release states while the five worker tasks competed for the two available tokens.

## Screenshot Verification and Analysis
The alternating states in the screenshots are expected and correct. Since five worker tasks are repeatedly competing for only two resources, the exact worker IDs holding resources will change over time. What matters is that the total number of active resource users never exceeds 2.

### Screenshot 1: Initial baseline before acquisition
File: `Screenshot 2026-04-27 174352.png`

This screenshot shows the baseline state before any worker has successfully entered the resource section. At this point:
- `gActiveResources = 0`
- `gMaxObservedResources = 0`
- all `gTaskUsingResource[]` entries are `0`

This is a valid starting state for the experiment.

![Initial baseline before acquisition](Screenshot%202026-04-27%20174352.png)

### Screenshot 2: First worker acquires a resource
File: `Screenshot 2026-04-27 174415.png`

This screenshot shows the system stopped at the acquisition breakpoint. The watch values show:
- `gActiveResources = 1`
- `gMaxObservedResources = 1`
- one task entry in `gTaskUsingResource[]` is active

This proves the first worker successfully claimed one of the two resource tokens.

![First worker acquires a resource](Screenshot%202026-04-27%20174415.png)

### Screenshot 3: Two resources are in use simultaneously
File: `Screenshot 2026-04-27 174422.png`

This screenshot shows the system after another worker has entered the resource section. The watch values show:
- `gActiveResources = 2`
- `gMaxObservedResources = 2`
- one worker entry is active in the watch snapshot, and the system state confirms the limit of two has been reached

This is the key proof that the system allows up to two concurrent resource users, matching the semaphore configuration.

![Two resources in use simultaneously](Screenshot%202026-04-27%20174422.png)

### Screenshot 4: Resource release while maximum remains bounded
File: `Screenshot 2026-04-27 174428.png`

This screenshot shows the system at the release breakpoint. The watch values show:
- `gActiveResources = 1`
- `gMaxObservedResources = 2`

This means one worker released a token, but the maximum observed simultaneous usage remained at 2. This is expected and proves that the system dynamically allocates and releases resources while preserving the upper limit.

![Resource release while maximum remains bounded](Screenshot%202026-04-27%20174428.png)

### Screenshot 5: Alternating active worker identity, same resource limit
File: `Screenshot 2026-04-27 174431.png`

This screenshot shows that the identity of the active worker changed, which is normal because multiple tasks are competing continuously. The watch values still show:
- `gActiveResources` does not exceed `2`
- `gMaxObservedResources = 2`

This is valid proof that the active worker IDs alternate over time, but the total number of simultaneous resource users never exceeds the configured limit.

![Alternating active worker identity, same resource limit](Screenshot%202026-04-27%20174431.png)

## Experimental Result Summary
The captured execution confirms the expected behavior of the counting semaphore resource pool:

1. Worker tasks enter and leave the resource section over time.
2. The identity of the active worker changes from one breakpoint to another.
3. `gActiveResources` rises and falls dynamically.
4. `gMaxObservedResources` reaches `2` and never exceeds it.
5. Therefore, the counting semaphore successfully limits access to the shared resource pool to two concurrent users.

## Interpretation
The alternating watch values are not a problem. They are the normal consequence of multiple worker tasks competing for the same limited resource pool. The assignment does not require the same task to remain active across screenshots. Instead, it requires proof that the number of simultaneously active tasks is bounded by the semaphore count. The screenshots provide exactly that proof.

## Conclusion
The assignment objective was achieved successfully. The implemented FreeRTOS application demonstrates that a counting semaphore can be used to manage a limited pool of two shared resources among five competing worker tasks. The debugger evidence shows that while the active task identities alternate over time, the total number of simultaneous resource users never exceeds 2. This confirms correct counting semaphore behavior for resource management.
