#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
#define configCPU_CLOCK_HZ                 16000000UL
/* El port ATmega usa watchdog (~16 ms); no es un tick de 1 ms. */
#define configTICK_RATE_HZ                  62U
#define configTICK_TYPE_WIDTH_IN_BITS      TICK_TYPE_WIDTH_16_BITS
#define configUSE_PREEMPTION               1
#define configUSE_TIME_SLICING             1
#define configMAX_PRIORITIES               2 /* Idle=0; las tres tareas=1. */
#define configMINIMAL_STACK_SIZE           128
#define configMAX_TASK_NAME_LEN            8
#define configSUPPORT_DYNAMIC_ALLOCATION   1
#define configSUPPORT_STATIC_ALLOCATION    0
#define configTOTAL_HEAP_SIZE              1400
#define configUSE_IDLE_HOOK                0
#define configUSE_TICK_HOOK                0
#define configUSE_TIMERS                   0
#define configUSE_MUTEXES                  0 /* El ejercicio usa semaforos. */
#define configUSE_COUNTING_SEMAPHORES      0
#define configUSE_TASK_NOTIFICATIONS       1
#define INCLUDE_vTaskDelay                 1
#define INCLUDE_vTaskSuspend               1 /* Espera indefinida con portMAX_DELAY. */
#define INCLUDE_vTaskDelete                0
#endif
