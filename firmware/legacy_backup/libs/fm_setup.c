/*
 * @brief	Pantallas del menu de configuración.
 * @notes	El orden de las funciones de este modulo no es alfabético, respetan el orden aparecen en el menu.
 *
 * Versión: 1
 * Autor: Daniel H Sagarra
 * Fecha: 10/11/2024
 * Modificaciones: version inicial.
 *
 *
 */

// Includes.
#include <fm_log.h>
#include "fm_debug.h"
#include "fm_factory.h"
#include "fm_fmc.h"
#include "fm_lcd.h"
#include "fm_rtc.h"
#include "fm_setup.h"
#include "fmx.h"
#include "fm_rtc.h"
#include "string.h"

// Typedef.

// Listado de las pantallas del menu de configuración.
typedef enum
{
    MENU_SETUP_INIT = 0,
    MENU_SETUP_PASSWORD,
    MENU_SETUP_FACTOR_C,
    MENU_SETUP_VOL_UNIT,
    MENU_SETUP_TIME_UNIT,
    MENU_SETUP_DATE,
    MENU_SETUP_TIME,
    MENU_SETUP_END,  // Se usa para chequeo de contorno.
} menu_user_t;

// Lista de comandos que se pude ejecutar al editar una dato de configuración.
typedef enum
{
    MENU_MODE_INIT,  // Se ingresa a un nuevo menu de edición.
    MENU_MODE_INC,  // Incrementa variable.
    MENU_MODE_DEC,  // Decrementa variable.
    MENU_MODE_NEXT,  // Se mueve al siguiente campo o variable a editar.
    MENU_MODE_EXIT,  // Finaliza la edición actual.
    MENU_MODE_REFRESH,  // Refresca valor.,
    MENU_MODE_END  // Se usa para chequeo de contorno.
} menu_mode_t;

// Const data.

// Defines.

#define TRUE  1
#define FALSE 0

// Debug.

// Project variables, non-static, at least used in other file.

// External variables.
extern RTC_HandleTypeDef hrtc;
extern ULONG global_menu_refresh;

// Global variables, statics.

const uint32_t power_of_10[] =
{ 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 };

static char setup_line_1[20];
static char setup_line_2[20];

// Private function prototypes

/*
 * Los siguientes prototipos de funciones se listan en el orden que luego aparacen sus definiciones, que
 * es el mismo orden en que aparecen en pantallas si se las recorre. no están ordenadas alfabéticamente.
 */
void MenuSetupInitEntry();
void MenuSetupPasswordEntry(uint8_t pass_len);
void MenuSetupPasswordEnter(menu_mode_t mode);
void MenuSetupFactorCalEntry();
ufp3_t MenuSetupFactorCalEdit(menu_mode_t mode);
void MenuSetupTimeUnitEntry();
void MenuSetupTimeUnitEdit(menu_mode_t mode);
void MenuSetupVolUnitEntry();
void MenuSetupVolUnitEdit(menu_mode_t mode);
void MenuSetupDateEntry();
void MenuSetupDateEdit(menu_mode_t mode);

// Private function bodies.

// Public function bodies.

/*
 *  Navegación por el menu.
 *
 */
