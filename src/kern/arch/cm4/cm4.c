/*
 * Copyright (c) 2022
 * Computer Science and Engineering, University of Dhaka
 * Credit: CSE Batch 25 (starter) and Prof. Mosaddek Tushar
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <cm4.h>
#include <sys_clock.h>
#include <syscall.h>

/************************************************************************************
 * __SysTick_init(uint32_t reload)
 * Function initialize the SysTick clock. The function with a weak attribute
 *enables redefining the function to change its characteristics whenever
 *necessary.
 **************************************************************************************/

void __attribute__((weak)) __SysTick_init(uint32_t reload) { (void)reload; }
void __attribute__((weak)) SysTickIntDisable(void) {}

void __attribute__((weak)) SysTickIntEnable(void) {}
/************************************************************************************
 * __sysTick_enable(void)
 * The function enables the SysTick clock if already not enabled.
 * redefining the function to change its characteristics whenever necessary.
 **************************************************************************************/
void __attribute__((weak)) __SysTick_enable(void) {}
void __attribute__((weak)) __sysTick_disable(void) {}
uint32_t __attribute__((weak)) __getSysTickCount(void) { return 0U; }
/************************************************************************************
 * __updateSysTick(uint32_t count)
 * Function reinitialize the SysTick clock. The function with a weak attribute
 *enables redefining the function to change its characteristics whenever
 *necessary.
 **************************************************************************************/

void __attribute__((weak)) __updateSysTick(uint32_t count) { (void)count; }

/************************************************************************************
 * __getTime(void)
 * Function return the SysTick elapsed time from the begining or reinitialing.
 *The function with a weak attribute enables redefining the function to change
 *its characteristics whenever necessary.
 **************************************************************************************/

uint32_t __attribute__((weak)) __getTime(void) { return 0U; }

uint32_t __attribute__((weak)) __get__Second(void) { return 0U; }
uint32_t __attribute__((weak)) __get__Minute(void) { return 0U; }
uint32_t __attribute__((weak)) __get__Hour(void) { return 0U; }
void __attribute__((weak)) SysTick_Handler(void) {}

void __enable_fpu() { SCB->CPACR |= ((0xFUL << 20)); }

uint8_t __attribute__((weak)) ms_delay(uint32_t delay) {
  (void)delay;
  return 0U;
}

uint32_t __attribute__((weak)) getmsTick(void) { return 0U; }

uint32_t __attribute__((weak)) wait_until(uint32_t delay) {
  (void)delay;
  return 0U;
}

void SYS_SLEEP_WFI(void) { __WFI(); }

/**
 * @brief Initialize privilege mode and stack pointers for user mode
 * @param psp_top Top of Process Stack Pointer (user stack)
 * @note This function sets up the system for unprivileged thread mode
 *       using PSP (Process Stack Pointer)
 */
void __init_user_mode(uint32_t psp_top) {
  __set_PSP(psp_top);
  __set_PSP_active();
  __set_unprivileged_mode();
}

/**
 * @brief Get current execution mode information
 * @param mode_info Pointer to store mode information (bit 0: privilege, bit 1: stack)
 * @return Current CONTROL register value
 */
uint32_t __get_execution_mode(uint32_t *mode_info) {
  uint32_t control = __get_CONTROL();
  if (mode_info != 0) {
    *mode_info = control & 0x3U;
  }
  return control;
}

/**
 * @brief Switch to kernel mode (privileged + MSP)
 * @note Only works from Handler mode or privileged Thread mode
 */
void __enter_kernel_mode(void) {
  __set_MSP_active();
  __set_privileged_mode();
}

/**
 * @brief Prepare to switch to user mode
 * @param user_sp User stack pointer value
 * @note This sets up PSP and switches to unprivileged mode
 */
void __enter_user_mode(uint32_t user_sp) {
  if (user_sp != 0) {
    __set_PSP(user_sp);
  }
  __set_PSP_active();
  __set_unprivileged_mode();
}
