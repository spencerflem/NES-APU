#include <F28x_Project.h>

void init_sram_spi() {
    EALLOW;
    GpioCtrlRegs.GPBGMUX2.bit.GPIO63 = 3;
    GpioCtrlRegs.GPCGMUX1.bit.GPIO64 = 3;
    GpioCtrlRegs.GPCGMUX1.bit.GPIO65 = 3;
    GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 3;
    GpioCtrlRegs.GPCMUX1.bit.GPIO64 = 3;
    GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 3;
    GpioCtrlRegs.GPCMUX1.bit.GPIO66 = 0;
    GpioCtrlRegs.GPCMUX1.bit.GPIO67 = 0;
    GpioCtrlRegs.GPCQSEL1.bit.GPIO64 = 3;
    GpioCtrlRegs.GPBDIR.bit.GPIO63 = 1;
    GpioCtrlRegs.GPCDIR.bit.GPIO64 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO65 = 1;
    GpioCtrlRegs.GPCDIR.bit.GPIO66 = 1;
    GpioCtrlRegs.GPCDIR.bit.GPIO67 = 1;
    ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 0;
    EDIS;
    SpibRegs.SPICCR.bit.SPISWRESET = 0;
    SpibRegs.SPICCR.bit.CLKPOLARITY = 0;
    SpibRegs.SPICCR.bit.HS_MODE = 1;
    SpibRegs.SPICCR.bit.SPICHAR = 0x7;
    SpibRegs.SPICTL.bit.CLK_PHASE = 1;
    SpibRegs.SPICTL.bit.MASTER_SLAVE = 1;
    SpibRegs.SPICTL.bit.TALK = 1;
    SpibRegs.SPIBRR.bit.SPI_BIT_RATE = 4;
    SpibRegs.SPICCR.bit.SPISWRESET = 1;
    SpibRegs.SPIPRI.bit.FREE = 1;
}

Uint16 spi_send(Uint16 data) {
    //Wait for Space
    while(SpibRegs.SPISTS.bit.BUFFULL_FLAG){};
    // WRITE TO TX
    SpibRegs.SPITXBUF = (data & 0x00FF) << 8;
    // WAIT FOR INT_FLAG
    while(!SpibRegs.SPISTS.bit.INT_FLAG) {};
    // READ FROM RX
    return SpibRegs.SPIRXBUF;
}

void write_sram1(Uint32 addr, Uint16 value) {
    // PULL CS LOW
    GpioDataRegs.GPCDAT.bit.GPIO66 = 0;
    spi_send(0x02);
    spi_send(addr>>16 & 0x00FF);
    spi_send(addr>>8 & 0x00FF);
    spi_send(addr & 0x00FF);
    spi_send(value);
    // PULL CS HIGH
    GpioDataRegs.GPCDAT.bit.GPIO66 = 1;
}

void write_sram2(Uint32 addr, Uint16 value) {
    // PULL CS LOW
    GpioDataRegs.GPCDAT.bit.GPIO67 = 0;
    spi_send(0x02);
    spi_send(addr>>16 & 0x00FF);
    spi_send(addr>>8 & 0x00FF);
    spi_send(addr & 0x00FF);
    spi_send(value);
    // PULL CS HIGH
    GpioDataRegs.GPCDAT.bit.GPIO67 = 1;
}

Uint16 read_sram1(Uint32 addr) {
    // PULL CS LOW
    GpioDataRegs.GPCDAT.bit.GPIO66 = 0;
    spi_send(0x03);
    spi_send(addr>>16 & 0x00FF);
    spi_send(addr>>8 & 0x00FF);
    spi_send(addr & 0x00FF);
    spi_send(0);
    Uint16 val = spi_send(0);
    // PULL CS HIGH
    GpioDataRegs.GPCDAT.bit.GPIO66 = 1;
    return val & 0xFF;
}

Uint16 read_sram2(Uint32 addr) {
    // PULL CS LOW
    GpioDataRegs.GPCDAT.bit.GPIO67 = 0;
    spi_send(0x03);
    spi_send(addr>>16 & 0x00FF);
    spi_send(addr>>8 & 0x00FF);
    spi_send(addr & 0x00FF);
    spi_send(0);
    Uint16 val = spi_send(0);
    // PULL CS HIGH
    GpioDataRegs.GPCDAT.bit.GPIO67 = 1;
    return val & 0xFF;
}

void write_sram(Uint32 addr, Uint16 data) {
    if(addr < 131072) {
        write_sram1(addr*2, data & 0x00FF);
        write_sram1(addr*2+1, data>>8 & 0x00FF);
    }
    else {
        write_sram2((addr-131072)*2, data & 0x00FF);
        write_sram2((addr-131072)*2+1, data>>8 & 0x00FF);
    }
}

Uint16 read_sram(Uint32 addr) {
    if(addr < 131072) {
        Uint16 data = read_sram1(addr*2);
        data |= read_sram1(addr*2+1)<<8;
        return data;
    }
    else {
        Uint16 data = read_sram2((addr-131072)*2);
        data |= read_sram2((addr-131072)*2+1)<<8;
        return data;
    }
}


