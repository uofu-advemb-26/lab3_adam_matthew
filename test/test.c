#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include "portmacro.h"
#include <task.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "functions.h"

#define TEST_RUNNER_PRIORITY ( tskIDLE_PRIORITY + 5UL ) // the base task has the highest priority

void setUp(void) {}

void tearDown(void) {}

void test_main_lock_and_logic(void)
{

    // initialize global variables for a particular test case
    int counter = 0;
    int on = 1;

    // create sempaphores
    SemaphoreHandle_t MainTurn = xSemaphoreCreateBinary();
    SemaphoreHandle_t SideTurn = xSemaphoreCreateBinary();

    // give the main task its semaphore and test whether it runs
    xSemaphoreGive(MainTurn);

    do_main_logic(&counter, &on, MainTurn, SideTurn, 0);
    
    // verify that main task ran
    int result_main_taken = xSemaphoreTake(MainTurn, 0); // if it ran, it should've taken its semaphore
    int result_side_given = xSemaphoreTake(SideTurn, 0); // if it ran, it should've given the side task its semaphore

    TEST_ASSERT_EQUAL_INT(result_main_taken, pdFALSE);
    TEST_ASSERT_EQUAL_INT(result_side_given, pdTRUE);
    TEST_ASSERT_EQUAL_INT(counter, 1); // if it ran, it should've incremented the counter by 1
    TEST_ASSERT_EQUAL_INT(on, 0); // if it ran, it should've toggled the 'on' variable

    // reset the main task and make sure it doesn't run if not given its semaphore
    counter = 0; 
    on = 1;
    xSemaphoreTake(MainTurn,0);

    do_main_logic(&counter, &on, MainTurn, SideTurn, 0);
    
    // verify that main task did not run
    result_side_given = xSemaphoreTake(SideTurn, 0); // if didn't run, it should not given the side task its semaphore


    TEST_ASSERT_EQUAL_INT(result_side_given, pdFALSE);
    TEST_ASSERT_EQUAL_INT(counter, 0); // if it didn't run, it should not have incremented the counter by 1
    TEST_ASSERT_EQUAL_INT(on, 1); // if it didn't run, it should not have toggled the 'on' variable

}

void test_side_lock_and_logic(void)
{

    // initialize global variables for a particular test case
    int counter = 0;
    int on = 1;

    // create sempaphores
    SemaphoreHandle_t MainTurn = xSemaphoreCreateBinary();
    SemaphoreHandle_t SideTurn = xSemaphoreCreateBinary();

    // give the side task its semaphore and test whether it runs
    xSemaphoreGive(SideTurn);

    do_side_logic(&counter, &on, MainTurn, SideTurn, 0);
    
    // verify that side task ran
    int result_side_taken = xSemaphoreTake(SideTurn, 0); // if it ran, it should've taken its semaphore
    int result_main_given = xSemaphoreTake(MainTurn, 0); // if it ran, it should've given the main task its semaphore

    TEST_ASSERT_EQUAL_INT(result_side_taken, pdFALSE);
    TEST_ASSERT_EQUAL_INT(result_main_given, pdTRUE);
    TEST_ASSERT_EQUAL_INT(counter, 1); // if it ran, it should've incremented the counter by 1

    // reset the side task and make sure it doesn't run if not given its semaphore
    counter = 0; 
    on = 1;
    xSemaphoreTake(SideTurn,0);

    do_side_logic(&counter, &on, MainTurn, SideTurn, 0);
    
    // verify that side task did not run
    result_main_given = xSemaphoreTake(SideTurn, 0); // if didn't run, it should not have given the main task its semaphore


    TEST_ASSERT_EQUAL_INT(result_main_given, pdFALSE);
    TEST_ASSERT_EQUAL_INT(counter, 0); // if it didn't run, it should not have incremented the counter by 1

}

void runner_task(__unused void *args)
{
    while (1) {
        sleep_ms(5000); // Give time for TTY to attach.
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_main_lock_and_logic);
        RUN_TEST(test_side_lock_and_logic);
        sleep_ms(5000);
        UNITY_END();
    }
}

int main (void)
{
    stdio_init_all();

    hard_assert(cyw43_arch_init() == PICO_OK);

    xTaskCreate(runner_task, "TestRunner",
                configMINIMAL_STACK_SIZE, NULL, TEST_RUNNER_PRIORITY, NULL);

    vTaskStartScheduler();
}
