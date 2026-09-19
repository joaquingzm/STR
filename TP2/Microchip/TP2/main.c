#define F_CPU 16000000UL 
#include <avr/io.h>
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"

	
	/* 
	-----MODELO DE TAREA EN FreeRTOS-----
	
	xTaskCreate(
		TareaUART,  --> Nombre de la funcion que implementa la tarea
		"UART",     --> Nombre identificador de la tarea
		128,        --> Tamaño de la stack de tipo StackType_t, el cual depende del tamaño de palabra de la arquitectura
		NULL,       --> Puntero a un parametro que se quiera enviar a la Task, cada implementacion de task recibe un void * pvParameters el cual puede castearse a otro tipo de puntero dependiendo lo que se envie. 
					   Por ejemplo si quisiesemos hacer tareas para Uart y cada una con distinto baudrate, podria crear tantas tareas como necesito y enviarle a cada una la configuracion de baudrate correspondiente
		2,		    --> Prioridad de la tarea para ser seleccionada por scheduler. Una Task puede estar en: 
					    READY (Lista para ejecutar pero el schedular esta ejecutando otra), RUNNING (Ejecutandose), BLOCKED (Se bloqueo esperando algo como semaforo, cola, evento,etc), SUSPENDED (Se duerme con vTaskSuspend(handleTarea), se despierta con vTaskResume(handleTarea) 
		&handleUART --> De tipo TaskHandle_t. Es el identificador de la tarea. Se utiliza para poder suspender/despertar a una tarea en especifico
	); 
	
	-----FUNCIONES PARA EL MANEJO DE TASKS-----
	- xTaskCreate(Parametros)		   --> Crea la tarea con los parametros explicados dejandola lista para el scheduler y su ejecucion
	- xTaskCreateStatic(Parametros de xTaskCreate, &stack, &tcbTarea) --> A diferencia de xTaskCreate que aloca de manera dinamica el espacio para la stack y su TCB (Task Control Block)
																	      De esta forma se le puede indicar el espacio de memoria de la stack y de la tcb mediante variables globales
	- vTaskDelete(handle)			   --> Elimina la tarea identificada por handle. vTaskDelete(NULL) permite a una tarea eliminarse a si misma. Util para tareas con trabajos puntuales (Inicializadores).
	- vTaskSuspend(handle)			   --> Suspende la tarea
	- vTaskResume(handle)			   --> La tarea puede volver a ser seleccionada por el scheduler
	- vTaskDelay(pdMS_TO_TICKS(1000)); --> Bloquea la tarea por 1 segundo en este caso
	- xQueueReceive(cola)		       --> Se bloquea esperando datos en la cola. Esto evita hacer polling constantemente para esperar un dato
	- vTaskPrioritySet(handle, 3);     --> Cambia la prioridad de la tarea
	- uxTaskPriorityGet(handleUART);   --> Obtener la prioridad de la tarea
	- taskYIELD()					   --> La tarea ejecutando esto le dice al scheduler que esta a dispuesta a ceder la CPU. No es tan comun por el uso de bloqueos.
	- vTaskStartScheduler();		   --> Arranca el Scheduler
	
	*/
	
/* -------------------------------------------------
 * Configuración
 * ------------------------------------------------- */

#define TASK_STACK_SIZE    128
#define TASK_PRIORITY      1


/* -------------------------------------------------
 * Tarea 1
 * ------------------------------------------------- */

void Task1(void *pvParameters)
{
    while (1)
    {
        UART_SendString("Task 1\r\n");

        //vTaskDelay(pdMS_TO_TICKS(1000/portTICK_PERIOD_MS));
    }
}


/* -------------------------------------------------
 * Tarea 2
 * ------------------------------------------------- */

void Task2(void *pvParameters)
{
    while (1)
    {
        UART_SendString("Task 2\r\n");

        //vTaskDelay(pdMS_TO_TICKS(2000/portTICK_PERIOD_MS));
    }
}


/* -------------------------------------------------
 * Tarea 3
 * ------------------------------------------------- */

void Task3(void *pvParameters)
{
    while (1)
    {
        UART_SendString("Task 3\r\n");

        //vTaskDelay(pdMS_TO_TICKS(3000/portTICK_PERIOD_MS));
    }
}


/* -------------------------------------------------
 * Main
 * ------------------------------------------------- */

int main(void)
{
    UART_Init(9600);

    xTaskCreate(
        Task1,
        "Task1",
        TASK_STACK_SIZE,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        Task2,
        "Task2",
        TASK_STACK_SIZE,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
        Task3,
        "Task3",
        TASK_STACK_SIZE,
        NULL,
        3,
        NULL
    );

    vTaskStartScheduler();

    while (1)
    {
    }

    return 0;
}
   

