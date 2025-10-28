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
#include <schedule.h>
#include <cm4.h>
#include <kstring.h>

/* Task stacks - each task gets 512 bytes of stack */
#define TASK_STACK_SIZE 512
static uint32_t task_stacks[MAX_TASKS][TASK_STACK_SIZE / sizeof(uint32_t)] __attribute__((aligned(8)));

/* TCB pool */
static TCB_TypeDef tcb_pool[MAX_TASKS];
static uint8_t tcb_used[MAX_TASKS] = {0};

/* Ready queue (circular linked list) */
static TCB_TypeDef *ready_queue_head = 0;
static TCB_TypeDef *current_task = 0;

/* Next task ID */
static uint16_t next_task_id = 1000;

/* External reference to current task ID for syscalls */
extern volatile uint16_t g_current_task_id;

/**
 * Initialize a task's stack frame
 * Sets up the stack as if the task was interrupted
 */
static void task_stack_init(TCB_TypeDef *tcb, void (*task_func)(void)) {
    uint32_t *stack_ptr = (uint32_t *)tcb->psp;
    
    /* Set up initial stack frame (as if exception occurred) */
    *(--stack_ptr) = (1U << 24);            /* xPSR - Thumb bit set */
    *(--stack_ptr) = (uint32_t)task_func;   /* PC - task entry point */
    *(--stack_ptr) = 0xFFFFFFFDU;           /* LR - EXC_RETURN (return to thread mode with PSP) */
    *(--stack_ptr) = 0x12121212U;           /* R12 */
    *(--stack_ptr) = 0x03030303U;           /* R3 */
    *(--stack_ptr) = 0x02020202U;           /* R2 */
    *(--stack_ptr) = 0x01010101U;           /* R1 */
    *(--stack_ptr) = 0x00000000U;           /* R0 */
    
    /* Additional registers that need to be saved manually */
    *(--stack_ptr) = 0x11111111U;           /* R11 */
    *(--stack_ptr) = 0x10101010U;           /* R10 */
    *(--stack_ptr) = 0x09090909U;           /* R9 */
    *(--stack_ptr) = 0x08080808U;           /* R8 */
    *(--stack_ptr) = 0x07070707U;           /* R7 */
    *(--stack_ptr) = 0x06060606U;           /* R6 */
    *(--stack_ptr) = 0x05050505U;           /* R5 */
    *(--stack_ptr) = 0x04040404U;           /* R4 */
    
    /* Update PSP to point to the top of the initialized stack */
    tcb->psp = stack_ptr;
}

/**
 * Initialize the task management system
 */
void task_init(void) {
    /* Clear all TCBs */
    kmemset(tcb_pool, 0, sizeof(tcb_pool));
    kmemset(tcb_used, 0, sizeof(tcb_used));
    
    ready_queue_head = 0;
    current_task = 0;
    next_task_id = 1000;
}

/**
 * Create a new task
 * Returns the task ID, or 0 if failed
 */
uint16_t task_create(void (*task_func)(void), uint8_t priority) {
    /* Find a free TCB */
    uint8_t idx = 0;
    for (idx = 0; idx < MAX_TASKS; idx++) {
        if (tcb_used[idx] == 0) {
            break;
        }
    }
    
    if (idx >= MAX_TASKS) {
        return 0; /* No free TCB */
    }
    
    /* Initialize the TCB */
    TCB_TypeDef *tcb = &tcb_pool[idx];
    kmemset(tcb, 0, sizeof(TCB_TypeDef));
    
    tcb->magic_number = TCB_MAGIC_NUMBER;
    tcb->task_id = next_task_id++;
    tcb->status = TASK_STATUS_READY;
    tcb->priority = priority;
    tcb->parent_id = 0;
    tcb->execution_time = 0;
    tcb->waiting_time = 0;
    tcb->digital_signature = TCB_DIGITAL_SIGNATURE;
    tcb->sem_waiting_count = 0;
    tcb->mutex_locked_count = 0;
    tcb->last_wakeup_time = 0;
    tcb->next = 0;
    
    /* Set up the task's stack */
    tcb->psp = (void *)(&task_stacks[idx][TASK_STACK_SIZE / sizeof(uint32_t)]);
    task_stack_init(tcb, task_func);
    
    /* Mark TCB as used */
    tcb_used[idx] = 1;
    
    /* Add to ready queue */
    if (ready_queue_head == 0) {
        /* First task in queue */
        ready_queue_head = tcb;
        tcb->next = tcb; /* Point to itself (circular) */
    } else {
        /* Insert at end of queue */
        TCB_TypeDef *tail = ready_queue_head;
        while (tail->next != ready_queue_head) {
            tail = tail->next;
        }
        tail->next = tcb;
        tcb->next = ready_queue_head;
    }
    
    /* If this is the first task, set it as current */
    if (current_task == 0) {
        current_task = tcb;
        tcb->status = TASK_STATUS_RUNNING;
        g_current_task_id = tcb->task_id;
    }
    
    return tcb->task_id;
}

/**
 * Get the current running task
 */
TCB_TypeDef* get_current_task(void) {
    return current_task;
}

/**
 * Get the next task to run
 */
TCB_TypeDef* get_next_task(void) {
    if (current_task == 0 || ready_queue_head == 0) {
        return 0;
    }
    
    /* Simple round-robin: next task in circular queue */
    TCB_TypeDef *next = current_task->next;
    
    /* Skip terminated tasks */
    uint8_t checked = 0;
    while (next->status == TASK_STATUS_TERMINATED && checked < MAX_TASKS) {
        next = next->next;
        checked++;
    }
    
    if (checked >= MAX_TASKS) {
        return current_task; /* No other runnable task */
    }
    
    return next;
}

/**
 * Switch to the next task
 * Called from PendSV handler
 */
void task_switch(void) {
    if (current_task == 0) {
        return;
    }
    
    /* Get next task */
    TCB_TypeDef *next = get_next_task();
    
    if (next == 0 || next == current_task) {
        return; /* No switch needed */
    }
    
    /* Update task states */
    if (current_task->status == TASK_STATUS_RUNNING) {
        current_task->status = TASK_STATUS_READY;
    }
    
    next->status = TASK_STATUS_RUNNING;
    current_task = next;
    g_current_task_id = next->task_id;
}

