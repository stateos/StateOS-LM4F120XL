/******************************************************************************
 * @file    lm4f_120xl_btns.h
 * @author  Rajmund Szymanski
 * @date    27.02.2016
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

#define WUB BITBAND(GPIOF->DATA)[0]  // wakeup button
#define BTN BITBAND(GPIOF->DATA)[4]  // user button

/* -------------------------------------------------------------------------- */

static inline
void BTN_Init( void ) // p0: wakeup button, p4: user button
{
	// ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	// ROM_GPIODirModeSet  (GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_DIR_MODE_IN);
	// ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_PIN_TYPE_STD_WPU);
	BITBAND(SYSCTL->RCGCGPIO)[5] = 1; // GPIOF
	SYSCTL->RCGCGPIO;
	GPIOF->DEN  |= 0x11;
	GPIOF->PUR  |= 0x11;
	GPIOF->DATA |= 0x11;
}

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif 
