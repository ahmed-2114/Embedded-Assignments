# Assignment 02 Report

## Course
CSE323: Advanced Embedded Systems Design  
CSE411/CSE411s: Real-Time and Embedded Systems Design

## Assignment
Assignment 02: Wrong Stack Pointer Restoration Demo

## Objective
The objective of this assignment is to demonstrate what happens when a task context is restored using the wrong stack pointer. Instead of restoring the correct saved exception frame, the program intentionally shifts the stack pointer by one word for Task 2. This causes the decoded program counter and status register fields to become invalid, which is then detected by the corruption check.

## System Design
The program builds two synthetic Cortex-M exception frames, one for `Task1()` and one for `Task2()`, using `BuildInitialFrame()`. A simulated context-restore routine named `AttemptContextRestore()` is called periodically from `SysTick_Handler()`.

The restore routine performs the following steps:
- selects the next task to restore
- chooses the correct saved stack pointer in `g_restoredSp`
- intentionally shifts the pointer for Task 2 using `g_faultyRestoreSp = g_restoredSp + 1`
- decodes the frame contents through `DecodeFrame(candidateSp)`
- compares the decoded values with the expected task entry point and xPSR thumb bit
- raises `g_contextCorruptionDetected` if the frame is corrupted

## Variables Used for Verification
The main watch variables used during debugging were:
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

## Debug Method
Two breakpoints were used in `AttemptContextRestore()`:
- the start of the restore attempt at `g_restoreAttemptCount++;`
- the corruption check `if ((g_lastDecodedFrame.pc != g_expectedPc) || ((g_lastDecodedFrame.xpsr & (1U << 24)) == 0U))`

The Memory windows were used to inspect both `g_restoredSp` and `g_faultyRestoreSp` directly so the one-word shift could be observed visually.

## Screenshot Verification and Analysis
### Screenshot 1: Correct restore pointer versus faulty shifted pointer
File: `Screenshot 2026-04-27 201316.png`

This screenshot shows the first restore attempt after the SysTick counter reaches `0x000003E8`. The selected next task is Task 2:
- `g_currentTaskId = 1`
- `g_nextTaskId = 2`
- `g_restoredSp = 0x200001E4`
- `g_faultyRestoreSp = 0x200001E8`

The two memory windows prove that the faulty restore pointer is shifted by exactly one 32-bit word relative to the correct restore pointer. This is the intentional error introduced by the demo.

![Correct restore pointer versus faulty shifted pointer](Screenshot%202026-04-27%20201316.png)

### Screenshot 2: Corrupted frame decode
File: `Screenshot 2026-04-27 201426.png`

This screenshot shows the state immediately after decoding the exception frame from the faulty pointer. The expected program counter for Task 2 is:
- `g_expectedPc = 0x000008B9`

But the observed decoded values are corrupted:
- `g_observedPc = 0x01000000`
- `g_lastDecodedFrame.pc = 0x01000000`
- `g_lastDecodedFrame.xpsr = 0x00000000`

This proves that the frame fields are no longer aligned correctly. The value that should have been interpreted as xPSR is now appearing in the PC position, and the xPSR itself becomes invalid.

![Corrupted frame decode](Screenshot%202026-04-27%20201426.png)

### Screenshot 3: Corruption detected and fault signature generated
File: `Screenshot 2026-04-27 201451.png`

This screenshot shows the program after entering the corruption-handling path. The validation condition has succeeded in detecting the bad restore:
- `g_contextCorruptionDetected = 1`
- `g_expectedPc = 0x000008B9`
- `g_observedPc = 0x01000000`
- `g_faultSignature = 0x01000000`

This is the final proof that using the shifted stack pointer leads to an invalid decoded context and that the program detects the corruption successfully.

![Corruption detected and fault signature generated](Screenshot%202026-04-27%20201451.png)

## Experimental Result Summary
The debugger results show a complete cause-and-effect chain:
- the correct saved frame for Task 2 exists at `g_restoredSp`
- the demo intentionally changes the restore address to `g_faultyRestoreSp = g_restoredSp + 1`
- decoding from this shifted address corrupts the frame fields
- the decoded `pc` no longer matches `g_expectedPc`
- the decoded `xpsr` loses the required Thumb bit
- the corruption flag is raised and a nonzero fault signature is produced

## Interpretation
On Cortex-M, exception return depends on the saved frame layout being read from the correct stack location. A one-word shift changes the meaning of every field in the frame. As a result, registers are decoded from the wrong offsets, the program counter becomes invalid, and the status register is corrupted. This is why restoring a task with the wrong stack pointer can immediately break execution.

## Conclusion
The assignment objective was achieved successfully. The implementation demonstrates that restoring a task context from the wrong stack pointer causes corrupted exception-frame decoding. The debugger evidence clearly shows the pointer shift, the resulting mismatch between expected and observed program counter values, and the final corruption detection flag. This confirms why correct stack-pointer restoration is essential in embedded context switching.
