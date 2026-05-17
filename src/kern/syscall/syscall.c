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

#include <UsartRingBuffer.h>
#include <cm4.h>
#include <errmsg.h>
#include <errno.h>
#include <kunistd.h>
#include <kstdio.h>
#include <sys_usart.h>
#include <syscall.h>
#include <syscall_def.h>
#include <system_config.h>

volatile uint16_t g_current_task_id = 0;
__attribute__((used)) volatile uint16_t *const __g_current_task_id_ref =
    &g_current_task_id;

uint32_t syscall_dispatch(uint16_t callno, uint32_t a0, uint32_t a1,
                          uint32_t a2, uint32_t a3) {
  switch (callno) {
  case SYS_read: {
    uint32_t fd = a0;
    uint8_t *buf = (uint8_t *)a1;
    uint32_t len = a2;
    if (fd != STDIN_FILENO || buf == 0 || len == 0U) {
      return 0U;
    }
    if (len > 256U) {
      len = 256U;
    }
    
    uint32_t count = read_available_nonblocking(buf, len);
    
    return count;
  }
  case SYS_write: {
    uint32_t fd = a0;
    const uint8_t *buf = (const uint8_t *)a1;
    uint32_t len = a2;
    if (fd != STDOUT_FILENO || buf == 0 || len == 0U) {
      return 0U;
    }
    for (uint32_t i = 0; i < len; i++) {
      Uart_write(buf[i], __CONSOLE);
    }
    return len;
  }
  case SYS_reboot:
    __NVIC_SystemReset();
    return 0U;
  case SYS__exit:
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    return 0U;
  case SYS_getpid:
    return (uint32_t)g_current_task_id;
  case SYS___time:
    return __getTime();
  case SYS_yield:
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    return 0U;
  default:
    return (uint32_t)ENOSYS;
  }
}
