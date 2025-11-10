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

// BLOCKING version - Legacy function, kept for compatibility
// WARNING: DO NOT call from syscall handlers (SVC) - will block indefinitely!
// NOTE: kscanf now uses non-blocking I/O, this function rarely needed
// Returns count of bytes read (excluding null terminator)
// uint32_t read_string_syscall(uint8_t *buff, size_t max_len) {
//   uint32_t count = 0U;

//   if (buff == 0 || max_len == 0U) {
//     return 0U;
//   }

//   while (count < (max_len - 1U)) {
//     // BLOCKING: wait for data
//     while (IsDataAvailable(__CONSOLE) <= 0);

//     int c = Uart_read(__CONSOLE);
//     if (c < 0) {
//       continue;
//     }

//     // Handle backspace/delete
//     if ((uint8_t)c == 0x08U || (uint8_t)c == 0x7FU) {
//       if (count > 0U) {
//         count--;
//         Uart_write('\b', __CONSOLE);
//         Uart_write(' ', __CONSOLE);
//         Uart_write('\b', __CONSOLE);
//       }
//       continue;
//     }

//     // Handle newline - end input
//     if ((uint8_t)c == '\n' || (uint8_t)c == '\r') {
//       Uart_write('\r', __CONSOLE);
//       Uart_write('\n', __CONSOLE);
//       break;
//     }

//     // Echo and store normal character
//     Uart_write((uint8_t)c, __CONSOLE);
//     buff[count++] = (uint8_t)c;
//   }

//   buff[count] = '\0';
//   return count;
// }

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



// Helper function for kscanf: poll for complete line using non-blocking reads
// Returns number of bytes read (NOT including newline - newline is removed)
static uint32_t kscanf_poll_line(uint8_t *buff, size_t max_len) {
  uint32_t count = 0U;
  uint32_t loop_iterations = 0U;
  uint32_t max_iterations = 100000U;  // Timeout after ~100K iterations

  while (count < (max_len - 1U)) {
    // Timeout protection
    if (loop_iterations++ > max_iterations) {
      kprintf("\r\n[TIMEOUT in kscanf_poll_line, count=%d]\r\n", count);
      break;
    }

    // Check if data available
    if (IsDataAvailable(__CONSOLE) <= 0) {
      // Small delay before retry
      for (volatile uint32_t i = 0; i < 1000; i++);
      continue;
    }

    int c = Uart_read(__CONSOLE);
    if (c < 0) {
      kprintf("\r\n[Uart_read returned -1]\r\n");
      continue;
    }

    // DEBUG: Print character code
    kprintf("[char=0x%x] ", (uint8_t)c);

    // Handle backspace
    if ((uint8_t)c == 0x08U || (uint8_t)c == 0x7FU) {
      if (count > 0U) {
        count--;
        Uart_write('\b', __CONSOLE);
        Uart_write(' ', __CONSOLE);
        Uart_write('\b', __CONSOLE);
      }
      continue;
    }

    // Handle newline - end input
    if ((uint8_t)c == '\r' || (uint8_t)c == '\n') {
      Uart_write('\r', __CONSOLE);
      Uart_write('\n', __CONSOLE);
      kprintf("[GOT NEWLINE, breaking]\r\n");

      // Consume any remaining newline character (handle \r\n or \n\r)
      for (volatile uint32_t i = 0; i < 1000; i++);
      if (IsDataAvailable(__CONSOLE) > 0) {
        int next_c = Uart_read(__CONSOLE);
        kprintf("[consumed next_c=0x%x]\r\n", (uint8_t)next_c);
      }
      break;
    }

    // Echo and store normal character
    Uart_write((uint8_t)c, __CONSOLE);
    buff[count++] = (uint8_t)c;
  }

  buff[count] = '\0';
  kprintf("[kscanf_poll_line returning, count=%d, buf='%s']\r\n", count, buff);
  return count;
}

