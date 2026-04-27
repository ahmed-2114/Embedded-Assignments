#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Global Handle for the Binary Semaphore
static SemaphoreHandle_t xBinarySemaphore = NULL;

// Global counters to observe in Keil Logic Analyzer or Watch Window
volatile uint32_t gInterruptsFired = 0;
volatile uint32_t gSemaphoreAccepted = 0;
volatile uint32_t gDroppedInterrupts = 0;
volatile uint32_t gTasksProcessed = 0;
volatile uint32_t gTriggerBursts = 0;
volatile uint32_t gHandlerBusy = 0;

#define PF1_RED_LED             0x02U
#define PF2_BLUE_LED            0x04U
#define PF3_GREEN_LED           0x08U
#define PF4_SWITCH              0x10U

/* -------------------------------------------------------------------------
 * Hardware Initialization (Port F for simulation purposes)
 * ------------------------------------------------------------------------- */
static void PortF_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x20;
    while((SYSCTL_PRGPIO_R & 0x20) == 0) {}; 

    GPIO_PORTF_DIR_R |= 0x0E;  
    GPIO_PORTF_DEN_R |= 0x1E;
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
    BaseType_t xSemaphoreStatus;

    GPIO_PORTF_ICR_R = PF4_SWITCH;

    gInterruptsFired++;
    GPIO_PORTF_DATA_R ^= PF2_BLUE_LED;

    if (xBinarySemaphore != NULL) {
        xSemaphoreStatus = xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
        if (xSemaphoreStatus == pdTRUE) {
            gSemaphoreAccepted++;
        } else {
            gDroppedInterrupts++;
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* -------------------------------------------------------------------------
 * Task 1: The Trigger Task (Simulates high-frequency hardware events)
 * ------------------------------------------------------------------------- */
static void vTriggerTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(3500));
        gTriggerBursts++;
        GPIO_PORTF_DATA_R ^= PF3_GREEN_LED;
        
        NVIC_SW_TRIG_R = 30;
        vTaskDelay(pdMS_TO_TICKS(30));
        
        NVIC_SW_TRIG_R = 30;
        vTaskDelay(pdMS_TO_TICKS(30));
        
        NVIC_SW_TRIG_R = 30;
    }
}

/* -------------------------------------------------------------------------
 * Task 2: The Handler Task (Simulates slow event processing)
 * ------------------------------------------------------------------------- */
static void vHandlerTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE) {
            gHandlerBusy = 1;
            gTasksProcessed++;

            GPIO_PORTF_DATA_R |= PF1_RED_LED;

            vTaskDelay(pdMS_TO_TICKS(1200));

            GPIO_PORTF_DATA_R &= ~PF1_RED_LED;
            gHandlerBusy = 0;
        }
    }
}

/* -------------------------------------------------------------------------
 * System Integration
 * ------------------------------------------------------------------------- */
int main(void) {
    PortF_Init();
    GPIO_PORTF_DATA_R &= ~(PF1_RED_LED | PF2_BLUE_LED | PF3_GREEN_LED);

    xBinarySemaphore = xSemaphoreCreateBinary();

    if (xBinarySemaphore != NULL) {
        xTaskCreate(vTriggerTask, "Trigger", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
        xTaskCreate(vHandlerTask, "Handler", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

        vTaskStartScheduler();
    }

    for (;;);
}