/*
 * @brief   Este modulo usa el UART3 para enviar y recibir datos del MXCHIP.
 *          Las respuestas del MXCHIP se re-envian al UART con motivo de debug.
 *
 * Fecha: 14/12/2024
 * Autor. DHS
 * Version: 1
 * Resumen: version inicial.
 *
 */

// Includes.
#include "fm_mxc.h"
#include "string.h"
#include "fm_cmd.h"
#include "fm_debug.h"
#include "fm_usart.h"
#include "stdbool.h"

// Sección define sin dependencia.
#define FALSE 0
#define TRUE  1
#define WAIT_3000       3000 // Tiempo de espera por respuesta al enviar comando AT
#define WAIT_2000       2000
#define WAIT_1000       1000
#define WAIT_500        500
#define WAIT_250        250
#define WAIT_100        100
#define WAIT_50         25
#define WAIT_25         25
#define WAIT_10         10
#define WAIT_5           5
#define WAIT_1           1
#define MAX_FM_LINE_LEN 128
#define FM_LINE_READY (((ULONG) 1) << 0)

// Sección enum y typedef sin dependencia.

// Lista de comandos AT del modulo MXChip EMC-3080-P
typedef enum
{
    AT_PLUS,            // Modo transparente.
    AT_UARTE,           // Consulta si UART tiene eco.
    AT_UARTE_ON,        // Activa el eco, el eco actúa sobre cada caracter recibido, no espera el \r
    AT_UARTE_OFF,       // Desactiva el eco.
    AT_STANDBY,         // Pasa a modo de bajo consumo. Para salir de este modo se hace hard reset
    AT_BROLE_MASTER,    // Configura a MXChip como maestro, para imprimir
    AT_BROLE_SLAVE,     // Configura a MXChip como esclavo, para descarga de log.
    AT_BINQ_NAME,       // Escanea buscando un nombre especifico de impresora.
    AT_BCONN_0,         // Se conecta al primer resultado del escaneado.
    AT_BLE,             // Consulta el estado del bluetooth,
    AT_BLE_ON,          // Activa el modulo bluetooth.
    AT_BLE_OFF,         // Desactiva el modulo bluetooth.
    AT_EVENT_ON, // Los eventos notifican por el puerto UART. El mas relevante es el de AT+BINQ=1,....
    AT_EVENT_OFF,       // No se notifican los evento.
    AT_BSENDRAW,        // Pasa la impresora a modo transparente.
    AT_REBOOT,          //
    AT_BSINQ,           //
    AT_BSERVUUID_1800,  //
    AT_BTXUUID_2A00,    //
    AT_BRXUUID_2A00,    //
    AT_FWVER,           //
} at_id_t;

/*
 *  Parámetros de commandos AT del MXChip. En el manual de modulo para el comando se tiene
 *  el nombre, y las respuesta. Esta estructura sirve para:
 *  En el envió del comando se selecciona un string con el nombre del comando mediante el ID
 *  En la respuesta se cuentan los retornos de carro, al tener la cantidad de retornos de carro
 *  esperados se considera la respuesta completa.
 *  Al enviar comandos sucesivos, uno tras otro sin esperar respuesta, el MXChip fallaba, se
 *  tiene un parametro llamado delay, que es la espera de cada comando, antes de permitir enviar
 *  otro.
 */
typedef struct
{
    const char *command; // Nombre del comando AT propio del MXChip.
    const at_id_t id;       // ID del comando
    const char *ret;      // Cantidad de retornos de carro en el mensaje de respuesta.
} at_commad_t;

typedef enum
{
    FM_TYPE_LITERAL, FM_TYPE_HANDLER, FM_TYPE_DEFERRED
} fm_command_type_t;

typedef struct
{
    const char *command;
    fm_command_type_t type;
    union
    {
        const char *literal;
        void (*handler)(const char *args);
    };
} fm_command_t;

// Sección define, enum y typedef con dependencia.

// Debug.

// Variables non-static, primero las tipo const.

// Variables statics, primero las tipo const.
static TX_QUEUE *queue_ptr = NULL;          // Puntero a la cola de comandos

/*
 *  Lista de comandos AT del EMC-3080. Para los comando que aun no estudie su respuesta, completo
 *  con \0. La repuesta se espera sin eco. La respuesta no incluye el evento esperado que puede
 *  estar asociado a que la respuesta se completamente exitosa.
 */
