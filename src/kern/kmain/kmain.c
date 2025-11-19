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
  g_current_task_id = 1000;

  //   display_group_info();

  kprintf("\r\n=== Privilege Mode Test ===\r\n");

  uint32_t control = __get_CONTROL();
  uint32_t is_priv = __is_privileged();
  kprintf("CONTROL register: 0x%x\r\n", control);
  kprintf("Privilege level: %s (bit 0 = %d)\r\n",
          is_priv ? "PRIVILEGED" : "UNPRIVILEGED", control & 0x01);

  kprintf("\r\nTask ID set to: %d\r\n", g_current_task_id);

  kprintf(
      "\r\nCalling syscalls (will enter privileged mode in handler)...\r\n");
  uint32_t t = getSysTickTime();
  int pid = getpid();
  kprintf("Current time: %d ms, pid: %d\r\n", t, pid);

  control = __get_CONTROL();
  is_priv = __is_privileged();
  kprintf("After syscalls - Privilege: %s (CONTROL = 0x%x)\r\n",
          is_priv ? "PRIVILEGED" : "UNPRIVILEGED", control);

  kprintf("\r\n=== Formatted I/O Test ===\r\n");
  kprintf("1. String  2. Integer  3. Float  4. Hex  5. Octal  6. Char\r\n");
  kprintf("Enter your choice: ");
  int choice = 0;

  while (kscanf("%d", &choice) == 0) {
  }
  kprintf("\r\nYou entered (choice): %d", choice);

  char str_buf[256];
  int int_val;
  float float_val;
  uint32_t hex_val;
  uint32_t oct_val;
  char char_val;

  kprintf("\r\nEnter the value: ");

  switch (choice) {
  case 1:
    while (kscanf("%s", str_buf) == 0)
      ;
    kprintf("\r\nYou entered (string): %s", str_buf);
    break;
  case 2:
    while (kscanf("%d", &int_val) == 0)
      ;
    kprintf("\r\nYou entered (int): %d", int_val);
    break;
  case 3:
    while (kscanf("%f", &float_val) == 0)
      ;
    kprintf("\r\nYou entered (float): %f", float_val);
    break;
  case 4:
    while (kscanf("%x", &hex_val) == 0)
      ;
    kprintf("\r\nYou entered (hex): 0x%x", hex_val);
    break;
  case 5:
    while (kscanf("%o", &oct_val) == 0)
      ;
    kprintf("\r\nYou entered (octal): %o", oct_val);
    break;
  case 6:
    while (kscanf("%c", &char_val) == 0)
      ;
    kprintf("\r\nYou entered (char): %c", char_val);
    break;
  default:
    kprintf("\r\nInvalid choice!");
    break;
  }
  kprintf("\r\n");

  uint32_t t2 = getSysTickTime();
  int pid2 = getpid();

  kprintf("\r\n");
  kprintf("\r\nSysTickTime: %u\r\n", t2);
  kprintf("pid: %d\r\n", pid2);

  kprintf("\nCalling yield()...\r\n");
  yield();

  kprintf("After yield(), calling exit(0)...\r\n");
  exit(0);

  while (1) {
  }
}
