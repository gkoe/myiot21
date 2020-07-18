#include <stdio.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <driver/adc.h>

/**
 * This is an example which echos any data it receives on UART1 back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: UART2
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */

#define ECHO_TEST_TXD (GPIO_NUM_17)
#define ECHO_TEST_RXD (GPIO_NUM_16)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024)

extern "C"
{
  void app_main(void);
}

/**
 * Ermittelt alle Sekunden die aktuelle Leistung.
 */
void echo_task(void *pvParameter)
{
  /* Configure parameters of an UART driver,
     * communication pins and install the driver */
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 120};
  uart_param_config(UART_NUM_2, &uart_config);
  uart_set_pin(UART_NUM_2, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
  uart_driver_install(UART_NUM_2, BUF_SIZE * 2, 0, 0, NULL, 0);

  // Configure a temporary buffer for the incoming data
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

  const char text[] = "Hello Uart!\n";
  char receivedText[200];

  while (1)
  {
    // Read data from the UART
    // int len = uart_read_bytes(UART_NUM_2, data, BUF_SIZE, 20 / portTICK_RATE_MS);
    int len = uart_read_bytes(UART_NUM_2, (uint8_t *)receivedText, 200, 20 / portTICK_RATE_MS);
    if (len > 0)
    {
      receivedText[len] = 0;
      printf("read %d bytes from uart2: %s\n", len, receivedText);
      uart_write_bytes(UART_NUM_2, receivedText, len);
    }
    // Write data back to the UART
    // uart_write_bytes(UART_NUM_2, text, 12);
    // uart_write_bytes(UART_NUM_2, (const char *) data, len);

    vTaskDelay(10);
  }
}

void app_main()
{
  printf("=========\n");
  printf("UART-Test\n");
  printf("=========\n");

  xTaskCreate(echo_task,   /* Task function. */
              "echo_task", /* String with name of task. */
              4096,        /* Stack size in words. */
              NULL,        /* Parameter passed as input of the task */
              1,           /* Priority of the task. */
              NULL         /* Task handle. */
  );
  while (true)
  {
    // printf("Loop\n");
    vTaskDelay(100);
  }
}
