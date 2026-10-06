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
//Needed for Deadlock tests
#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define SIDE_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define SIDE_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

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

void test_deadlock_2_threads(void)
{
    //Create semaphores
    SemaphoreHandle_t first_sem = xSemaphoreCreateBinary();
    SemaphoreHandle_t second_sem = xSemaphoreCreateBinary();
    //Give both threads their semaphores so each can run
    xSemaphoreGive(first_sem);
    xSemaphoreGive(second_sem);
    //Run both functions
    //deadlock_2_threads(first, second, 0);
    //deadlock_2_threads(second, first, 0);
    TaskHandle_t first_t, second_t;
    struct deadlock_args first_args = {first_sem, second_sem, 0};
    struct deadlock_args second_args = {second_sem, first_sem, 0};
    xTaskCreate(deadlock_2_threads, "FirstThread", MAIN_TASK_STACK_SIZE, (void *)&first_args, MAIN_TASK_PRIORITY, &first_t);
    xTaskCreate(deadlock_2_threads, "SecondThread", SIDE_TASK_STACK_SIZE, (void *)&second_args, SIDE_TASK_PRIORITY, &second_t);
    //Wait a little to make sure they are deadlocked
    vTaskDelay(100);
    //suspend the tasks so we can check them
    vTaskSuspend(first_t);
    vTaskSuspend(second_t);
    //Check that neither task gave back its semaphore
    int result_first = xSemaphoreTake(first_sem, 0);
    int result_second = xSemaphoreTake(second_sem, 0);
    TEST_ASSERT_EQUAL_INT(result_first, pdFALSE);
    TEST_ASSERT_EQUAL_INT(result_second, pdFALSE);
}

void rest_orphaned_lock(void)
{
    // create semaphore
    SemaphoreHandle_t sem = xSemaphoreCreateBinary();
    // Give the semaphore so it can run:
    xSemaphoreGive(sem);
    TaskHandle_t task;
    struct deadlock_args args = {sem, NULL, 0};
    xTaskCreate(orphaned_lock, "Orphan_thread", MAIN_TASK_STACK_SIZE, (void *)&args, MAIN_TASK_PRIORITY, &task);
    //Wait a little to make sure it is deadlocked
    vTaskDelay(100);
    //suspend the task so we can check
    vTaskSuspend(task);
    // Check status and results of task
    int result = xSemaphoreTake(sem, 0);
    TEST_ASSERT_EQUAL_INT(result, pdFALSE);
    TEST_ASSERT_EQUAL_INT(args.counter, 2); // Check that the function did indeed deadlock on count==2
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
