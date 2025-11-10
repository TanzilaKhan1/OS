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
#include <kmain.h>
#include <kstdio.h>
#include <kstring.h>
#include <stdint.h>
#include <sys_init.h>
#include <sys_rtc.h>
#include <sys_usart.h>
#include <unistd.h>

extern volatile uint16_t g_current_task_id;

#ifndef DEBUG
#define DEBUG 1
#endif
void kmain(void) {
  __sys_init();
  g_current_task_id = 1000;

  kprintf("Task ID set to: %d\r\n", g_current_task_id);

  char msg[] = "Hello from userland via write()\r\n";
  write(STDOUT_FILENO, msg, sizeof(msg) - 1U);

  uint32_t t = getSysTickTime();
  int pid = getpid();
  // kprintf("getSysTickTime() returned: %d ms\r\n", t);
  // kprintf("getpid() returned: %d\r\n", pid);
  kprintf("Current time: %d ms, pid: %d\r\n", t, pid);
  char msg2[1024];
  kprintf("Enter text: ");
  kscanf("%s", msg2);
  //int len = read(STDIN_FILENO, msg2, __strlen((uint8_t *)msg2));
  //kprintf("len: %d\r\n", len);
  kprintf("You entered: ");
  write(STDOUT_FILENO, msg2, __strlen((uint8_t *)msg2));

  uint32_t t2 = getSysTickTime();
  int pid2 = getpid();

  kprintf("\n");
  kprintf("\r\nSysTickTime: ");
  uint8_t *t2_str = convertu32(t2, 10);
  write(STDOUT_FILENO, t2_str, __strlen(t2_str));
  write(STDOUT_FILENO, "\r\n", 2);

  kprintf("pid: ");
  uint8_t *pid2_str = convert(pid2, 10);
  write(STDOUT_FILENO, pid2_str, __strlen(pid2_str));
  write(STDOUT_FILENO, "\r\n", 2);

  kprintf("\nCalling yield()...\r\n");
  yield();

  kprintf("After yield(), calling exit(0)...\r\n");
  exit(0);

  while (1) {
  }
}
