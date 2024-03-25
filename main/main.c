#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

#define DEADZONE 30

QueueHandle_t xQueueAdc;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void x_task(void *p) {
    adc_init();
    adc_gpio_init(27);

    while (1) {
        adc_select_input(1);
        int result = (adc_read() - 2048)/8;
        if (abs(result) > DEADZONE){
           adc_t data = {0, result};
           xQueueSend(xQueueAdc, &data, portMAX_DELAY);
        }
        else {
            adc_t data = {0, 0};
            xQueueSend(xQueueAdc, &data, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void y_task(void *p) {
    adc_init();
    adc_gpio_init(26);

    while (1) {
        adc_select_input(0);
        int result = (adc_read() - 2048)/8;
        if (abs(result) > DEADZONE){
           adc_t data = {1, result};
           xQueueSend(xQueueAdc, &data, portMAX_DELAY);
        }
        else{
            adc_t data = {1, 0};
            xQueueSend(xQueueAdc, &data, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void uart_task(void *p) {
    adc_t data; 

    while (1) {       
        if(xQueueReceive(xQueueAdc, &data, portMAX_DELAY)){
            int val = data.val;
            int msb = val >> 8;
            int lsb = val & 0xFF ;

            uart_putc_raw(uart0, data.axis);
            uart_putc_raw(uart0, lsb);
            uart_putc_raw(uart0, msb);
            uart_putc_raw(uart0, -1);
        }
    }
}

int main() {
    stdio_init_all();
    printf("Começando!!!\n");
    adc_init();

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(x_task, "x_task", 4095, NULL, 1, NULL);
    xTaskCreate(y_task, "y_task", 4095, NULL, 1, NULL);
    xTaskCreate(uart_task, "uart_task", 4095, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