uint8_t FM_SETUP_MenuNav(fmx_events_t this_event)
{
    static menu_user_t menu_index = 0;
    uint8_t menu_user = FALSE;

    /*
     *  La variable entry_counter se usa para ejecutar código cada vez que se ingresa a un nuevo menu.
     *  también se lo usa en algún caso como variable de control de otro aspecto del menu actual.
     */
    static uint8_t entry_counter = 0;
    uint8_t entries_to_exit;

    // Debe ser TRUE al finalizar el ingreso de la password para poder modificar valores del setup
    static uint8_t password_ok = TRUE;

    // La password correcta es apretar ABAJO ABAJO ARRIBA ENTER.
    const fmx_events_t password[] =
    {
    FMX_EVENT_KEY_DOWN,
    FMX_EVENT_KEY_DOWN,
    FMX_EVENT_KEY_UP,
    FMX_EVENT_KEY_ENTER };

    if ((password_ok == FALSE) && (menu_index > MENU_SETUP_PASSWORD))
    {
        if (this_event & (FMX_EVENT_KEY_DOWN + FMX_EVENT_KEY_UP + FMX_EVENT_KEY_ENTER))
        {
            this_event = FMX_EVENT_MENU_REFRESH;
        }
    }

    switch (menu_index)
    {
    case MENU_SETUP_INIT:
        if (!entry_counter)
        {
            FM_LCD_Init(0xff);
            password_ok = TRUE; // se presume password correcta hasta que se ingrese secuencia incorrecta.
        }
        entry_counter++;
        entries_to_exit = 3;
        if (entry_counter >= entries_to_exit)
        {
            entry_counter = 0; // Permite ejecutar función Entry del siguiente menu.
            menu_index++;  // en la próxima entada se ejecuta el siguiente menu.
            FMX_RefreshEventTrue();
        }
        break;
    case MENU_SETUP_PASSWORD:
        if (!entry_counter)
        {
            MenuSetupPasswordEntry(sizeof(password) / sizeof(fmx_events_t));
            MenuSetupPasswordEnter(MENU_MODE_INIT);
            entry_counter++;  // controla
        }
        switch (this_event)
        {
        case FMX_EVENT_MENU_REFRESH:
            break;
        case FMX_EVENT_KEY_DOWN:
            if (password[entry_counter - 1] != this_event)
            {
                password_ok = FALSE;
            }
            entry_counter++;
            MenuSetupPasswordEnter(MENU_MODE_INC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_UP:
            if (password[entry_counter - 1] != this_event)
            {
                password_ok = FALSE;
            }
            entry_counter++;
            MenuSetupPasswordEnter(MENU_MODE_INC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_ESC:
            if (password[entry_counter - 1] != this_event)
            {
                password_ok = FALSE;
            }
            entry_counter++;
            MenuSetupPasswordEnter(MENU_MODE_INC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_ENTER:
            if (password[entry_counter - 1] != this_event)
            {
                password_ok = FALSE;
            }
            entry_counter++;
            MenuSetupPasswordEnter(MENU_MODE_INC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_DOWN_LONG:
            break;
        case FMX_EVENT_KEY_UP_LONG:
            break;
        case FMX_EVENT_KEY_ESC_LONG:
            break;
        case FMX_EVENT_KEY_ENTER_LONG:
            break;
        default:
            FM_DEBUG_LedError(1);
            break;
        }

        /*
         * entry_counter cuenta la cantidad de teclas presionadas, si este valor es igual al tamaño de la
         * password es hora de salir de esta pantalla.
         */
        if (entry_counter > (sizeof(password) / sizeof(fmx_events_t)))
        {
            entry_counter = 0;
            menu_index++;
        }
        break;
    case MENU_SETUP_FACTOR_C:
        if (!entry_counter)
        {
            MenuSetupFactorCalEntry();
            entry_counter++;
        }
        switch (this_event)
        {
        case FMX_EVENT_MENU_REFRESH:
            MenuSetupFactorCalEdit(MENU_MODE_REFRESH);
            break;
        case FMX_EVENT_KEY_DOWN:
            MenuSetupFactorCalEdit(MENU_MODE_DEC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_UP:
            MenuSetupFactorCalEdit(MENU_MODE_INC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_ESC:
            MenuSetupFactorCalEdit(MENU_MODE_EXIT);
            FMX_RefreshEventTrue();
            entry_counter = 0;
            menu_index++;
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_ENTER:
            MenuSetupFactorCalEdit(MENU_MODE_NEXT);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_DOWN_LONG:
            break;
        case FMX_EVENT_KEY_UP_LONG:
            break;
        case FMX_EVENT_KEY_ESC_LONG:
            break;
        case FMX_EVENT_KEY_ENTER_LONG:
            break;
        default:
            FM_DEBUG_LedError(1);
            break;
        }
        break;
    case MENU_SETUP_VOL_UNIT:
        if (!entry_counter)
        {
            entry_counter++;
            MenuSetupVolUnitEntry();
        }
        switch (this_event)
        {
        case FMX_EVENT_MENU_REFRESH:
            MenuSetupVolUnitEdit(MENU_MODE_REFRESH);
            break;
        case FMX_EVENT_KEY_DOWN:
            MenuSetupVolUnitEdit(MENU_MODE_DEC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_UP:
            MenuSetupVolUnitEdit(MENU_MODE_INC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_ESC:
            MenuSetupVolUnitEdit(MENU_MODE_EXIT);
            entry_counter = 0;
            menu_index++;
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_ENTER:
            break;
        case FMX_EVENT_KEY_DOWN_LONG:
            break;
        case FMX_EVENT_KEY_UP_LONG:
            break;
        case FMX_EVENT_KEY_ESC_LONG:
            break;
        case FMX_EVENT_KEY_ENTER_LONG:
            break;
            FM_DEBUG_LedError(1);
        default:
            break;
        }
        break;
    case MENU_SETUP_TIME_UNIT:
        if (!entry_counter)
        {
            MenuSetupTimeUnitEntry();
            entry_counter++;
        }
        switch (this_event)
        {
        case FMX_EVENT_MENU_REFRESH:
            MenuSetupTimeUnitEdit(MENU_MODE_REFRESH);
            break;
        case FMX_EVENT_KEY_DOWN:
            MenuSetupTimeUnitEdit(MENU_MODE_DEC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_UP:
            MenuSetupTimeUnitEdit(MENU_MODE_INC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_ESC:
            MenuSetupTimeUnitEdit(MENU_MODE_EXIT);
            FMX_RefreshEventTrue();
            entry_counter = 0;
            menu_index++;
            break;
        case FMX_EVENT_KEY_ENTER:
            break;
        case FMX_EVENT_KEY_DOWN_LONG:
            break;
        case FMX_EVENT_KEY_UP_LONG:
            break;
        case FMX_EVENT_KEY_ESC_LONG:
            break;
        case FMX_EVENT_KEY_ENTER_LONG:
            FM_FMC_TtlReset();
            break;
        default:
            FM_DEBUG_LedError(1);
            break;
        }
        break;
    case MENU_SETUP_DATE:
        if (!entry_counter)
        {
            MenuSetupDateEntry();
            entry_counter++;
        }
        switch (this_event)
        {
        case FMX_EVENT_MENU_REFRESH:
            MenuSetupDateEdit(MENU_MODE_REFRESH);
            break;
        case FMX_EVENT_KEY_DOWN:
            MenuSetupDateEdit(MENU_MODE_DEC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_UP:
            MenuSetupDateEdit(MENU_MODE_INC);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_ESC:
            MenuSetupDateEdit(MENU_MODE_EXIT);
            FMX_RefreshEventTrue();
            entry_counter = 0;

            /*
             * Al salir de la ultima pantalla de configuración va la siguiente linea de código.
             * Si se agrega una pantalla de configuración, reemplazar MENU_SETUP_END por la nueva
             * pantalla y mover este a la nueva ultima pantalla
             */
            menu_index = MENU_SETUP_END;
            break;
        case FMX_EVENT_KEY_ENTER:
            MenuSetupDateEdit(MENU_MODE_NEXT);
            FMX_RefreshEventTrue();
            break;
        case FMX_EVENT_KEY_DOWN_LONG:
            break;
        case FMX_EVENT_KEY_UP_LONG:
            break;
        case FMX_EVENT_KEY_ESC_LONG:
            break;
        case FMX_EVENT_KEY_ENTER_LONG:
            break;
        default:
            FM_DEBUG_LedError(1);
            break;
        }
        break;
    case MENU_SETUP_END:
        FMX_RefreshEventTrue();
        FM_FMC_Init(FM_FACTORY_RAM_BACKUP); // Necesario para calcular factores con nueva configuración.
        menu_index = MENU_SETUP_INIT; // El indice ajustado para la próxima entrada al menu de configuración.
        menu_user = TRUE;  // Retorna al menu de usuario.
        FM_DEBUG_Init(); // Le los jumper de configuración y ajusta comportamiento UART y LEDs de debug.
        break;
    default:
        FM_DEBUG_LedError(1);
        break;
    }
    return menu_user;
}

/*
 *	@brief	Función llamada al ingresar al menu de configuraciones.
 *	@note
 *	@param	line_2, buffer que contiene lo que se imprime en la linea 2.
 *	@param	len, tamaño de la password.
 *	@retval	ninguno.
 */
void MenuSetupInitEntry()
{
    FM_LCD_Init(0xff);
}

/*
 * @brief
 * @note
 * @param	pass_tam, longitud de la password en caracteres.
 * @retval
 */
void MenuSetupPasswordEntry(uint8_t pass_len)
{
    const char msg_pass[] = "PASS";
    int i;

    FM_LCD_LL_Clear();
    FM_LCD_LL_BlinkClear();

    for (i = 0; i < pass_len; i++)
    {
        setup_line_2[i] = '_';
    }

    setup_line_2[i] = 0;  // Agrego terminador nulo

    FM_LCD_PutString(msg_pass, sizeof(msg_pass) - 1, FM_LCD_LL_ROW_1);
    FM_LCD_PutString(setup_line_2, strlen(setup_line_2), FM_LCD_LL_ROW_2);
}

/*
 * @brief		función para registrar el ingreso de la password.
 * @param 	mode 0, indica que se aca de ingresar al menu, se debe hacer una configuración inicial.
 * 					mode 1, registra tecla presionada.
 * 					mode 2, solo refresca la pantalla.
 * @retval	ninguno.
 */
void MenuSetupPasswordEnter(menu_mode_t cmd)
{
    const char debug_msg[] = "MenuSetupPasswordEnter";

    static uint8_t my_index = 0;

    FM_DEBUG_UartMsg(debug_msg, sizeof(debug_msg));

    switch (cmd)
    {
    case MENU_MODE_INIT:
        my_index = 0;
        break;
    case MENU_MODE_INC:
        my_index++;
        setup_line_2[my_index - 1] = '8';
        break;
    case MENU_MODE_EXIT: // no esta implementado, omito el break para detectar error de entrada.
    default:
        FM_DEBUG_LedError(1);
        break;
    }
    FM_LCD_PutString(setup_line_2, strlen(setup_line_2), FM_LCD_LL_ROW_2);
}

/*
 * @brief	función llamada al ingresar al menu de configuración del factor de calibración.
 * @param	ninguno.
 * retval	ninguno.
 */
void MenuSetupFactorCalEntry()
{
    const char msg_cal[] = "  PULS_";

    FM_LCD_LL_Clear();
    FM_LCD_LL_BlinkClear();

    // Existe una nueva función que hace estos dos pasos en uno, cambiar a nueva función.
    FM_LCD_LL_PutChar_1('L');
    FM_LCD_LL_PutChar_2('T');

    FM_LCD_PutString(msg_cal, strlen(msg_cal), FM_LCD_LL_ROW_2);

    // Activa parpadeo del digito menos significativo.
    FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, FM_LCD_LL_ROW_1_COLS - 1, FM_LCD_LL_BLINK_ON_ON);

    MenuSetupFactorCalEdit(MENU_MODE_INIT);
}

/*
 * @brief	Edita el factor de calibración
 * @note
 * @param	mode 0, al ingresar al menu de edición lee el factor actual y se prepara para editar.
 * 			mode 1, incrementa el digito en edición si es menor a 9
 * 			mode 2, decrementa el digito en edición si es mayor a 0
 * 			mode 99, indica que se esta saliendo de este menu,
 * @retval 	retorna una copia del factor original con los cambios realizados desde el ultimo mode 0.
 *
 */
ufp3_t MenuSetupFactorCalEdit(menu_mode_t mode)
{
    static ufp3_t factor_cal = 0; // Se usa una copia del factor de calibración hasta terminar la edición.
    static uint8_t my_index = 0;

    switch (mode)
    {
    case MENU_MODE_INIT:
        factor_cal = FM_FMC_FactorCalGet();
        my_index = 0;
        break;
    case MENU_MODE_INC:  // incrementa
        if ((factor_cal / power_of_10[my_index]) % 10 != 9)
        {
            factor_cal += power_of_10[my_index];
        }
        break;
    case MENU_MODE_DEC:  // decrementa
        if ((factor_cal / power_of_10[my_index]) % 10 != 0)
        {
            factor_cal -= power_of_10[my_index];
        }
        break;
    case MENU_MODE_NEXT:
        if (my_index < 7)
        {
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, (FM_LCD_LL_ROW_1_COLS - 1) - my_index,
                    FM_LCD_LL_BLINK_OFF);
            my_index++;
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, (FM_LCD_LL_ROW_1_COLS - 1) - my_index,
                    FM_LCD_LL_BLINK_ON_ON);
        }
        else
        {
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, (FM_LCD_LL_ROW_1_COLS - 1) - my_index,
                    FM_LCD_LL_BLINK_OFF);
            my_index = 0;
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, (FM_LCD_LL_ROW_1_COLS - 1) - my_index,
                    FM_LCD_LL_BLINK_ON_ON);
        }
        break;
    case MENU_MODE_EXIT: // Guarda el valor editado en en la variable de entorno.
        FM_FMC_FactorCalSet(factor_cal);
        break;
    case MENU_MODE_REFRESH:
        break;
    default:
        FM_DEBUG_LedError(1);  // Estado no permitido, enciendo led de error.
        break;
    }
    snprintf(setup_line_1, sizeof(setup_line_1), "%05lu.%03lu", factor_cal / 1000,
            factor_cal % 1000);
    FM_LCD_PutString(setup_line_1, FM_LCD_LL_ROW_1_COLS + 1, FM_LCD_LL_ROW_1);

    if (FM_LCD_LL_BlinkRefresh(0))
    {
        global_menu_refresh = 150;
    }

    return factor_cal;
}

/*
 * @brief	función llamada al ingresar el menu de configuración de la unidad de volumen.
 * @param	ninguno.
 * @retval	ninguno.
 */
void MenuSetupVolUnitEntry()
{

    FM_LCD_LL_Clear();
    FM_LCD_LL_BlinkClear();
    FM_LCD_LL_BlinkChar(1);
    MenuSetupVolUnitEdit(MENU_MODE_INIT);

    // Muestro unidad de tiempo.
    FM_FMC_TotalizerTimeUnitSel(FM_FMC_TotalizerTimeUnitGet());

    // Se muestra la barra invertida de las unidades.
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_BACKSLASH, 1);
}

/*
 * @brief 	cambia unidad de volumen
 * @param	mode 0, solo refresca
 * 			mode 1, cambia a siguiente unidad de volumen.
 * 			mode 2, cambia a anterior unidad de volumen.
 * @retval
 */
void MenuSetupVolUnitEdit(menu_mode_t mode)
{
    static ufp3_t factor_cal;
    double factor_k;
    char *ptr;

    static fm_fmc_vol_unit_t vol_unit = VOL_UNIT_LT;

    switch (mode)
    {
    case MENU_MODE_INIT:
        factor_cal = FM_FMC_FactorCalGet();
        vol_unit = FM_FMC_TotalizerVolUnitGet();
        break;
    case MENU_MODE_INC:
        if (vol_unit < (VOL_UNIT_END - 1))
        {
            vol_unit++;
        }
        break;
    case MENU_MODE_DEC:
        if (vol_unit > 0)
        {
            vol_unit--;
        }
        break;
    case MENU_MODE_EXIT:
        FM_FMC_TotalizerVolUnitSet(vol_unit);
        factor_k = FM_FMC_FactorKCalc(factor_cal, vol_unit);
        FM_FMC_FactorKSet(factor_k);
        break;
    case MENU_MODE_REFRESH:
        break;
    default:
        FM_DEBUG_LedError(1);
        break;
    }

    // Calculo el factor K con la ultima unidad selccionada.
    factor_k = FM_FMC_FactorKCalc(factor_cal, vol_unit);
    snprintf(setup_line_1, sizeof(setup_line_1), "%09.3f", factor_k);
    FM_LCD_PutString(setup_line_1, FM_LCD_LL_ROW_1_COLS + 1, FM_LCD_LL_ROW_1);

    // Muestro la unidad de volumen
    FM_FMC_TotalizerStrUnitGet(&ptr, vol_unit);
    FM_LCD_PutChar(ptr);

    if (FM_LCD_LL_BlinkRefresh(0))
    {
        global_menu_refresh = 150;
    }
}

/*
 * @brief	función llamada al ingresar el menu de configuración de la unidad de tiempo.
 * @param	ninguno.
 * @retval	ninguno.
 */
void MenuSetupTimeUnitEntry()
{
    char *ptr;
    static fm_fmc_vol_unit_t vol_unit = VOL_UNIT_LT;

    FM_LCD_LL_Clear();
    FM_LCD_LL_BlinkClear();

    // Escribe en pantalla unidad de volumen
    vol_unit = FM_FMC_TotalizerVolUnitGet();
    FM_FMC_TotalizerStrUnitGet(&ptr, vol_unit);
    FM_LCD_PutChar(ptr);

    // Se muestra la barra invertida de las unidades.
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_BACKSLASH, 1);

    // Habilito el parpadeo de todos las undades de tiempo.
    FM_LCD_LL_BlinkSymbol(FM_LCD_LL_SYM_S, FM_LCD_LL_BLINK_ON_ON);
    FM_LCD_LL_BlinkSymbol(FM_LCD_LL_SYM_M, FM_LCD_LL_BLINK_ON_ON);
    FM_LCD_LL_BlinkSymbol(FM_LCD_LL_SYM_H, FM_LCD_LL_BLINK_ON_ON);
    FM_LCD_LL_BlinkSymbol(FM_LCD_LL_SYM_D, FM_LCD_LL_BLINK_ON_ON);

    // Muestro leyenda de TTL por no tener una leyenda de pulsos TTL
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_TTL, 1);

    // Inicia la edición de este menu.
    MenuSetupTimeUnitEdit(MENU_MODE_INIT);
}

/*
 * @brief 	cambia unidad de volumen
 * @param	mode 0, solo refresca
 * 			mode 1, cambia a siguiente unidad tiempo.
 * 			mode 2, cambia a anterior unidad de tiempo.
 *
 */
void MenuSetupTimeUnitEdit(menu_mode_t mode)
{
    double factor_rate;
    static fm_fmc_time_unit_t time_unit = TIME_UNIT_SECOND;
    static double factor_k;
    uint32_t pulse_ttl;

    switch (mode)
    {
    case MENU_MODE_INIT:
        time_unit = FM_FMC_TotalizerTimeUnitGet();
        factor_k = FM_FMC_FactorKGet();
        break;
    case MENU_MODE_INC:
        if (time_unit < (TIME_UNIT_END - 1))
        {
            time_unit++;
        }
        break;
    case MENU_MODE_DEC:
        if (time_unit > TIME_UNIT_SECOND)
        {
            time_unit--;
        }
        break;
    case MENU_MODE_EXIT:
        if (FM_FMC_TotalizerTimeUnitSet(time_unit) == FMX_STATUS_OK)
        {
            // Si se cambio la unidad exitosamente se recalcula el factor de caudal.
            factor_rate = FM_FMC_FactorRateCalc(factor_k, time_unit);
            if (FM_FMC_FactorRateSet(factor_rate) != FMX_STATUS_OK)
            {
                FM_DEBUG_LedError(1);
            }
        }
        else
        {
            // No se pudo cambiar la unidad de tiempo.
            FM_DEBUG_LedError(1);
        }
        break;
    case MENU_MODE_REFRESH:
        break;
    default:
        FM_DEBUG_LedError(1);
        break;
    }

    // Muestro pulsos acumulados
    pulse_ttl = (uint32_t) FM_FMC_TtlPulseGet();

    // Cantidad de pulsos acumulados.
    snprintf(setup_line_2, sizeof(setup_line_2), "%07lu", pulse_ttl);
    FM_LCD_PutString(setup_line_2, FM_LCD_LL_ROW_2_COLS + 1, FM_LCD_LL_ROW_2);

    snprintf(setup_line_1, sizeof(setup_line_1), "%08lu", pulse_ttl / 10000000);
    FM_LCD_PutString(setup_line_1, FM_LCD_LL_ROW_1_COLS + 1, FM_LCD_LL_ROW_1);

    // Refresco la unidad de tiempo mostrada en pantalla.
    FM_FMC_TotalizerTimeUnitSel(time_unit);

    if (FM_LCD_LL_BlinkRefresh(0))
    {
        global_menu_refresh = 150;
    }

}

/*
 * @brief		ingreso al menu de configuración de hora, minuto y segundo.
 * @param		ninguno.
 * @retval	ninguno.
 */
void MenuSetupDateEntry()
{

    FM_LCD_LL_Clear();
    FM_LCD_LL_BlinkClear();

    // activa parpadeo del digito menos significativo.
    FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, FM_LCD_LL_ROW_1_COLS - 1, FM_LCD_LL_BLINK_ON_ON);

    MenuSetupDateEdit(MENU_MODE_INIT);
}

/*
 * @brief 	edición de hora, minuto y segundo.
 * @param	mode, comando a ejecutar.
 * @retval	ninguno.
 */
void MenuSetupDateEdit(menu_mode_t sel)
{
    static fm_rtc_set_t idx = FM_RTC_SET_YEAR;  //

    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    switch (sel)
    {
    case MENU_MODE_INIT:
        idx = FM_RTC_SET_YEAR;
        break;
    case MENU_MODE_INC:
        FM_RTC_Set(idx, 1);
        break;
    case MENU_MODE_DEC:
        FM_RTC_Set(idx, 0);
        break;
    case MENU_MODE_NEXT:
        if (idx < FM_RTC_SET_SECOND)
        {
            idx++;
        }
        else
        {
            idx = 0;
        }

        FM_LCD_LL_BlinkClear();
        switch (idx)
        {
        case FM_RTC_SET_YEAR:
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, FM_LCD_LL_ROW_1_COLS - 1, FM_LCD_LL_BLINK_ON_ON);
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, FM_LCD_LL_ROW_1_COLS - 2, FM_LCD_LL_BLINK_ON_ON);
            break;
        case FM_RTC_SET_MONTH:
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, FM_LCD_LL_ROW_1_COLS - 5, FM_LCD_LL_BLINK_ON_ON);
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, FM_LCD_LL_ROW_1_COLS - 6, FM_LCD_LL_BLINK_ON_ON);
            break;
        case FM_RTC_SET_DAY:
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, FM_LCD_LL_ROW_1_COLS - 7, FM_LCD_LL_BLINK_ON_ON);
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_1, FM_LCD_LL_ROW_1_COLS - 8, FM_LCD_LL_BLINK_ON_ON);
            break;
        case FM_RTC_SET_HOUR:
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_2, FM_LCD_LL_ROW_2_COLS - 6, FM_LCD_LL_BLINK_ON_ON);
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_2, FM_LCD_LL_ROW_2_COLS - 7, FM_LCD_LL_BLINK_ON_ON);
            break;
        case FM_RTC_SET_MINUTE:
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_2, FM_LCD_LL_ROW_2_COLS - 4, FM_LCD_LL_BLINK_ON_ON);
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_2, FM_LCD_LL_ROW_2_COLS - 5, FM_LCD_LL_BLINK_ON_ON);
            break;
        case FM_RTC_SET_SECOND:
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_2, FM_LCD_LL_ROW_2_COLS - 2, FM_LCD_LL_BLINK_ON_ON);
            FM_LCD_LL_BlinkNumber(FM_LCD_LL_ROW_2, FM_LCD_LL_ROW_2_COLS - 3, FM_LCD_LL_BLINK_ON_ON);
            break;
        default:
            break;
        }
        break;
    case MENU_MODE_EXIT:
        FM_LCD_LL_BlinkClear();
        break;
    case MENU_MODE_REFRESH:
        break;
    default:
        FM_DEBUG_LedError(1);
        break;
    }

    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);

    sprintf(setup_line_2, "%02d.%02d.%02d ", time.Hours, time.Minutes, time.Seconds);
    sprintf(setup_line_1, "%02d.%02d.20%02d ", date.Date, date.Month, date.Year);

    FM_LCD_PutString(setup_line_1, FM_LCD_LL_ROW_1_COLS, FM_LCD_LL_ROW_1);
    FM_LCD_PutString(setup_line_2, FM_LCD_LL_ROW_2_COLS, FM_LCD_LL_ROW_2);

    if (FM_LCD_LL_BlinkRefresh(0))
    {
        global_menu_refresh = 150;
    }
}

// Interrupts

/*** end of file ***/

