#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Global Handle for the Binary Semaphore
SemaphoreHandle_t xBinarySemaphore = NULL;

/* -------------------------------------------------------------------------
 * Hardware Initialization (Port F: SW1 and LEDs)
 * ------------------------------------------------------------------------- */
void PortF_Init(void) {
    // 1. Enable Clock for Port F
    SYSCTL_RCGCGPIO_R |= 0x20;
    while((SYSCTL_PRGPIO_R & 0x20) == 0) {};

    // 2. Configure SW1 (PF4) as Input and LEDs (PF1, PF2, PF3) as Output
    GPIO_PORTF_DIR_R &= ~0x10;
    GPIO_PORTF_DIR_R |= 0x0E;
    GPIO_PORTF_DEN_R |= 0x1E;
    GPIO_PORTF_PUR_R |= 0x10;

    // 3. Configure External Interrupt on PF4 (SW1)
    GPIO_PORTF_IM_R &= ~0x10;
    GPIO_PORTF_IS_R &= ~0x10;
    GPIO_PORTF_IBE_R &= ~0x10;
    GPIO_PORTF_IEV_R &= ~0x10;
    GPIO_PORTF_ICR_R |= 0x10;
    GPIO_PORTF_IM_R |= 0x10;

    // 4. Configure NVIC for Port F (Interrupt 30)
    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000; 
    
    // Enable Interrupt 30 in NVIC
    NVIC_EN0_R = (1 << 30); 
}

/* -------------------------------------------------------------------------
 * Part 2: ISR Implementation
 * ------------------------------------------------------------------------- */
void GPIOF_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIO_PORTF_ICR_R = 0x10; 

    // 2. Give the binary semaphore to unblock the handler task
    if (xBinarySemaphore != NULL) {
        xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
    }

    // 3. Force a context switch if the woken task has a higher priority
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* -------------------------------------------------------------------------
 * Part 3: FreeRTOS Handler Task
 * ------------------------------------------------------------------------- */
void vActionTask(void *pvParameters) {
    for (;;) {
        // Wait indefinitely for the binary semaphore to be given by the ISR
        if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE) {
            
            // Action Triggered! 
            // Toggle the Red LED (PF1) on the simulated LaunchPad
            GPIO_PORTF_DATA_R ^= 0x02;
        }
    }
}

/* -------------------------------------------------------------------------
 * Part 4: System Integration
 * ------------------------------------------------------------------------- */
int main(void) {
    PortF_Init();
    GPIO_PORTF_DATA_R &= ~0x0E;

    xBinarySemaphore = xSemaphoreCreateBinary();

    if (xBinarySemaphore != NULL) {
        
        xTaskCreate(vActionTask,            
                    "ActionTask",           
                    configMINIMAL_STACK_SIZE, 
                    NULL,                   
                    2,
                    NULL);    

        vTaskStartScheduler();
    }

    for (;;);
}