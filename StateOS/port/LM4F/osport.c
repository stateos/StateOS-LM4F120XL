/******************************************************************************

    @file    StateOS: osport.c
    @author  Rajmund Szymanski
    @date    01.08.2017
    @brief   StateOS port file for LM4F uC.

 ******************************************************************************

    StateOS - Copyright (C) 2013 Rajmund Szymanski.

    This file is part of StateOS distribution.

    StateOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.

    StateOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

 ******************************************************************************/

#include <oskernel.h>

/* -------------------------------------------------------------------------- */

void port_sys_init( void )
{
#if OS_TICKLESS

/******************************************************************************
 Put here configuration of system timer for tick-less mode
*******************************************************************************/

	#if CPU_FREQUENCY/OS_FREQUENCY-1 > UINT16_MAX
	#error Incorrect Timer frequency!
	#endif

	SYSCTL->RCGCWTIMER |= SYSCTL_RCGCWTIMER_R0;
	#if OS_ROBIN
	NVIC_SetPriority(WTIMER0A_IRQn, 0xFF);
	NVIC_EnableIRQ(WTIMER0A_IRQn);
	#endif
	WTIMER0->CFG  = 4;
	#if OS_ROBIN
	WTIMER0->TAMR = TIMER_TAMR_TAMR_PERIOD | TIMER_TAMR_TAMIE;
	#else
	WTIMER0->TAMR = TIMER_TAMR_TAMR_PERIOD;
	#endif
	WTIMER0->TAPR = CPU_FREQUENCY/OS_FREQUENCY-1;
	WTIMER0->CTL  = TIMER_CTL_TAEN;

/******************************************************************************
 End of configuration
*******************************************************************************/

/******************************************************************************
 Put here configuration of interrupt for context switch triggering
*******************************************************************************/

	#if OS_ROBIN

	#if (CPU_FREQUENCY/OS_ROBIN-1 <= SysTick_LOAD_RELOAD_Msk)

	SysTick_Config(CPU_FREQUENCY/OS_ROBIN);

	#elif defined(ST_FREQUENCY) && (ST_FREQUENCY/OS_ROBIN-1 <= SysTick_LOAD_RELOAD_Msk)

	NVIC_SetPriority(SysTick_IRQn, 0xFF);

	SysTick->LOAD = ST_FREQUENCY/OS_ROBIN-1;
	SysTick->VAL  = 0U;
	SysTick->CTRL = SysTick_CTRL_ENABLE_Msk|SysTick_CTRL_TICKINT_Msk;

	#else
	#error Incorrect SysTick frequency!
	#endif
	
	#endif//OS_ROBIN

/******************************************************************************
 End of configuration
*******************************************************************************/

#else //OS_TICKLESS == 0

/******************************************************************************
 Put here configuration of system timer for non-tick-less mode
*******************************************************************************/

	#if (CPU_FREQUENCY/OS_FREQUENCY-1 <= SysTick_LOAD_RELOAD_Msk)

	SysTick_Config(CPU_FREQUENCY/OS_FREQUENCY);

	#elif defined(ST_FREQUENCY) && (ST_FREQUENCY/OS_FREQUENCY-1 <= SysTick_LOAD_RELOAD_Msk)

	NVIC_SetPriority(SysTick_IRQn, 0xFF);

	SysTick->LOAD = ST_FREQUENCY/OS_FREQUENCY-1;
	SysTick->VAL  = 0U;
	SysTick->CTRL = SysTick_CTRL_ENABLE_Msk|SysTick_CTRL_TICKINT_Msk;

	#else
	#error Incorrect SysTick frequency!
	#endif

/******************************************************************************
 End of configuration
*******************************************************************************/

#endif//OS_TICKLESS

/******************************************************************************
 Put here configuration of interrupt for context switch
*******************************************************************************/

	NVIC_SetPriority(PendSV_IRQn, 0xFF);

/******************************************************************************
 End of configuration
*******************************************************************************/
}

/* -------------------------------------------------------------------------- */

#if OS_TICKLESS == 0

/******************************************************************************
 Put here the procedure of interrupt handler of system timer for non-tick-less mode
*******************************************************************************/

void SysTick_Handler( void )
{
	SysTick->CTRL;
	System.cnt++;
#if OS_ROBIN
	core_tmr_handler();
	System.dly++;
	if (System.dly >= OS_FREQUENCY/OS_ROBIN)
	core_ctx_switch();
#endif
}

/******************************************************************************
 End of the procedure of interrupt handler
*******************************************************************************/

#endif//OS_TICKLESS

/* -------------------------------------------------------------------------- */

#if OS_TICKLESS != 0
#if OS_ROBIN

/******************************************************************************
 Put here procedures of interrupt handlers of system timer for tick-less mode witch preemption
*******************************************************************************/

void WTIMER0A_Handler( void )
{
	WTIMER0->ICR = ~0U;
	core_tmr_handler();
}

void SysTick_Handler( void )
{
	SysTick->CTRL;
	core_ctx_switch();
}

/******************************************************************************
 End of the procedure of interrupt handler
*******************************************************************************/

#endif
#endif

/* -------------------------------------------------------------------------- */
