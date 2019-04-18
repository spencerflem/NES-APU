#include <F28x_Project.h>
#include <filter.h>
#include <math.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "bandpass.h"
#include "AIC23.h"
#include "lcd.h"
#include "io.h"

interrupt void mcBspRx();

#define PI 3.14159265358979323846

//Since Global, Buffer Init to 0;
#define DFTLEN 256 //256
#define MASK 0xFF //FF
int16 buffer[DFTLEN];
Uint16 index;

volatile Uint16 isLeft = 1;
volatile int32 halfValLeft = 0;
volatile int32 val = 0;
volatile Uint16 go = 0;

Uint16 prevDb = 0;
Uint16 prevHz = 0;

float sinlut[DFTLEN];

#define BIN_WIDTH 46875 / DFTLEN

int main() {
    InitSysCtrl();
    setup_lcd();
    EALLOW;
    GpioCtrlRegs.GPCMUX2.bit.GPIO95 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO95 = 1;
    EDIS;
    InitBigBangedCodecSPI();
    InitAIC23(SR48);
    InitMcBSPb();
    Interrupt_initModule();
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_enableMaster();

    //INIT COS TABLES
    for(int i=0; i < DFTLEN; i++) {
        float sindex = (i * 2 * PI) / DFTLEN;
        sinlut[i] = sinf(sindex);
    }

    while(1) {
        if(go) {

            // Insert Into Buffers
            buffer[index & MASK] = val>>16 & 0xFFFF;

            // Start DFT
            if((index++ & MASK) == MASK) {

                Interrupt_disable(INT_MCBSPB_RX);

                double max_freq_sq = 0;
                int max_k = 0;
                for(int k=0; k < DFTLEN/2; k++) {
                    int j = 0;
                    double xre = 0;
                    double xim = 0;
                    double freq_sq = 0;
                    for(int n=0; n < DFTLEN; n++) {
                        xre += buffer[n] * sinlut[(j+(DFTLEN/4)) & MASK]; //=cos
                        xim -= buffer[n] * sinlut[j & MASK];
                        j += k;
                    }
                    freq_sq = xre * xre + xim * xim;
                    if(freq_sq > max_freq_sq) {
                        max_freq_sq = freq_sq;
                        max_k = k;
                    }
                }

                double max_db = 10 * log( sqrt(max_freq_sq) );
                Uint16 db = max_db;
                Uint16 hz = max_k * BIN_WIDTH;
                if(db > prevDb + 2 || db < prevDb - 2 || hz > prevHz + 2 || hz < prevHz - 2) {
                    prevDb = db;
                    prevHz = hz;
                    char hz_string[] = "Max Freq 0000 Hz";
                    hz_string[9] = hz/1000 % 10 + '0';
                    hz_string[10] = hz/100 % 10 + '0';
                    hz_string[11] = hz/10 % 10 + '0';
                    hz_string[12] = hz % 10 + '0';
                    char db_string[] = "Max Mag  0000 dB";
                    db_string[9] = db/1000 % 10 + '0';
                    db_string[10] = db/100 % 10 + '0';
                    db_string[11] = db/10 % 10 + '0';
                    db_string[12] = db % 10 + '0';
                    clear_screen();
                    write_string(hz_string);
                    shift_screen();
                    write_string(db_string);
                }

                Interrupt_enable(INT_MCBSPB_RX);

            }

            go = 0;
        }
    }
}

interrupt void mcBspRx() {
    Uint16 lo = McbspbRegs.DRR1.all;
    Uint16 hi = McbspbRegs.DRR2.all;
    int32 halfVal = ( ( ((int32) hi) << 16) | lo ) >> 1;
    if(isLeft) {
        isLeft = 0;
        halfValLeft = halfVal;
    }
    else {
        isLeft = 1;
        val = halfValLeft + halfVal;
        go = 1;
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
