#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"

#define PF1_RED_LED             0x02U
#define PF2_BLUE_LED            0x04U
#define PF3_GREEN_LED           0x08U
#define PF4_SWITCH              0x10U

#define USE_ISR_YIELD           1U
#define POST_INTERRUPT_BUSY_SPIN 600000U

static TaskHandle_t g_highTaskHandle = NULL;

volatile uint32_t gInterruptCount = 0;
volatile uint32_t gPendSvRequests = 0;
volatile uint32_t gHighTaskRuns = 0;
volatile uint32_t gLowTaskIterations = 0;
volatile uint32_t gLastIsrTick = 0;
volatile uint32_t gLastHighTaskTick = 0;
volatile uint32_t gObservedLatencyTicks = 0;
volatile uint32_t gTriggerSpinProgress = 0;
volatile uint32_t gSpinProgressAtHighTask = 0;

static void PortF_Init(void)
{
    SYSCTL_RCGCGPIO_R |= 0x20U;
    while ((SYSCTL_PRGPIO_R & 0x20U) == 0U)
    {
    }

    GPIO_PORTF_DIR_R &= ~PF4_SWITCH;
    GPIO_PORTF_DIR_R |= (PF1_RED_LED | PF2_BLUE_LED | PF3_GREEN_LED);
    GPIO_PORTF_DEN_R |= (PF1_RED_LED | PF2_BLUE_LED | PF3_GREEN_LED | PF4_SWITCH);
    GPIO_PORTF_PUR_R |= PF4_SWITCH;

    GPIO_PORTF_IM_R &= ~PF4_SWITCH;
    GPIO_PORTF_IS_R &= ~PF4_SWITCH;
    GPIO_PORTF_IBE_R &= ~PF4_SWITCH;
    GPIO_PORTF_IEV_R &= ~PF4_SWITCH;
    GPIO_PORTF_ICR_R = PF4_SWITCH;
    GPIO_PORTF_IM_R |= PF4_SWITCH;

    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFFU) | 0x00A00000U;
    NVIC_EN0_R = (1UL << 30);
}

void GPIOF_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIO_PORTF_ICR_R = PF4_SWITCH;
    gInterruptCount++;
    gLastIsrTick = xTaskGetTickCountFromISR();
    GPIO_PORTF_DATA_R ^= PF2_BLUE_LED;

    if (g_highTaskHandle != NULL)
    {
        vTaskNotifyGiveFromISR(g_highTaskHandle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE)
        {
            gPendSvRequests++;
        }
    }

#if USE_ISR_YIELD
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
}

static void vHighPriorityTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0U)
        {
            gHighTaskRuns++;
            gLastHighTaskTick = xTaskGetTickCount();
            gObservedLatencyTicks = gLastHighTaskTick - gLastIsrTick;
            gSpinProgressAtHighTask = gTriggerSpinProgress;
            GPIO_PORTF_DATA_R ^= PF1_RED_LED;
        }
    }
}

static void vLowPriorityTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        gLowTaskIterations++;
        GPIO_PORTF_DATA_R ^= PF3_GREEN_LED;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void vTriggerTask(void *pvParameters)
{
    volatile uint32_t spinCount;

    (void)pvParameters;

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        gTriggerSpinProgress = 0;
        NVIC_SW_TRIG_R = 30;

        for (spinCount = 0; spinCount < POST_INTERRUPT_BUSY_SPIN; ++spinCount)
        {
            gTriggerSpinProgress = spinCount;
        }
    }
}

int main(void)
{
    PortF_Init();
    GPIO_PORTF_DATA_R &= ~(PF1_RED_LED | PF2_BLUE_LED | PF3_GREEN_LED);

    xTaskCreate(vHighPriorityTask,
                "HighTask",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY + 3,
                &g_highTaskHandle);

    xTaskCreate(vLowPriorityTask,
                "LowTask",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY + 1,
                NULL);

    xTaskCreate(vTriggerTask,
                "Trigger",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY + 2,
                NULL);

    vTaskStartScheduler();

    for (;;)
    {
    }
}