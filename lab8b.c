#include <F28x_Project.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "AIC23.h"

#define PI 3.14159265358979323846
#define DFTLEN 512
#define BIN_WIDTH 46875 / DFTLEN

interrupt void local_D_INTCH1_ISR(void); // Channel 1 Rx ISR
interrupt void local_D_INTCH2_ISR(void); // Channel 2 Tx ISR

// 2 buffers, PING & PONG are used. When PING is being filled up, PONG data is processed and vice versa.
// L & R channel data is stored contigously in PING. Likewise in PONG.
// In both buffers, First 512 32-bit locations are L-channel data. Next 512 32-bit locations are R-channel data.
#pragma DATA_SECTION(ping_buffer, "RFFTdata1");
#pragma DATA_SECTION(pong_buffer, "RFFTdata2");
int32 ping_buffer[DFTLEN*2];
int32 pong_buffer[DFTLEN*2];

int32 * L_channel = & ping_buffer[0]; // This pointer points to the beginning of the L-C data in either of the buffers
int32 * R_channel = & ping_buffer[DFTLEN]; // This pointer points to the beginning of the R-C data in either of the buffers

volatile Uint16 * ping_buff_offset = (volatile Uint16 *) & ping_buffer[0];
volatile Uint16 * pong_buff_offset = (volatile Uint16 *) & pong_buffer[0];
volatile Uint16 * DMA_RX;
volatile Uint16 * DMA_TX;

float sinlut[DFTLEN];
enum State {Wait, GoPing, GoPong};
volatile enum State state = Wait;
volatile Uint16 db;
volatile Uint16 hz;

void main(void) {
    InitSysCtrl();
    InitPieCtrl();
    InitPieVectTable();

    // Init Cos Table
    for(int i=0; i < DFTLEN; i++) {
        float sindex = (i * 2 * PI) / DFTLEN;
        sinlut[i] = sinf(sindex);
    }

    ping_buff_offset++; // Start at location 1 (32-bit r/w from loc. 1, then 0)
    pong_buff_offset++; // Start at location 1 (32-bit r/w from loc. 1, then 0)

    DMA_RX = (volatile Uint16 * ) & McbspbRegs.DRR2.all;
    DMA_TX = (volatile Uint16 * ) & McbspbRegs.DXR2.all;

    DMAInitialize();

    DMACH1AddrConfig(ping_buff_offset, DMA_RX); //rx
    DMACH1BurstConfig(1, 1, -1);
    DMACH1TransferConfig(DFTLEN*2-1, -1, DFTLEN*2+1);
    DMACH1WrapConfig(0xFFFF, 0, 1, 2); //srcstep?
    DMACH1ModeConfig(CLA_TRIG_MREVTB, PERINT_ENABLE, ONESHOT_DISABLE, CONT_ENABLE, SYNC_DISABLE,
        SYNC_SRC, OVRFLOW_DISABLE, SIXTEEN_BIT, CHINT_BEGIN, CHINT_ENABLE);

    DMACH2BurstConfig(1, -1, 1);
    DMACH2TransferConfig(DFTLEN*2-1, DFTLEN*2+1, -1);
    DMACH2AddrConfig(DMA_TX, ping_buff_offset);
    DMACH2WrapConfig(1, 2, 0xFFFF, 0); //dest step?
    DMACH2ModeConfig(CLA_TRIG_MXEVTB, PERINT_ENABLE, ONESHOT_DISABLE, CONT_ENABLE, SYNC_DISABLE,
        SYNC_SRC, OVRFLOW_DISABLE, SIXTEEN_BIT, CHINT_BEGIN, CHINT_ENABLE);

    //something about a bandgap voltage -> stolen from TI code
    EALLOW;
    CpuSysRegs.SECMSEL.bit.PF2SEL = 1;
    EDIS;

    Interrupt_initModule();
    Interrupt_enable(INT_DMA_CH1);
    Interrupt_enable(INT_DMA_CH2);
    Interrupt_register(INT_DMA_CH1, & local_D_INTCH1_ISR);
    Interrupt_register(INT_DMA_CH2, & local_D_INTCH2_ISR);
    Interrupt_enableMaster();

    StartDMACH1();
    StartDMACH2();

    InitMcBSPb();
    InitBigBangedCodecSPI();
    InitAIC23(SR48);

    EALLOW;
    GpioCtrlRegs.GPCMUX2.bit.GPIO95 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO95 = 1;
    EDIS;

    while (1) {
        if (state != Wait) {
            int32 *buffer;
            if(state = GoPing) {
                buffer = &ping_buffer;
            }
            else {
                buffer = &pong_buffer;
            }

            double max_freq_sq = 0;
            int max_k = 0;
            for(int k=0; k < DFTLEN/2; k++) {
                int j = 0;
                double xre = 0;
                double xim = 0;
                double freq_sq = 0;
                for(int n=0; n < DFTLEN; n++) { // Only uses left channel
                    xre += buffer[n] * sinlut[(j+(DFTLEN/4)) & (DFTLEN-1)]; //=cos
                    xim -= buffer[n] * sinlut[j & (DFTLEN-1)];
                    j += k;
                }
                freq_sq = xre * xre + xim * xim;
                if(freq_sq > max_freq_sq) {
                    max_freq_sq = freq_sq;
                    max_k = k;
                }
            }

            db = (Uint16) 10.0 * log( sqrt(max_freq_sq) );
            hz = max_k * BIN_WIDTH;
            state = Wait;
        }
    }
}

interrupt void local_D_INTCH1_ISR(void) // DMA Ch1 - McBSP-B Rx
{
    if ((volatile Uint16 * ) DmaRegs.CH1.DST_ADDR_SHADOW == ping_buff_offset) {
        DMACH1AddrConfig(pong_buff_offset, DMA_RX);
        state = GoPing;
    } else {
        DMACH1AddrConfig(ping_buff_offset, DMA_RX);
        state = GoPong;
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP7);
}

interrupt void local_D_INTCH2_ISR(void) // DMA Ch2 - McBSP-B Tx
{
    if ((volatile Uint16 * ) DmaRegs.CH2.SRC_ADDR_SHADOW == ping_buff_offset) {
        DMACH2AddrConfig(DMA_TX, pong_buff_offset);
    } else {
        DMACH2AddrConfig(DMA_TX, ping_buff_offset);
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP7);

}
