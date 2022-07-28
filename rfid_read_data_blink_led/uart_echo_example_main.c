/* UART Echo Example
*/

// kütüphaneler

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/gpio.h"



//tanımlamalar

#define ECHO_TEST_TXD (GPIO_NUM_1)
#define ECHO_TEST_RXD (GPIO_NUM_3)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (UART_NUM_0)
#define ECHO_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)
#define BUF_SIZE (1024)

#define RED_LED (GPIO_NUM_5);
#define BLUE_LED (GPIO_NUM_6);

void uart_config(void)
{
   
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

}

void led_config(void){

    gpio_reset_pin(RED_LED);
    gpio_set_direction(RED_LED, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BLUE_LED);
    gpio_set_direction(BLUE_LED, GPIO_MODE_OUTPUT);

}


static void echo_task(void *arg)
{

    led_config();
    uart_config();

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        // Write data back to the UART
        //uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, len);
        if(data==1){
            gpio_set_level(BLUE_LED, 1);
            gpio_set_level(RED_LED, 0);
        }
        else{
            gpio_set_level(BLUE_LED, 0);
            gpio_set_level(RED_LED, 1);
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    xTaskCreate(echo_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}
