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

void deadlock_2_threads(void* vanilla_args)
{
    struct deadlock_args *args = (struct deadlock_args *)vanilla_args;
    if (xSemaphoreTake(args->first, args->wait_ticks))
            {
                vTaskDelay(100);
                printf("hello world from %s!", "deadlock");
                xSemaphoreGive(args->second);
            }
}

void orphaned_lock(void* vanilla_args)
{
    struct deadlock_args *args = (struct deadlock_args *)vanilla_args;
    args->counter++;
    while (1) {
        xSemaphoreTake(args->first, args->wait_ticks);
        args->counter++;
        if (args->counter % 2) {
            continue;
        }
        printf("Count %d\n", args->counter);
        xSemaphoreGive(&args->first);
    }
}
void orphaned_lock_fixed(void* vanilla_args)
{
    struct deadlock_args *args = (struct deadlock_args *)vanilla_args;
    args->counter++;
    while (1) {
        xSemaphoreTake(args->first, args->wait_ticks);
        args->counter++;
        if ((args->counter % 2) == 0) {
            printf("Count %d\n", args->counter);
        }
        xSemaphoreGive(&args->first);
    }
}
