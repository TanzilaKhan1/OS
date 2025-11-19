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
#include <float.h>
#include <kstdio.h>
#include <kstring.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys_usart.h>
#include <system_config.h>
#include <unistd.h>

/**
 * first argument define the type of string to kprintf and kscanf,
 * %c for charater
 * %s for string,
 * %d for integer
 * %x hexadecimal
 * %o octal number
 * %f for floating point number
 */

static void append_char(char *buf, uint32_t *pos, uint32_t max_size, char c) {
  if (*pos < max_size - 1) {
    buf[(*pos)++] = c;
  }
}


static void append_string(char *buf, uint32_t *pos, uint32_t max_size, const char *str) {
  while (*str && *pos < max_size - 1) {
    buf[(*pos)++] = *str++;
  }
}


void kprintf(char *format, ...) {
  static char output_buf[1024];  
  uint32_t buf_pos = 0;

  char *tr;
  uint32_t i;
  uint8_t *str;
  va_list list;
  double dval;

  va_start(list, format);
  for (tr = format; *tr != '\0'; tr++) {
    while (*tr != '%' && *tr != '\0') {
      append_char(output_buf, &buf_pos, sizeof(output_buf), *tr);
      tr++;
    }
    if (*tr == '\0')
      break;
    tr++;
    switch (*tr) {
    case 'c':
      i = va_arg(list, int);
      append_char(output_buf, &buf_pos, sizeof(output_buf), (char)i);
      break;
    case 'd':
      i = va_arg(list, int);
      if (i < 0) {
        append_char(output_buf, &buf_pos, sizeof(output_buf), '-');
        i = -i;
      }
      append_string(output_buf, &buf_pos, sizeof(output_buf), (char *)convert(i, 10));
      break;
    case 'o':
      i = va_arg(list, int);
      if (i < 0) {
        append_char(output_buf, &buf_pos, sizeof(output_buf), '-');
        i = -i;
      }
      append_string(output_buf, &buf_pos, sizeof(output_buf), (char *)convert(i, 8));
      break;
    case 'x':
      i = va_arg(list, int);
      append_string(output_buf, &buf_pos, sizeof(output_buf), (char *)convertu32(i, 16));
      break;
    case 'u':
      i = va_arg(list, unsigned int);
      append_string(output_buf, &buf_pos, sizeof(output_buf), (char *)convertu32(i, 10));
      break;
    case 's':
      str = va_arg(list, uint8_t *);
      append_string(output_buf, &buf_pos, sizeof(output_buf), (char *)str);
      break;
    case 'f':
      dval = va_arg(list, double);
      append_string(output_buf, &buf_pos, sizeof(output_buf), (char *)float2str(dval));
      break;
    default:
      break;
    }
  }
  va_end(list);


  if (buf_pos > 0) {
    write(STDOUT_FILENO, output_buf, buf_pos, 0);
  }
}

void putstr(const uint8_t *str, size_t size) {
  write(STDOUT_FILENO, str, size, 0);
}



// Reads characters until newline or buffer full
// Returns count of bytes read (0 if no complete line yet)
uint32_t read_nonblocking(uint8_t *buff, size_t max_len) {
  static uint8_t temp_buffer[256];
  static uint32_t temp_count = 0;

  if (buff == 0 || max_len == 0U) {
    return 0U;
  }

  if (max_len > 256U) {
    max_len = 256U;
  }


  while (temp_count < sizeof(temp_buffer) - 1U) {
    if (IsDataAvailable(__CONSOLE) <= 0) {
      return 0;  
    }

    int c = Uart_read(__CONSOLE);
    if (c < 0) {
      return 0;  
    }

    // Handle backspace/delete
    if ((uint8_t)c == 0x08U || (uint8_t)c == 0x7FU) {
      if (temp_count > 0U) {
        temp_count--;  
        // Echo backspace sequence
        Uart_write('\b', __CONSOLE);
        Uart_write(' ', __CONSOLE);
        Uart_write('\b', __CONSOLE);
      }
      continue;
    }

    // Handle newline
    if ((uint8_t)c == '\n' || (uint8_t)c == '\r') {
      // Echo newline
      Uart_write('\r', __CONSOLE);
      Uart_write('\n', __CONSOLE);

      // Copy to output buffer and null terminate
      uint32_t copy_len = (temp_count < max_len - 1) ? temp_count : (max_len - 1);
      for (uint32_t i = 0; i < copy_len; i++) {
        buff[i] = temp_buffer[i];
      }
      buff[copy_len] = '\0';

      // Reset temp buffer for next read
      uint32_t result = temp_count;
      temp_count = 0;

      return result;
    }

    // Echo and store normal character
    Uart_write((uint8_t)c, __CONSOLE);
    temp_buffer[temp_count++] = (uint8_t)c;
  }

  // Buffer full without newline
  uint32_t copy_len = (temp_count < max_len - 1) ? temp_count : (max_len - 1);
  for (uint32_t i = 0; i < copy_len; i++) {
    buff[i] = temp_buffer[i];
  }
  buff[copy_len] = '\0';

  uint32_t result = temp_count;
  temp_count = 0;

  return result;
}


// Returns: 0 if input not ready (would block), 1 if successful, -1 on error/invalid input
int kscanf(char *format, ...) {
  uint8_t input_buffer[256];

  int bytes_read = read(STDIN_FILENO, input_buffer, sizeof(input_buffer), 0);

  if (bytes_read <= 0) {
    return 0;  
  }

  // input parse
  va_list list;
  va_start(list, format);

  char *ptr = format;
  uint8_t *str;
  int len = __strlen(input_buffer);

  while (*ptr) {
    if (*ptr == '%') {  // format of an input
      ptr++;
      switch (*ptr) {
      case 'c':  
        if (len > 0) {
          *(uint8_t *)va_arg(list, uint8_t *) = input_buffer[0];
        } else {
          *(uint8_t *)va_arg(list, uint8_t *) = 0;
        }
        break;

      case 'd':  
        if (!is_valid_int_str(input_buffer, 10)) {
          va_end(list);
          return -1;  
        }
        *(uint32_t *)va_arg(list, uint32_t *) = __str_to_num(input_buffer, 10);
        break;

      case 's': 
        str = va_arg(list, uint8_t *);
        for (int u = 0; u <= len; u++) {
          str[u] = input_buffer[u];
        }
        break;

      case 'x':  
        if (!is_valid_int_str(input_buffer, 16)) {
          va_end(list);
          return -1; 
        }
        *(int *)va_arg(list, uint32_t *) = __str_to_num(input_buffer, 16);
        break;

      case 'o':  
        if (!is_valid_int_str(input_buffer, 8)) {
          va_end(list);
          return -1;  
        }
        *(uint32_t *)va_arg(list, uint32_t *) = __str_to_num(input_buffer, 8);
        break;

      case 'f':  
        if (!is_valid_float_str(input_buffer)) {
          va_end(list);
          return -1;  
        }
        *(float *)va_arg(list, float *) = str2float(input_buffer);
        break;

      default:  
        break;
      }
    }
    ptr++;
  }

  va_end(list);

  return 1; 
}
