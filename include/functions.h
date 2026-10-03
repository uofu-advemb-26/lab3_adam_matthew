#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>


void do_main_logic(int *counter, int *on, SemaphoreHandle_t main, SemaphoreHandle_t side, TickType_t wait_ticks);
void do_side_logic(int *counter, int *on, SemaphoreHandle_t main, SemaphoreHandle_t side, TickType_t wait_ticks);
struct deadlock_args {
    SemaphoreHandle_t first,
    SemaphoreHandle_t second,
    TickType_t wait_ticks
};
