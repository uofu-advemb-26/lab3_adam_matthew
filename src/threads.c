#include "portmacro.h"
#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "functions.h"

#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

#define SIDE_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define SIDE_TASK_STACK_SIZE configMINIMAL_STACK_SIZE


SemaphoreHandle_t MainTurn;
SemaphoreHandle_t SideTurn;

int counter_global;
int on_global;


void side_thread(void *params)
{
    while(1)
    { 
        do_side_logic(&counter_global, &on_global, MainTurn, SideTurn, portMAX_DELAY);
    } 
	
}

void main_thread(void *params)
{
    while(1)
    {
        do_main_logic(&counter_global, &on_global, MainTurn, SideTurn, portMAX_DELAY);
    } 
	
}


int main(void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    on_global = false;
    counter_global = 0;
    TaskHandle_t main, side;
    MainTurn = xSemaphoreCreateBinary();
    SideTurn = xSemaphoreCreateBinary();

    xSemaphoreGive(MainTurn);

    xTaskCreate(main_thread, "MainThread",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &main);
    xTaskCreate(side_thread, "SideThread",
                SIDE_TASK_STACK_SIZE, NULL, SIDE_TASK_PRIORITY, &side);
    vTaskStartScheduler();

	return 0;
}