// Simplified version of scanf - NOW USES NON-BLOCKING I/O
// Safe to use from kernel code, polls internally until input complete
void kscanf(char *format, ...) {
  va_list list;
  char *ptr;
  uint8_t buff[256];  // Reduced from 1024 to avoid stack overflow in SVC handler
  uint8_t *str;

  ptr = format;
  va_start(list, format);

  while (*ptr) {
    if (*ptr == '%') {
      ptr++;
      switch (*ptr) {
      case 'c': { // character - poll for single char
        uint8_t ch = 0;
        while (1) {
          if (IsDataAvailable(__CONSOLE) <= 0) {
            for (volatile uint32_t i = 0; i < 1000; i++);
            continue;
          }

          int c = Uart_read(__CONSOLE);
          if (c < 0) {
            continue;
          }

          ch = (uint8_t)c;

          // Echo character
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
          break;
        }
        *(uint8_t *)va_arg(list, uint8_t *) = ch;
        break;
      }
      case 'd': { // integer number
        kscanf_poll_line(buff, sizeof(buff));
        uint8_t numtok[50];
        uint32_t i = 0U, j = 0U;

        // Skip leading whitespace
        while (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\r' ||
               buff[i] == '\n') {
          i++;
        }

        // Optional sign
        if (buff[i] == '-' || buff[i] == '+') {
          if (j < sizeof(numtok) - 1U && buff[i] == '-') {
            numtok[j++] = '-';
          }
          i++;
        }

        // Digits
        while (buff[i] >= '0' && buff[i] <= '9') {
          if (j >= sizeof(numtok) - 1U) {
            break;
          }
          numtok[j++] = buff[i++];
        }
        numtok[j] = '\0';
        *(uint32_t *)va_arg(list, uint32_t *) = __str_to_num(numtok, 10);
        break;
      }
      case 's': { // string (read entire line, including spaces)
        kscanf_poll_line(buff, sizeof(buff));
        str = (uint8_t *)va_arg(list, uint8_t *);
        uint32_t i = 0U;

        // Copy string, removing trailing newline
        while (buff[i] != '\0' && buff[i] != '\r' && buff[i] != '\n') {
          str[i] = buff[i];
          i++;
        }
        str[i] = '\0';
        break;
      }
      case 'x': { // hexadecimal number
        kscanf_poll_line(buff, sizeof(buff));
        uint8_t hextok[50];
        uint32_t i = 0U, j = 0U;

        // Skip leading whitespace
        while (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\r' ||
               buff[i] == '\n') {
          i++;
        }

        // Skip optional 0x prefix
        if (buff[i] == '0' && (buff[i + 1] == 'x' || buff[i + 1] == 'X')) {
          i += 2;
        }

        // Hex digits
        while ((buff[i] >= '0' && buff[i] <= '9') ||
               (buff[i] >= 'a' && buff[i] <= 'f') ||
               (buff[i] >= 'A' && buff[i] <= 'F')) {
          if (j >= sizeof(hextok) - 1U) {
            break;
          }
          hextok[j++] = buff[i++];
        }
        hextok[j] = '\0';
        *(uint32_t *)va_arg(list, uint32_t *) = __str_to_num(hextok, 16);
        break;
      }
      case 'o': { // octal number
        kscanf_poll_line(buff, sizeof(buff));
        uint8_t octtok[50];
        uint32_t i = 0U, j = 0U;

        // Skip leading whitespace
        while (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\r' ||
               buff[i] == '\n') {
          i++;
        }

        // Octal digits
        while (buff[i] >= '0' && buff[i] <= '7') {
          if (j >= sizeof(octtok) - 1U) {
            break;
          }
          octtok[j++] = buff[i++];
        }
        octtok[j] = '\0';
        *(uint32_t *)va_arg(list, uint32_t *) = __str_to_num(octtok, 8);
        break;
      }
      case 'f': { // floating point number
        kscanf_poll_line(buff, sizeof(buff));
        uint8_t floattok[50];
        uint32_t i = 0U, j = 0U;
        uint8_t neg = 0U;

        // Skip leading whitespace
        while (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\r' ||
               buff[i] == '\n') {
          i++;
        }

        // Optional sign
        if (buff[i] == '-') {
          neg = 1U;
          i++;
        } else if (buff[i] == '+') {
          i++;
        }

        // Digits and decimal point
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
        break;
      }
      default:
        break;
      }
    }
    ptr++;
  }
  va_end(list);
}
