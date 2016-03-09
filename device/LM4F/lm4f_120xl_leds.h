/******************************************************************************
 * @file    lm4f_120xl_leds.h
 * @author  Rajmund Szymanski
 * @date    09.03.2016
 * @brief   This file contains definitions for EK-LM4F120XL Kit.
 ******************************************************************************/

#pragma once

/* Includes ----------------------------------------------------------------- */

#ifndef PART_LM4F120H5QR
#define PART_LM4F120H5QR
#endif
#include <lm4f.h>

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif 

/* Board macros ------------------------------------------------------------- */

#define LED (BITBAND(GPIOF->DATA)+1) // leds array
#define LEDR BITBAND(GPIOF->DATA)[1] // red led
#define LEDB BITBAND(GPIOF->DATA)[2] // blue led
#define LEDG BITBAND(GPIOF->DATA)[3] // green led
#define LEDs ((uint32_t*)(GPIOF))[0x0E]

/* -------------------------------------------------------------------------- */

static inline
void LED_Config( void ) // p1: red led, p2:blue led, p3:green led
{
	// ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	// ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); // Red
	// ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // Blue
	// ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // Green
	BITBAND(SYSCTL->RCGCGPIO)[5] = 1; // GPIOF
	SYSCTL->RCGCGPIO;
	GPIOF->DEN  |= 0x0E;
	GPIOF->DIR  |= 0x0E;
}

/* -------------------------------------------------------------------------- */

static inline
void LED_Tick( void )
{
	int t = LEDs << 1;
	LEDs  = (t & 0x0E) ? t : 2;
}

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif 
