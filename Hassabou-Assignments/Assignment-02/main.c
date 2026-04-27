//*****************************************************************************
//
// main.c - Context Switch Demo using Stack Pointers (SP)
// Demonstrates context switching by saving/restoring SP for each task
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

//*****************************************************************************
// Global Variables
//*****************************************************************************
volatile uint32_t g_ui32SysTickCount = 0;

//*****************************************************************************
// Task1 - Runs in an infinite loop, does some work
//*****************************************************************************
void Task1(void)
{
    volatile uint32_t counter1 = 0;
    
    while(1)
    {
        counter1++;
        
        // Simulate some work
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
    volatile uint32_t counter2 = 0;
    
    while(1)
    {
        counter2++;
        
        // Simulate different work
        for(volatile int j = 0; j < 500; j++)
        {
            __asm("NOP");
        }
    }
}

//*****************************************************************************
// SysTick Handler - Fires every 1ms
// Performs context switch by swapping stack pointers
//*****************************************************************************
void SysTick_Handler(void)
{
    // Increment tick counter
    g_ui32SysTickCount++;
    
    // Context switch logic using SP
    // The stacked frame on MSP:
    // [MSP+0x00] = R0
    // [MSP+0x04] = R1
    // [MSP+0x08] = R2
    // [MSP+0x0C] = R3
    // [MSP+0x10] = R12
    // [MSP+0x14] = LR
    // [MSP+0x18] = PC
    // [MSP+0x1C] = xPSR
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
    // Fabricate Cortex-M ISR stack frame for Task1
    *(--sp_task1) = (1U << 24);           /* xPSR (Thumb bit set) */
    *(--sp_task1) = (uint32_t)&Task1;     /* PC */
    *(--sp_task1) = 0x0000000EU;          /* LR  */
    *(--sp_task1) = 0x0000000CU;          /* R12 */
    *(--sp_task1) = 0x00000003U;          /* R3  */
    *(--sp_task1) = 0x00000002U;          /* R2  */
    *(--sp_task1) = 0x00000001U;          /* R1  */
    *(--sp_task1) = 0x00000000U;          /* R0  */

    // Fabricate Cortex-M ISR stack frame for Task2
    *(--sp_task2) = (1U << 24);           /* xPSR (Thumb bit set) */
    *(--sp_task2) = (uint32_t)&Task2;     /* PC */
    *(--sp_task2) = 0x0000000EU;          /* LR  */
    *(--sp_task2) = 0x0000000CU;          /* R12 */
    *(--sp_task2) = 0x00000003U;          /* R3  */
    *(--sp_task2) = 0x00000002U;          /* R2  */
    *(--sp_task2) = 0x00000001U;          /* R1  */
    *(--sp_task2) = 0x00000000U;          /* R0  */

    // Initialize SysTick for 1ms interrupt
    SysTick_Init();
		
    while(1)
    {
    }
}
