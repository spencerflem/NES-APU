#ifndef SRAM_H_
#define SRAM_H_

#include <F28x_Project.h>

void init_sram_spi();
void write_sram(Uint32 addr, Uint16 data);
Uint16 read_sram(Uint32 addr);

#endif /* SRAM_H_ */
