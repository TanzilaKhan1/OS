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

/**
 * first argument define the type of string to kprintf and kscanf,
 * %c for charater
 * %s for string,
 * %d for integer
 * %x hexadecimal
 * %o octal number
 * %f for floating point number
 */
// Simplified version of printf
void kprintf(char *format, ...) {
  // write your code here
  char *tr;
  uint32_t i;
  uint8_t *str;
  va_list list;
  double dval;
  // uint32_t *intval;
  va_start(list, format);
  for (tr = format; *tr != '\0'; tr++) {
    while (*tr != '%' && *tr != '\0') {
      Uart_write(*tr, __CONSOLE);
      tr++;
    }
    if (*tr == '\0')
      break;
    tr++;
    switch (*tr) {
    case 'c':
      i = va_arg(list, int);
      Uart_write(i, __CONSOLE);
      break;
    case 'd':
      i = va_arg(list, int);
      if (i < 0) {
        Uart_write('-', __CONSOLE);
        i = -i;
      }
      Uart_sendstring((char *)convert(i, 10), __CONSOLE);
      break;
    case 'o':
      i = va_arg(list, int);
      if (i < 0) {
        Uart_write('-', __CONSOLE);
        i = -i;
      }
      Uart_sendstring((char *)convert(i, 8), __CONSOLE);
      break;
    case 'x':
      i = va_arg(list, int);
      /*if(i<0)
      {
              Uart_write('-',__CONSOLE);
              i=-i;
      }*/
      Uart_sendstring((char *)convertu32(i, 16), __CONSOLE);
      break;
    case 'u':
      i = va_arg(list, unsigned int);
      Uart_sendstring((char *)convertu32(i, 10), __CONSOLE);
      break;
    case 's':
      str = va_arg(list, uint8_t *);
      Uart_sendstring((char *)str, __CONSOLE);
      break;
    case 'f':
      dval = va_arg(list, double);
      Uart_sendstring((char *)float2str(dval), __CONSOLE);
      break;
    default:
      break;
    }
  }
  va_end(list);
}

void putstr(const uint8_t *str, size_t size) {
  for (uint32_t i = 0; i < size; i++) {
    Uart_write(str[i], __CONSOLE);
  }
}


// NON-BLOCKING version - For use in syscall handlers (SYS_read)
// Reads only available characters without waiting
// Returns count of bytes read, or 0 if no data available
// NO ECHO - application is responsible for echo if needed
uint32_t read_available_nonblocking(uint8_t *buff, size_t max_len) {
  uint32_t count = 0U;

  if (buff == 0 || max_len == 0U) {
    return 0U;
  }

  // Cap at 256 bytes as per assignment requirements
  if (max_len > 256U) {
    max_len = 256U;
  }

  // Read all available characters without blocking
  while (count < (max_len - 1U)) {
    // Check if data available - DON'T WAIT!
    if (IsDataAvailable(__CONSOLE) <= 0) {
      break;  // No more data, return immediately
    }

    int c = Uart_read(__CONSOLE);
    if (c < 0) {
      break;  // Error or no data, stop reading
    }

    // Handle backspace/delete
    if ((uint8_t)c == 0x08U || (uint8_t)c == 0x7FU) {
      if (count > 0U) {
        count--;  // Remove last character
      }
      continue;
    }

    // Store character
    buff[count++] = (uint8_t)c;

    // Stop at newline
    if ((uint8_t)c == '\n' || (uint8_t)c == '\r') {
      break;
    }
  }

  // Null terminate if there's room
  if (count < max_len) {
    buff[count] = '\0';
  }

  return count;
}


