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



// Maximum iterations to check for data before returning (prevents busy-waiting)
#define MAX_POLL_ITERATIONS 1000U

// TRULY NON-BLOCKING - reads ONLY available data with iteration limit
// Returns immediately with whatever is available right now
// Returns number of bytes read (NOT including newline - newline is removed)
// IMPORTANT: This function will NOT wait for newline if max iterations reached
static uint32_t kscanf_poll_line(uint8_t *buff, size_t max_len) {
  uint32_t count = 0U;
  uint32_t iterations = 0U;

  // Read ONLY currently available characters - LIMITED POLLING
  // Exit if: (1) buffer full, (2) newline received, (3) max iterations reached
  while (count < (max_len - 1U) && iterations < MAX_POLL_ITERATIONS) {

    // Check if data available - but don't wait forever
    if (IsDataAvailable(__CONSOLE) <= 0) {
      iterations++;
      // Return immediately if we've checked enough times and have no data
      if (iterations >= MAX_POLL_ITERATIONS && count == 0U) {
        break;  // No data available after max iterations
      }
      // If we have some data but no newline yet, return what we have
      if (count > 0U && iterations >= MAX_POLL_ITERATIONS / 10U) {
        break;  // Return partial data rather than wait
      }
      continue;
    }

    int c = Uart_read(__CONSOLE);
    if (c < 0) {
      iterations++;
      continue;
    }

    // Reset iteration counter when we successfully read data
    iterations = 0U;

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

      // Consume paired newline if immediately available (no waiting!)
      if (IsDataAvailable(__CONSOLE) > 0) {
        int next_c = Uart_peek(__CONSOLE);
        if (next_c >= 0 && ((uint8_t)next_c == '\r' || (uint8_t)next_c == '\n')) {
          Uart_read(__CONSOLE);
        }
      }
      break;  // Done with this line
    }

    // Echo and store normal character
    Uart_write((uint8_t)c, __CONSOLE);
    buff[count++] = (uint8_t)c;
  }

  buff[count] = '\0';
  return count;
}

// Input accumulator state for non-blocking formatted input
typedef struct {
  uint8_t buffer[256];     // Accumulated input buffer
  uint32_t count;          // Current position in buffer
  uint8_t line_complete;   // Flag: 1 if newline received, 0 otherwise
} input_accumulator_t;

// Static accumulator for formatted input (one per system)
static input_accumulator_t g_input_accum = {0};

// NON-BLOCKING input accumulator - builds up a complete line over multiple calls
// Returns: 0 = no complete line yet, 1 = line complete and ready, -1 = error
// When returns 1, the complete line (without newline) is in the accumulator buffer
int accumulate_input_line(void) {
  // If we already have a complete line, return success immediately
  if (g_input_accum.line_complete) {
    return 1;
  }

  // Read available data without blocking
  uint32_t iterations = 0U;
  while (g_input_accum.count < sizeof(g_input_accum.buffer) - 1U &&
         iterations < 100U) {  // Limit iterations to prevent blocking

    // Check if data available
    if (IsDataAvailable(__CONSOLE) <= 0) {
      iterations++;
      if (iterations >= 100U) {
        return 0;  // No complete line yet, but not an error
      }
      continue;
    }

    int c = Uart_read(__CONSOLE);
    if (c < 0) {
      iterations++;
      continue;
    }

    // Reset iteration counter when we read data
    iterations = 0U;

    // Handle backspace
    if ((uint8_t)c == 0x08U || (uint8_t)c == 0x7FU) {
      if (g_input_accum.count > 0U) {
        g_input_accum.count--;
        Uart_write('\b', __CONSOLE);
        Uart_write(' ', __CONSOLE);
        Uart_write('\b', __CONSOLE);
      }
      continue;
    }

    // Handle newline - line is complete!
    if ((uint8_t)c == '\r' || (uint8_t)c == '\n') {
      Uart_write('\r', __CONSOLE);
      Uart_write('\n', __CONSOLE);

      // Consume paired newline if available
      if (IsDataAvailable(__CONSOLE) > 0) {
        int next_c = Uart_peek(__CONSOLE);
        if (next_c >= 0 && ((uint8_t)next_c == '\r' || (uint8_t)next_c == '\n')) {
          Uart_read(__CONSOLE);
        }
      }

      // Mark line as complete
      g_input_accum.buffer[g_input_accum.count] = '\0';
      g_input_accum.line_complete = 1U;
      return 1;  // Complete line ready
    }

    // Echo and store normal character
    Uart_write((uint8_t)c, __CONSOLE);
    g_input_accum.buffer[g_input_accum.count++] = (uint8_t)c;
  }

  // Buffer full but no newline - treat as complete
  if (g_input_accum.count >= sizeof(g_input_accum.buffer) - 1U) {
    g_input_accum.buffer[g_input_accum.count] = '\0';
    g_input_accum.line_complete = 1U;
    return 1;
  }

  return 0;  // Incomplete, need more data
}

