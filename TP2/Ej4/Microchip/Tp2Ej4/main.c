/*
 * GUIA: SINCRONIZACION Y COMUNICACION EN FREERTOS
 *
 * Conceptos comunes:
 * - Handle: identificador que devuelve Create y se pasa a las demas funciones.
 * - Bloquear una tarea: dejarla dormida hasta un evento o un timeout. Mientras
 *   espera, el procesador puede ejecutar otras tareas; no hace polling.
 * - Los tiempos son ticks: 0=no esperar; portMAX_DELAY=esperar indefinidamente
 *   si INCLUDE_vTaskSuspend=1. En este AVR el watchdog da un tick de ~16 ms.
 * - Comprobar Create contra NULL y Take/Send/Receive contra pdTRUE o pdPASS,
 *   segun la API. Si vencio la espera, no se obtuvo el recurso o dato.
 * - En ISR usar solo las variantes ...FromISR disponibles, nunca bloquear,
 *   e indicar el cambio de contexto como corresponda al port.
 *
 * 1. MUTEX: USAR UN RECURSO DE A UNA TAREA (semphr.h)
 * Ejemplo: dos tareas imprimen por UART. Cada una toma el mutex antes de su
 * mensaje completo y lo devuelve al terminar, para no mezclar caracteres.
 *
 * SemaphoreHandle_t m = xSemaphoreCreateMutex(); // Nace disponible.
 * if (xSemaphoreTake(m, portMAX_DELAY) == pdTRUE) {
 *     imprimir_mensaje();
 *     xSemaphoreGive(m);
 * }
 * Take(m, espera): intenta obtener la exclusividad; espera si otra la tiene.
 * Give(m): devuelve la exclusividad. Debe hacerlo la misma tarea que tomo m.
 *
 * HERENCIA DE PRIORIDAD (no tiene relacion con herencia de clases):
 * Imagina tres tareas: Baja=1, Media=2, Alta=3.
 * Baja toma el mutex; Alta lo necesita y queda bloqueada esperando a Baja.
 * Sin herencia, Media podria ocupar la CPU e impedir que Baja termine:
 * indirectamente, Media estaria retrasando a Alta. Es inversion de prioridad.
 * Con un mutex FreeRTOS eleva temporalmente la prioridad de Baja a la de Alta,
 * para ayudarla a terminar y liberar el recurso. Luego Alta puede tomarlo.
 * En este ejemplo de un solo mutex, Baja recupera su prioridad al liberarlo.
 * Con varios mutex retenidos, la herencia simplificada puede conservar la
 * prioridad elevada hasta liberarlos todos. No evita por si sola un deadlock.
 * En nuestro ejercicio todas las tareas tienen prioridad 1: no hay diferencia
 * de prioridades que heredar y el orden se resuelve con semaforos de turno.
 *
 * Requiere configUSE_MUTEXES=1 y queue.c. No se usa desde ISR.
 * Mutex recursivo: una tarea puede tomarlo otra vez, por ejemplo si una funcion
 * llama a otra que protege el mismo recurso. Debe devolverlo tantas veces como
 * lo tomo. API: xSemaphoreCreateRecursiveMutex(),
 * xSemaphoreTakeRecursive(m, espera), xSemaphoreGiveRecursive(m).
 * Requiere configUSE_RECURSIVE_MUTEXES=1; no mezclar con Take/Give normales.
 *
 * 2. SEMAFORO BINARIO: DAR UN AVISO O UN TURNO (semphr.h)
 * Guarda 0 o 1 permisos. No tiene propietario ni herencia de prioridad:
 * una tarea puede dar el permiso y otra consumirlo.
 * s=xSemaphoreCreateBinary(); // Nace VACIO, a diferencia del mutex.
 * xSemaphoreGive(s); // Deja un permiso; falla si ya estaba lleno.
 * xSemaphoreTake(s,portMAX_DELAY); // Consume el permiso o espera que aparezca.
 * Take devuelve pdTRUE si lo obtuvo; si no, no corresponde seguir como si nada.
 * Dos Give seguidos no guardan dos avisos: para eso usar contador o cola.
 * Requiere queue.c. En este ejercicio cada tarea espera su propio semaforo.
 *
 * 3. SEMAFORO CONTADOR: CONTAR AVISOS O RECURSOS (semphr.h)
 * s=xSemaphoreCreateCounting(5, 5);
 * Primer parametro: maximo de permisos; segundo: cantidad inicial.
 * Con 5 iniciales representa cinco recursos libres. Take consume uno y Give
 * devuelve uno. Con 0 iniciales puede contar eventos pendientes de atender.
 * Take espera si esta en cero; Give falla si alcanzo el maximo.
 * Requiere configUSE_COUNTING_SEMAPHORES=1 y queue.c.
 *
 * 4. COLAS: ENVIAR DATOS EN ORDEN FIFO (queue.h)
 * FIFO significa que sale primero lo que entro primero.
 * QueueHandle_t q=xQueueCreate(4, sizeof(int));
 * Parametros: cantidad maxima de elementos y bytes que ocupa CADA elemento.
 * int dato=25;
 * xQueueSend(q, &dato, portMAX_DELAY);
 * Parametros: cola, direccion de los bytes a copiar, espera maxima si esta llena.
 * int recibido;
 * if (xQueueReceive(q, &recibido, portMAX_DELAY)==pdTRUE) { usar(recibido); }
 * Parametros: cola, direccion donde copiar lo extraido, espera si esta vacia.
 * Send y Receive devuelven pdPASS al completar (equivale a pdTRUE).
 *
 * POR VALOR: en el ejemplo la cola copia el entero 25. Aunque luego dato=90,
 * lo ya enviado sigue siendo 25. El &dato le indica DONDE leer ese entero;
 * usar & en Send NO significa por si solo que la cola almacene punteros.
 *
 * POR PUNTERO: depende del tamano con el que creaste la cola:
 * QueueHandle_t qp=xQueueCreate(4, sizeof(char *));
 * static char texto[20]="Hola";
 * char *p=texto;
 * xQueueSend(qp, &p, portMAX_DELAY); // Copia la DIRECCION guardada en p.
 * char *recibido;
 * xQueueReceive(qp, &recibido, portMAX_DELAY); // Recibe esa misma direccion.
 * La cola NO guarda una copia de las letras "Hola". Ambas tareas acceden al
 * mismo buffer. Si el productor lo cambia antes de la lectura, el consumidor
 * vera los cambios. Si el buffer era local a una funcion que termino, o memoria
 * dinamica que ya se libero, el puntero deja de ser valido: eso es su vida util.
 * 'static' evita que el buffer desaparezca, pero NO evita cambios simultaneos.
 * Soluciones: enviar una estructura que contenga el texto POR VALOR, o acordar
 * que el productor no reutilice/libere el buffer hasta que el receptor termine.
 * Admite varios productores y consumidores. Requiere queue.c.
 *
 * 5. EVENT GROUPS: ESPERAR CONDICIONES (event_groups.h)
 * Pensalo como un tablero de banderas: cada bit representa una condicion.
 * #define SENSOR_LISTO (1U << 0) // 00000001: bit 0.
 * #define DATOS_LISTOS (1U << 1) // 00000010: bit 1.
 * EventGroupHandle_t g=xEventGroupCreate(); // Todos los bits comienzan en 0.
 * Una tarea hace xEventGroupSetBits(g,SENSOR_LISTO) y otra hace
 * xEventGroupSetBits(g,DATOS_LISTOS). SetBits pone los bits indicados en 1
 * conservando los demas. El operador | combina las banderas: 00000011.
 *
 * EventBits_t resultado=xEventGroupWaitBits(
 *     g,                          // Grupo que se consulta.
 *     SENSOR_LISTO | DATOS_LISTOS, // Bits que me interesan.
 *     pdTRUE,                     // Borrarlos si se cumple la espera.
 *     pdTRUE,                     // Esperar TODOS; pdFALSE: al menos UNO.
 *     portMAX_DELAY);             // Tiempo maximo bloqueado.
 *
 * Esa tarea sigue cuando SENSOR y DATOS estan listos. Si ya estaban en 1,
 * no se bloquea. Con el cuarto argumento pdFALSE alcanza con uno de ellos.
 * El tercer argumento pdFALSE conserva los bits: sirve para condiciones
 * persistentes; pdTRUE permite volver a esperar nuevos avisos en otro ciclo.
 * La funcion devuelve los bits observados ANTES del borrado automatico.
 * Con timeout finito comprobar si se cumplio la condicion, por ejemplo:
 * if ((resultado & (SENSOR_LISTO|DATOS_LISTOS)) ==
 *                   (SENSOR_LISTO|DATOS_LISTOS)) { usar_datos(); }
 * Si no coinciden, vencio la espera sin que estuvieran ambos.
 *
 * xEventGroupClearBits(g,SENSOR_LISTO): marca esa condicion como no cumplida.
 * xEventGroupGetBits(g): consulta los bits actuales sin esperar.
 * xEventGroupSync(g,miBit,bitsDeTodos,espera): marca mi llegada y espera que
 * todos hayan llegado; sirve como barrera para reunir tareas en un punto.
 * Varias tareas pueden despertar por el mismo evento. Un bit NO cuenta:
 * marcar SENSOR_LISTO tres veces antes de atenderlo sigue dejando un solo 1.
 * En esta configuracion con ticks de 16 bits hay 8 bits de evento utilizables.
 * Requiere event_groups.c. SetBits desde ISR tambien necesita el servicio de
 * timers y su configuracion. Este ejercicio no habilita esos archivos.
 *
 * 6. NOTIFICACIONES: AVISAR DIRECTAMENTE A UNA TAREA (task.h)
 * Cada tarea tiene su propio valor de notificacion de 32 bits y un estado que
 * indica si hay un aviso pendiente. No hay que crear otro objeto tipo semaforo.
 * Se obtiene el handle del destino al crearlo:
 * TaskHandle_t receptor;
 * xTaskCreate(tareaReceptora,"Rx",128,NULL,1,&receptor);
 * El ultimo argumento guarda el identificador en receptor. No notificar hasta
 * que xTaskCreate haya tenido exito. La tarea receptora espera SUS propios avisos.
 *
 * MODO CONTADOR (el mas simple):
 * Emisor: xTaskNotifyGive(receptor); // Suma uno al contador del destino.
 * Receptor: uint32_t n=ulTaskNotifyTake(pdFALSE,portMAX_DELAY);
 * Primer parametro: pdFALSE resta UNO; pdTRUE borra TODO el contador al recibir.
 * Segundo: espera maxima si el contador esta en cero.
 * Retorno: valor ANTES de restar/borrar; 0 si vencio la espera sin aviso.
 * Ejemplo: llegan 3 Give. Take(pdFALSE,...) devuelve 3 y deja 2; la siguiente
 * llamada devuelve 2 y deja 1. Take(pdTRUE,...) devuelve 3 y deja 0.
 * Con pdFALSE normalmente se atiende un evento por Take exitoso; con pdTRUE
 * se puede procesar el lote de n eventos. No llega un dato distinto por evento.
 *
 * MODO BITS O VALOR:
 * xTaskNotify(receptor, SENSOR_LISTO, eSetBits);
 * Parametros: tarea destino, valor enviado, accion sobre su valor interno.
 * eSetBits: combina con OR; eIncrement: suma 1 (ignora el segundo parametro);
 * eSetValueWithOverwrite: reemplaza aunque haya un aviso pendiente;
 * eSetValueWithoutOverwrite: falla si hay uno pendiente, evita pisarlo;
 * eNoAction: marca el aviso sin modificar el valor.
 *
 * En la tarea receptora:
 * uint32_t avisos;
 * if (xTaskNotifyWait(0, 0xFFFFFFFFUL, &avisos, portMAX_DELAY)==pdTRUE) {
 *     if (avisos & SENSOR_LISTO) { atender_sensor(); }
 * }
 * Parametro 1: mascara de bits a borrar AL ENTRAR, si no habia aviso pendiente;
 *              0 significa no borrar ninguno.
 * Parametro 2: mascara a borrar AL SALIR cuando se recibe; 0xFFFFFFFFUL borra
 *              todos, pero primero se copia el valor a avisos.
 * Parametro 3: direccion donde guardar el valor recibido (puede ser NULL).
 * Parametro 4: espera maxima. Retorno pdTRUE=aviso; pdFALSE=timeout.
 * Wait espera un aviso, NO una combinacion de bits como EventGroupWaitBits.
 * No mezclar modo contador y bits en el mismo indice. Estas API sin Indexed
 * usan el indice 0. Requiere configUSE_TASK_NOTIFICATIONS=1; vive en tasks.c.
 *
 * 7. STREAM/MESSAGE BUFFERS: TRANSFERIR BYTES O MENSAJES
 * Stream (stream_buffer.h): flujo de bytes sin fronteras de mensaje.
 * s=xStreamBufferCreate(64,1); // Capacidad en bytes, umbral para despertar lector.
 * n=xStreamBufferSend(s,datos,cantidad,espera);
 * n=xStreamBufferReceive(s,destino,capacidadDestino,espera);
 * Devuelven bytes transferidos; puede ser menos que lo pedido.
 * Message (message_buffer.h): conserva mensajes de longitud variable.
 * m=xMessageBufferCreate(64); // Capacidad total; incluye metadata por mensaje.
 * xMessageBufferSend(m,datos,longitud,espera);
 * xMessageBufferReceive(m,destino,capacidadDestino,espera);
 * Cada mensaje se transfiere completo o no se transfiere; si el destino es
 * pequeno, Receive devuelve 0 y el mensaje queda pendiente.
 * Pensados para UN escritor y UN lector; varios requieren proteccion adicional.
 * Requieren stream_buffer.c y usan notificaciones de tarea internamente.
 *
 * 8. SECCIONES CRITICAS: PROTEGER POCAS INSTRUCCIONES (task.h)
 * Ejemplo: leer un contador de 16 bits compartido con una ISR en un AVR de 8 bits.
 * taskENTER_CRITICAL(); copia=contador; taskEXIT_CRITICAL();
 * En este port deshabilitan interrupciones: la ISR no cambia el contador en
 * medio de la lectura. No imprimir, esperar ni usar API bloqueantes dentro.
 * Cada ENTER debe tener su EXIT; mantener la seccion lo mas corta posible.
 * vTaskSuspendAll()/xTaskResumeAll() suspenden/reanudan solo el scheduler:
 * las ISR siguen ocurriendo. Tampoco permiten API bloqueantes en ese intervalo.
 *
 * 9. QUEUE SETS: ESPERAR EN VARIAS COLAS/SEMAFOROS (queue.h)
 * set=xQueueCreateSet(capacidadTotal); // Suma de capacidades de sus miembros.
 * xQueueAddToSet(cola,set); // Agregar cola/semaforo vacio antes de usarlo.
 * listo=xQueueSelectFromSet(set,espera); // Devuelve el miembro listo o NULL.
 * Luego hacer Receive/Take sobre listo: Select NO consume su dato/permiso.
 * Util cuando una tarea debe atender varias fuentes; requiere
 * configUSE_QUEUE_SETS=1. No consumir miembros sin seleccionarlos antes.
 *
 * Este ejercicio enlaza tasks.c, list.c, queue.c, heap_4.c y port.c. Los ejemplos
 * son orientativos y omiten algunas comprobaciones para mostrar las API.
 * Otras herramientas requieren habilitar opciones/agregar archivos indicados.
 * vTaskDelay(ticks) da una pausa, pero NO garantiza orden entre tareas.
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "uart.h"

/* 0: A, B y C en ciclo. 1: solo A. 2: solo B. 3: solo C. */
#ifndef SECUENCIA
#define SECUENCIA 0
#endif
#if SECUENCIA < 0 || SECUENCIA > 3
#error "SECUENCIA debe ser 0, 1, 2 o 3"
#endif
#define PRIORIDAD_TAREAS 1
#define STACK_TAREA 128
#define PAUSA_TICKS 20 /* Entre lineas; el tick del watchdog dura ~16 ms. */