// Simplified version of scanf
void kscanf(char *format, ...) {
  // write your code here
  va_list list;
  char *ptr;
  uint8_t buff[1024];
  uint8_t *str;
  ptr = format;
  va_start(list, format);
  while (*ptr) {
    if (*ptr == '%') // looking for format of an input
    {
      ptr++;
      switch (*ptr) {
      case 'c': // charater
        while (IsDataAvailable(__CONSOLE) == 0) {
        }
        {
          uint8_t ch = (uint8_t)Uart_read(__CONSOLE);
          // echo character
          if (ch == '\r' || ch == '\n') {
            Uart_write('\r', __CONSOLE);
            Uart_write('\n', __CONSOLE);
          } else if (ch == 0x08U || ch == 0x7FU) {
            Uart_write('\b', __CONSOLE);
            Uart_write(' ', __CONSOLE);
            Uart_write('\b', __CONSOLE);
          } else {
            Uart_write(ch, __CONSOLE);
          }
          *(uint8_t *)va_arg(list, uint8_t *) = ch;
        }
        break;
      case 'd': // integer number
        read_available_nonblocking(buff, sizeof(buff));
        {
          uint8_t numtok[50];
          uint32_t i = 0U, j = 0U;
          // skip leading whitespace
          while (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\r' ||
                 buff[i] == '\n') {
            i++;
          }
          // optional sign
          if (buff[i] == '-') {
            if (j < sizeof(numtok) - 1U) {
              numtok[j++] = '-';
            }
            i++;
          }
          // digits
          while (buff[i] >= '0' && buff[i] <= '9') {
            if (j >= sizeof(numtok) - 1U) {
              break;
            }
            numtok[j++] = buff[i++];
          }
          numtok[j] = '\0';
          *(uint32_t *)va_arg(list, uint32_t *) = __str_to_num(numtok, 10);
        }
        break;
      case 's': // string (read entire line, including spaces)
        read_available_nonblocking(buff, sizeof(buff));
        {
          str = (uint8_t *)va_arg(list, uint8_t *);
          uint32_t i = 0U;
          while (buff[i] != '\0') {
            str[i] = buff[i];
            i++;
          }
          str[i] = '\0';
        }
        break;
      case 'x': // hexadecimal number
        read_available_nonblocking(buff, sizeof(buff));
        {
          uint8_t hextok[50];
          uint32_t i = 0U, j = 0U;
          while (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\r' ||
                 buff[i] == '\n') {
            i++;
          }
          while ((buff[i] >= '0' && buff[i] <= '9') ||
                 (buff[i] >= 'a' && buff[i] <= 'f') ||
                 (buff[i] >= 'A' && buff[i] <= 'F')) {
            if (j >= sizeof(hextok) - 1U) {
              break;
            }
            hextok[j++] = buff[i++];
          }
          hextok[j] = '\0';
          *(int *)va_arg(list, uint32_t *) = __str_to_num(hextok, 16);
        }
        break;
      case 'o': // octal number
        read_available_nonblocking(buff, sizeof(buff));
        {
          uint8_t octtok[50];
          uint32_t i = 0U, j = 0U;
          while (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\r' ||
                 buff[i] == '\n') {
            i++;
          }
          while (buff[i] >= '0' && buff[i] <= '7') {
            if (j >= sizeof(octtok) - 1U) {
              break;
            }
            octtok[j++] = buff[i++];
          }
          octtok[j] = '\0';
          *(uint32_t *)va_arg(list, uint32_t *) = __str_to_num(octtok, 8);
        }
        break;
      case 'f': // floating point number
        read_available_nonblocking(buff, sizeof(buff));
        {
          uint8_t floattok[50];
          uint32_t i = 0U, j = 0U;
          uint8_t neg = 0U;
          while (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\r' ||
                 buff[i] == '\n') {
            i++;
          }
          if (buff[i] == '-') {
            neg = 1U;
            i++;
          }
          while ((buff[i] >= '0' && buff[i] <= '9') || buff[i] == '.') {
            if (j >= sizeof(floattok) - 1U) {
              break;
            }
            floattok[j++] = buff[i++];
          }
          floattok[j] = '\0';
          float fv = str2float(floattok);
          if (neg) {
            fv = -fv;
          }
          *(float *)va_arg(list, float *) = fv;
        }
        break;
      default: // rest not recognized
        break;
      }
    }
    ptr++;
  }
  va_end(list);
}
