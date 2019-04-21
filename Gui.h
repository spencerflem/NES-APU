#ifndef GUI_H_
#define GUI_H_

#include "apu.h"

struct Gui;

typedef struct {
    char name[13];
    void (*select)(struct Gui *gui);
    bool selected;
} Choice;

typedef struct {
    Choice choices[17];
} GuiState;

typedef struct Gui {
    GuiState state;
    int16 choice;
    bool bottomCursor;
    bool unchanged;
    Uint16 upPressed;
    Uint16 downPressed;
    Uint16 selPressed;
    Apu *apu;
    const char *file;
    int32 fileLoopIndex;
    bool playFile;
    int32 index;
    Uint32 wait;
    Uint32 cycles;
    char *oldBuff;
    char *newBuff;
    bool playLoop;
    int16 recordLoop;
    regs pu1;
    regs pu2;
    regs tri;
    regs noi;
    regs sta;
    int16 keyPressed;
    int16 octave;
    int32 newIndex;
    int32 waitTotal;
    Uint16 press0;
    Uint16 press1;
    Uint16 press2;
    Uint16 press3;
    Uint16 press4;
    Uint16 press5;
    Uint16 press6;
    Uint16 press7;
    Uint16 press8;
    Uint16 press9;
    Uint16 press10;
    Uint16 press11;
    Uint16 press12;
    Uint16 press13;
    bool recPlaying;
} Gui;

void initGuiGpio();
void initGui(Gui *gui, Apu *apu);
void displayGuiState(Gui *gui);
void updateGuiState(Gui *gui);
void gotoState(Gui *gui, GuiState state);

extern const GuiState guiRecording;
extern const GuiState guiMain;
extern const GuiState guiFile;
extern const GuiState guiLoop;
extern const GuiState guiLoopPlay;
extern const GuiState guiRecord;
extern const GuiState guiPulse1;
extern const GuiState guiPulse2;
extern const GuiState guiTriangle;
extern const GuiState guiNoise;
extern const GuiState guiRecPu1;
extern const GuiState guiRecPu2;
extern const GuiState guiRecTri;
extern const GuiState guiRecNoi;
extern const GuiState guiSetPu1;
extern const GuiState guiSetPu2;
extern const GuiState guiSetNoi;
extern const GuiState guiSetPu1Duty;
extern const GuiState guiSetPu1Env;
extern const GuiState guiSetPu1Swe;
extern const GuiState guiSetPu2Duty;
extern const GuiState guiSetPu2Env;
extern const GuiState guiSetPu2Swe;
extern const GuiState guiSetNoiMode;
extern const GuiState guiSetNoiEnv;
extern const GuiState guiSetPu1EnvVol;
extern const GuiState guiSetPu1EnvConst;
extern const GuiState guiSetPu2EnvVol;
extern const GuiState guiSetPu2EnvConst;
extern const GuiState guiSetNoiEnvVol;
extern const GuiState guiSetNoiEnvConst;
extern const GuiState guiSetPu1SweEnabled;
extern const GuiState guiSetPu1SwePeriod;
extern const GuiState guiSetPu1SweNegate;
extern const GuiState guiSetPu1SweShift;
extern const GuiState guiSetPu2SweEnabled;
extern const GuiState guiSetPu2SwePeriod;
extern const GuiState guiSetPu2SweNegate;
extern const GuiState guiSetPu2SweShift;

#endif /* GUI_H_ */
