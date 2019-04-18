#include <F28x_Project.h>
#include <math.h>
#include <stdbool.h>
#include "adc.h"
#include "cputimer.h"
#include "interrupt.h"
#include "lcd.h"
#include "InitAIC23.h"
#include "AIC23.h"

interrupt void TIMER1_INT();
interrupt void mcBspRx();

#define samplesPerSecond 44100
#define cyclesPerSecond 1789772
#define cyclesPerSample (float)cyclesPerSecond / (float)samplesPerSecond / 4
volatile float cyclesToProcess = 0;
int32 sample;

struct Envelope {
    Uint16 output;
};

struct LinearCounter {
    Uint16 counter;
    Uint16 counterReloadVal;
    bool reloadFlag;
    bool reloadHold;
};

struct LengthCounter {
    Uint16 counter;
    bool halt;
};

bool pulseSteps[4][8] = {
    {false, true, false, false, false, false, false, false},
    {false, true, true, false, false, false, false, false},
    {false, true, true, true, true, false, false, false},
    {true, false, false, true, true, true, true, true}
};

struct Pulse {
    struct Sweep sweep;
    struct Envelope envelope;
    struct LengthCounter lengthCounter;
    Uint16 timer;
    Uint16 timerPeriod;
    Uint16 duty;
    Uint16 currentStep;
    bool oddPulse;
    Uint16 output;
};

Uint16 triangleSteps[] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,  4,  3,  2,  1,  0,
    0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

struct Triangle {
    Uint16 timerPeriod;
    Uint16 timer;
    struct LengthCounter lengthCounter;
    struct LinearCounter linearCounter;
    Uint16 currentStep;
    Uint16 output;
};

struct Noise {
    Uint16 timerPeriod;
    Uint16 timer;
    struct LengthCounter lengthCounter;
    bool mode;
    Uint16 reg;
    struct Envelope envelope;
    Uint16 output;
};

struct Apu {
    struct Pulse pulse1, pulse2;
    struct Triangle triangle;
    struct Noise noise;
    bool frameMode;
    Uint16 frameCounter;
};

void initApu(struct Apu *apu) {
    // testing vals
    apu->triangle.timerPeriod = 256;
    apu->triangle.lengthCounter.counter = 192;
    apu->triangle.currentStep = 0;
    apu->triangle.lengthCounter.halt = false;
    apu->triangle.linearCounter.counter = 384;
    apu->noise.reg = 1;
    apu->noise.timerPeriod = 4;
    apu->noise.envelope.output = 15;
    apu->noise.lengthCounter.counter = 192;
    apu->pulse1.envelope.output = 15;
    apu->pulse1.lengthCounter.counter = 192;
    apu->pulse1.timerPeriod = 256;
    apu->pulse2.envelope.output = 15;
    apu->pulse2.lengthCounter.counter = 192;
    apu->pulse2.timerPeriod = 511;
}

void clockLinearCounter(struct LinearCounter *counter) {
    if (counter->reloadFlag == true) {
        counter->counter = counter->counterReloadVal;
    }
    else if (counter->counter > 0) {
        counter->counter--;
    }
    if (!counter->reloadHold) {
        counter->reloadFlag = false;
    }
}

void clockEnvelope(struct Envelope *envelope) {
    //TODO
}

void clockSweep(struct Pulse *pulse) {
    //TODO
}

void clockLengthCounter(struct LengthCounter *counter) {
    if (counter->counter > 0 && !counter->halt) {
        counter->counter--;
    }
}


void quarterClock(struct Apu *apu) {
    // Envelope and Linear
    clockEnvelope(&apu->pulse1.envelope);
    clockEnvelope(&apu->pulse2.envelope);
    clockEnvelope(&apu->noise.envelope);
    clockLinearCounter(&apu->triangle.linearCounter);
}

void halfClock(struct Apu *apu) {
    // Length and Sweep
    clockLengthCounter(&apu->pulse1.lengthCounter);
    clockLengthCounter(&apu->pulse2.lengthCounter);
    clockLengthCounter(&apu->triangle.lengthCounter);
    clockLengthCounter(&apu->noise.lengthCounter);
    clockSweep(&apu->pulse1);
    clockSweep(&apu->pulse2);
}

