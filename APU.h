#ifndef APU_H_
#define APU_H_

#define samplesPerSecond 46875 //44100
#define cyclesPerSecond 1789772
#define cyclesPerSample (float)cyclesPerSecond / (float)samplesPerSecond

typedef struct {
    int16 volume;
    bool loop;
    bool constantVolume;
    bool startFlag;
    int16 dividerPeriod;
    int16 dividerCounter;
    int16 decay;
    int16 output;
} Envelope;

typedef struct {
    int16 counter;
    int16 counterReloadVal;
    bool reloadFlag;
    bool reloadHold;
} LinearCounter;

typedef struct {
    int16 counter;
    bool halt;
} LengthCounter;

typedef struct {
    Envelope envelope;
    LengthCounter lengthCounter;
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
} Pulse;

typedef struct {
    int16 timerPeriod;
    int16 timer;
    LengthCounter lengthCounter;
    LinearCounter linearCounter;
    int16 currentStep;
    int16 output;
    bool enabled;
} Triangle;

typedef struct {
    int16 timerPeriod;
    int16 timer;
    LengthCounter lengthCounter;
    bool mode;
    Uint16 reg;
    Envelope envelope;
    int16 output;
    bool enabled;
} Noise;

typedef struct {
    Pulse pulse1, pulse2;
    Triangle triangle;
    Noise noise;
    bool frameMode;
    Uint16 frameCounter;
    float remainingCycles;
    int32 output;
} Apu;

static const Apu EmptyApu;

int32 processCycles(Apu *apu, float cycles);
void setPulse1EnvelopeParameters(Apu *apu, bool loop, bool useConstantVolume, int16 periodAndVolume);
void setPulse1EnvelopeLoop(Apu *apu, bool loop);
void setPulse1EnvelopeUseConstantVolume(Apu *apu, bool useConstantVolume);
void setPulse1EnvelopePeriodAndVolume(Apu *apu, int16 periodAndVolume);
void setPulse2EnvelopeParameters(Apu *apu, bool loop, bool useConstantVolume, int16 periodAndVolume);
void setPulse2EnvelopeLoop(Apu *apu, bool loop);
void setPulse2EnvelopeUseConstantVolume(Apu *apu, bool useConstantVolume);
void setPulse2EnvelopePeriodAndVolume(Apu *apu, int16 periodAndVolume);
void setNoiseEnvelopeParameters(Apu *apu, bool loop, bool useConstantVolume, int16 periodAndVolume);
void setNoiseEnvelopeLoop(Apu *apu, bool loop);
void setNoiseEnvelopeUseConstantVolume(Apu *apu, bool useConstantVolume);
void setNoiseEnvelopePeriodAndVolume(Apu *apu, int16 periodAndVolume);
void setPulse1SweepParameters(Apu *apu, bool enabled, int16 period, bool negate, int16 shift);
void setPulse1SweepEnabled(Apu *apu, bool enabled);
void setPulse1SweepPeriod(Apu *apu, int16 period);
void setPulse1SweepNegate(Apu *apu, bool negate);
void setPulse1SweepShift(Apu *apu, int16 shift);
void setPulse2SweepParameters(Apu *apu, bool enabled, int16 period, bool negate, int16 shift);
void setPulse2SweepEnabled(Apu *apu, bool enabled);
void setPulse2SweepPeriod(Apu *apu, int16 period);
void setPulse2SweepNegate(Apu *apu, bool negate);
void setPulse2SweepShift(Apu *apu, int16 shift);
void setPulse1Duty(Apu *apu, int16 duty);
void setPulse2Duty(Apu *apu, int16 duty);
void setPulse1LengthCounterHalt(Apu *apu, bool halt);
void setPulse2LengthCounterHalt(Apu *apu, bool halt);
void setPulse1Period(Apu *apu, int16 period);
void setPulse2Period(Apu *apu, int16 period);
void setPulse1Length(Apu *apu, int16 length);
void setPulse2Length(Apu *apu, int16 length);
void setPulse1PeriodLo(Apu *apu, int16 periodLo);
void setPulse2PeriodLo(Apu *apu, int16 periodLo);
void setPulse1LengthAndPeriodHi(Apu *apu, int16 length, int16 periodHi);
void setPulse2LengthAndPeriodHi(Apu *apu, int16 length, int16 periodHi);
void setTriangleLinearCounterParamsAndLengthCounterHalt(Apu *apu, bool linearReloadHoldLengthHalt, int16 reloadVal);
void setTriangleLengthCounterHaltAndLinearCounterReloadHold(Apu *apu, bool linearReloadHoldLengthHalt);
void setTrianglePeriod(Apu *apu, int16 period);
void setTriangleLength(Apu *apu, int16 length);
void setTrianglePeriodLo(Apu *apu, int16 periodLo);
void setTriangleLengthAndPeriodHi(Apu *apu, int16 length, int16 periodHi);
void setNoiseLengthCounterHalt(Apu *apu, bool halt);
void setNoiseMode(Apu *apu, bool mode);
void setNoisePeriod(Apu *apu, int16 period);
void setNoiseLengthCounter(Apu *apu, int16 length);
void setEnableChannels(Apu *apu, bool pulse1, bool pulse2, bool triangle, bool noise);
void setFrameCounterMode(Apu *apu, bool mode);
void writeRegister(Apu *apu, int16 reg, int16 data);
void initApu(Apu *apu);

#endif /* APU_H_ */
