/*
 * SINCRONIZACION Y COMUNICACION EN FREERTOS
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
 * ----- MUTEX: USAR UN RECURSO DE A UNA TAREA (semphr.h) -----
 * Ejemplo: dos tareas imprimen por UART. Cada una toma el mutex antes de su
 * mensaje completo y lo devuelve al terminar, para no mezclar caracteres.
  * Requiere configUSE_MUTEXES=1 y queue.c. No se usa desde ISR.
 *
 * SemaphoreHandle_t m = xSemaphoreCreateMutex(); // Nace disponible.
 * if (xSemaphoreTake(m, portMAX_DELAY) == pdTRUE) {
 *     imprimir_mensaje();
 *     xSemaphoreGive(m);
 * }
 * Take(m, espera): intenta obtener la exclusividad; espera si otra la tiene.
 * Give(m): devuelve la exclusividad. Debe hacerlo la misma tarea que tomo m.
 *
 * HERENCIA DE PRIORIDAD:
 * Con un mutex FreeRTOS eleva temporalmente la prioridad de Baja a la de Alta,
 * para ayudarla a terminar y liberar el recurso. Luego Alta puede tomarlo.
 *
 *
 * ----- SEMAFORO BINARIO: DAR UN AVISO O UN TURNO (semphr.h) -----
 * Guarda 0 o 1 permisos. No tiene propietario ni herencia de prioridad: una tarea puede dar el permiso y otra consumirlo.
 * s=xSemaphoreCreateBinary(); // Nace VACIO, a diferencia del mutex.
 * xSemaphoreGive(s); // Deja un permiso; falla si ya estaba lleno.
 * xSemaphoreTake(s,portMAX_DELAY); // Consume el permiso o espera que aparezca.
 * Take devuelve pdTRUE si lo obtuvo; si no, no corresponde seguir como si nada.
 * Dos Give seguidos no guardan dos avisos: para eso usar semaforo contador o cola.
 * Requiere queue.c. 
 *
 *  ----- SEMAFORO CONTADOR: CONTAR AVISOS O RECURSOS (semphr.h) -----
 * Lo mismo que binario pero con mas de un permiso.
 * s=xSemaphoreCreateCounting(5, 5);
 * Primer parametro: maximo de permisos; segundo: cantidad inicial.
 * Con 5 iniciales representa cinco recursos libres. Take consume uno y Give
 * devuelve uno. Con 0 iniciales puede contar eventos pendientes de atender.
 * Take espera si esta en cero; Give falla si alcanzo el maximo.
 * Requiere configUSE_COUNTING_SEMAPHORES=1 y queue.c.
 *
 *  ----- COLAS: ENVIAR DATOS EN ORDEN FIFO (queue.h)  -----
 * QueueHandle_t q=xQueueCreate(4, sizeof(int));
 * Parametros: cantidad maxima de elementos y bytes que ocupa CADA elemento.
 * int dato=25;
 * xQueueSend(q, &dato, portMAX_DELAY);
 * Parametros: cola, direccion de los bytes a copiar, espera maxima si esta llena.
 * int recibido;
 * if (xQueueReceive(q, &recibido, portMAX_DELAY)==pdTRUE) { usar(recibido); }
 * Parametros: cola, direccion donde copiar lo extraido, espera si esta vacia.
 * Send y Receive devuelven pdPASS al completar (equivale a pdTRUE).

 * Admite varios productores y consumidores. Requiere queue.c.
 *
 *  ----- EVENT GROUPS: ESPERAR CONDICIONES (event_groups.h)  -----
 * Es como un tablero de banderas: cada bit representa una condicion.
 * #define SENSOR_LISTO (1U << 0) // 00000001: bit 0.
 * #define DATOS_LISTOS (1U << 1) // 00000010: bit 1.
 * EventGroupHandle_t g=xEventGroupCreate(); // Todos los bits comienzan en 0.
 * Una tarea hace xEventGroupSetBits(g,SENSOR_LISTO) y otra hace xEventGroupSetBits(g,DATOS_LISTOS).
 *
 * EventBits_t resultado=xEventGroupWaitBits(
 *     g,                          // Grupo que se consulta.
 *     SENSOR_LISTO | DATOS_LISTOS, // Bits que me interesan.
 *     pdTRUE,                     // Borrarlos si se cumple la espera.
 *     pdTRUE,                     // Esperar TODOS; pdFALSE: al menos UNO.
 *     portMAX_DELAY);             // Tiempo maximo bloqueado.
 *
 * Esa tarea sigue cuando SENSOR y DATOS estan listos. Si ya estaban en 1,
 * no se bloquea. 
 * La funcion devuelve los bits observados ANTES del borrado automatico.
 * Con timeout finito comprobar si se cumplio la condicion, por ejemplo:
 * if ((resultado & (SENSOR_LISTO|DATOS_LISTOS)) == (SENSOR_LISTO|DATOS_LISTOS)) { usar_datos(); }
 * Si no coinciden, vencio la espera sin que estuvieran ambos.
 *
 * xEventGroupClearBits(g,SENSOR_LISTO): marca esa condicion como no cumplida.
 * xEventGroupGetBits(g): consulta los bits actuales sin esperar.
 * xEventGroupSync(g,miBit,bitsDeTodos,espera): marca mi llegada y espera que  todos hayan llegado; sirve como barrera para reunir tareas en un punto.
 * Varias tareas pueden despertar por el mismo evento. 
 * Requiere event_groups.c. 
 *
 *  ----- NOTIFICACIONES: AVISAR DIRECTAMENTE A UNA TAREA (task.h)  -----
 * Cada tarea tiene su propio valor de notificacion de 32 bits y un estado que
 * indica si hay un aviso pendiente. 
 * Se obtiene el handle del destino al crearlo:
 * TaskHandle_t receptor;
 * xTaskCreate(tareaReceptora,"Rx",128,NULL,1,&receptor);
 *  No notificar hasta que xTaskCreate haya tenido exito. La tarea receptora espera SUS propios avisos.
 *
 * MODO CONTADOR:
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
 *  ----- STREAM/MESSAGE BUFFERS  -----
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
 *  ----- SECCIONES CRITICAS(task.h)  -----
 * Ejemplo: leer un contador de 16 bits compartido con una ISR en un AVR de 8 bits.
 * taskENTER_CRITICAL(); copia=contador; taskEXIT_CRITICAL();
 * En este port deshabilitan interrupciones: la ISR no cambia el contador en
 * medio de la lectura. No imprimir, esperar ni usar API bloqueantes dentro.
 * Cada ENTER debe tener su EXIT; mantener la seccion lo mas corta posible.
 * vTaskSuspendAll()/xTaskResumeAll() suspenden/reanudan solo el scheduler:
 * las ISR siguen ocurriendo. Tampoco permiten API bloqueantes en ese intervalo.
 *
 *  ----- QUEUE SETS: ESPERAR EN VARIAS COLAS/SEMAFOROS (queue.h)  -----
 * set=xQueueCreateSet(capacidadTotal); // Suma de capacidades de sus miembros.
 * xQueueAddToSet(cola,set); // Agregar cola/semaforo vacio antes de usarlo.
 * listo=xQueueSelectFromSet(set,espera); // Devuelve el miembro listo o NULL.
 * Luego hacer Receive/Take sobre listo: Select NO consume su dato/permiso.
 * Util cuando una tarea debe atender varias fuentes; requiere
 * configUSE_QUEUE_SETS=1. No consumir miembros sin seleccionarlos antes.
 *
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "uart.h"

/* Cada semaforo representa el permiso de una tarea para imprimir.
 * Solo circula un permiso: no hace falta otro mutex para la UART. */