static const uint8_t secuencias[3][5] = {
    {1, 3, 2, 0, 0}, /* A */
    {2, 2, 3, 1, 0}, /* B */
    {3, 3, 3, 1, 2}  /* C */
};
static const uint8_t longitudes[3] = {3, 4, 5};
static const uint8_t ids[3] = {1, 2, 3};
static SemaphoreHandle_t turnos[3];
static uint8_t paso = 0;
#if SECUENCIA == 0
static uint8_t secuenciaActual = 0;
#else
static uint8_t secuenciaActual = SECUENCIA - 1;
#endif

static void errorFatal(void)
{
    taskDISABLE_INTERRUPTS();
    UART_SendString("ERROR: no se pudo iniciar FreeRTOS\r\n");
    for (;;) { }
}

/* TRES tareas independientes comparten esta funcion, cada una con su id.
 * Solo quien posee el turno imprime y modifica el paso compartido.
 * No hace falta otro mutex para la UART ni para el indice.
 */
static void tareaSecuencia(void *parametro)
{
    const uint8_t id = *(const uint8_t *)parametro;
    for (;;)
    {
        /* Dormir hasta recibir el turno de esta tarea. */
        if (xSemaphoreTake(turnos[id - 1], portMAX_DELAY) != pdTRUE)
            continue;
        if (paso == 0)
        {
            UART_SendChar('A' + secuenciaActual);
            UART_SendString(": ");
        }
        UART_SendString("Tarea ");
        UART_SendChar('0' + id);
        paso++;
        if (paso == longitudes[secuenciaActual])
        {
            UART_SendString("\r\n");
            paso = 0;
#if SECUENCIA == 0
            secuenciaActual = (secuenciaActual + 1) % 3;
#endif
            /* Pausa para leer; el orden lo garantizan los semaforos. */
            vTaskDelay(PAUSA_TICKS);
        }
        else
        {
            UART_SendString(" - ");
        }
        /* Pasar el unico permiso. Puede ser para esta misma tarea,
         * permitiendo las repeticiones consecutivas de B y C. */
        if (xSemaphoreGive(turnos[secuencias[secuenciaActual][paso] - 1]) != pdTRUE)
            errorFatal();
    }
}

int main(void)
{
    UART_Init(9600); /* ATmega328P a 16 MHz; terminal 9600 baudios, 8N1. */
    /* Crear los tres semaforos vacios antes de iniciar el scheduler. */
    for (uint8_t i = 0; i < 3; i++)
    {
        turnos[i] = xSemaphoreCreateBinary();
        if (turnos[i] == NULL)
            errorFatal();
    }
    /* Las tres tareas tienen exactamente la misma prioridad. */
    for (uint8_t i = 0; i < 3; i++)
    {
        const char *nombre = (i == 0) ? "Tarea1" : ((i == 1) ? "Tarea2" : "Tarea3");
        if (xTaskCreate(tareaSecuencia, nombre, STACK_TAREA,
                       (void *)&ids[i], PRIORIDAD_TAREAS, NULL) != pdPASS)
            errorFatal();
    }
    /* Semilla: habilitar solo la primera tarea de la secuencia elegida. */
    if (xSemaphoreGive(turnos[secuencias[secuenciaActual][0] - 1]) != pdTRUE)
        errorFatal();
    vTaskStartScheduler();
    errorFatal(); /* Solo vuelve si el scheduler no pudo arrancar. */
    return 0;
}
