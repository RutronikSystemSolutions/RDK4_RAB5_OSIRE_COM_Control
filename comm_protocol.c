/******************************************************************************
* File Name:   comm_protocol.c
*
*
* Related Document: See README.md
*
*
*  Created on: 2026-10-22
*  Company: Rutronik Elektronische Bauelemente GmbH
*  Author: RJ030
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*
* Rutronik Elektronische Bauelemente GmbH Disclaimer: The evaluation board
* including the software is for testing purposes only and,
* because it has limited functions and limited resilience, is not suitable
* for permanent use under real conditions. If the evaluation board is
* nevertheless used under real conditions, this is done at one’s responsibility;
* any liability of Rutronik is insofar excluded
*******************************************************************************/
#include "comm_protocol.h"

#include "cyhal.h"

#define TIMEOUT_MS	10

#define HEADER_VALUE 0x31

#define ADDR_BROADCAST 	0
#define ADDR_MAX		6

/**
 * @def PARAM_LEN
 * Len of the parameters (R,G,B) each on 2 bytes
 */
#define PARAM_LEN		6

typedef enum
{
	WAITING_HEADER,
	WAITING_CMD,
	WAITING_PARAMETERS
} internal_status_e;

static cyhal_timer_t time_timer;
const cyhal_timer_cfg_t cfg = {
    .compare_value = 0,
    .period        = 0xFFFFFFFF,       // free-running 32-bit
    .direction     = CYHAL_TIMER_DIR_UP,
    .is_compare    = false,
    .is_continuous = true,
    .value         = 0
};

static internal_status_e internal_status;

/**
 * @var internal_listener
 * Function to be called when an event is detected
 */
static comm_protocol_listener_func_t internal_listener = NULL;

void comm_protocol_init(comm_protocol_listener_func_t listener)
{
	internal_listener = listener;
	
	cyhal_timer_init(&time_timer, NC, NULL);
	cyhal_timer_configure(&time_timer, &cfg);
	cyhal_timer_set_frequency(&time_timer, 1000);   // 1 kHz -> 1 tick = 1 ms
	cyhal_timer_start(&time_timer);

	internal_status = WAITING_HEADER;
}

static uint8_t transaction_addr = 0;
static uint8_t transaction_parameters[PARAM_LEN];
static uint8_t transaction_param_index = 0;

static void internal_push(uint8_t val)
{
	// Check for timeout (in case not waiting for header)
	if (internal_status != WAITING_HEADER)
	{
		uint32_t elapsed_time = cyhal_timer_read(&time_timer);
		if (elapsed_time > TIMEOUT_MS)
		{
			internal_status = WAITING_HEADER;
			return;
		}
	}

	// Restart timer
	cyhal_timer_reset(&time_timer);

	switch(internal_status)
	{
		case WAITING_HEADER:
			if (val == HEADER_VALUE)
			{
				internal_status = WAITING_CMD;
			}
			break;

		case WAITING_CMD:
			if (val > ADDR_MAX)
			{
				internal_status = WAITING_HEADER;
			}
			transaction_addr = val;
			transaction_param_index = 0;
			internal_status = WAITING_PARAMETERS;
			break;

		case WAITING_PARAMETERS:
			transaction_parameters[transaction_param_index] = val;
			transaction_param_index++;
			if (transaction_param_index == PARAM_LEN)
			{
				internal_status = WAITING_HEADER;
				uint16_t r = ((uint16_t*)&transaction_parameters)[0];
				uint16_t g = ((uint16_t*)&transaction_parameters)[1];
				uint16_t b = ((uint16_t*)&transaction_parameters)[2];
				internal_listener(transaction_addr, r, g, b);
			}
			break;
	}

}

void comm_protocol_push(uint8_t* buffer, size_t size)
{
	for(int i = 0; i < size; ++i)
	{
		internal_push(buffer[i]);
	}
}
