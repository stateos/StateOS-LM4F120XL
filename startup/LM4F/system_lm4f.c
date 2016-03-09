/******************************************************************************
 * @file    system_lm4f.c
 * @author  Rajmund Szymanski
 * @date    08.03.2016
 * @brief   This file provides set of configuration functions for LM4F uC.
 ******************************************************************************/

#include <lm4f.h>
#include <hw_types.h>
#include <hw_sysctl.h>
#include <sysctl.h>
#include <rom.h>

/* -------------------------------------------------------------------------- */

__attribute__ (( weak ))
void SystemInit( void )
{
	ROM_SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_SYSDIV_2_5 | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);     // 80MHz
//	ROM_SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_SYSDIV_2_5 | SYSCTL_OSC_INT    | SYSCTL_MAIN_OSC_DIS); // 80MHz
}

/* -------------------------------------------------------------------------- */

__attribute__ (( weak ))
uint32_t SystemCoreClock = 80000000;

/* -------------------------------------------------------------------------- */
