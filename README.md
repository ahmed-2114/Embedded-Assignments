# Embedded Assignments Repo Guide

A simple guide for anyone who wants to use this repo to understand, run, and submit the same assignments.

> If you feel lost, do not read everything. Start with the run guide, then open one assignment at a time.

---

## Start Here

| If you want to... | Open this first |
| --- | --- |
| run the assignments | [Submissions/Run Guide/Embedded_Assignments_Run_Guide.md](Submissions/Run%20Guide/Embedded_Assignments_Run_Guide.md) |
| read the instructor task | `Assignment PDFs/` |
| see the final reports | `Submissions/` |
| build and debug code | `Hassabou-Assignments/` |

If you want the easiest path, follow this order:

1. Open [Submissions/Run Guide/Embedded_Assignments_Run_Guide.md](Submissions/Run%20Guide/Embedded_Assignments_Run_Guide.md)
2. Open the assignment folder you want inside `Hassabou-Assignments`
3. Open the matching report inside `Submissions`

If you only want the final ready-made material, go straight to `Submissions`.

If you want to understand how each assignment was done, use this path:

1. Read the assignment PDF
2. Open the matching Keil project and `main.c`
3. Follow the run guide
4. Check the final report and screenshots

---

## Repo Map

### `Assignment PDFs/`
The original assignment sheets.

Use this if you want to see the exact task wording from the instructor.

### `Hassabou-Assignments/`
The actual Keil projects and code.

Each assignment folder contains:
- the `.uvprojx` project file for Keil
- `main.c`
- the support files needed by the project

This is the folder you open when you want to build, debug, and test the assignment.

### `Submissions/`
The final reports, screenshots, and exported PDFs.

This is the folder you open when you want:
- the final answer
- the final screenshots
- the final report PDF

### `Status Report/`
A plain-English summary of all assignments.

This is useful if you want a quick overview before opening code.

---

## Best Way To Use This Repo

### Option 1: You want the final result fast
Use this when you mainly care about submission structure.

1. Open `Submissions`
2. Pick the assignment folder
3. Read the PDF or markdown report
4. Open the matching code in `Hassabou-Assignments` if needed

### Option 2: You want to learn how each one works
Use this when you want to repeat the same debugging steps yourself.

1. Read the assignment PDF
2. Open the matching project in Keil
3. Follow the run guide step by step
4. Compare your debugger state with the screenshots in `Submissions`

### Option 3: You want to copy the exact workflow I used
Use this order:

1. Assignment 03
2. Assignment 05
3. Assignment 07
4. Assignment 06
5. Assignment 04
6. Assignment 02

That order was chosen because it moves from simpler proof to more custom debugging.

---

## Assignment By Assignment

### Assignment 02
#### Where to open the code
- `Hassabou-Assignments/Assignment-02/Assignment-02.uvprojx`
- `Hassabou-Assignments/Assignment-02/main.c`

#### What this one is about
This assignment demonstrates a wrong stack pointer restore and shows how frame decoding becomes corrupted.

#### Where the final material is
- `Submissions/Assignment 2/Assignment_02_Report.md`
- `Submissions/Assignment 2/Assignment_02_Report.pdf`

#### What to look for
- `g_restoredSp`
- `g_faultyRestoreSp`
- `g_observedPc`
- `g_contextCorruptionDetected`

---

### Assignment 03
#### Where to open the code
- `Hassabou-Assignments/Assignment-03/Assignment-03.uvprojx`
- `Hassabou-Assignments/Assignment-03/main.c`

#### What this one is about
This assignment shows an interrupt waking a task correctly using FreeRTOS task notification.

#### Where the final material is
- `Submissions/Assignment 3/Assignment_03_Report.md`

#### What to look for
- ISR runs first
- task wakes after ISR
- counters increment in the expected order

---

### Assignment 04
#### Where to open the code
- `Hassabou-Assignments/Assignment-04/Assignment-03.uvprojx`
- `Hassabou-Assignments/Assignment-04/main.c`

