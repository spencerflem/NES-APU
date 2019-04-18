/*
 * initAIC23.c
 */

#include <stdint.h>
#include <F28x_Project.h>
#include "AIC23.h"

/***************** Defines ***************/
#define SmallDelay() for(volatile long  i = 0; i < 500000; i++)
/***************** Defines ***************/

/***************** User Functions *****************/
void InitMcBSPb();
void InitSPIA();
void InitAIC23();
void SpiTransmit(uint16_t data);
/***************** User Functions *****************/

void InitAIC23()
{
    uint16_t command;
    command = reset();
    SpiTransmit (command);
    SmallDelay();
    command = softpowerdown();       // Power down everything except device and clocks
    SpiTransmit (command);
    SmallDelay();
    command = linput_volctl(LIV);    // Unmute left line input and maintain default volume
    SpiTransmit (command);
    SmallDelay();
    command = rinput_volctl(RIV);    // Unmute right line input and maintain default volume
    SpiTransmit (command);
    SmallDelay();
    command = lhp_volctl(LHV);       // Left headphone volume control
    SpiTransmit (command);
    SmallDelay();
    command = rhp_volctl(RHV);       // Right headphone volume control
    SpiTransmit (command);
    SmallDelay();
    command = nomicaaudpath();      // Turn on DAC, mute mic
    SpiTransmit (command);
    SmallDelay();
    command = digaudiopath();       // Disable DAC mute, add de-emph
    SpiTransmit (command);
    SmallDelay();

    // I2S
    command = I2Sdigaudinterface(); // AIC23 master mode, I2S mode,32-bit data, LRP=1 to match with XDATADLY=1
    SpiTransmit (command);
    SmallDelay();
    command = CLKsampleratecontrol (SR48);
    SpiTransmit (command);
    SmallDelay();

    command = digact();             // Activate digital interface
    SpiTransmit (command);
    SmallDelay();
    command = nomicpowerup();      // Turn everything on except Mic.
    SpiTransmit (command);
    SmallDelay();
}

