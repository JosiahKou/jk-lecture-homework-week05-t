#include "uart.h"
#include <stdint.h>

int cs122a_main() {
  int i;
  UART* up;
  
  uart_init();
  
  // Use only UART[0] for output (it's connected to the console)
  up = &uart[0];
  
  for (i = 0; i < 4; i++) {
    uint32_t divisor = *(uint32_t*)(uart[i].base + UARTIBRD);
    uint32_t baudrate = 7380000 / (16 * divisor);
    uint32_t lcr = *(uint32_t*)(uart[i].base + UARTLCR);
    
    uprintf(up, "UART[%d] Baudrate: %d\n\r", i, baudrate);
    uprintf(up, "UART[%d] Line Control Register: 0x%x\n\r", i, lcr);
  }
  
  return 0;
}