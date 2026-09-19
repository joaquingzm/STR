#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Hardware
 *-----------------------------------------------------------*/

#define configCPU_CLOCK_HZ              ( 16000000UL )
#define configTICK_RATE_HZ              ( 1000U )

/*-----------------------------------------------------------
 * Scheduler
 *-----------------------------------------------------------*/

#define configMAX_PRIORITIES             3
#define configMINIMAL_STACK_SIZE         128

#define configUSE_PREEMPTION             1
#define configUSE_IDLE_HOOK               0
#define configUSE_TICK_HOOK               0

/*-----------------------------------------------------------
 * Memory
 *-----------------------------------------------------------*/

#define configSUPPORT_DYNAMIC_ALLOCATION  1
#define configSUPPORT_STATIC_ALLOCATION   0
#define configTOTAL_HEAP_SIZE    ( 1048 )

/*-----------------------------------------------------------
 * Tasks
 *-----------------------------------------------------------*/

#define configUSE_MUTEXES                 1

/*-----------------------------------------------------------
 * Tick type
 *-----------------------------------------------------------*/

#define configTICK_TYPE_WIDTH_IN_BITS     TICK_TYPE_WIDTH_16_BITS

/*-----------------------------------------------------------
 * FreeRTOS API inclusion
 *-----------------------------------------------------------*/

#define INCLUDE_vTaskDelay                1
#define INCLUDE_vTaskDelete               1
#define INCLUDE_vTaskSuspend              1

#define portUSE_TIMER0 1

#endif