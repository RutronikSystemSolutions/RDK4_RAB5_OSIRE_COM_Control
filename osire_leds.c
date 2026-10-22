/******************************************************************************
* File Name:   osire_leds.c
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


#include "osire_leds.h"

#include "osp/aospi/aospi.h"
#include "hal/osp_spi.h"
#include "osp/aoosp/aoosp_send.h"

#include "cybsp.h"

/**
 * @def OSP_NODE_COUNT
 * @brief For this demo, we use the RAB5-OSIRE board with 5 nodes
 * 4 are E3731i LEDs and 1 (address = 3) is an AS1163 (SAID driver)
 */
#define OSP_NODE_COUNT 5

int osire_leds_init()
{
	// Initialize AOSPI layer
	// MCU-A -> we are using only SPI_MOSI (no CLK) therefore the communication is Manchester encoded
	aoresult_t osp_result = aospi_init(aospi_phy_mcua, osp_spi_init, osp_spi_tx, osp_spi_txrx);
	if (osp_result != aoresult_ok) return -1;

	// Send reset to all OSP nodes (0 for broadcast)
	osp_result = aoosp_send_reset(0);
	if (osp_result != aoresult_ok) return -2;

	// Delay time after reset
	Cy_SysLib_Delay(1);

	// Assumption: BIDIR mode
	uint16_t last_addr = 0;
	uint8_t node_temperature = 0;
	uint8_t node_status = 0;
	osp_result = aoosp_send_initbidir(0x1, &last_addr, &node_temperature, &node_status);
	if (osp_result != aoresult_ok) return -3;

	// Clear error (broadcast)
	osp_result = aoosp_send_clrerror(0x0);

	// Go active (at start, the OSP devices are in sleep mode)
	osp_result = aoosp_send_goactive(0);

	// Check the status of each node (should be active now)
	for(uint16_t i = 0; i < last_addr; ++i)
	{
		uint16_t node_addr = (i+1);
		uint8_t node_status = 0;
		osp_result = aoosp_send_readstat(node_addr, &node_status);
	}

	// For this example, expect OSP_NODE_COUNT
	if (last_addr != OSP_NODE_COUNT) return -4;

	return 0;
}

int osire_leds_set_init_state()
{
	// Set some colors (red/green)
	if (aoosp_send_setpwm(1, 0, 3000, 0, 0) != aoresult_ok) return -1;
	if (aoosp_send_setpwm(2, 3000, 0, 0, 0) != aoresult_ok) return -1;
	if (aoosp_send_setpwmchn(3, 0, 3000, 0, 0) != aoresult_ok) return -1;
	if (aoosp_send_setpwmchn(3, 1, 3000, 0, 0) != aoresult_ok) return -1;
	if (aoosp_send_setpwm(4, 3000, 0, 0, 0) != aoresult_ok) return -1;
	if (aoosp_send_setpwm(5, 0, 3000, 0, 0) != aoresult_ok) return -1;

	return 0;
}

void osire_leds_set(uint8_t addr, uint16_t r, uint16_t g, uint16_t b)
{
	if (addr == 0)
	{
		// Set all (SAID included)
		aoosp_send_setpwm(0, r, g, b, 0);
		aoosp_send_setpwmchn(3, 0, r, g, b);
		aoosp_send_setpwmchn(3, 1, r, g, b);
	}
	else if (addr == 1)
	{
		// SAID channel 0
		aoosp_send_setpwmchn(3, 0, r, g, b);
	}
	else if (addr == 2)
	{
		aoosp_send_setpwm(1, r, g, b, 0);
	}
	else if (addr == 3)
	{
		aoosp_send_setpwm(2, r, g, b, 0);
	}
	else if (addr == 4)
	{
		aoosp_send_setpwm(4, r, g, b, 0);
	}
	else if (addr == 5)
	{
		aoosp_send_setpwm(5, r, g, b, 0);
	}
	else if (addr == 6)
	{
		// SAID channel 1
		aoosp_send_setpwmchn(3, 1, r, g, b);
	}
	
	
}
