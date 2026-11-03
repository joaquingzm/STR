/* Pr?ctica 3: esqueleto inicial.
 * Las tareas, la inicializaci?n de UART y el arranque del scheduler se
 * agregar?n cuando se defina la l?gica del ejercicio.
 */




/*

POR AHORA NO LE VOY A DAR BOLA A LA IMPLEMENTACION DE ROBALDI DEL TIMER1 EN 
LA LIBRERIA DE FREERTOS PORQUE TOTAL NUNCA DA 15MS EXACTOS JIJU

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include <util/delay.h>
#include <stdio.h>
#include "uart.h"

#define tick_ms 16 
#define reporte_ticks 192 // 3072 ms
#define unidad_tiempo_tick 4

typedef struct {
	uint16_t activaciones;
	uint16_t plazos_perdidos;
	TickType_t r_max;       // Tiempo de respuesta máximo (en ticks)
	TickType_t jitter_max;  // Jitter máximo de activación (en ticks)
} MetricasTarea;

// Instancias globales para cada tarea
volatile MetricasTarea metricaA = {0, 0, 0, 0};
volatile MetricasTarea metricaB = {0, 0, 0, 0};
volatile MetricasTarea metricaC = {0, 0, 0, 0};
volatile MetricasTarea metricaD = {0, 0, 0, 0};

#define EJ1

#if defined(EJ1) 

	#define A_C 2
	#define A_PD 12
	
	#define B_C 2
	#define B_PD 24
	
	#define C_C 2
	#define C_PD 6
	
	#define D_C 1
	#define D_PD 3

#elif defined(EJ2)

	#define A_C 1
	#define A_PD 10
	
	#define B_C 3
	#define B_PD 12
	
	#define C_C 7
	#define C_PD 20
	
	#define D_C 1
	#define D_PD 5

#endif

void TareaA(void *parametros);
void TareaB(void *parametros);
void TareaC(void *parametros);
void TareaD(void *parametros);

void ejercicio3(void);
void ejercicio4(void);
void TareaTerminal(void *parametros);
void ActualizarMetricas(volatile MetricasTarea *m, TickType_t t_teorico, TickType_t t_inicio, TickType_t t_fin, TickType_t plazo_ticks);

int main(void)
{
	
	// Configurar PB0, PB1, PB2 y PB3 como salidas (1 en DDRB)
	DDRB |= (1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3);

	// Garantizar estado inicial BAJO en todos los pines (0 en PORTB)
	PORTB &= ~((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3)); 
	
	UART_Init(9600);
	ejercicio4();
	xTaskCreate(TareaTerminal, "Terminal", 240, NULL, 0, NULL);
	

	vTaskStartScheduler();

    while(1){}
}

void ejercicio3(void)
{
	xTaskCreate(
	TareaD,            // 1. Nombre de la función de la tarea
	"TareaD",                // 2. Nombre descriptivo (texto corto para depurar)
	configMINIMAL_STACK_SIZE, // 3. Tamaño de la pila (Stack) en bytes/words
	NULL,                     // 4. Parámetro pasado a la tarea (NULL si no usas nada)
	1,                        // 5. Prioridad de la tarea (0 a configMAX_PRIORITIES-1)
	NULL                      // 6. Manejador/Handle de la tarea (NULL si no necesitas manipularla luego)
	);
		
}

void ejercicio4(void)
{
	xTaskCreate(
	TareaA,            // 1. Nombre de la función de la tarea
	"TareaA",                // 2. Nombre descriptivo (texto corto para depurar)
	configMINIMAL_STACK_SIZE, // 3. Tamaño de la pila (Stack) en bytes/words
	NULL,                     // 4. Parámetro pasado a la tarea (NULL si no usas nada)
	1,                        // 5. Prioridad de la tarea (0 a configMAX_PRIORITIES-1)
	NULL                      // 6. Manejador/Handle de la tarea (NULL si no necesitas manipularla luego)
	);
	
	xTaskCreate(
	TareaB,            // 1. Nombre de la función de la tarea
	"TareaB",                // 2. Nombre descriptivo (texto corto para depurar)
	configMINIMAL_STACK_SIZE, // 3. Tamaño de la pila (Stack) en bytes/words
	NULL,                     // 4. Parámetro pasado a la tarea (NULL si no usas nada)
	1,                        // 5. Prioridad de la tarea (0 a configMAX_PRIORITIES-1)
	NULL                      // 6. Manejador/Handle de la tarea (NULL si no necesitas manipularla luego)
	);
	
	xTaskCreate(
	TareaC,            // 1. Nombre de la función de la tarea
	"TareaC",                // 2. Nombre descriptivo (texto corto para depurar)
	configMINIMAL_STACK_SIZE, // 3. Tamaño de la pila (Stack) en bytes/words
	NULL,                     // 4. Parámetro pasado a la tarea (NULL si no usas nada)
	1,                        // 5. Prioridad de la tarea (0 a configMAX_PRIORITIES-1)
	NULL                      // 6. Manejador/Handle de la tarea (NULL si no necesitas manipularla luego)
	);
	
	xTaskCreate(
	TareaD,            // 1. Nombre de la función de la tarea
	"TareaD",                // 2. Nombre descriptivo (texto corto para depurar)
	configMINIMAL_STACK_SIZE, // 3. Tamaño de la pila (Stack) en bytes/words
	NULL,                     // 4. Parámetro pasado a la tarea (NULL si no usas nada)
	1,                        // 5. Prioridad de la tarea (0 a configMAX_PRIORITIES-1)
	NULL                      // 6. Manejador/Handle de la tarea (NULL si no necesitas manipularla luego)
	);
}

void TareaA(void *parametros){
	TickType_t ult_llamado = 0; // Origen comun para las cuatro tareas.
	while(1){
		TickType_t inicio = xTaskGetTickCount();
		metricaA.activaciones++;
		PORTB |= (1<<PORTB0);
		_delay_ms(A_C*unidad_tiempo_tick*tick_ms);
		PORTB &= ~(1 << PORTB0);
		ActualizarMetricas(&metricaA, ult_llamado, inicio, xTaskGetTickCount(), A_PD * unidad_tiempo_tick);
		vTaskDelayUntil(&ult_llamado, A_PD*unidad_tiempo_tick);
	}
}

void TareaB(void *parametros){
	TickType_t ult_llamado = 0; // Origen comun para las cuatro tareas.
	
	while(1){
		TickType_t inicio = xTaskGetTickCount();
		metricaB.activaciones++;
		PORTB |= (1 << PORTB1);
		_delay_ms(B_C * unidad_tiempo_tick * tick_ms);
		PORTB &= ~(1 << PORTB1);
		ActualizarMetricas(&metricaB, ult_llamado, inicio, xTaskGetTickCount(), B_PD * unidad_tiempo_tick);
		vTaskDelayUntil(&ult_llamado, B_PD * unidad_tiempo_tick);
	}
}

void TareaC(void *parametros){
	TickType_t ult_llamado = 0; // Origen comun para las cuatro tareas.
	
	while(1){
		TickType_t inicio = xTaskGetTickCount();
		metricaC.activaciones++;
		PORTB |= (1 << PORTB2);
		_delay_ms(C_C * unidad_tiempo_tick * tick_ms);
		PORTB &= ~(1 << PORTB2);
		ActualizarMetricas(&metricaC, ult_llamado, inicio, xTaskGetTickCount(), C_PD * unidad_tiempo_tick);
		vTaskDelayUntil(&ult_llamado, C_PD * unidad_tiempo_tick);
	}
}

void TareaD(void *parametros){
	TickType_t ult_llamado = 0; // Origen comun para las cuatro tareas.
	
	while(1){
		TickType_t inicio = xTaskGetTickCount();
		metricaD.activaciones++;
		PORTB |= (1 << PORTB3);
		_delay_ms(D_C * unidad_tiempo_tick * tick_ms);
		PORTB &= ~(1 << PORTB3);
		ActualizarMetricas(&metricaD, ult_llamado, inicio, xTaskGetTickCount(), D_PD * unidad_tiempo_tick);
		vTaskDelayUntil(&ult_llamado, D_PD * unidad_tiempo_tick);
	}
}

void TareaTerminal(void *parametros) {
	TickType_t ult_llamado = xTaskGetTickCount();
	static char buffer[80]; // Evita cargar el buffer en la pila de la tarea.

	for (;;) {
		// Enviar reporte cada 192 ticks = 3072 ms.
		vTaskDelayUntil(&ult_llamado, reporte_ticks);

		// Lectura segura de métricas
		taskENTER_CRITICAL();
		MetricasTarea mA = metricaA;
		MetricasTarea mB = metricaB;
		MetricasTarea mC = metricaC;
		MetricasTarea mD = metricaD;
		taskEXIT_CRITICAL();

		// Transmisión a la Virtual Terminal
		UART_SendString("\r\n=== REPORTE METRICAS RTOS ===\r\n");

		sprintf(buffer, "%c: Act=%u | Miss=%u | Rmax=%u ms | JitterInicioMax=%u ms\r\n",
		'A', mA.activaciones, mA.plazos_perdidos,
		(uint16_t)(mA.r_max * tick_ms), (uint16_t)(mA.jitter_max * tick_ms));
		UART_SendString(buffer);

		sprintf(buffer, "%c: Act=%u | Miss=%u | Rmax=%u ms | JitterInicioMax=%u ms\r\n",
		'B', mB.activaciones, mB.plazos_perdidos,
		(uint16_t)(mB.r_max * tick_ms), (uint16_t)(mB.jitter_max * tick_ms));
		UART_SendString(buffer);

		sprintf(buffer, "%c: Act=%u | Miss=%u | Rmax=%u ms | JitterInicioMax=%u ms\r\n",
		'C', mC.activaciones, mC.plazos_perdidos,
		(uint16_t)(mC.r_max * tick_ms), (uint16_t)(mC.jitter_max * tick_ms));
		UART_SendString(buffer);

		sprintf(buffer, "%c: Act=%u | Miss=%u | Rmax=%u ms | JitterInicioMax=%u ms\r\n",
		'D', mD.activaciones, mD.plazos_perdidos,
		(uint16_t)(mD.r_max * tick_ms), (uint16_t)(mD.jitter_max * tick_ms));
		UART_SendString(buffer);
	}
}


void ActualizarMetricas(volatile MetricasTarea *m, TickType_t t_teorico, TickType_t t_inicio, TickType_t t_fin, TickType_t plazo_ticks) {
	// Jitter de inicio: demora en conseguir CPU respecto al instante previsto.
	TickType_t jitter = (TickType_t)(t_inicio - t_teorico);
	if (jitter > m->jitter_max) {
		m->jitter_max = jitter;
	}

	// Tiempo de respuesta R (desde la liberación teórica hasta fin de ejecución)
	TickType_t r = t_fin - t_teorico;
	if (r > m->r_max) {
		m->r_max = r;
	}

	// Verificación de plazo perdido
	if (r > plazo_ticks) {
		m->plazos_perdidos++;
	}
}