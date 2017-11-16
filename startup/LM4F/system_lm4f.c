/******************************************************************************
 * @file    system_lm4f.c
 * @author  Rajmund Szymanski
 * @date    02.11.2017
 * @brief   This file provides set of configuration functions for LM4F uC.
 ******************************************************************************/

#include <lm4f.h>
#include <inc/hw_types.h>
#include <inc/hw_sysctl.h>
#include <driverlib/sysctl.h>
#include <driverlib/rom.h>

/* -------------------------------------------------------------------------- */

#ifndef __NO_SYSTEM_INIT
__WEAK
void SystemInit( void )
{
	ROM_SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_SYSDIV_2_5 | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);     // 80MHz
//	ROM_SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_SYSDIV_2_5 | SYSCTL_OSC_INT    | SYSCTL_MAIN_OSC_DIS); // 80MHz
}
#endif//__NO_SYSTEM_INIT

/* -------------------------------------------------------------------------- */

#ifndef __NO_SYSTEM_INIT
__WEAK
void SystemCoreClockUpdate( void )
{
}
#endif//__NO_SYSTEM_INIT

/* -------------------------------------------------------------------------- */

#ifndef __NO_SYSTEM_INIT
__WEAK
uint32_t SystemCoreClock = 80000000;
#else
__WEAK
uint32_t SystemCoreClock = 16000000;
#endif//__NO_SYSTEM_INIT

/* -------------------------------------------------------------------------- */
