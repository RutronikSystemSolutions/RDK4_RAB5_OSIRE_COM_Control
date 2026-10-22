/******************************************************************************
* File Name:   osire_leds.h
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

#ifndef OSIRE_LEDS_H_
#define OSIRE_LEDS_H_

#include <stdint.h>

/**
 * @brief Initialize the module
 *
 * Check the LEDs that are on the bus (over SPI)
 *
 * @retval 0 Success
 * @retval != 0 Some error occurred
 */
int osire_leds_init();

/**
 * @brief Set initialization color (red and green)
 *
 * @retval 0 Success
 * @retval != 0 Some error occurred
 */
int osire_leds_set_init_state();

/**
 * @brief Set color for a LED
 *
 * addr: Address of the LED to be changed (0 for broadcast)
 * @param r: Red color
 * @param g: Green color
 * @param b: Blue color
 */
void osire_leds_set(uint8_t addr, uint16_t r, uint16_t g, uint16_t b);

#endif /* OSIRE_LEDS_H_ */