static SemaphoreHandle_t sem1, sem2, sem3;

static void errorFatal(void)
{
    taskDISABLE_INTERRUPTS();
    UART_SendString("ERROR FreeRTOS\r\n");
    for (;;) { }
}

static void crearSemaforos(void)
{
    sem1 = xSemaphoreCreateBinary(); /* Nacen vacios. */
    sem2 = xSemaphoreCreateBinary();
    sem3 = xSemaphoreCreateBinary();
    if (sem1 == NULL || sem2 == NULL || sem3 == NULL)
        errorFatal();
}

/* SECUENCIA A */

static void Tarea1_A(void *parametro)
{
    for (;;)
    {
        /* Esperar mi turno; las otras tareas esperan en sus semaforos. */
        if (xSemaphoreTake(sem1, portMAX_DELAY) != pdTRUE)
            continue;
        UART_SendString("Tarea 1 - ");
        /* Habilitar a Tarea 3 solamente despues de imprimir. */
        if (xSemaphoreGive(sem3) != pdTRUE)
            errorFatal();
    }
}

static void Tarea2_A(void *parametro)
{
    for (;;)
    {
        /* Esperar mi turno; las otras tareas esperan en sus semaforos. */
        if (xSemaphoreTake(sem2, portMAX_DELAY) != pdTRUE)
            continue;
        UART_SendString("Tarea 2\r\n");
        /* Habilitar a Tarea 1 solamente despues de imprimir. */
        if (xSemaphoreGive(sem1) != pdTRUE)
            errorFatal();
    }
}

