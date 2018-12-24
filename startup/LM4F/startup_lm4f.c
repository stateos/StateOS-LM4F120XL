/*******************************************************************************
@file     startup.c
@author   Rajmund Szymanski
@date     21.12.2018
@brief    LM4F120H5QR startup file.
          After reset the Cortex-M4 processor is in thread mode,
          priority is privileged, and the stack is set to main.
*******************************************************************************/

#include <LM4F120H5QR.h>

/*******************************************************************************
 Specific definitions for the chip
*******************************************************************************/

#define   RAM_start 0x20000000
#define   RAM_end   0x20008000

/*******************************************************************************
 Configuration of stacks
*******************************************************************************/

#ifndef   main_stack_size
#define   main_stack_size    0    // <- default size of main stack
#endif
#ifndef   proc_stack_size
#define   proc_stack_size    0    // <- default size of process stack
#endif

/*******************************************************************************
 Initial stacks' pointers
*******************************************************************************/

extern char __initial_msp[];
extern char __initial_sp [];

/*******************************************************************************
 Default _exit handler
*******************************************************************************/

__WEAK __NO_RETURN
void _exit( int status )
{
	(void) status;
	/* Go into an infinite loop */
	for (;;);
}

/*******************************************************************************
 Default fault handler
*******************************************************************************/

__NO_RETURN
void Fault_Handler( void )
{
	/* Go into an infinite loop */
	for (;;);
}

/*******************************************************************************
 Specific definitions for the compiler
*******************************************************************************/

#include "startup.h"

/*******************************************************************************
 Default reset handler
*******************************************************************************/

__WEAK __NO_RETURN
void Reset_Handler( void )
{
#if proc_stack_size > 0
	/* Initialize the process stack pointer */
	__set_PSP((uint32_t) __initial_sp);
	__set_CONTROL(CONTROL_SPSEL_Msk);
#endif
#if __FPU_USED
#ifndef __ICCARM__
	/* Set CP10 and CP11 Full Access */
	SCB->CPACR = 0x00F00000U;
#endif
#endif
#ifndef __NO_SYSTEM_INIT
	/* Call the system clock intitialization function */
	SystemInit();
#endif
	/* Call the application's entry point */
	__main();
}

/*******************************************************************************
 Declaration of exception handlers
*******************************************************************************/

/* Core exceptions */
__ALIAS(Fault_Handler) void NMI_Handler                   (void);
__ALIAS(Fault_Handler) void HardFault_Handler             (void);
__ALIAS(Fault_Handler) void MemManage_Handler             (void);
__ALIAS(Fault_Handler) void BusFault_Handler              (void);
__ALIAS(Fault_Handler) void UsageFault_Handler            (void);
__ALIAS(Fault_Handler) void SVC_Handler                   (void);
__ALIAS(Fault_Handler) void DebugMon_Handler              (void);
__ALIAS(Fault_Handler) void PendSV_Handler                (void);
__ALIAS(Fault_Handler) void SysTick_Handler               (void);