static const at_commad_t at_list[] =
{
{ "+++", AT_PLUS, "\0" },
{ "AT+UARTE?\r\n", AT_UARTE, "\0" },
{ "AT+UARTE=ON\r\n", AT_UARTE_ON, "\r\nOK\r\n" },
{ "AT+UARTE=OFF\r\n", AT_UARTE_OFF, "\r\nOK\r\n" },
{ "AT+STANDBY\r\n", AT_STANDBY, "\r\nOK\r\n" },
{ "AT+BROLE=1\r\n", AT_BROLE_MASTER, "\r\nOK\r\n" },
{ "AT+BROLE=0\r\n", AT_BROLE_SLAVE, "\r\nOK\r\n" },
{ "AT+BINQ=1,FM-BLE\r\n", AT_BINQ_NAME, "\r\n+BEVENT" },
{ "AT+BCONN=0\r\n", AT_BCONN_0, "\r\n+BEVENT" },
{ "AT+BLE?\r\n", AT_BLE, "\0" },
{ "AT+BLE=ON\r\n", AT_BLE_ON, "\r\nOK\r\n" },
{ "AT+BLE=OFF\r\n", AT_BLE_OFF, "\r\nOK\r\n" },
{ "AT+BEVENT=ON\r\n", AT_EVENT_ON, "\r\nOK\r\n" },
{ "AT+BEVENT=OFF\r\n", AT_EVENT_OFF, "\r\nOK\r\n" },
{ "AT+BSENDRAW\r\n", AT_BSENDRAW, "\r\nOK\r\n" },
{ "AT+REBOOT\r\n", AT_REBOOT, "\r\nOK\r\n" },
{ "AT+BSINQ\r", AT_BSINQ, "\r\nOK\r\n" },
{ "AT+BSERVUUID=1800\r", AT_BSERVUUID_1800, "\r\nOK\r\n" },
{ "AT+BTXUUID=2A00\r\n", AT_BTXUUID_2A00, "\r\nOK\r\n" },
{ "AT+BRXUUID=2A00\r\n", AT_BRXUUID_2A00, "\r\nOK\r\n" },
{ "AT+FWVER?\r\n", AT_FWVER, "\0" } };

// Variables extern, las que no estan en .h.

// Private function prototypes.
fmx_status_t SendAt(at_id_t id, int retry, UINT wait_ms);
void FM_HandleLogDeferred(const char *args); // solo declarada, no se usa directamente

// Private function bodies.

int my_cmp(const char *str1, const char *str2)
{
    while (*str2)
    {
        if (*str1 != *str2)
        {
            return 1;
        }
        str1++;
        str2++;
    }
    return 0;
}

// Public function bodies.

/**
 * @brief Inicializa la recepción UART con interrupciones.
 * @param queue Puntero a la cola donde se enviarán los comandos recibidos
 * @retval Ninguno
 */
void FM_MXC_InitRtos(TX_QUEUE *queue)
{
    queue_ptr = queue;
}

/*
 * @brief   manejo el pin del MXChip enable, y su alimentación.
 * @param   mode: ver switch(mode)
 * @retval  ninguno.
 */
void FM_MXC_Mode(fm_mxc_mode_t mode)
{
    switch (mode)
    {
    case FM_MXC_MODE_OFF:
        HAL_GPIO_WritePin(MXC_ENABLE_GPIO_Port, MXC_ENABLE_Pin, GPIO_PIN_RESET); // Linea enable de chip disable
        HAL_GPIO_WritePin(MXC_ON_GPIO_Port, MXC_ON_Pin, GPIO_PIN_SET); // Quito alimentación
        break;
    case FM_MXC_MODE_ON:
        HAL_GPIO_WritePin(MXC_ENABLE_GPIO_Port, MXC_ENABLE_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MXC_ON_GPIO_Port, MXC_ON_Pin, GPIO_PIN_RESET);
        break;
    case FM_MXC_MODE_DISABLE:
        HAL_GPIO_WritePin(MXC_ENABLE_GPIO_Port, MXC_ENABLE_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MXC_ON_GPIO_Port, MXC_ON_Pin, GPIO_PIN_RESET);
    case FM_MXC_MODE_ENABLE:
        HAL_GPIO_WritePin(MXC_ENABLE_GPIO_Port, MXC_ENABLE_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MXC_ON_GPIO_Port, MXC_ON_Pin, GPIO_PIN_RESET);
        break;
    default:
        HAL_GPIO_WritePin(MXC_ENABLE_GPIO_Port, MXC_ENABLE_Pin, GPIO_PIN_RESET); // Linea enable de chip disable
        HAL_GPIO_WritePin(MXC_ON_GPIO_Port, MXC_ON_Pin, GPIO_PIN_SET); // Quito alimentación
    }

    // Luego de cambiar de modo le doy algo de tiempo al EMC3080 que estabilice.
    tx_thread_sleep(100);
}

/*
 * @brief   Se conecta con impresoras térmicas con el nombre "FM-BLE"
 * @Note
 * @param   Ninguno.
 * @retval  Ninguno.
 */
void FM_MXC_PowerOn()
{
    fmx_status_t ret_status;

    FM_MXC_Mode(FM_MXC_MODE_ON);

    ret_status = FM_USART_Uart3PowerOn();
    if (ret_status != FMX_STATUS_OK)
    {
        FM_DEBUG_LedError(1);
    }
}

/*
 * @brief
 * @param   Ninguno.
 * @retval  Ninguno.
 */
void FM_MXC_PowerOff()
{
    HAL_UART_AbortReceive(&huart3);
    HAL_UART_MspDeInit(&huart3);
    FM_MXC_Mode(FM_MXC_MODE_OFF);
}