static void Tarea3_A(void *parametro)
{
    for (;;)
    {
        /* Esperar mi turno; las otras tareas esperan en sus semaforos. */
        if (xSemaphoreTake(sem3, portMAX_DELAY) != pdTRUE)
            continue;
        UART_SendString("Tarea 3 - ");
        /* Habilitar a Tarea 2 solamente despues de imprimir. */
        if (xSemaphoreGive(sem2) != pdTRUE)
            errorFatal();
    }
}

void secuenciaA(void)
{
    crearSemaforos();
    /* Tres tareas con la misma prioridad: 1. Stack: 128 cada una. */
    if (xTaskCreate(Tarea1_A, "Tarea1", 128, NULL, 1, NULL) != pdPASS)
        errorFatal();
    if (xTaskCreate(Tarea2_A, "Tarea2", 128, NULL, 1, NULL) != pdPASS)
        errorFatal();
    if (xTaskCreate(Tarea3_A, "Tarea3", 128, NULL, 1, NULL) != pdPASS)
        errorFatal();
    /* Permiso inicial: solo Tarea 1 puede comenzar. */
    if (xSemaphoreGive(sem1) != pdTRUE)
        errorFatal();
}

/* SECUENCIA B */

static void Tarea1_B(void *parametro)
{
    for (;;)
    {
        /* Esperar mi turno; las otras tareas esperan en sus semaforos. */
        if (xSemaphoreTake(sem1, portMAX_DELAY) != pdTRUE)
            continue;
        UART_SendString("Tarea 1\r\n");
        /* Habilitar a Tarea 2 solamente despues de imprimir. */
        if (xSemaphoreGive(sem2) != pdTRUE)
            errorFatal();
    }
}

static void Tarea2_B(void *parametro)
{
    uint8_t impresiones = 0; /* Cuenta solo las impresiones de este ciclo. */
    for (;;)
    {
        if (xSemaphoreTake(sem2, portMAX_DELAY) != pdTRUE)
            continue;

        UART_SendString("Tarea 2 - "); /* Una impresion por permiso. */
        impresiones++;

        if (impresiones < 2)
        {
            /* Conservar el turno: el proximo Take consume este nuevo permiso.
             * No bloquea si el permiso ya esta disponible. */
            if (xSemaphoreGive(sem2) != pdTRUE)
                errorFatal();
        }
        else
        {
            /* Completar el grupo y preparar el contador para el proximo ciclo. */
            impresiones = 0;
            if (xSemaphoreGive(sem3) != pdTRUE)
                errorFatal();
        }
    }
}
static void Tarea3_B(void *parametro)
{
    for (;;)
    {
        /* Esperar mi turno; las otras tareas esperan en sus semaforos. */
        if (xSemaphoreTake(sem3, portMAX_DELAY) != pdTRUE)
            continue;
        UART_SendString("Tarea 3 - ");
        /* Habilitar a Tarea 1 solamente despues de imprimir. */
        if (xSemaphoreGive(sem1) != pdTRUE)
            errorFatal();
    }
}