void clockFrame(struct Apu *apu) {
    if (apu->frameCounter == 29830) {
        apu->frameCounter = 0;
    }
    else {
        apu->frameCounter++;
    }
    //if mode == 1:
    if (apu->frameCounter == 7457) {
        quarterClock(apu);
    }
    else if (apu->frameCounter == 14913) {
        quarterClock(apu);
        halfClock(apu);
    }
    else if (apu->frameCounter == 22371) {
        quarterClock(apu);
    }
    else if (apu->frameCounter == 29829) {
        quarterClock(apu);
        halfClock(apu);
    }
}

void clockPulse(struct Pulse *pulse) {
    // square wave
    if (pulse->oddPulse == true) {
        pulse->oddPulse = false;
    }
    else {
        pulse->oddPulse = true;
        if (pulse->timer == 0) {
            pulse->timer = pulse->timerPeriod;
            pulse->currentStep = (pulse->currentStep+1) & 0x07;
        }
        else {
            pulse->timer--;
        }
    }
    if(pulse->lengthCounter.counter == 0 || pulse->timer < 8 || pulseSteps[pulse->duty][pulse->currentStep] == false) {
        pulse->output = 0;
    }
    else {
        pulse->output = pulse->envelope.output;
    }
}

void clockTriangle(struct Triangle *triangle) {
    // triangle wave
    if (triangle->linearCounter.counter == 0 || triangle->lengthCounter.counter == 0) {
        triangle->output = 0;
    }
    if (triangle->timer == 0) {
        triangle->timer = triangle->timerPeriod;
        if (triangle->linearCounter.counter > 0 && triangle->lengthCounter.counter > 0) {
            triangle->currentStep = (triangle->currentStep+1) & 0x1F;
            triangle->output = triangleSteps[triangle->currentStep];
        }
    }
    else {
        triangle->timer--;
    }
}

void clockNoise(struct Noise *noise) {
    //psudorandom values either on or off
    if (noise->timer == 0) {
        noise->timer = noise->timerPeriod;
        Uint16 feedback;
        if (noise->mode == false) {
            feedback = (noise->reg & 1) ^ (noise->reg & 2);
        }
        else {
            feedback = (noise->reg & 1) ^ (noise->reg & 64);
        }
        noise->reg = (noise->reg >> 1) | (feedback << 14);
    }
    else {
        noise->timer--;
    }
    if(noise->reg & 1 == 1 || noise->lengthCounter.counter == 0) {
        noise->output = 0;
    }
    else {
        noise->output = noise->envelope.output;
    }
}

int32 mixSample(struct Apu *apu) {
    float output = 0.00752 * (apu->pulse1.output + apu->pulse2.output) + 0.00851 * apu->triangle.output + 0.00494 * apu->noise.output;
    return output * 0x7FFFFFFF;
}

void processCycle(struct Apu *apu) {
    clockFrame(apu);
    clockPulse(&apu->pulse1);
    clockPulse(&apu->pulse2);
    clockTriangle(&apu->triangle);
    clockNoise(&apu->noise);
}

int main(void) {

    // inits
    InitSysCtrl();
    InitBigBangedCodecSPI();
    InitAIC23(SR48);
    InitMcBSPb();
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer1, 200, 1000000.0f/samplesPerSecond);
    Interrupt_initModule();
    Interrupt_enable(INT_TIMER1);
    Interrupt_register(INT_TIMER1, &TIMER1_INT);
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_enableMaster();
    CPUTimer_startTimer(CPUTIMER1_BASE); //test

    // Init APU (have function to set up values!!)
    struct Apu apu = {};
    initApu(&apu);

    while (1) {
        // calculate next sample
        Uint16 numCycles = cyclesToProcess;
        if (numCycles > 1) {
            cyclesToProcess -= numCycles;
            for (Uint16 i = 0; i < numCycles; i++) {
                processCycle(&apu);
            }
            sample = mixSample(&apu);
        }
    }

}

interrupt void TIMER1_INT() {
    // start processing next sample
    cyclesToProcess += cyclesPerSample;
    Interrupt_clearACKGroup(INTERRUPT_CPU_INT1);
}

interrupt void mcBspRx() {
    // dummy read
    Uint16 lo = McbspbRegs.DRR1.all;
    Uint16 hi = McbspbRegs.DRR2.all;
    // output the current sample
    McbspbRegs.DXR1.all = sample & 0xFFFF;
    McbspbRegs.DXR2.all = (sample >> 16) & 0xFFFF;
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
