#include <F28x_Project.h>
#include "APU.h"

Uint16 lengthTable[] = {10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
                        12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};

bool pulseSteps[4][8] = {
    {false, true, false, false, false, false, false, false},
    {false, true, true, false, false, false, false, false},
    {false, true, true, true, true, false, false, false},
    {true, false, false, true, true, true, true, true}
};


Uint16 triangleSteps[] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,  4,  3,  2,  1,  0,
    0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};


Uint16 noisePeriodTableNtsc[] = {4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068};

void setLengthCounter(struct LengthCounter *counter, bool enabled, int16 length) {
    if(enabled) {
        counter->counter = lengthTable[length & 0x1F];
    }
}

void clockLinearCounter(struct LinearCounter *counter) {
    // more precise counter for triangle wave
    if (counter->reloadFlag) {
        counter->counter = counter->counterReloadVal;
        if (!counter->reloadHold) {
            counter->reloadFlag = false;
        }
    }
    else if(counter->counter > 0) {
        counter->counter--;
    }
}

void clockEnvelope(struct Envelope *envelope) {
    // create a sawtooth wave
    if (envelope->startFlag) {
        envelope->startFlag = false;
        envelope->dividerCounter = envelope->dividerPeriod;
        envelope->decay = 15;
    }
    else {
        if (envelope->dividerCounter > 0) {
            envelope->dividerCounter--;
        }
        else {
            envelope->dividerCounter = envelope->dividerPeriod;
            if (envelope->decay > 0) {
                envelope->decay--;
            }
            else if (envelope->loop) {
                envelope->decay = 15;
            }
        }
    }
    // choose sawtooth or constant
    if (envelope->constantVolume) {
        envelope->output = envelope->volume;
    }
    else {
        envelope->output = envelope->decay;
    }
}

void calcSweepTarget(struct Pulse *pulse) {
    // calc target with a barrel shift
    int16 change = ((pulse->timerPeriod >> pulse->sweepShift) | (pulse->timerPeriod << (-pulse->sweepShift & 0xFFF))) & 0xFFF;
    if (!pulse->sweepNegate) {
        pulse->sweepTarget = pulse->timerPeriod + change;
    }
    else if (!pulse->sweepNegateMode) {
        pulse->sweepTarget = pulse->timerPeriod - change;
    }
    else {
        pulse->sweepTarget = pulse->timerPeriod - change - 1;
    }

    // check mute
    if (pulse->sweepTarget > 0xFFF || pulse->timerPeriod < 16) {
        pulse->sweepMute = true;
    }
}

void clockSweep(struct Pulse *pulse) {
    // if sweep is enabled, set the period to the target
    if (pulse->sweepCounter == 0
            && pulse->sweepEnabled
            && !pulse->sweepMute) {
        pulse->timerPeriod = pulse->sweepTarget;
        calcSweepTarget(pulse);
    }
    if (pulse->sweepCounter == 0 || pulse->sweepReload) {
        pulse->sweepCounter = pulse->sweepPeriod;
        pulse->sweepReload = false;
    }
    else {
        pulse->sweepCounter--;
    }
}