void secuenciaB(void)
{
    crearSemaforos();
    /* Tres tareas con la misma prioridad: 1. Stack: 128 cada una. */
    if (xTaskCreate(Tarea1_B, "Tarea1", 128, NULL, 1, NULL) != pdPASS)
        errorFatal();
    if (xTaskCreate(Tarea2_B, "Tarea2", 128, NULL, 1, NULL) != pdPASS)
        errorFatal();
    if (xTaskCreate(Tarea3_B, "Tarea3", 128, NULL, 1, NULL) != pdPASS)
        errorFatal();
    /* Permiso inicial: solo Tarea 2 puede comenzar. */
    if (xSemaphoreGive(sem2) != pdTRUE)
        errorFatal();
}

/* SECUENCIA C */

static void Tarea1_C(void *parametro)
{
    for (;;)
    {
        /* Esperar mi turno; las otras tareas esperan en sus semaforos. */
        if (xSemaphoreTake(sem1, portMAX_DELAY) != pdTRUE)
            continue;
        UART_SendString("Tarea 1 - ");
        /* Habilitar a Tarea 2 solamente despues de imprimir. */
        if (xSemaphoreGive(sem2) != pdTRUE)
            errorFatal();
    }
}

static void Tarea2_C(void *parametro)
{
    for (;;)
    {
        /* Esperar mi turno; las otras tareas esperan en sus semaforos. */
        if (xSemaphoreTake(sem2, portMAX_DELAY) != pdTRUE)
            continue;
        UART_SendString("Tarea 2\r\n");
        /* Habilitar a Tarea 3 solamente despues de imprimir. */
        if (xSemaphoreGive(sem3) != pdTRUE)
            errorFatal();
    }
}

static void Tarea3_C(void *parametro)
{
    uint8_t impresiones = 0; /* Cuenta solo las impresiones de este ciclo. */
    for (;;)
    {
        if (xSemaphoreTake(sem3, portMAX_DELAY) != pdTRUE)
            continue;

        UART_SendString("Tarea 3 - "); /* Una impresion por permiso. */
        impresiones++;

        if (impresiones < 3)
        {
            /* Conservar el turno: el proximo Take consume este nuevo permiso.
             * No bloquea si el permiso ya esta disponible. */
            if (xSemaphoreGive(sem3) != pdTRUE)
                errorFatal();
        }
        else
        {
            /* Completar el grupo y preparar el contador para el proximo ciclo. */
            impresiones = 0;
            if (xSemaphoreGive(sem1) != pdTRUE)
                errorFatal();
        }
    }
}
void secuenciaC(void)
{
    crearSemaforos();
    /* Tres tareas con la misma prioridad: 1. Stack: 128 cada una. */
    if (xTaskCreate(Tarea1_C, "Tarea1", 128, NULL, 1, NULL) != pdPASS)
        errorFatal();
    if (xTaskCreate(Tarea2_C, "Tarea2", 128, NULL, 1, NULL) != pdPASS)
        errorFatal();
    if (xTaskCreate(Tarea3_C, "Tarea3", 128, NULL, 1, NULL) != pdPASS)
        errorFatal();
    /* Permiso inicial: solo Tarea 3 puede comenzar. */
    if (xSemaphoreGive(sem3) != pdTRUE)
        errorFatal();
}

int main(void)
{
    UART_Init(9600); /* ATmega328P a 16 MHz; terminal 9600 baudios, 8N1. */

 
    // secuenciaA();
    // secuenciaB();
    secuenciaC();

    vTaskStartScheduler();
    errorFatal(); 
    return 0;
}
