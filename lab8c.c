#include <F28x_Project.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "fpu_rfft.h"
#include "math.h"

#define RFFT_STAGES     6
#define RFFT_SIZE       (1 << RFFT_STAGES)
#define F_PER_SAMPLE    (46875/(float)RFFT_SIZE)
#define EPSILON         0.1

#define PI 3.14159265358979323846
#define BIN_WIDTH 46875 / DFTLEN

interrupt void local_D_INTCH1_ISR(void); // Channel 1 Rx ISR
interrupt void local_D_INTCH2_ISR(void); // Channel 2 Tx ISR

// 2 buffers, PING & PONG are used. When PING is being filled up, PONG data is processed and vice versa.
// L & R channel data is stored contigously in PING. Likewise in PONG.
// In both buffers, First 512 32-bit locations are L-channel data. Next 512 32-bit locations are R-channel data.
#pragma DATA_SECTION(ping_buffer, "Ping");
int32 ping_buffer[RFFT_SIZE*2];

#pragma DATA_SECTION(pong_buffer, "Pong");
int32 pong_buffer[RFFT_SIZE*2];

#pragma DATA_SECTION(RFFTin1Buff,"RFFTdata1")
float RFFTin1Buff[RFFT_SIZE];

int32 * L_channel = & ping_buffer[0]; // This pointer points to the beginning of the L-C data in either of the buffers
int32 * R_channel = & ping_buffer[RFFT_SIZE]; // This pointer points to the beginning of the R-C data in either of the buffers

volatile Uint16 * ping_buff_offset = (volatile Uint16 *) & ping_buffer[0];
volatile Uint16 * pong_buff_offset = (volatile Uint16 *) & pong_buffer[0];
volatile Uint16 * DMA_RX;
volatile Uint16 * DMA_TX;

#pragma DATA_SECTION(RFFTmagBuff,"RFFTdata2")
float RFFTmagBuff[RFFT_SIZE/2+1];

#pragma DATA_SECTION(RFFToutBuff,"RFFTdata3")
float RFFToutBuff[RFFT_SIZE];

#pragma DATA_SECTION(RFFTF32Coef,"RFFTdata4")
float RFFTF32Coef[RFFT_SIZE];

RFFT_F32_STRUCT rfft;
RFFT_F32_STRUCT_Handle hnd_rfft = &rfft;

enum State {Wait, GoPing, GoPong};
volatile enum State state = Wait;

uint16_t i, j;
float freq = 0.0;
float hz, db;

void main(void) {
    InitSysCtrl();
    InitPieCtrl();
    InitPieVectTable();

    ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = 0x00;

    hnd_rfft->FFTSize   = RFFT_SIZE;
    hnd_rfft->FFTStages = RFFT_STAGES;
    hnd_rfft->InBuf     = &RFFTin1Buff[0];  //Input buffer
    hnd_rfft->OutBuf    = &RFFToutBuff[0];  //Output buffer
    hnd_rfft->MagBuf    = &RFFTmagBuff[0];  //Magnitude buffer
    hnd_rfft->CosSinBuf = &RFFTF32Coef[0];  //Twiddle factor buffer
    RFFT_f32_sincostable(hnd_rfft);         //Calculate twiddle factor

    ping_buff_offset++; // Start at location 1 (32-bit r/w from loc. 1, then 0)
    pong_buff_offset++; // Start at location 1 (32-bit r/w from loc. 1, then 0)

    DMA_RX = (volatile Uint16 * ) & McbspbRegs.DRR2.all;
    DMA_TX = (volatile Uint16 * ) & McbspbRegs.DXR2.all;

    DMAInitialize();

    DMACH1AddrConfig(ping_buff_offset, DMA_RX); //rx
    DMACH1BurstConfig(1, 1, -1);
    DMACH1TransferConfig(RFFT_SIZE*2-1, -1, RFFT_SIZE*2+1);
    DMACH1WrapConfig(0xFFFF, 0, 1, 2); //srcstep?
    DMACH1ModeConfig(CLA_TRIG_MREVTB, PERINT_ENABLE, ONESHOT_DISABLE, CONT_ENABLE, SYNC_DISABLE,
        SYNC_SRC, OVRFLOW_DISABLE, SIXTEEN_BIT, CHINT_BEGIN, CHINT_ENABLE);

    DMACH2BurstConfig(1, -1, 1);
    DMACH2TransferConfig(RFFT_SIZE*2-1, RFFT_SIZE*2+1, -1);
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
        while(state == Wait){};
        EALLOW;
        GpioDataRegs.GPCTOGGLE.bit.GPIO95 = 1;
        EDIS;
        if(state == GoPing) {
            for(Uint16 i = 0; i < RFFT_SIZE; i++) {
                RFFTin1Buff[i] = ping_buffer[i];
            }
        }
        else {
            for(Uint16 i = 0; i < RFFT_SIZE; i++) {
                RFFTin1Buff[i] = pong_buffer[i];
            }
        }

        RFFT_f32(hnd_rfft);
        state = Wait;
        RFFT_f32_mag_TMU0(hnd_rfft);

        EALLOW;
        GpioDataRegs.GPCTOGGLE.bit.GPIO95 = 0;
        EDIS;

        j = 1;
        freq = RFFTmagBuff[1];
        for(i=2;i<RFFT_SIZE/2+1;i++){
            if(RFFTmagBuff[i] > freq){
                j = i;
                freq = RFFTmagBuff[i];
            }
        }
        freq = F_PER_SAMPLE * (float)j;
        hz = freq;
        db = 10 * log( RFFTmagBuff[j] );
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
