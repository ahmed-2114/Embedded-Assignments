#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Global Handle for the Counting Semaphore
static SemaphoreHandle_t xResourcePool = NULL;

// Global variables to observe in Keil Logic Analyzer / Watch Window
volatile uint32_t gActiveResources = 0;
volatile uint32_t gMaxObservedResources = 0;
volatile uint32_t gAcquireCount[5] = {0, 0, 0, 0, 0};
volatile uint32_t gReleaseCount[5] = {0, 0, 0, 0, 0};
volatile uint8_t gTaskUsingResource[5] = {0, 0, 0, 0, 0};

#define PF1_RED_LED             0x02U
#define PF2_BLUE_LED            0x04U
#define PF3_GREEN_LED           0x08U

/* -------------------------------------------------------------------------
 * Hardware Initialization (Port F for activity LEDs)
 * ------------------------------------------------------------------------- */
static void PortF_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x20;
    while((SYSCTL_PRGPIO_R & 0x20) == 0) {}; 

    GPIO_PORTF_DIR_R |= (PF1_RED_LED | PF2_BLUE_LED | PF3_GREEN_LED);
    GPIO_PORTF_DEN_R |= (PF1_RED_LED | PF2_BLUE_LED | PF3_GREEN_LED);
}

/* -------------------------------------------------------------------------
 * The Worker Task (5 instances of this will run concurrently)
 * ------------------------------------------------------------------------- */
static void vWorkerTask(void *pvParameters) {
    uintptr_t taskID = (uintptr_t)pvParameters;

    for (;;) {
        if (xSemaphoreTake(xResourcePool, portMAX_DELAY) == pdTRUE) {
            gActiveResources++;
            if (gActiveResources > gMaxObservedResources) {
                gMaxObservedResources = gActiveResources;
            }

            gTaskUsingResource[taskID] = 1;
            gAcquireCount[taskID]++;

            GPIO_PORTF_DATA_R = (gActiveResources == 1U) ? PF2_BLUE_LED : PF3_GREEN_LED;

            vTaskDelay(pdMS_TO_TICKS(250 + (taskID * 120U)));

            gTaskUsingResource[taskID] = 0;
            gReleaseCount[taskID]++;
            gActiveResources--;

            xSemaphoreGive(xResourcePool);
        }

        GPIO_PORTF_DATA_R = PF1_RED_LED;
        vTaskDelay(pdMS_TO_TICKS(100 + (taskID * 20U)));
    }
}

/* -------------------------------------------------------------------------
 * System Integration
 * ------------------------------------------------------------------------- */
int main(void) {
    PortF_Init();
    GPIO_PORTF_DATA_R = 0;

    xResourcePool = xSemaphoreCreateCounting(2, 2);

    if (xResourcePool != NULL) {
        xTaskCreate(vWorkerTask, "Worker0", configMINIMAL_STACK_SIZE, (void *)0, tskIDLE_PRIORITY + 1, NULL);
        xTaskCreate(vWorkerTask, "Worker1", configMINIMAL_STACK_SIZE, (void *)1, tskIDLE_PRIORITY + 1, NULL);
        xTaskCreate(vWorkerTask, "Worker2", configMINIMAL_STACK_SIZE, (void *)2, tskIDLE_PRIORITY + 1, NULL);
        xTaskCreate(vWorkerTask, "Worker3", configMINIMAL_STACK_SIZE, (void *)3, tskIDLE_PRIORITY + 1, NULL);
        xTaskCreate(vWorkerTask, "Worker4", configMINIMAL_STACK_SIZE, (void *)4, tskIDLE_PRIORITY + 1, NULL);

        vTaskStartScheduler();
    }

    for (;;);
}