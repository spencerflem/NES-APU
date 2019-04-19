#ifndef APU_H_
#define APU_H_

#define samplesPerSecond 46875 //44100
#define cyclesPerSecond 1789772
#define cyclesPerSample (float)cyclesPerSecond / (float)samplesPerSecond

struct Envelope {
    int16 volume;
    bool loop;
    bool constantVolume;
    bool startFlag;
    int16 dividerPeriod;
    int16 dividerCounter;
    int16 decay;
    int16 output;
};

struct LinearCounter {
    int16 counter;
    int16 counterReloadVal;
    bool reloadFlag;
    bool reloadHold;
};

struct LengthCounter {
    int16 counter;
    bool halt;
};

struct Pulse {
    struct Envelope envelope;
    struct LengthCounter lengthCounter;
    int16 timer;
    int16 timerPeriod;
    int16 duty;
    int16 currentStep;
    int16 output;
    int16 sweepTarget;
    bool sweepEnabled;
    bool sweepNegate;
    bool sweepNegateMode;
    bool sweepReload;
    int16 sweepShift;
    int16 sweepCounter;
    int16 sweepPeriod;
    bool sweepMute;
    bool enabled;
};

struct Triangle {
    int16 timerPeriod;
    int16 timer;
    struct LengthCounter lengthCounter;
    struct LinearCounter linearCounter;
    int16 currentStep;
    int16 output;
    bool enabled;
};

struct Noise {
    int16 timerPeriod;
    int16 timer;
    struct LengthCounter lengthCounter;
    bool mode;
    Uint16 reg;
    struct Envelope envelope;
    int16 output;
    bool enabled;
};

struct Apu {
    struct Pulse pulse1, pulse2;
    struct Triangle triangle;
    struct Noise noise;
    bool frameMode;
    Uint16 frameCounter;
    float remainingCycles;
    int32 output;
};

int32 processCycles(struct Apu *apu, float cycles);
void setPulse1EnvelopeParameters(struct Apu *apu, bool loop, bool useConstantVolume, int16 periodAndVolume);
void setPulse2EnvelopeParameters(struct Apu *apu, bool loop, bool useConstantVolume, int16 periodAndVolume);
void setNoiseEnvelopeParameters(struct Apu *apu, bool loop, bool useConstantVolume, int16 periodAndVolume);
void setPulse1SweepParameters(struct Apu *apu, bool enabled, int16 period, bool negate, int16 shift);
void setPulse2SweepParameters(struct Apu *apu, bool enabled, int16 period, bool negate, int16 shift);
void setPulse1Duty(struct Apu *apu, int16 duty);
void setPulse2Duty(struct Apu *apu, int16 duty);
void setPulse1LengthCounterHalt(struct Apu *apu, bool halt);
void setPulse2LengthCounterHalt(struct Apu *apu, bool halt);
void setPulse1Period(struct Apu *apu, int16 period);
void setPulse2Period(struct Apu *apu, int16 period);
void setPulse1Length(struct Apu *apu, int16 length);
void setPulse2Length(struct Apu *apu, int16 length);
void setPulse1PeriodLo(struct Apu *apu, int16 periodLo);
void setPulse2PeriodLo(struct Apu *apu, int16 periodLo);
void setPulse1LengthAndPeriodHi(struct Apu *apu, int16 length, int16 periodHi);
void setPulse2LengthAndPeriodHi(struct Apu *apu, int16 length, int16 periodHi);
void setTriangleLinearCounterParamsAndLengthCounterHalt(struct Apu *apu, bool linearReloadHoldLengthHalt, int16 reloadVal);
void setTriangleLengthCounterHaltAndLinearCounterReloadHold(struct Apu *apu, bool linearReloadHoldLengthHalt);
void setTrianglePeriod(struct Apu *apu, int16 period);
void setTriangleLength(struct Apu *apu, int16 length);
void setTrianglePeriodLo(struct Apu *apu, int16 periodLo);
void setTriangleLengthAndPeriodHi(struct Apu *apu, int16 length, int16 periodHi);
void setNoiseLengthCounterHalt(struct Apu *apu, bool halt);
void setNoiseMode(struct Apu *apu, bool mode);
void setNoisePeriod(struct Apu *apu, int16 period);
void setNoiseLengthCounter(struct Apu *apu, int16 length);
void setEnableChannels(struct Apu *apu, bool pulse1, bool pulse2, bool triangle, bool noise);
void setFrameCounterMode(struct Apu *apu, bool mode);
void writeRegister(struct Apu *apu, int16 reg, int16 data);
void initApu(struct Apu *apu);

#endif /* APU_H_ */
