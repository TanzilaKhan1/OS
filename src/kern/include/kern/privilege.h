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

#ifndef _KERN_PRIVILEGE_H
#define _KERN_PRIVILEGE_H

#include <cm4.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Privilege mode type definitions
 */
typedef enum {
  KERNEL_MODE = 0,  /* Privileged mode with MSP */
  USER_MODE = 1     /* Unprivileged mode with PSP */
} privilege_mode_t;

/**
 * @brief Get the current privilege level
 * @return KERNEL_MODE if privileged, USER_MODE if unprivileged
 */
static inline privilege_mode_t get_privilege_level(void) {
  return __is_privileged() ? KERNEL_MODE : USER_MODE;
}

/**
 * @brief Check if running in kernel mode
 * @return 1 if in kernel mode, 0 otherwise
 */
static inline uint32_t is_kernel_mode(void) {
  return __is_privileged() && (__get_stack_pointer_selection() == STACK_POINTER_MSP);
}

/**
 * @brief Check if running in user mode
 * @return 1 if in user mode, 0 otherwise
 */
static inline uint32_t is_user_mode(void) {
  return __is_unprivileged() && (__get_stack_pointer_selection() == STACK_POINTER_PSP);
}

/**
 * @brief Drop privileges to user mode
 * @param user_stack_top Top of user stack (PSP value)
 * @note Once dropped, privileges cannot be regained without a syscall/exception
 */
static inline void drop_to_user_mode(uint32_t user_stack_top) {
  __enter_user_mode(user_stack_top);
}

/**
 * @brief Elevate to kernel mode
 * @note Only works from Handler mode or if already privileged
 */
static inline void elevate_to_kernel_mode(void) {
  __enter_kernel_mode();
}

/**
 * @brief Get current stack pointer value
 * @return Current active stack pointer (MSP or PSP)
 */
static inline uint32_t get_current_sp(void) {
  if (__get_stack_pointer_selection() == STACK_POINTER_MSP) {
    return __get_MSP();
  } else {
    return __get_PSP();
  }
}

/**
 * @brief Set user stack pointer
 * @param sp Stack pointer value
 */
static inline void set_user_sp(uint32_t sp) {
  __set_PSP(sp);
}

/**
 * @brief Get user stack pointer
 * @return Current PSP value
 */
static inline uint32_t get_user_sp(void) {
  return __get_PSP();
}

/**
 * @brief Set kernel stack pointer
 * @param sp Stack pointer value
 */
static inline void set_kernel_sp(uint32_t sp) {
  __set_MSP(sp);
}

/**
 * @brief Get kernel stack pointer
 * @return Current MSP value
 */
static inline uint32_t get_kernel_sp(void) {
  return __get_MSP();
}

#ifdef __cplusplus
}
#endif

#endif /* _KERN_PRIVILEGE_H */