#### Important note
The folder is Assignment 04, but the Keil project file still has the old name `Assignment-03.uvprojx`.

#### What this one is about
This assignment compares behavior with and without `portYIELD_FROM_ISR()`.

#### Where the final material is
- `Submissions/Assignment 4/Assignment_04_Report.md`
- `Submissions/Assignment 4/Assignment_04_Report.pdf`

#### What to look for
- with `USE_ISR_YIELD = 1U`, `gObservedLatencyTicks = 0`
- with `USE_ISR_YIELD = 0U`, `gObservedLatencyTicks = 1`

---

### Assignment 05
#### Where to open the code
- `Hassabou-Assignments/Assignment-05/Assignment-05.uvprojx`
- `Hassabou-Assignments/Assignment-05/main.c`

#### What this one is about
This assignment shows that a binary semaphore can drop events if interrupts happen while the handler is already busy.

#### Where the final material is
- `Submissions/Assignment 5/Assignment_05_Report.md`
- `Submissions/Assignment 5/Assignment_05_Report.pdf`
- `Submissions/Assignment 5/Assignment_05_Report_Final.pdf`

#### What to look for
- `gDroppedInterrupts > 0`
- binary semaphore behavior
- handler still busy while new interrupt arrives

---

### Assignment 06
#### Where to open the code
- `Hassabou-Assignments/Assignment-06/Assignment-06.uvprojx`
- `Hassabou-Assignments/Assignment-06/main.c`

#### What this one is about
This assignment shows a resource pool using a counting semaphore.

#### Where the final material is
- `Submissions/Assignment 6/Assignment_06_Report.md`
- `Submissions/Assignment 6/Assignment_06_Report.pdf`

#### What to look for
- no more than 2 tasks use the resource at the same time
- `gMaxObservedResources = 2`
- worker order may change, and that is normal

---

### Assignment 07
#### Where to open the code
- `Hassabou-Assignments/Assignment-07/Assignment-07.uvprojx`
- `Hassabou-Assignments/Assignment-07/main.c`

#### What this one is about
This assignment shows that a counting semaphore stores multiple interrupt events instead of dropping them.

#### Where the final material is
- `Submissions/Assignment 7/Assignment_07_Report.md`
- `Submissions/Assignment 7/Assignment_07_Report.pdf`
- `Submissions/Assignment 7/Assignment_07_Report_Final.pdf`

#### What to look for
- `gTokensQueued`
- `gTokensProcessed`
- Logic Analyzer staircase pattern

---

## The One File That Helps The Most

If your classmate only opens one file, make it this one:

- [Submissions/Run Guide/Embedded_Assignments_Run_Guide.md](Submissions/Run%20Guide/Embedded_Assignments_Run_Guide.md)

It contains:
- what project file to open
- what variables to watch
- where to place breakpoints
- what result should appear

---

## Suggested Simple Workflow For A Classmate

Use this every time:

1. Pick an assignment number
2. Open the PDF in `Assignment PDFs`
3. Open the matching folder in `Hassabou-Assignments`
4. Open the matching report in `Submissions`
5. Use the run guide if you want to reproduce the debugger proof

If they get lost, tell them to ignore most of the repo and focus on only these three places:
- `Assignment PDFs`
- `Hassabou-Assignments`
- `Submissions`

---

## Small Notes

- Keil is the real source of truth for these embedded projects.
- VS Code may show false errors because include paths for the embedded environment are not always resolved there.
- The reports in `Submissions` are already written in a submission-friendly format.
- The screenshots in each `Submissions/Assignment X` folder match the final verified behavior.

---

## Quick Answer To "Where Do I Start?"

If someone opens this repo for the first time, tell them:

1. Read the run guide
2. Pick one assignment folder in `Submissions`
3. Open the same-numbered folder in `Hassabou-Assignments`
4. Use the report as the finished example

That is the easiest way to move through the repo without feeling overloaded.
