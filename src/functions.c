#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "portmacro.h"
#include <task.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "functions.h"

void do_main_logic(int *counter, int *on, SemaphoreHandle_t main, SemaphoreHandle_t side, TickType_t wait_ticks)
{
     if (xSemaphoreTake(main, wait_ticks))
            {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, *on);
            vTaskDelay(100);
            (*counter)++;
            printf("hello world from %s! Count %d\n", "main", *counter);
            *on = !(*on); 
            xSemaphoreGive(side);
            }
}

void do_side_logic(int *counter, int *on, SemaphoreHandle_t main, SemaphoreHandle_t side, TickType_t wait_ticks)
{
    if (xSemaphoreTake(side, wait_ticks))
            { 
                vTaskDelay(100);
                (*counter)++;
                printf("hello world from %s! Count %d\n", "thread", *counter);
                xSemaphoreGive(main);
            }
}
