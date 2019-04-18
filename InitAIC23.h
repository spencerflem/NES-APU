#ifndef INITAIC23_H_
#define INITAIC23_H_

void InitMcBSPb();
void InitSPIA();
void InitAIC23();
void SpiTransmit(uint16_t data);

void InitBigBangedCodecSPI();
void BitBangedCodecSpiTransmit(Uint16 data);


#endif /* INITAIC23_H_ */
