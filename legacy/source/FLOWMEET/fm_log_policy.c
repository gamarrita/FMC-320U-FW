/**
 * @file
 * @brief
 *
 *
 *
 *
 *
 */

#include <string.h> // For memset
#include "fm_log_policy.h"
#include "fmx.h"


// --- Constants ---

#define CREDITS_MAX		(128u)
#define CREDIT_NEW_MIN	(60u * 5u)
#define CREDITS_RATE_ON	(1u)

// --- Globals ---


// --- Static Data ---
static uint8_t 	global_credits = CREDITS_MAX;





/**
 * @brief	Evalúa la política y determina si se debe registrar un evento.
 * 			Se llama a esta funcion cada un segundo.
 * @param in Puntero a la estructura de entrada con el estado actual.
 * @return Acción a realizar (ver fm_log_action_t).
 */
uint32_t FM_LOG_POLICY_Step(fmx_ack_t ack) {

	uint8_t credits;
	credits = global_credits;
	static fmx_ack_t ack_last;


	//uint32_t static
	if(credits)
	{

	}
	else
	{
		return 0;
	}



	// Podria considerar una accion especial para logger aun sin creditos.
	switch(ack)
	{
	case FMX_ACK_NONE:
		break;
	case FMX_ACK_RATE_OFF:
		if (ack_last != ack)
		{
			credits--;
		}
		break;
	case FMX_ACK_RATE_ON:
		if (ack_last != ack)
		{
			credits--;
		}
		break;
	case FMX_ACK_RATE_STARTED:
		credits--;
		break;
	case FMX_ACK_RATE_STOPED:
		credits--;
		break;
	default:
		break;
	}

	ack_last = ack;

	// Si se desconto un credigo, loggeo el dato.
	if(global_credits == (credits + 1))
	{
		global_credits--;
		return 1;
	}

	return 0;
}


/**
 * @brief	Evalúa la política y determina si se debe registrar un evento.
 * 			Se llama a esta funcion cada un segundo.
 * @param in Puntero a la estructura de entrada con el estado actual.
 * @return Acción a realizar (ver fm_log_action_t).
 */
uint32_t FM_LOG_POLICY_Timer() {
	static uint32_t credit_timer = CREDIT_NEW_MIN;


	// TODO, aun no tengo una llamada a esta funcion cada un segundo, hay que implementar.

	if(credit_timer)
	{
		credit_timer--;
	}
	else
	{
		credit_timer = CREDIT_NEW_MIN;
		if(global_credits < CREDITS_MAX)
		{
			global_credits++;
		}
	}
	return 0;
}




