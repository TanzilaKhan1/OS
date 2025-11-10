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

#ifndef __UNISTD_H
#define __UNISTD_H
#include <stddef.h>
#include <stdint.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1

// Format types for formatted I/O (used in read/write a3 parameter)
#define FMT_RAW    0  // Raw bytes (default, backward compatible)
#define FMT_STRING 1  // String (%s)
#define FMT_INT    2  // Integer (%d)
#define FMT_FLOAT  3  // Float (%f)
#define FMT_HEX    4  // Hexadecimal (%x)
#define FMT_OCTAL  5  // Octal (%o)
#define FMT_CHAR   6  // Character (%c)

typedef int32_t ssize_t;

ssize_t write(int fd, const void *buf, size_t count, uint32_t fmt);
ssize_t read(int fd, void *buf, size_t count, uint32_t fmt);
int reboot(void);
int getpid(void);
void _exit(int status);
void exit(int status);
int yield(void);
uint32_t getSysTickTime(void);

#endif
