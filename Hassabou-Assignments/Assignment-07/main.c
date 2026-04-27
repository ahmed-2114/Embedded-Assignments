#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Global Handle for the Counting Semaphore
static SemaphoreHandle_t xCountingSemaphore = NULL;

// Global counters to track the exact sequence of events
volatile uint32_t gInterruptsFired = 0;
volatile uint32_t gTokensQueued = 0;
volatile uint32_t gTokensProcessed = 0;
volatile uint32_t gBurstCounter = 0;
volatile uint32_t gHandlerActive = 0;

#define PF1_RED_LED             0x02U
#define PF2_BLUE_LED            0x04U
#define PF3_GREEN_LED           0x08U
#define PF4_SWITCH              0x10U

/* -------------------------------------------------------------------------
 * Hardware Initialization (Port F for simulation)
 * ------------------------------------------------------------------------- */
static void PortF_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x20;
    while((SYSCTL_PRGPIO_R & 0x20) == 0) {}; 

    GPIO_PORTF_DIR_R |= 0x0E;  
    GPIO_PORTF_DEN_R |= 0x0E;  
    GPIO_PORTF_PUR_R |= PF4_SWITCH;
    GPIO_PORTF_ICR_R = PF4_SWITCH;
    
    GPIO_PORTF_IM_R |= PF4_SWITCH;   

    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000; 
    NVIC_EN0_R = (1 << 30); 
}

/* -------------------------------------------------------------------------
 * The ISR (Simulated Hardware Interrupt)
 * ------------------------------------------------------------------------- */
void GPIOF_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIO_PORTF_ICR_R = PF4_SWITCH;

    gInterruptsFired++;
		gTokensQueued++;
    GPIO_PORTF_DATA_R ^= PF1_RED_LED;

    if (xCountingSemaphore != NULL) {
        xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* -------------------------------------------------------------------------
 * Task 1: The Trigger Task (Priority 2 - High)
 * ------------------------------------------------------------------------- */
static void vTriggerTask(void *pvParameters) {
    uint32_t firedBeforeBurst;

    (void)pvParameters;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(3000));
        gBurstCounter++;
        GPIO_PORTF_DATA_R ^= PF3_GREEN_LED;
        firedBeforeBurst = gInterruptsFired;
        
        NVIC_SW_TRIG_R = 30;
        while (gInterruptsFired == firedBeforeBurst) {
        }
        
        NVIC_SW_TRIG_R = 30;
        while (gInterruptsFired == (firedBeforeBurst + 1U)) {
        }
        
        NVIC_SW_TRIG_R = 30;
        while (gInterruptsFired == (firedBeforeBurst + 2U)) {
        }
    }
}

/* -------------------------------------------------------------------------
 * Task 2: The Handler Task (Priority 1 - Low)
 * ------------------------------------------------------------------------- */
static void vHandlerTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        if (xSemaphoreTake(xCountingSemaphore, portMAX_DELAY) == pdTRUE) {
            gHandlerActive = 1;
            gTokensProcessed++;
            GPIO_PORTF_DATA_R |= PF2_BLUE_LED;

            vTaskDelay(pdMS_TO_TICKS(1500));

            GPIO_PORTF_DATA_R &= ~PF2_BLUE_LED;
            gHandlerActive = 0;
        }
    }
}

/* -------------------------------------------------------------------------
 * System Integration
 * ------------------------------------------------------------------------- */
int main(void) {
    PortF_Init();
    GPIO_PORTF_DATA_R &= ~(PF1_RED_LED | PF2_BLUE_LED | PF3_GREEN_LED);

    xCountingSemaphore = xSemaphoreCreateCounting(5, 0);

    if (xCountingSemaphore != NULL) {
        xTaskCreate(vTriggerTask, "Trigger", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
        xTaskCreate(vHandlerTask, "Handler", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

        vTaskStartScheduler();
    }

    for (;;);
}