void InitMcBSPb()
{
    /* initialize GPIO */
    EALLOW;

    GpioCtrlRegs.GPAGMUX2.bit.GPIO24 = 0; //MDXB
    GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 3;
    GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;

    GpioCtrlRegs.GPAGMUX2.bit.GPIO25 = 0; //MDRB
    GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 3;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO25 = 3;
    GpioCtrlRegs.GPADIR.bit.GPIO25 = 0;

    GpioCtrlRegs.GPAGMUX2.bit.GPIO26 = 0; //MCLKXB
    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 3;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO26 = 3;
    GpioCtrlRegs.GPADIR.bit.GPIO26 = 0;

    GpioCtrlRegs.GPAGMUX2.bit.GPIO27 = 0; //MFSXB
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 3;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO27 = 3;
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 0;

    GpioCtrlRegs.GPBGMUX2.bit.GPIO60 = 0; //MCLKRB
    GpioCtrlRegs.GPBMUX2.bit.GPIO60 = 1;
    GpioCtrlRegs.GPBQSEL2.bit.GPIO60 = 3;
    GpioCtrlRegs.GPBDIR.bit.GPIO60 = 0;

    GpioCtrlRegs.GPBGMUX2.bit.GPIO61 = 0; //MFSRB
    GpioCtrlRegs.GPBMUX2.bit.GPIO61 = 1;
    GpioCtrlRegs.GPBQSEL2.bit.GPIO61 = 3;
    GpioCtrlRegs.GPBDIR.bit.GPIO61 = 0;

    /* Init McBSPb for I2S mode */
    McbspbRegs.SPCR2.all = 0; // Reset FS generator, sample rate generator & transmitter
    McbspbRegs.SPCR1.all = 0; // Reset Receiver, Right justify word
    McbspbRegs.SPCR1.bit.RJUST = 2; // left-justify word in DRR and zero-fill LSBs
    McbspbRegs.MFFINT.all=0x0; // Disable all interrupts
    McbspbRegs.SPCR1.bit.RINTM = 0; // McBSP interrupt flag - RRDY
    McbspbRegs.SPCR2.bit.XINTM = 0; // McBSP interrupt flag - XRDY
    // Clear Receive Control Registers
    McbspbRegs.RCR2.all = 0x0;
    McbspbRegs.RCR1.all = 0x0;
    // Clear Transmit Control Registers
    McbspbRegs.XCR2.all = 0x0;
    McbspbRegs.XCR1.all = 0x0;
    // Set Receive/Transmit to 32-bit operation
    McbspbRegs.RCR2.bit.RWDLEN2 = 5;
    McbspbRegs.RCR1.bit.RWDLEN1 = 5;
    McbspbRegs.XCR2.bit.XWDLEN2 = 5;
    McbspbRegs.XCR1.bit.XWDLEN1 = 5;
    McbspbRegs.RCR2.bit.RPHASE = 1; // Dual-phase frame for receive
    McbspbRegs.RCR1.bit.RFRLEN1 = 0; // Receive frame length = 1 word in phase 1
    McbspbRegs.RCR2.bit.RFRLEN2 = 0; // Receive frame length = 1 word in phase 2
    McbspbRegs.XCR2.bit.XPHASE = 1; // Dual-phase frame for transmit
    McbspbRegs.XCR1.bit.XFRLEN1 = 0; // Transmit frame length = 1 word in phase 1
    McbspbRegs.XCR2.bit.XFRLEN2 = 0; // Transmit frame length = 1 word in phase 2
    // I2S mode: R/XDATDLY = 1 always
    McbspbRegs.RCR2.bit.RDATDLY = 1;
    McbspbRegs.XCR2.bit.XDATDLY = 1;
    // Frame Width = 1 CLKG period, CLKGDV must be 1 as slave
    McbspbRegs.SRGR1.all = 0x0001;
    McbspbRegs.PCR.all=0x0000;
    // Transmit frame synchronization is supplied by an external source via the FSX pin
    McbspbRegs.PCR.bit.FSXM = 0;
    // Receive frame synchronization is supplied by an external source via the FSR pin
    McbspbRegs.PCR.bit.FSRM = 0;
    // Select sample rate generator to be signal on MCLKR pin
    McbspbRegs.PCR.bit.SCLKME = 1;
    McbspbRegs.SRGR2.bit.CLKSM = 0;
    // Receive frame-synchronization pulses are active low - (L-channel first)
    McbspbRegs.PCR.bit.FSRP = 1;
    // Transmit frame-synchronization pulses are active low - (L-channel first)
    McbspbRegs.PCR.bit.FSXP = 1;
    // Receive data is sampled on the rising edge of MCLKR
    McbspbRegs.PCR.bit.CLKRP = 1;
    // Transmit data is sampled on the rising edge of CLKX
    McbspbRegs.PCR.bit.CLKXP = 1;
    // The transmitter gets its clock signal from MCLKX
    McbspbRegs.PCR.bit.CLKXM = 0;
    // The receiver gets its clock signal from MCLKR
    McbspbRegs.PCR.bit.CLKRM = 0;
    // Enable Receive Interrupt
    McbspbRegs.MFFINT.bit.RINT = 1;
    // Ignore unexpected frame sync
    //McbspbRegs.XCR2.bit.XFIG = 1;
    McbspbRegs.SPCR2.all |=0x00C0; // Frame sync & sample rate generators pulled out of reset

    DELAY_US(5E3);

    McbspbRegs.SPCR2.bit.XRST=1; // Enable Transmitter
    McbspbRegs.SPCR1.bit.RRST=1; // Enable Receiver
    EDIS;
}

void InitSPIA()
{
    /* Init SPI peripheral */

    //18: SPICLKA  0/1
    //19: SPISTEA  0/1
    //58: SPISIMOA 3/3

    EALLOW;

    GpioCtrlRegs.GPAGMUX2.bit.GPIO18 = 0; //SPICLKA
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO63 = 1;

    GpioCtrlRegs.GPAGMUX2.bit.GPIO19 = 0; //SPISTEA
    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO63 = 1;

    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 3; //SPISIMOA
    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 3;
    GpioCtrlRegs.GPBDIR.bit.GPIO63 = 1;

    ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 0;
    EDIS;
    SpiaRegs.SPICCR.bit.SPISWRESET = 0;
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 1;
    SpiaRegs.SPICCR.bit.HS_MODE = 0;
    SpiaRegs.SPICCR.bit.SPICHAR = 0x7;
    SpiaRegs.SPICTL.bit.CLK_PHASE = 0;
    SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1;
    SpiaRegs.SPICTL.bit.TALK = 1;
    SpiaRegs.SPIBRR.bit.SPI_BIT_RATE = 4;
    SpiaRegs.SPICCR.bit.SPISWRESET = 1;
    SpiaRegs.SPIPRI.bit.FREE = 1;
}

void SpiTransmit(uint16_t data)
{
    /* Transmit 16 bit data */
    //Wait for Space
    while(SpiaRegs.SPISTS.bit.BUFFULL_FLAG){};
    // WRITE TO TX
    SpiaRegs.SPITXBUF = data;
    // WAIT FOR INT_FLAG
    while(!SpiaRegs.SPISTS.bit.INT_FLAG) {};
    // READ FROM RX
    return SpiaRegs.SPIRXBUF;
}