/* External interrupts */
__ALIAS(Fault_Handler) void GPIOA_Handler                 (void);
__ALIAS(Fault_Handler) void GPIOB_Handler                 (void);
__ALIAS(Fault_Handler) void GPIOC_Handler                 (void);
__ALIAS(Fault_Handler) void GPIOD_Handler                 (void);
__ALIAS(Fault_Handler) void GPIOE_Handler                 (void);
__ALIAS(Fault_Handler) void UART0_Handler                 (void);
__ALIAS(Fault_Handler) void UART1_Handler                 (void);
__ALIAS(Fault_Handler) void SSI0_Handler                  (void);
__ALIAS(Fault_Handler) void I2C0_Handler                  (void);
__ALIAS(Fault_Handler) void PWM0_FAULT_Handler            (void);
__ALIAS(Fault_Handler) void PWM0_0_Handler                (void);
__ALIAS(Fault_Handler) void PWM0_1_Handler                (void);
__ALIAS(Fault_Handler) void PWM0_2_Handler                (void);
__ALIAS(Fault_Handler) void QEI0_Handler                  (void);
__ALIAS(Fault_Handler) void ADC0SS0_Handler               (void);
__ALIAS(Fault_Handler) void ADC0SS1_Handler               (void);
__ALIAS(Fault_Handler) void ADC0SS2_Handler               (void);
__ALIAS(Fault_Handler) void ADC0SS3_Handler               (void);
__ALIAS(Fault_Handler) void WATCHDOG_Handler              (void);
__ALIAS(Fault_Handler) void TIMER0A_Handler               (void);
__ALIAS(Fault_Handler) void TIMER0B_Handler               (void);
__ALIAS(Fault_Handler) void TIMER1A_Handler               (void);
__ALIAS(Fault_Handler) void TIMER1B_Handler               (void);
__ALIAS(Fault_Handler) void TIMER2A_Handler               (void);
__ALIAS(Fault_Handler) void TIMER2B_Handler               (void);
__ALIAS(Fault_Handler) void COMP0_Handler                 (void);
__ALIAS(Fault_Handler) void COMP1_Handler                 (void);
__ALIAS(Fault_Handler) void COMP2_Handler                 (void);
__ALIAS(Fault_Handler) void SYSCTL_Handler                (void);
__ALIAS(Fault_Handler) void FLASH_Handler                 (void);
__ALIAS(Fault_Handler) void GPIOF_Handler                 (void);
__ALIAS(Fault_Handler) void GPIOG_Handler                 (void);
__ALIAS(Fault_Handler) void GPIOH_Handler                 (void);
__ALIAS(Fault_Handler) void UART2_Handler                 (void);
__ALIAS(Fault_Handler) void SSI1_Handler                  (void);
__ALIAS(Fault_Handler) void TIMER3A_Handler               (void);
__ALIAS(Fault_Handler) void TIMER3B_Handler               (void);
__ALIAS(Fault_Handler) void I2C1_Handler                  (void);
__ALIAS(Fault_Handler) void QEI1_Handler                  (void);
__ALIAS(Fault_Handler) void CAN0_Handler                  (void);
__ALIAS(Fault_Handler) void CAN1_Handler                  (void);
__ALIAS(Fault_Handler) void CAN2_Handler                  (void);
__ALIAS(Fault_Handler) void ETH_Handler                   (void);
__ALIAS(Fault_Handler) void HIBERNATE_Handler             (void);
__ALIAS(Fault_Handler) void USB0_Handler                  (void);
__ALIAS(Fault_Handler) void PWM0_3_Handler                (void);
__ALIAS(Fault_Handler) void UDMA_Handler                  (void);
__ALIAS(Fault_Handler) void UDMAERR_Handler               (void);
__ALIAS(Fault_Handler) void ADC1SS0_Handler               (void);
__ALIAS(Fault_Handler) void ADC1SS1_Handler               (void);
__ALIAS(Fault_Handler) void ADC1SS2_Handler               (void);
__ALIAS(Fault_Handler) void ADC1SS3_Handler               (void);
__ALIAS(Fault_Handler) void I2S0_Handler                  (void);
__ALIAS(Fault_Handler) void EPI0_Handler                  (void);
__ALIAS(Fault_Handler) void GPIOJ_Handler                 (void);
__ALIAS(Fault_Handler) void GPIOK_Handler                 (void);
__ALIAS(Fault_Handler) void GPIOL_Handler                 (void);
__ALIAS(Fault_Handler) void SSI2_Handler                  (void);
__ALIAS(Fault_Handler) void SSI3_Handler                  (void);
__ALIAS(Fault_Handler) void UART3_Handler                 (void);
__ALIAS(Fault_Handler) void UART4_Handler                 (void);
__ALIAS(Fault_Handler) void UART5_Handler                 (void);
__ALIAS(Fault_Handler) void UART6_Handler                 (void);
__ALIAS(Fault_Handler) void UART7_Handler                 (void);
__ALIAS(Fault_Handler) void I2C2_Handler                  (void);
__ALIAS(Fault_Handler) void I2C3_Handler                  (void);
__ALIAS(Fault_Handler) void TIMER4A_Handler               (void);
__ALIAS(Fault_Handler) void TIMER4B_Handler               (void);
__ALIAS(Fault_Handler) void TIMER5A_Handler               (void);
__ALIAS(Fault_Handler) void TIMER5B_Handler               (void);
__ALIAS(Fault_Handler) void WTIMER0A_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER0B_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER1A_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER1B_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER2A_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER2B_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER3A_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER3B_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER4A_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER4B_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER5A_Handler              (void);
__ALIAS(Fault_Handler) void WTIMER5B_Handler              (void);
__ALIAS(Fault_Handler) void SYSEXC_Handler                (void);
__ALIAS(Fault_Handler) void PECI0_Handler                 (void);
__ALIAS(Fault_Handler) void LPC0_Handler                  (void);
__ALIAS(Fault_Handler) void I2C4_Handler                  (void);
__ALIAS(Fault_Handler) void I2C5_Handler                  (void);
__ALIAS(Fault_Handler) void GPIOM_Handler                 (void);
__ALIAS(Fault_Handler) void GPION_Handler                 (void);
__ALIAS(Fault_Handler) void QEI2_Handler                  (void);
__ALIAS(Fault_Handler) void FAN0_Handler                  (void);
__ALIAS(Fault_Handler) void GPIOP0_Handler                (void);
__ALIAS(Fault_Handler) void GPIOP1_Handler                (void);
__ALIAS(Fault_Handler) void GPIOP2_Handler                (void);
__ALIAS(Fault_Handler) void GPIOP3_Handler                (void);
__ALIAS(Fault_Handler) void GPIOP4_Handler                (void);
__ALIAS(Fault_Handler) void GPIOP5_Handler                (void);
__ALIAS(Fault_Handler) void GPIOP6_Handler                (void);
__ALIAS(Fault_Handler) void GPIOP7_Handler                (void);
__ALIAS(Fault_Handler) void GPIOQ0_Handler                (void);
__ALIAS(Fault_Handler) void GPIOQ1_Handler                (void);
__ALIAS(Fault_Handler) void GPIOQ2_Handler                (void);
__ALIAS(Fault_Handler) void GPIOQ3_Handler                (void);
__ALIAS(Fault_Handler) void GPIOQ4_Handler                (void);
__ALIAS(Fault_Handler) void GPIOQ5_Handler                (void);
__ALIAS(Fault_Handler) void GPIOQ6_Handler                (void);
__ALIAS(Fault_Handler) void GPIOQ7_Handler                (void);
__ALIAS(Fault_Handler) void PWM1_0_Handler                (void);
__ALIAS(Fault_Handler) void PWM1_1_Handler                (void);
__ALIAS(Fault_Handler) void PWM1_2_Handler                (void);
__ALIAS(Fault_Handler) void PWM1_3_Handler                (void);
__ALIAS(Fault_Handler) void PWM1_FAULT_Handler            (void);