/*
 * @brief   Se conecta con impresoras térmicas con el nombre "FM-BLE"
 * @Note
 * @param   Ninguno.
 * @retval  Ninguno.
 */
void FM_MXC_Sleep()
{
    SendAt(AT_STANDBY, 1, WAIT_250);
}

/*
 * @brief
 * @param   Ninguno.
 * @retval  Ninguno.
 */
void FM_MXC_Wakeup()
{
    FM_MXC_Mode(FM_MXC_MODE_DISABLE);
    tx_thread_sleep(10);
    FM_MXC_Mode(FM_MXC_MODE_ENABLE);
    tx_thread_sleep(25);
}

/*
 * @brief   Se conecta con impresoras térmicas con el nombre "FM-BLE".
 *          El Modulo EMC3080 es maestro, la impresora esclavo.
 *          Se usa el modo de envió de datos "transparente" para el EMC-3080.
 * @param   Ninguno.
 * @retval  Comunica si la se conecto exitosamente o no.
 */
fmx_status_t FM_MXC_ConnectMaster()
{
    fmx_status_t ret_status;

    FM_MXC_PowerOn();

    /*
     * Deberia intentar mas de una vez, enviar este comando si responde error.
     */
    ret_status = SendAt(AT_BROLE_MASTER, 1, WAIT_250);
    if (ret_status != FMX_STATUS_OK)
    {
        ret_status = FMX_STATUS_ERROR;
    }

    // Scan para descubrir impresora esclavo.
    ret_status = SendAt(AT_BINQ_NAME, 1, WAIT_250);
    if (ret_status != FMX_STATUS_OK)
    {
        ret_status = FMX_STATUS_ERROR;
    }

    ret_status = SendAt(AT_BCONN_0, 1, WAIT_250);
    if (ret_status != FMX_STATUS_OK)
    {
        ret_status = FMX_STATUS_ERROR;
    }

    ret_status = SendAt(AT_BSENDRAW, 1, WAIT_250);
    if (ret_status != FMX_STATUS_OK)
    {
        ret_status = FMX_STATUS_ERROR;
    }

    return FMX_STATUS_OK;
}

/*
 * @brief	Se enciende el modulo en modo maestro
 * @param   Ninguno.
 * @retval  Ninguno.
 */
fmx_status_t FM_MXC_ConnectSlave()
{
    fmx_status_t ret_status;

    FM_MXC_PowerOn();

    ret_status = SendAt(AT_BROLE_SLAVE, 1, WAIT_250);
    if (ret_status != FMX_STATUS_OK)
    {
        return FMX_STATUS_ERROR;
    }

    ret_status = SendAt(AT_BSENDRAW, 1, WAIT_250);
    if (ret_status != FMX_STATUS_OK)
    {
        return FMX_STATUS_ERROR;
    }

    return ret_status;
}

/*
 * @brief   Se envía un comando AT al MXChip.
 * @param   id, identificador del comando a enviar.
 *          retry, cantidad de intentos hasta recibir una respuesta de "OK".
 *          delay,  máximo tiempo de espera por intento.
 * @retval  MXC_OK si el MXC responde con OK dentro de los intentos.
 *          MXC_FAIL si no se recibe MCX_OK al agotar los intentos.
 */
fmx_status_t SendAt(at_id_t id, int retry, UINT wait_ms)
{
    fmx_status_t fmx_status = FMX_STATUS_NULL;
    bool matched = FALSE;

    /*
     *  EMC-3080 necesita un retardo entre el envío de comandos. Aseguro un retado mínimo en este
     *  punto. Comandos de descubrimiento o de conexión necesitaran tiempos de esperas mas
     *  "exagerados". Algunos de los tiempos de espera pudiesen estar justificados, otros parecen
     *  radicar en una debilidad del EMC-3080 en recibir comandos continuamente y procesarlos en una
     *  cola.
     */
    //tx_thread_sleep(WAIT_25);

    while ((retry > 0) && (!matched))
    {

        FM_USART_Uart3TransmitDma(at_list[id].command, wait_ms);

        if ((id == AT_BINQ_NAME) || (id == AT_BCONN_0))
        {

            HAL_Delay(WAIT_3000);
        }
        else
        {
            HAL_Delay(WAIT_250);
        }

        /*
         * Comparo solo los primeros caracteres de la respuesta, del mismo largo que la esperada,
         * porque el EMC-3080 puede enviar eventos adicionales, ejemplo en  AT+BINQ y AT+BCONN.
         * Aunque se desactiven las notificaciones, el módulo sigue enviándolas. Para evitar falsos
         * errores por respuestas más largas, solo verifico si el inicio coincide con lo esperado.
         */
        if (my_cmp(fm_usart_rx3_buf, at_list[id].ret) == 0)
        {
            matched = TRUE;
        }
        else
        {
            retry--;
        }
    }

    if (retry)
    {
        fmx_status = FMX_STATUS_OK;
    }
    else
    {
        FM_DEBUG_LedError(1);
        fmx_status = FMX_STATUS_ERROR;
    }

    return fmx_status;
}

// Interrupts

/*** end of file ***/

