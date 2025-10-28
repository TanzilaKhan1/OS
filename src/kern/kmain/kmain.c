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
#include <schedule.h>
#include <stdint.h>
#include <sys_init.h>
#include <sys_rtc.h>
#include <sys_usart.h>
#include <unistd.h>

extern volatile uint16_t g_current_task_id;

#ifndef DEBUG
#define DEBUG 1
#endif

/* Test Task 1: Tests write syscall and yield */
void task1_func(void) {
  for (uint8_t i = 0; i < 3; i++) {
    int pid = getpid();
    uint32_t time = getSysTickTime();
    kprintf("[Task1] PID=%d, Time=%u ms, Iteration=%d\r\n", pid, time, i);

    char msg[] = "[Task1] Using write() syscall\r\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1U);

    /* Voluntarily yield CPU */
    yield();
  }

  kprintf("[Task1] Exiting...\r\n");
  exit(0);

  /* Should never reach here */
  while (1)
    ;
}

/* Test Task 2: Tests getpid and time syscalls */
void task2_func(void) {
  for (uint8_t i = 0; i < 3; i++) {
    int pid = getpid();
    uint32_t time = getSysTickTime();
    kprintf("[Task2] PID=%d, Time=%u ms, Iteration=%d\r\n", pid, time, i);

    /* Let SysTick trigger context switch */
    ms_delay(15);
  }

  kprintf("[Task2] Exiting...\r\n");
  exit(0);

  /* Should never reach here */
  while (1)
    ;
}

/* Test Task 3: Tests all basic syscalls */
void task3_func(void) {
  int pid = getpid();
  uint32_t time = getSysTickTime();

  kprintf("[Task3] Starting - PID=%d, Time=%u ms\r\n", pid, time);

  /* Test write syscall */
  char msg[] = "[Task3] Testing write syscall\r\n";
  ssize_t written = write(STDOUT_FILENO, msg, sizeof(msg) - 1U);
  kprintf("[Task3] Wrote %d bytes\r\n", (int)written);

  /* Test getpid multiple times */
  for (uint8_t i = 0; i < 2; i++) {
    pid = getpid();
    time = getSysTickTime();
    kprintf("[Task3] PID=%d, Time=%u ms\r\n", pid, time);
    yield();
  }

  kprintf("[Task3] All tests completed, exiting...\r\n");
  exit(0);

  /* Should never reach here */
  while (1)
    ;
}

/* Idle task - runs when all other tasks are terminated */
void idle_task_func(void) {
  uint32_t idle_count = 0;

  kprintf("[Idle] Idle task started\r\n");

  while (1) {
    idle_count++;
    if ((idle_count % 1000) == 0) {
      int pid = getpid();
      uint32_t time = getSysTickTime();
      kprintf("[Idle] PID=%d, Time=%u ms, Count=%u\r\n", pid, time, idle_count);
    }

    /* Test reboot after some time (optional) */
    if (idle_count > 10000) {
      kprintf("[Idle] System has been running for a while. To test reboot, "
              "uncomment the next line.\r\n");
      // reboot();  /* Uncomment to test reboot syscall */
      idle_count = 0;
    }
  }
}

void kmain(void) {
  /* Initialize system */
  __sys_init();

  kprintf("\r\n");
  kprintf("========================================\r\n");
  kprintf("  DUOS - Operating System Lab Assignment 02\r\n");
  kprintf("  System Call and Task Management Demo\r\n");
  kprintf("========================================\r\n");
  kprintf("\r\n");

  /* Initialize task management system */
  task_init();
  kprintf("[KERN] Task management system initialized\r\n");

  /* Create tasks */
  uint16_t task1_id = task_create(task1_func, 1);
  kprintf("[KERN] Created Task1, ID=%d\r\n", task1_id);

  uint16_t task2_id = task_create(task2_func, 1);
  kprintf("[KERN] Created Task2, ID=%d\r\n", task2_id);

  uint16_t task3_id = task_create(task3_func, 1);
  kprintf("[KERN] Created Task3, ID=%d\r\n", task3_id);

  uint16_t idle_id = task_create(idle_task_func, 255);
  kprintf("[KERN] Created Idle task, ID=%d\r\n", idle_id);

  kprintf("\r\n[KERN] Starting task execution...\r\n");
  kprintf("[KERN] SysTick configured for 10ms context switch interval\r\n");
  kprintf("\r\n");

  /* Set the processor to use PSP for the first task */
  TCB_TypeDef *first_task = get_current_task();
  if (first_task != 0) {
    __asm volatile("msr psp, %0" : : "r"(first_task->psp));
    __asm volatile("msr control, %0" : : "r"(0x3)); /* Use PSP, unprivileged */
    __asm volatile("isb");

    /* Jump to first task */
    void (*task_entry)(void) =
        (void (*)(void))((uint32_t *)first_task->psp)[13];
    task_entry();
  }

  /* Should never reach here */
  kprintf("[KERN] ERROR: Returned to kmain!\r\n");
  while (1) {
  }
}