/*******************************************************************************
 Vector table for LM4F120H5QR (Cortex-M4F)
*******************************************************************************/

__VECTORS
void (* const __vector_table[])(void) =
{
	/* Initial stack pointer */
	(void(*)(void)) __initial_msp,

	/* Core exceptions */
	Reset_Handler,      /* Reset                                   */
	NMI_Handler,        /* Non-maskable interrupt                  */
	HardFault_Handler,  /* All classes of faults                   */
	MemManage_Handler,  /* Memory management                       */
	BusFault_Handler,   /* Pre-fetch fault, memory access fault    */
	UsageFault_Handler, /* Undefined instruction or illegal state  */
	0, 0, 0, 0,         /* Reserved                                */
	SVC_Handler,        /* System service call via SWI instruction */
	DebugMon_Handler,   /* Debug Monitor                           */
	0,                  /* Reserved                                */
	PendSV_Handler,     /* Pendable request for system service     */
	SysTick_Handler,    /* System tick timer                       */

#ifndef __NO_EXTERNAL_INTERRUPTS

	/* External interrupts */
	GPIOA_Handler,
	GPIOB_Handler,
	GPIOC_Handler,
	GPIOD_Handler,
	GPIOE_Handler,
	UART0_Handler,
	UART1_Handler,
	SSI0_Handler,
	I2C0_Handler,
	PWM0_FAULT_Handler,
	PWM0_0_Handler,
	PWM0_1_Handler,
	PWM0_2_Handler,
	QEI0_Handler,
	ADC0SS0_Handler,
	ADC0SS1_Handler,
	ADC0SS2_Handler,
	ADC0SS3_Handler,
	WATCHDOG_Handler,
	TIMER0A_Handler,
	TIMER0B_Handler,
	TIMER1A_Handler,
	TIMER1B_Handler,
	TIMER2A_Handler,
	TIMER2B_Handler,
	COMP0_Handler,
	COMP1_Handler,
	COMP2_Handler,
	SYSCTL_Handler,
	FLASH_Handler,
	GPIOF_Handler,
	GPIOG_Handler,
	GPIOH_Handler,
	UART2_Handler,
	SSI1_Handler,
	TIMER3A_Handler,
	TIMER3B_Handler,
	I2C1_Handler,
	QEI1_Handler,
	CAN0_Handler,
	CAN1_Handler,
	CAN2_Handler,
	ETH_Handler,
	HIBERNATE_Handler,
	USB0_Handler,
	PWM0_3_Handler,
	UDMA_Handler,
	UDMAERR_Handler,
	ADC1SS0_Handler,
	ADC1SS1_Handler,
	ADC1SS2_Handler,
	ADC1SS3_Handler,
	I2S0_Handler,
	EPI0_Handler,
	GPIOJ_Handler,
	GPIOK_Handler,
	GPIOL_Handler,
	SSI2_Handler,
	SSI3_Handler,
	UART3_Handler,
	UART4_Handler,
	UART5_Handler,
	UART6_Handler,
	UART7_Handler,
	0, 0, 0, 0,
	I2C2_Handler,
	I2C3_Handler,
	TIMER4A_Handler,
	TIMER4B_Handler,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	TIMER5A_Handler,
	TIMER5B_Handler,
	WTIMER0A_Handler,
	WTIMER0B_Handler,
	WTIMER1A_Handler,
	WTIMER1B_Handler,
	WTIMER2A_Handler,
	WTIMER2B_Handler,
	WTIMER3A_Handler,
	WTIMER3B_Handler,
	WTIMER4A_Handler,
	WTIMER4B_Handler,
	WTIMER5A_Handler,
	WTIMER5B_Handler,
	SYSEXC_Handler,
	PECI0_Handler,
	LPC0_Handler,
	I2C4_Handler,
	I2C5_Handler,
	GPIOM_Handler,
	GPION_Handler,
	QEI2_Handler,
	FAN0_Handler,
	0,
	GPIOP0_Handler,
	GPIOP1_Handler,
	GPIOP2_Handler,
	GPIOP3_Handler,
	GPIOP4_Handler,
	GPIOP5_Handler,
	GPIOP6_Handler,
	GPIOP7_Handler,
	GPIOQ0_Handler,
	GPIOQ1_Handler,
	GPIOQ2_Handler,
	GPIOQ3_Handler,
	GPIOQ4_Handler,
	GPIOQ5_Handler,
	GPIOQ6_Handler,
	GPIOQ7_Handler,
	0, 0,
	PWM1_0_Handler,
	PWM1_1_Handler,
	PWM1_2_Handler,
	PWM1_3_Handler,
	PWM1_FAULT_Handler,

#endif//__NO_EXTERNAL_INTERRUPTS
};

/******************************************************************************/
