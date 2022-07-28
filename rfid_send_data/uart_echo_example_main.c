/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "rc522.h"


/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define ECHO_TEST_TXD (GPIO_NUM_1)
#define ECHO_TEST_RXD (GPIO_NUM_3)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (UART_NUM_0)
#define ECHO_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

#define BUF_SIZE (1024)

void uart_config(void){

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

int data_state = 0 ;
uint8_t sn_num [5];
void tag_handler(uint8_t* sn) { // serial number is always 5 bytes long
    /*ESP_LOGI(TAG, "Tag: %#x %#x %#x %#x %#x",
        sn[0], sn[1], sn[2], sn[3], sn[4]
    );*/
    uint8_t card_sn[5] = {0x9d,0x8c,0x6b,0x5a,0x6e};
    for(int i=0;i<=4;i++){
        sn_num[i]=sn[i];
        if(card_sn[i] == sn_num[i]){
            data_state ++;
        }
    }
}


static void echo_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    int len=32;
    if (data_state==5){
        uart_write_bytes(ECHO_UART_PORT_NUM, 1, len);
    }
    else {
        uart_write_bytes(ECHO_UART_PORT_NUM, 0, len);
    }




    // Configure a temporary buffer for the incoming data
    //uint8_t *data = (uint8_t *) malloc(BUF_SIZE);


    /*while (1) {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        // Write data back to the UART
        uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, len);
    }*/
}

void app_main(void)
{   
        const rc522_start_args_t start_args = {
        .miso_io  = 25,
        .mosi_io  = 23,
        .sck_io   = 19,
        .sda_io   = 22,
        .callback = &tag_handler,

        // Uncomment next line for attaching RC522 to SPI2 bus. Default is VSPI_HOST (SPI3)
        //.spi_host_id = HSPI_HOST
    };
    uart_config();
    xTaskCreate(echo_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
    rc522_start(start_args);
}