// Reset the input accumulator (call after consuming accumulated input)
void reset_input_accumulator(void) {
  g_input_accum.count = 0U;
  g_input_accum.line_complete = 0U;
  g_input_accum.buffer[0] = '\0';
}

// Get pointer to accumulated input buffer (for parsing)
uint8_t* get_accumulated_input(void) {
  return g_input_accum.buffer;
}

// NON-BLOCKING kscanf - uses accumulator pattern for syscall safety
// Returns: 0 if input not ready (would block), 1 if successful, -1 on error
// When called from SVC handler, this function will NOT block indefinitely
int kscanf_nonblocking(char *format, va_list list) {
  char *ptr = format;

  // First, try to accumulate a complete line of input (bounded operation)
  int status = accumulate_input_line();

  if (status == 0) {
    // No complete line yet - return 0 (would block)
    return 0;
  }

  if (status < 0) {
    // Error
    return -1;
  }

  // We have a complete line in the accumulator - parse it
  uint8_t *input = get_accumulated_input();
  uint8_t *str;
  int len = __strlen(input);

  while (*ptr) {
    if (*ptr == '%') {  // looking for format of an input
      ptr++;
      switch (*ptr) {
      case 'c':  // character
        if (len > 0) {
          *(uint8_t *)va_arg(list, uint8_t *) = input[0];
        } else {
          *(uint8_t *)va_arg(list, uint8_t *) = 0;
        }
        break;

      case 'd':  // integer number
        *(uint32_t *)va_arg(list, uint32_t *) = __str_to_num(input, 10);
        break;

      case 's':  // string without spaces
        str = va_arg(list, uint8_t *);
        for (int u = 0; u <= len; u++) {
          str[u] = input[u];
        }
        break;

      case 'x':  // hexadecimal number
        *(int *)va_arg(list, uint32_t *) = __str_to_num(input, 16);
        break;

      case 'o':  // octal number
        *(uint32_t *)va_arg(list, uint32_t *) = __str_to_num(input, 8);
        break;

      case 'f':  // floating point number
        *(float *)va_arg(list, float *) = str2float(input);
        break;

      default:  // rest not recognized
        break;
      }
    }
    ptr++;
  }

  // Reset accumulator for next input
  reset_input_accumulator();
  return 1;  // Success
}

// NON-BLOCKING version of kscanf with BOUNDED retry attempts
// Safe to call from SVC handlers - will NOT block indefinitely
// Returns: number of items successfully read, or 0 if would block
// This is the main kscanf function that should be used everywhere
int kscanf(char *format, ...) {
  va_list list;
  va_start(list, format);

  // Try to get input with BOUNDED retries (prevents indefinite blocking)
  // This makes kscanf safe to use from syscall handlers
  int result;
  uint32_t max_retries = 1000U;  // Maximum retry attempts
  uint32_t retry_count = 0U;

  while ((result = kscanf_nonblocking(format, list)) == 0 && retry_count < max_retries) {
    // Input not ready yet, retry with bounded limit
    retry_count++;
  }

  va_end(list);

  // Return 1 if successful, 0 if timed out (no complete input)
  return (result == 1) ? 1 : 0;
}
