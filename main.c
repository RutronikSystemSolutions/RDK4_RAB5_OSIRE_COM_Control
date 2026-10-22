/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the RDK4 Rev2 RAB5-OSIRE E3731i
*              communication control example for ModusToolbox.
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

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "osire_leds.h"
#include "comm_protocol.h"
#include "sbc_rab5_osire.h"

/**
 * Buffer used to read stream coming from the UART port
 */
#define RX_READ_BUFFER_SIZE	32
static uint8_t rx_buffer[RX_READ_BUFFER_SIZE];

/**
 * @brief Hook function called when the command listener requests a color change
 *
 * @param addr: Address of the LED to be changed (0 for broadcast)
 * @param r: Red color
 * @param g: Green color
 * @param b: Blue color
 */
static void cmd_listener(uint8_t addr, uint16_t r, uint16_t g, uint16_t b)
{
	printf("%d -> %d, %d, %d\r\n", addr, r, g, b);
	osire_leds_set(addr, r, g, b);

}

/**
 * This timer is used to trigger the SBC watchdog at periodic interval
 */
static cyhal_timer_t watchdog_timer;
static const cyhal_timer_cfg_t watchdog_cfg = {
    .compare_value = 0,
    .period        = 0xFFFFFFFF,       // free-running 32-bit
    .direction     = CYHAL_TIMER_DIR_UP,
    .is_compare    = false,
    .is_continuous = true,
    .value         = 0
};

int main(void)
{
	cy_rslt_t result = CY_RSLT_SUCCESS;

	// Initialize the device and board peripherals
	result = cybsp_init();
	if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

	// Initialize the SBC (power supply, watchdog)
	sbc_rab5_osire_init();

	// Enable debug output via KitProg UART
	result = cy_retarget_io_init( KITPROG_TX, KITPROG_RX, 115200);
	if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

	__enable_irq();

	printf("RDK4 - RAB5-OSIRE E3731i COM Control \r\n");

	// Initialize the LEDs
	int osire_retval = osire_leds_init();
	if (osire_retval != 0)
	{
		printf("osire_leds_init error : %d\r\n", osire_retval);
		return 0;
	}

	osire_retval = osire_leds_set_init_state();
	if (osire_retval != 0)
	{
		printf("osire_leds_set_init_state error : %d\r\n", osire_retval);
		return 0;
	}

	printf("LEDs initialized\r\n");

	// Initialize the communication protocol
	comm_protocol_init(cmd_listener);

	printf("Communication protocol initialized\r\n");

	// Initialize watchdog timer
	cyhal_timer_init(&watchdog_timer, NC, NULL);
	cyhal_timer_configure(&watchdog_timer, &watchdog_cfg);
	cyhal_timer_set_frequency(&watchdog_timer, 1000);   // 1 kHz -> 1 tick = 1 ms
	cyhal_timer_start(&watchdog_timer);

	// Endless loop
	for(;;)
	{
		// Read from UART
		uint32_t readable_bytes = cyhal_uart_readable(&cy_retarget_io_uart_obj);
		if (readable_bytes > 0)
		{
			size_t to_read = (readable_bytes > RX_READ_BUFFER_SIZE) ? RX_READ_BUFFER_SIZE : readable_bytes;
			if (cyhal_uart_read (&cy_retarget_io_uart_obj, rx_buffer, &to_read) != CY_RSLT_SUCCESS)
			{
				printf("Error reading\r\n");
			}

			// Push the received data
			// in case of success, the listener function will be called
			comm_protocol_push(rx_buffer, to_read);
		}

		// Need to trigger the watchdog? (every 500ms)
		uint32_t elapsed_time = cyhal_timer_read(&watchdog_timer);
		if (elapsed_time > 500)
		{
			cyhal_timer_reset(&watchdog_timer);
			sbc_rab5_osire_feed_wdt();
		}
	}
	
}

/* [] END OF FILE */
