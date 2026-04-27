//*****************************************************************************
//
// main.c - Wrong stack pointer restoration demo
// Builds two Cortex-M exception frames and intentionally restores one task with
// a shifted stack pointer to expose corrupted frame decoding in the debugger.
//
//*****************************************************************************

#include <stdint.h>
#include "TM4C123GH6PM.h"  // CMSIS header for Tiva C

//*****************************************************************************
// System Clock Frequency (16 MHz default for Tiva C)
//*****************************************************************************
#define SYSTEM_CLOCK_HZ         16000000
#define SYSTICK_1MS             (SYSTEM_CLOCK_HZ / 1000) - 1  // Reload for 1ms tick

//*****************************************************************************
// Task Stacks and Stack Pointers
//*****************************************************************************
uint32_t stack_task1[40];
uint32_t *sp_task1 = &stack_task1[40];

uint32_t stack_task2[40];
uint32_t *sp_task2 = &stack_task2[40];

uint32_t *g_currentSp = 0;
uint32_t *g_restoredSp = 0;
uint32_t *g_faultyRestoreSp = 0;

typedef struct
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;
} ExceptionFrame;

volatile ExceptionFrame g_lastDecodedFrame;

//*****************************************************************************
// Global Variables
//*****************************************************************************
volatile uint32_t g_ui32SysTickCount = 0;
volatile uint32_t g_currentTaskId = 1;
volatile uint32_t g_nextTaskId = 2;
volatile uint32_t g_restoreAttemptCount = 0;
volatile uint32_t g_contextCorruptionDetected = 0;
volatile uint32_t g_expectedPc = 0;
volatile uint32_t g_observedPc = 0;
volatile uint32_t g_expectedXpsr = 0x01000000U;
volatile uint32_t g_faultSignature = 0;
volatile uint32_t g_useBrokenRestore = 1;
volatile uint32_t g_task1Heartbeat = 0;
volatile uint32_t g_task2Heartbeat = 0;

//*****************************************************************************
// Task1 - Runs in an infinite loop, does some work
//*****************************************************************************
void Task1(void)
{
    while(1)
    {
        g_task1Heartbeat++;
        for(volatile int i = 0; i < 1000; i++)
        {
            __asm("NOP");
        }
    }
}

//*****************************************************************************
// Task2 - Runs in an infinite loop, does different work
//*****************************************************************************
void Task2(void)
{
    while(1)
    {
        g_task2Heartbeat++;
        for(volatile int j = 0; j < 500; j++)
        {
            __asm("NOP");
        }
    }
}

//*****************************************************************************
// Build a synthetic exception frame exactly as Cortex-M stacks it on exception
// entry so the debugger can inspect a realistic restore target.
//*****************************************************************************
static void BuildInitialFrame(uint32_t **stackPointer, void (*entryPoint)(void), uint32_t seed)
{
    *(--(*stackPointer)) = (1U << 24);
    *(--(*stackPointer)) = (uint32_t)(uintptr_t)entryPoint;
    *(--(*stackPointer)) = 0xFFFFFFFDU;
    *(--(*stackPointer)) = 0x12121212U + seed;
    *(--(*stackPointer)) = 0x03030303U + seed;
    *(--(*stackPointer)) = 0x02020202U + seed;
    *(--(*stackPointer)) = 0x01010101U + seed;
    *(--(*stackPointer)) = 0x00000000U + seed;
}

//*****************************************************************************
// Decode an exception frame from a candidate stack pointer.
//*****************************************************************************
static ExceptionFrame DecodeFrame(const uint32_t *stackPointer)
{
    ExceptionFrame frame;

    frame.r0 = stackPointer[0];
    frame.r1 = stackPointer[1];
    frame.r2 = stackPointer[2];
    frame.r3 = stackPointer[3];
    frame.r12 = stackPointer[4];
    frame.lr = stackPointer[5];
    frame.pc = stackPointer[6];
    frame.xpsr = stackPointer[7];

    return frame;
}

//*****************************************************************************
// Simulate a restore attempt and flag the broken one.
//*****************************************************************************
static void AttemptContextRestore(void)
{
    const uint32_t *candidateSp;

    g_restoreAttemptCount++;
    g_nextTaskId = (g_currentTaskId == 1U) ? 2U : 1U;

    if (g_nextTaskId == 1U)
    {
        g_restoredSp = sp_task1;
        g_expectedPc = (uint32_t)(uintptr_t)&Task1;
    }
    else
    {
        g_restoredSp = sp_task2;
        g_expectedPc = (uint32_t)(uintptr_t)&Task2;
    }

    candidateSp = g_restoredSp;

    if ((g_nextTaskId == 2U) && (g_useBrokenRestore != 0U))
    {
        g_faultyRestoreSp = g_restoredSp + 1;
        candidateSp = g_faultyRestoreSp;
    }
    else
    {
        g_faultyRestoreSp = g_restoredSp;
    }

    g_lastDecodedFrame = DecodeFrame(candidateSp);
    g_observedPc = g_lastDecodedFrame.pc;

    if ((g_lastDecodedFrame.pc != g_expectedPc) ||
        ((g_lastDecodedFrame.xpsr & (1U << 24)) == 0U))
    {
        g_contextCorruptionDetected = 1U;
        g_faultSignature = g_lastDecodedFrame.pc ^ g_lastDecodedFrame.xpsr;
    }
    else
    {
        g_contextCorruptionDetected = 0U;
        g_currentTaskId = g_nextTaskId;
        g_currentSp = g_restoredSp;
    }
}

//*****************************************************************************
// SysTick Handler - Fires every 1ms
// Performs a debugger-friendly simulated restore attempt.
//*****************************************************************************
void SysTick_Handler(void)
{
    g_ui32SysTickCount++;

    if ((g_ui32SysTickCount % 1000U) == 0U)
    {
        AttemptContextRestore();
    }
}

//*****************************************************************************
// Configure SysTick for 1ms interrupt
//*****************************************************************************
void SysTick_Init(void)
{
    // Disable SysTick during configuration
    SysTick->CTRL = 0;
    
    // Set the reload value for 1ms tick
    SysTick->LOAD = SYSTICK_1MS;
    
    // Clear the current count
    SysTick->VAL = 0;
    
    // Enable SysTick with system clock and interrupt
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
                    SysTick_CTRL_TICKINT_Msk | 
                    SysTick_CTRL_ENABLE_Msk;
}

//*****************************************************************************
// Main function
//*****************************************************************************
int main(void)
{
    BuildInitialFrame(&sp_task1, Task1, 0x10U);
    BuildInitialFrame(&sp_task2, Task2, 0x20U);

    g_currentSp = sp_task1;
    g_restoredSp = sp_task1;
    g_faultyRestoreSp = sp_task2 + 1;
    g_lastDecodedFrame = DecodeFrame(sp_task1);
    g_observedPc = g_lastDecodedFrame.pc;
    g_expectedPc = (uint32_t)(uintptr_t)&Task2;

    SysTick_Init();
		
    while(1)
    {
        if (g_contextCorruptionDetected != 0U)
        {
            for(volatile int k = 0; k < 500; k++)
            {
                __asm("NOP");
            }
        }
    }
}