void clockLengthCounter(struct LengthCounter *counter) {
    // for note durations
    if (!counter->halt && counter->counter > 0) {
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

void clockFrame(struct Apu *apu, int16 clocks) {
    // Assumes the number of clocks is small

    // Pick frame mode
    Uint16 quarter1 = 7457;
    Uint16 half1 = 14913;
    Uint16 quarter2 = 22371;
    Uint16 half2, period;
    if(!apu->frameMode) {
        half2 = 29829;
        period = 29830;
    }
    else {
        half2 = 37281;
        period = 37282;
    }

    // Check if frame clock
    if((quarter1-apu->frameCounter > 0 && quarter1-apu->frameCounter <= clocks)
            || (quarter2-apu->frameCounter > 0 && quarter2-apu->frameCounter <= clocks)) {
        quarterClock(apu);
    }
    else if((half1-apu->frameCounter > 0 && half1-apu->frameCounter <= clocks)
            || (half2-apu->frameCounter > 0 && half2-apu->frameCounter <= clocks)) {
        quarterClock(apu);
        halfClock(apu);
    }

    // Increment counter
    if (apu->frameCounter + clocks < period) {
        apu->frameCounter = apu->frameCounter + clocks;
    }
    else {
        apu->frameCounter = apu->frameCounter + clocks - period;
    }
}

void clockPulse(struct Pulse *pulse, int16 clocks) {
    // square wave
    //note: the period is double the usual

    // increment timer
    if(pulse->timer - clocks > 0) {
        pulse->timer = pulse->timer - clocks;
    }
    else {
        int16 steps = (clocks - pulse->timer)/pulse->timerPeriod + 1;
        pulse->timer = pulse->timerPeriod - (clocks - pulse->timer)%pulse->timerPeriod;
        pulse->currentStep = (pulse->currentStep + steps) & 0x07;
    }

    // output
    if(pulse->lengthCounter.counter == 0
            || pulse->timer < 16
            || !pulseSteps[pulse->duty][pulse->currentStep]
            || pulse->sweepMute
            || !pulse->enabled) {
        pulse->output = 0;
    }
    else {
        pulse->output = pulse->envelope.output;
    }
}

void clockTriangle(struct Triangle *triangle, int16 clocks) {
    // triangle wave
    if (triangle->linearCounter.counter == 0
            || triangle->lengthCounter.counter == 0
            || !triangle->enabled) {
        triangle->output = 0;
    }
    else {
        // increment timer
        if(triangle->timer - clocks > 0) {
            triangle->timer = triangle->timer - clocks;
        }
        else {
            int16 steps = (clocks - triangle->timer)/triangle->timerPeriod + 1;
            triangle->timer = triangle->timerPeriod - (clocks - triangle->timer)%triangle->timerPeriod;
            triangle->currentStep = (triangle->currentStep + steps) & 0x1F;
        }
        triangle->output = triangleSteps[triangle->currentStep];
    }
}

void clockNoise(struct Noise *noise, int16 clocks) {
    //psudorandom values either on or off

    // increment timer
    if(noise->timer - clocks > 0) {
        noise->timer = noise->timer - clocks;
    }
    else {
        int16 steps = (clocks - noise->timer)/noise->timerPeriod + 1;
        noise->timer = noise->timerPeriod - (clocks - noise->timer)%noise->timerPeriod;
        for (int16 i = 0; i < steps; i++) {
            Uint16 feedback;
            if (!noise->mode) {
                feedback = (noise->reg & 1) ^ (noise->reg & 2);
            }
            else {
                feedback = (noise->reg & 1) ^ (noise->reg & 64);
            }
            noise->reg = (noise->reg >> 1) | (feedback << 14);
        }
    }

    // output
    if(noise->reg & 1 == 1
            || noise->lengthCounter.counter == 0
            || !noise->enabled) {
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

void clockApu(struct Apu *apu, int16 clocks) {
    clockFrame(apu, clocks);
    clockPulse(&apu->pulse1, clocks);
    clockPulse(&apu->pulse2, clocks);
    clockTriangle(&apu->triangle, clocks);
    clockNoise(&apu->noise, clocks);
}

// THE PUBLIC API:
int32 processCycles(struct Apu *apu, float cycles) {
    apu->remainingCycles += cycles;
    Uint16 numCycles = apu->remainingCycles;
    if (numCycles > 1) {
        apu->remainingCycles -= numCycles;
        clockApu(apu, numCycles);
        apu->output = mixSample(apu);
    }
    return apu->output;
}

void setPulse1EnvelopeParameters(struct Apu *apu, bool loop, bool useConstantVolume, int16 periodAndVolume) {
    apu->pulse1.envelope.loop = loop;
    apu->pulse1.envelope.constantVolume = useConstantVolume;
    apu->pulse1.envelope.dividerPeriod = (periodAndVolume & 0xF) + 1;
    apu->pulse1.envelope.volume = (periodAndVolume & 0xF);
}

void setPulse2EnvelopeParameters(struct Apu *apu, bool loop, bool useConstantVolume, int16 periodAndVolume) {
    apu->pulse2.envelope.loop = loop;
    apu->pulse2.envelope.constantVolume = useConstantVolume;
    apu->pulse2.envelope.dividerPeriod = (periodAndVolume & 0xF) + 1;
    apu->pulse2.envelope.volume = (periodAndVolume & 0xF);
}

void setNoiseEnvelopeParameters(struct Apu *apu, bool loop, bool useConstantVolume, int16 periodAndVolume) {
    apu->noise.envelope.loop = loop;
    apu->noise.envelope.constantVolume = useConstantVolume;
    apu->noise.envelope.dividerPeriod = (periodAndVolume & 0xF) + 1;
    apu->noise.envelope.volume = (periodAndVolume & 0xF);
}

void setPulse1SweepParameters(struct Apu *apu, bool enabled, int16 period, bool negate, int16 shift) {
    apu->pulse1.sweepReload = true;
    apu->pulse1.sweepEnabled = enabled;
    apu->pulse1.sweepPeriod = (period & 0x7) + 1;
    apu->pulse1.sweepNegate = negate;
    apu->pulse1.sweepShift = (shift & 0x7);
}

void setPulse2SweepParameters(struct Apu *apu, bool enabled, int16 period, bool negate, int16 shift) {
    apu->pulse2.sweepReload = true;
    apu->pulse2.sweepEnabled = enabled;
    apu->pulse2.sweepPeriod = (period & 0x7) + 1;
    apu->pulse2.sweepNegate = negate;
    apu->pulse2.sweepShift = (shift & 0x7);
}

void setPulse1Duty(struct Apu *apu, int16 duty) {
    apu->pulse1.duty = (duty & 0x3);
}

void setPulse2Duty(struct Apu *apu, int16 duty) {
    apu->pulse2.duty = (duty & 0x3);
}

void setPulse1LengthCounterHalt(struct Apu *apu, bool halt) {
    apu->pulse1.lengthCounter.halt = halt;
}

void setPulse2LengthCounterHalt(struct Apu *apu, bool halt) {
    apu->pulse2.lengthCounter.halt = halt;
}

void setPulse1Period(struct Apu *apu, int16 period) {
    apu->pulse1.timerPeriod = ((period*2) & 0xFFF);
    apu->pulse1.currentStep = 0;
    apu->pulse1.envelope.startFlag = true;
    calcSweepTarget(&apu->pulse1);
}

void setPulse2Period(struct Apu *apu, int16 period) {
    apu->pulse2.timerPeriod = ((period*2) & 0xFFF);
    apu->pulse2.currentStep = 0;
    apu->pulse2.envelope.startFlag = true;
    calcSweepTarget(&apu->pulse2);
}

void setPulse1Length(struct Apu *apu, int16 length) {
    setLengthCounter(&apu->pulse1.lengthCounter, apu->pulse1.enabled, (length & 0x1F));
    apu->pulse1.envelope.startFlag = true;
}

void setPulse2Length(struct Apu *apu, int16 length) {
    setLengthCounter(&apu->pulse2.lengthCounter, apu->pulse2.enabled, (length & 0x1F));
    apu->pulse2.envelope.startFlag = true;
}

void setPulse1PeriodLo(struct Apu *apu, int16 periodLo) {
    apu->pulse1.timerPeriod &= 0x1FF;
    apu->pulse1.timerPeriod |= ((periodLo*2) & 0x1FF);
    calcSweepTarget(&apu->pulse1);
}

void setPulse2PeriodLo(struct Apu *apu, int16 periodLo) {
    apu->pulse2.timerPeriod &= 0x1FF;
    apu->pulse2.timerPeriod |= ((periodLo*2) & 0x1FF);
    calcSweepTarget(&apu->pulse2);
}

void setPulse1LengthAndPeriodHi(struct Apu *apu, int16 length, int16 periodHi) {
    apu->pulse1.timerPeriod &= 0xE00;
    apu->pulse1.timerPeriod |= ((periodHi << 9) & 0xE00);
    apu->pulse1.currentStep = 0;
    calcSweepTarget(&apu->pulse1);
    setLengthCounter(&apu->pulse1.lengthCounter, apu->pulse1.enabled, (length & 0x1F));
    apu->pulse1.envelope.startFlag = true;
}

void setPulse2LengthAndPeriodHi(struct Apu *apu, int16 length, int16 periodHi) {
    apu->pulse2.timerPeriod &= 0xE00;
    apu->pulse2.timerPeriod |= ((periodHi << 9) & 0xE00);
    apu->pulse2.currentStep = 0;
    calcSweepTarget(&apu->pulse2);
    setLengthCounter(&apu->pulse2.lengthCounter, apu->pulse2.enabled, (length & 0x1F));
    apu->pulse2.envelope.startFlag = true;
}


void setTriangleLinearCounterParamsAndLengthCounterHalt(struct Apu *apu, bool linearReloadHoldLengthHalt, int16 reloadVal) {
    apu->triangle.linearCounter.reloadHold = linearReloadHoldLengthHalt;
    apu->triangle.lengthCounter.halt = linearReloadHoldLengthHalt;
    apu->triangle.linearCounter.counterReloadVal = reloadVal & 0x7F;
}

void setTriangleLengthCounterHaltAndLinearCounterReloadHold(struct Apu *apu, bool linearReloadHoldLengthHalt) {
    apu->triangle.linearCounter.reloadHold = linearReloadHoldLengthHalt;
    apu->triangle.lengthCounter.halt = linearReloadHoldLengthHalt;
}

void setTrianglePeriod(struct Apu *apu, int16 period) {
    apu->triangle.timerPeriod = (period & 0x7FF);
    apu->triangle.linearCounter.reloadFlag = true;
}

void setTriangleLength(struct Apu *apu, int16 length) {
    setLengthCounter(&apu->triangle.lengthCounter, apu->triangle.enabled, (length & 0x1F));
    apu->triangle.linearCounter.reloadFlag = true;
}

void setTrianglePeriodLo(struct Apu *apu, int16 periodLo) {
    apu->pulse1.timerPeriod &= 0xFF;
    apu->pulse1.timerPeriod |= (periodLo & 0xFF);
}

void setTriangleLengthAndPeriodHi(struct Apu *apu, int16 length, int16 periodHi) {
    apu->pulse1.timerPeriod &= 0x700;
    apu->pulse1.timerPeriod |= ((periodHi << 8) & 0x700);
    setLengthCounter(&apu->triangle.lengthCounter, apu->triangle.enabled, (length & 0x1F));
    apu->triangle.linearCounter.reloadFlag = true;
}

void setNoiseLengthCounterHalt(struct Apu *apu, bool halt) {
    apu->noise.lengthCounter.halt = halt;
}

void setNoiseMode(struct Apu *apu, bool mode) {
    apu->noise.mode = mode;
}

void setNoisePeriod(struct Apu *apu, int16 period) {
    // Assuming NTSC:
    apu->noise.timerPeriod = noisePeriodTableNtsc[period & 0x0F];
}

void setNoiseLengthCounter(struct Apu *apu, int16 length) {
    setLengthCounter(&apu->noise.lengthCounter, apu->noise.enabled, (length & 0x1F));
    apu->pulse1.envelope.startFlag = true;
}

void setEnableChannels(struct Apu *apu, bool pulse1, bool pulse2, bool triangle, bool noise) {
    apu->pulse1.enabled = pulse1;
    apu->pulse2.enabled = pulse2;
    apu->triangle.enabled = triangle;
    apu->noise.enabled = noise;
    if (!pulse1) {
        apu->pulse1.lengthCounter.counter = 0;
    }
    if (!pulse2) {
        apu->pulse2.lengthCounter.counter = 0;
    }
    if (!triangle) {
        apu->triangle.lengthCounter.counter = 0;
    }
    if (!noise) {
        apu->noise.lengthCounter.counter = 0;
    }
}

void setFrameCounterMode(struct Apu *apu, bool mode) {
    apu->frameMode = mode;
}

void writeRegister(struct Apu *apu, int16 reg, int16 data) {
    switch(reg & 0xFF) {
    case 0x00:
        setPulse1Duty(apu, (data & 0xC0) >> 6);
        setPulse1LengthCounterHalt(apu, (data & 0x20) >> 5);
        setPulse1EnvelopeParameters(apu, (data & 0x20) >> 5, (data & 0x10) >> 4, (data & 0xF));
        break;
    case 0x01:
        setPulse1SweepParameters(apu, (data & 0x80) >> 7, (data & 0x70) >> 4, (data & 0x08) >> 3, (data & 0x07));
        break;
    case 0x02:
        setPulse1PeriodLo(apu, data);
        break;
    case 0x03:
        setPulse1LengthAndPeriodHi(apu, (data & 0xF8) >> 3, (data & 0x07));
        break;
    case 0x04:
        setPulse2Duty(apu, (data & 0xC0) >> 6);
        setPulse2LengthCounterHalt(apu, (data & 0x20) >> 5);
        setPulse2EnvelopeParameters(apu, (data & 0x20) >> 5, (data & 0x10) >> 4, (data & 0xF));
        break;
    case 0x05:
        setPulse1SweepParameters(apu, (data & 0x80) >> 7, (data & 0x70) >> 4, (data & 0x08) >> 3, (data & 0x07));
        break;
    case 0x06:
        setPulse2PeriodLo(apu, data);
        break;
    case 0x07:
        setPulse1LengthAndPeriodHi(apu, (data & 0xF8) >> 3, (data & 0x07));
        break;
    case 0x08:
        setTriangleLinearCounterParamsAndLengthCounterHalt(apu, (data & 0x80) >> 7, (data & 0x7F));
        break;
    case 0x0A:
        setTrianglePeriodLo(apu, data);
        break;
    case 0x0B:
        setTriangleLengthAndPeriodHi(apu, (data & 0xF8) >> 3, (data & 0x07));
        break;
    case 0x0C:
        setNoiseLengthCounterHalt(apu, (data & 0x20) >> 5);
        setNoiseEnvelopeParameters(apu, (data & 0x20) >> 5, (data & 0x10) >> 4, (data & 0xF));
        break;
    case 0x0E:
        setNoiseMode(apu, (data & 0x80) >> 7);
        setNoisePeriod(apu, (data & 0x0F));
        break;
    case 0x0F:
        setNoiseLengthCounter(apu, (data & 0xF8) >> 3);
        break;
    case 0x15:
        setEnableChannels(apu, (data & 0x01), (data & 0x02) >> 1, (data & 0x04) >> 2, (data & 0x08) >> 3);
        break;
    case 0x17:
        setFrameCounterMode(apu, (data & 0x80) >> 7);
        break;
    }
}

void initApu(struct Apu *apu) {
    // TODO! WHAT SHOULD THIS BE?
    // AT MINIMUM: pulse1 sweep mode, pulse2 sweep mode, noise reg
    // testing vals


    //apu->triangle.enabled = true;
    apu->pulse1.enabled = true;
    //apu->pulse2.enabled = true;
    //apu->noise.enabled = true;


    apu->triangle.timerPeriod = 256;
    apu->triangle.lengthCounter.counter = 192;
    apu->triangle.currentStep = 0;
    apu->triangle.lengthCounter.halt = false;
    apu->triangle.linearCounter.counter = 384;

    apu->noise.reg = 1;
    apu->noise.timerPeriod = 128;
    apu->noise.envelope.constantVolume = true;
    apu->noise.envelope.volume = 15;
    apu->noise.lengthCounter.counter = 192;

    apu->pulse1.envelope.output = 15;
    apu->pulse1.lengthCounter.counter = 192;
    apu->pulse1.sweepEnabled = true;
    apu->pulse1.sweepMute = false;
    apu->pulse1.sweepPeriod = 1;
    apu->pulse1.sweepShift = 1;
    apu->pulse1.sweepNegate = true;
    setPulse1Period(apu, 512);
    apu->pulse2.envelope.output = 15;
    apu->pulse2.lengthCounter.counter = 192;
    apu->pulse2.timerPeriod = 128;
    apu->pulse2.envelope.loop = true;
    apu->pulse2.envelope.constantVolume = false;
    apu->pulse2.sweepNegate = true;
    apu->pulse2.sweepShift = 0;
}
