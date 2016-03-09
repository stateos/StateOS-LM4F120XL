/******************************************************************************
 * @file    lm4f.h
 * @author  Rajmund Szymanski
 * @date    27.02.2016
 * @brief   This file contains macro definitions for the LM4F4 devices.
 ******************************************************************************/

#pragma once

/* Includes ----------------------------------------------------------------- */

#include <bitband.h>
#include <stdint.h>

#ifndef TARGET_IS_BLIZZARD_RA2
#define TARGET_IS_BLIZZARD_RA2
#endif

#ifdef  PART_LM4F120H5QR
#include    <LM4F120H5QR.h>
#else
#error  Unknown device!
#endif

/* -------------------------------------------------------------------------- */
