#include "uart.h"
#include <stdint.h>

UART uart[4];

// Helper function for integer to string conversion (renamed to avoid conflict)
static char* int_to_str(int val, char* str, int base) {
  static char* digits = "0123456789abcdef";
  char* ptr = str;
  int num = val;
  int is_negative = 0;
  
  if (base == 10 && val < 0) {
    is_negative = 1;
    num = -val;
  }
  
  if (num == 0) {
    *ptr++ = '0';
    *ptr = '\0';
    return str;
  }
  
  char* start = ptr;
  while (num > 0) {
    *ptr++ = digits[num % base];
    num /= base;
  }
  
  if (is_negative) {
    *ptr++ = '-';
  }
  
  *ptr = '\0';
  
  char* end = ptr - 1;
  while (start < end) {
    char temp = *start;
    *start++ = *end;
    *end-- = temp;
  }
  
  return str;
}

int uart_init() {
  int i;
  UART* up;
  
  // Set base addresses
  for (i = 0; i < 3; i++) {
    up = &uart[i];
    up->base = (char*)(0x101F1000 + i * 0x1000);
    up->n = i;
  }
  uart[3].base = (char*)(0x10009000);
  uart[3].n = 3;
  
  // Configure each UART
  uint32_t divisors[] = {0x30, 0x20, 0x18, 0x0C};
  
  for (i = 0; i < 4; i++) {
    up = &uart[i];
    *(uint32_t*)(up->base + UARTIBRD) = divisors[i];
    *(uint32_t*)(up->base + UARTLCR) = 0x60;
  }
  
  return 0;
}

int ugetc(UART* up) {
  while (*(uint32_t*)(up->base + UARTFR) & 0x10);
  return *(uint32_t*)(up->base + UARTDR);
}

int uputc(UART* up, char c) {
  while (*(uint32_t*)(up->base + UARTFR) & 0x20);
  *(uint32_t*)(up->base + UARTDR) = c;
  return 0;
}

int ugets(UART* up, char* s) {
  while ((*s = ugetc(up)) != '\r') {
    uputc(up, *s);
    s++;
  }
  *s = 0;
  return 0;
}

int uprints(UART* up, char* s) {
  while (*s) uputc(up, *s++);
  return 0;
}

int uprintu(UART* up, uint32_t val) {
  char output[32];
  uprints(up, int_to_str(val, output, 10));
  return 0;
}

int uprintd(UART* up, int val) {
  char output[32];
  uprints(up, int_to_str(val, output, 10));
  return 0;
}

int uprintx(UART* up, uint32_t val) {
  char output[32];
  uprints(up, int_to_str(val, output, 16));
  return 0;
}

int uprintf(UART* up, char* fmt, ...) {
  char* cp = fmt;
  int* ip = (int*)&fmt + 1;
  
  while (*cp) {
    if (*cp != '%') {
      uputc(up, *cp);
      if (*cp == '\n')
        uputc(up, '\r');
      cp++;
      continue;
    }
    cp++;
    switch (*cp) {
      case 'c':
        uputc(up, (char)*ip);
        break;
      case 's':
        uprints(up, (char*)*ip);
        break;
      case 'u':
        uprintu(up, (uint32_t)*ip);
        break;
      case 'd':
        uprintd(up, (int)*ip);
        break;
      case 'x':
        uprintx(up, (uint32_t)*ip);
        break;
    }
    cp++;
    ip++;
  }
  return 0;
}