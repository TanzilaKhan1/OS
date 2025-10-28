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

#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include <stdint.h>

/* Task status definitions */
#define TASK_STATUS_NEW 0
#define TASK_STATUS_READY 1
#define TASK_STATUS_RUNNING 2
#define TASK_STATUS_WAITING 3
#define TASK_STATUS_TERMINATED 4
#define TASK_STATUS_KILLED 5

/* Magic numbers for integrity checking */
#define TCB_MAGIC_NUMBER 0xFECABAA0
#define TCB_DIGITAL_SIGNATURE 0x00000001

/* Task Control Block (TCB) structure */
typedef struct t_task_tcb {
  uint32_t magic_number; // Magic number for stack/structure integrity (e.g.,
                         // 0xFECABAA0)
  uint16_t task_id;      // Unique 16-bit task ID starting from 1000
  void *psp; // Task stack pointer (points to the task's stack frame)
  uint16_t
      status; // Task status: running, waiting, ready, killed, or terminated
  uint8_t priority;           // Task priority (lower value = higher priority)
  uint16_t parent_id;         // Task ID of parent process (if any)
  uint32_t execution_time;    // Total execution time in milliseconds
  uint32_t waiting_time;      // Total waiting time in milliseconds
  uint32_t digital_signature; // Digital signature or integrity check (e.g.,
                              // 0x00000001)
  uint32_t w_chld[16]; // Array to store suspended child task IDs or signals
                       // from parent
  uint32_t *heap_mem_start; // Pointer to dynamically allocated heap memory for
                            // the task
  uint32_t heap_mem_size;   // Size of allocated heap memory in bytes
  uint32_t *open_resources[8]; // Pointers to resources (files, devices) opened
                               // by the task
  uint8_t sem_waiting_count;   // Number of semaphores the task is currently
                               // waiting on
  uint8_t mutex_locked_count;  // Number of mutexes the task currently holds
  uint32_t last_wakeup_time;   // Timestamp of last wakeup (for sleep or delay)
  struct t_task_tcb *next;     // Pointer to next TCB in ready queue
} TCB_TypeDef;

/* Maximum number of tasks */
#define MAX_TASKS 8

/* Task management functions */
void task_init(void);
uint16_t task_create(void (*task_func)(void), uint8_t priority);
void task_switch(void);
TCB_TypeDef *get_current_task(void);
TCB_TypeDef *get_next_task(void);

/* Context switch function (called from PendSV) */
void PendSV_Handler(void);

#endif
