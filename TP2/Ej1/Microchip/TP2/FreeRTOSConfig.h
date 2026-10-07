#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H



/*-----------------------------------------------------------
 * Hardware
 *-----------------------------------------------------------*/

#define configCPU_CLOCK_HZ               16000000UL 
#define configTICK_RATE_HZ               62  

/*-----------------------------------------------------------
 * Scheduler
 *-----------------------------------------------------------*/

#define configMAX_PRIORITIES             3 //Cantidad maxima de prioridades (Comenzando desde el 0)
#define configMINIMAL_STACK_SIZE         128

#define configUSE_PREEMPTION             1 //Hablita preemptive, una tarea de mayor prioridad puede obtener el uso de la CPU si otra de menor prioridad se esta ejecutando.
#define configUSE_IDLE_HOOK              0 //FreeRTOS crea automáticamente una tarea llamada Idle, de prioridad 0. Puede ejecutarse cuando no hay tareas de mayor prioridad listas.
#define configUSE_TICK_HOOK              0 //Función adicional en cada tick

/*-----------------------------------------------------------
 * Memoria
 *-----------------------------------------------------------*/

#define configSUPPORT_DYNAMIC_ALLOCATION  1
#define configSUPPORT_STATIC_ALLOCATION   0
#define configTOTAL_HEAP_SIZE     1048 

/*-----------------------------------------------------------
 * Tasks
 *-----------------------------------------------------------*/

#define configUSE_MUTEXES                 1 //Permite uso de mutex
#define configUSE_TIME_SLICING 1


/*-----------------------------------------------------------
 * Tick type
 *-----------------------------------------------------------*/

#define configTICK_TYPE_WIDTH_IN_BITS     TICK_TYPE_WIDTH_16_BITS //Esta opción cambia el rango del contador

/*-----------------------------------------------------------
 * Inclusion de funciones de la API de FreeRTOS
 *-----------------------------------------------------------*/

#define INCLUDE_vTaskDelay                1
#define INCLUDE_vTaskDelete               1
#define INCLUDE_vTaskSuspend              1


#endif