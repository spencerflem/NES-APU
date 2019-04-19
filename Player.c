#include <F28x_Project.h>
#include <math.h>
#include <stdbool.h>
#include "adc.h"
#include "interrupt.h"
#include "lcd.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "APU.h"
#include "secret.h"
#include "intro.h"
#include "lcd.h"
#include "io.h"

// TODO: IMPLEMENT READING WHOLE FILE
// TODO: IMPLEMENT LOOP
// TODO: SPLIT INTO BYTE SIZE CHUNKS?
// TODO: READ FILE AND METADATA
// TODO: PIANO PLAYER

interrupt void mcBspRx();

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
    // TODO MORE VARS?
} Gui;

#pragma DATA_SECTION(guiMain, "gui");
GuiState guiMain;
#pragma DATA_SECTION(guiFile, "gui");
GuiState guiFile;
#pragma DATA_SECTION(guiLoop, "gui");
GuiState guiLoop;
#pragma DATA_SECTION(guiLoopPlay, "gui");
GuiState guiLoopPlay;
#pragma DATA_SECTION(guiRecord, "gui");
GuiState guiRecord;
#pragma DATA_SECTION(guiPulse1, "gui");
GuiState guiPulse1;
#pragma DATA_SECTION(guiPulse2, "gui");
GuiState guiPulse2;
#pragma DATA_SECTION(guiTriangle, "gui");
GuiState guiTriangle;
#pragma DATA_SECTION(guiNoise, "gui");
GuiState guiNoise;
#pragma DATA_SECTION(guiRecPu1, "gui");
GuiState guiRecPu1;
#pragma DATA_SECTION(guiRecPu2, "gui");
GuiState guiRecPu2;
#pragma DATA_SECTION(guiRecTri, "gui");
GuiState guiRecTri;
#pragma DATA_SECTION(guiRecNoi, "gui");
GuiState guiRecNoi;
#pragma DATA_SECTION(guiSetPu1, "gui");
GuiState guiSetPu1;
#pragma DATA_SECTION(guiSetPu2, "gui");
GuiState guiSetPu2;
#pragma DATA_SECTION(guiSetNoi, "gui");
GuiState guiSetNoi;
#pragma DATA_SECTION(guiSetPu1Duty, "gui");
GuiState guiSetPu1Duty;
#pragma DATA_SECTION(guiSetPu1Env, "gui");
GuiState guiSetPu1Env;
#pragma DATA_SECTION(guiSetPu1Swe, "gui");
GuiState guiSetPu1Swe;
#pragma DATA_SECTION(guiSetPu2Duty, "gui");
GuiState guiSetPu2Duty;
#pragma DATA_SECTION(guiSetPu2Env, "gui");
GuiState guiSetPu2Env;
#pragma DATA_SECTION(guiSetPu2Swe, "gui");
GuiState guiSetPu2Swe;
#pragma DATA_SECTION(guiSetNoiMode, "gui");
GuiState guiSetNoiMode;
#pragma DATA_SECTION(guiSetNoiEnv, "gui");
GuiState guiSetNoiEnv;
#pragma DATA_SECTION(guiSetPu1EnvVol, "gui");
GuiState guiSetPu1EnvVol;
#pragma DATA_SECTION(guiSetPu1EnvConst, "gui");
GuiState guiSetPu1EnvConst;
#pragma DATA_SECTION(guiSetPu2EnvVol, "gui");
GuiState guiSetPu2EnvVol;
#pragma DATA_SECTION(guiSetPu2EnvConst, "gui");
GuiState guiSetPu2EnvConst;
#pragma DATA_SECTION(guiSetNoiEnvVol, "gui");
GuiState guiSetNoiEnvVol;
#pragma DATA_SECTION(guiSetNoiEnvConst, "gui");
GuiState guiSetNoiEnvConst;
#pragma DATA_SECTION(guiSetPu1SweEnabled, "gui");
GuiState guiSetPu1SweEnabled;
#pragma DATA_SECTION(guiSetPu1SwePeriod, "gui");
GuiState guiSetPu1SwePeriod;
#pragma DATA_SECTION(guiSetPu1SweNegate, "gui");
GuiState guiSetPu1SweNegate;
#pragma DATA_SECTION(guiSetPu1SweShift, "gui");
GuiState guiSetPu1SweShift;
#pragma DATA_SECTION(guiSetPu2SweEnabled, "gui");
GuiState guiSetPu2SweEnabled;
#pragma DATA_SECTION(guiSetPu2SwePeriod, "gui");
GuiState guiSetPu2SwePeriod;
#pragma DATA_SECTION(guiSetPu2SweNegate, "gui");
GuiState guiSetPu2SweNegate;
#pragma DATA_SECTION(guiSetPu2SweShift, "gui");
GuiState guiSetPu2SweShift;


void gotoState(Gui *gui, GuiState state) {
    gui->state = state;
    gui->bottomCursor = false;
    gui->choice = 0;
    gui->unchanged = 0;
}

void actFile(Gui *gui) {
    gotoState(gui, guiFile);
    // TODO: PLAY FILE
}
void actLoop(Gui *gui) { gotoState(gui, guiLoop); }
GuiState guiMain = { { {"File", &actFile},
                       {"Loop", &actLoop} } };


void actFileBack(Gui *gui) { gotoState(gui, guiMain); }
GuiState guiFile = { { {"Back", actFileBack} } };

void actLoopBack(Gui *gui) { gotoState(gui, guiMain); }
void actLoopPlay(Gui *gui) { gotoState(gui, guiLoopPlay); }
void actRecord(Gui *gui) { gotoState(gui, guiRecord); }
GuiState guiLoop = { { {"Back", &actLoopBack},
                       {"Play", &actLoopPlay},
                       {"Record", &actRecord} } };

void actLoopPlayBack(Gui *gui) { gotoState(gui, guiLoop); }
GuiState guiLoopPlay = { { {"Back", &actLoopPlayBack} } };

void actRecordBack(Gui *gui) { gotoState(gui, guiLoop); }
void actPulse1(Gui *gui) { gotoState(gui, guiPulse1); }
void actPulse2(Gui *gui) { gotoState(gui, guiPulse2); }
void actTriangle(Gui *gui) { gotoState(gui, guiTriangle); }
void actNoise(Gui *gui) { gotoState(gui, guiNoise); }
GuiState guiRecord = { { { "Back", &actRecordBack},
                         {"Pulse1", &actPulse1},
                         {"Pulse2", &actPulse2},
                         {"Triangle", &actTriangle},
                         {"Noise", &actNoise} } };

void actPulse1Back(Gui *gui) { gotoState(gui, guiRecord); }
void actPulse1Record(Gui *gui) { gotoState(gui, guiRecPu1); /* TODO */; }
void actPulse1Settings(Gui *gui) { gotoState(gui, guiSetPu1); }
void actPulse1Reset(Gui *gui) { /* TODO */; }
GuiState guiPulse1 = { { {"Back", &actPulse1Back},
                         {"Record", &actPulse1Record},
                         {"Settings", &actPulse1Settings},
                         {"Reset", &actPulse1Reset} } };

void actPulse2Back(Gui *gui) { gotoState(gui, guiRecord); }
void actPulse2Record(Gui *gui) { gotoState(gui, guiRecPu2); /* TODO */; }
void actPulse2Settings(Gui *gui) { gotoState(gui, guiSetPu2); }
void actPulse2Reset(Gui *gui) { /* TODO */; }
GuiState guiPulse2 = { { {"Back", &actPulse2Back},
                         {"Record", &actPulse2Record},
                         {"Settings", &actPulse2Settings},
                         {"Reset", &actPulse2Reset} } };

void actTriangleBack(Gui *gui) { gotoState(gui, guiRecord); }
void actTriangleRecord(Gui *gui) { gotoState(gui, guiRecTri); /* TODO */; }
void actTriangleReset(Gui *gui) { /* TODO */; }
GuiState guiTriangle = { { {"Back", &actTriangleBack},
                           {"Record", &actTriangleRecord},
                           {"Reset", &actTriangleReset} } };

void actNoiseBack(Gui *gui) { gotoState(gui, guiRecord); }
void actNoiseRecord(Gui *gui) { gotoState(gui, guiRecNoi); /* TODO */; }
void actNoiseSettings(Gui *gui) { gotoState(gui, guiSetNoi); }
void actNoiseReset(Gui *gui) { /* TODO */; }
GuiState guiNoise = { { {"Back", &actNoiseBack},
                        {"Record", &actNoiseRecord},
                        {"Settings", &actNoiseSettings},
                        {"Reset", &actNoiseReset} } };

void actRecPu1Back(Gui *gui) { gotoState(gui, guiPulse1); }
GuiState guiRecPu1 = { { {"Back", &actRecPu1Back} } };

void actRecPu2Back(Gui *gui) { gotoState(gui, guiPulse2); }
GuiState guiRecPu2 = { { {"Back", &actRecPu2Back} } };

void actRecTriBack(Gui *gui) { gotoState(gui, guiTriangle); }
GuiState guiRecTri = { { {"Back", &actRecTriBack} } };

void actRecNoiBack(Gui *gui) { gotoState(gui, guiNoise); }
GuiState guiRecNoi = { { {"Back", &actRecNoiBack} } };

void actSetPu1Back(Gui *gui) { gotoState(gui, guiPulse1); }
void actSetPu1Duty(Gui *gui) { gotoState(gui, guiSetPu1Duty); }
void actSetPu1Env(Gui *gui) { gotoState(gui, guiSetPu1Env); }
void actSetPu1Swe(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
GuiState guiSetPu1 = { { {"Back", &actSetPu1Back},
                         {"Duty", &actSetPu1Duty},
                         {"Envelope", &actSetPu1Env},
                         {"Sweep", &actSetPu1Swe} } };

void actSetPu2Back(Gui *gui) { gotoState(gui, guiPulse2); }
void actSetPu2Duty(Gui *gui) { gotoState(gui, guiSetPu2Duty); }
void actSetPu2Env(Gui *gui) { gotoState(gui, guiSetPu2Env); }
void actSetPu2Swe(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
GuiState guiSetPu2 = { { {"Back", &actSetPu2Back},
                         {"Duty", &actSetPu2Duty},
                         {"Envelope", &actSetPu2Env},
                         {"Sweep", &actSetPu2Swe} } };

void actSetNoiBack(Gui *gui) { gotoState(gui, guiNoise); }
void actSetNoiMode(Gui *gui) { gotoState(gui, guiSetNoiMode); }
void actSetNoiEnv(Gui *gui) { gotoState(gui, guiSetNoiEnv); }
GuiState guiSetNoi = { { {"Back", &actSetNoiBack},
                         {"Mode", &actSetNoiMode},
                         {"Envelope", &actSetNoiEnv} } };

void actSetPu1DutyBack(Gui *gui) { gotoState(gui, guiSetPu1); }
void actSetPu1Duty0(Gui *gui) { /*TODO*/; }
void actSetPu1Duty1(Gui *gui) { /*TODO*/; }
void actSetPu1Duty2(Gui *gui) { /*TODO*/; }
void actSetPu1Duty3(Gui *gui) { /*TODO*/; }
GuiState guiSetPu1Duty = { { {"Back", &actSetPu1DutyBack},
                             {"12.5%", &actSetPu1Duty0, true},
                             {"25%", &actSetPu1Duty1},
                             {"50%", &actSetPu1Duty2},
                             {"75%", &actSetPu1Duty3} } };

void actSetPu1EnvBack(Gui *gui) { gotoState(gui, guiSetPu1); }
void actSetPu1EnvVol(Gui *gui) { gotoState(gui, guiSetPu1EnvVol); }
void actSetPu1EnvMode(Gui *gui) { gotoState(gui, guiSetPu1EnvConst); }
GuiState guiSetPu1Env = { { {"Back", &actSetPu1EnvBack},
                            {"Vol/Period", &actSetPu1EnvVol},
                            {"Mode", &actSetPu1EnvMode} } };

void actSetPu1SweBack(Gui *gui) { gotoState(gui, guiSetPu1); }
void actSetPu1SweEnable(Gui *gui) { gotoState(gui, guiSetPu1SweEnabled); }
void actSetPu1SwePeriod(Gui *gui) { gotoState(gui, guiSetPu1SwePeriod); }
void actSetPu1SweNegate(Gui *gui) { gotoState(gui, guiSetPu1SweNegate); }
void actSetPu1SweShift(Gui *gui) { gotoState(gui, guiSetPu1SweShift); }
GuiState guiSetPu1Swe = { { {"Back", &actSetPu1SweBack},
                            {"Enable", &actSetPu1SweEnable},
                            {"Period", &actSetPu1SwePeriod},
                            {"Negate", &actSetPu1SweNegate},
                            {"Shift", &actSetPu1SweShift} } };

void actSetPu2DutyBack(Gui *gui) { gotoState(gui, guiSetPu2); }
void actSetPu2Duty0(Gui *gui) { /*TODO*/; }
void actSetPu2Duty1(Gui *gui) { /*TODO*/; }
void actSetPu2Duty2(Gui *gui) { /*TODO*/; }
void actSetPu2Duty3(Gui *gui) { /*TODO*/; }
GuiState guiSetPu2Duty = { { {"Back", &actSetPu2DutyBack},
                             {"12.5%", &actSetPu2Duty0, true},
                             {"25%", &actSetPu2Duty1},
                             {"50%", &actSetPu2Duty2},
                             {"75%", &actSetPu2Duty3} } };

void actSetPu2EnvBack(Gui *gui) { gotoState(gui, guiSetPu2); }
void actSetPu2EnvVol(Gui *gui) { gotoState(gui, guiSetPu2EnvVol); }
void actSetPu2EnvMode(Gui *gui) { gotoState(gui, guiSetPu2EnvConst); }
GuiState guiSetPu2Env = { { {"Back", &actSetPu2EnvBack},
                            {"Vol/Period", &actSetPu2EnvVol},
                            {"Mode", &actSetPu2EnvMode} } };

void actSetPu2SweBack(Gui *gui) { gotoState(gui, guiSetPu2); }
void actSetPu2SweEnable(Gui *gui) { gotoState(gui, guiSetPu2SweEnabled); }
void actSetPu2SwePeriod(Gui *gui) { gotoState(gui, guiSetPu2SwePeriod); }
void actSetPu2SweNegate(Gui *gui) { gotoState(gui, guiSetPu2SweNegate); }
void actSetPu2SweShift(Gui *gui) { gotoState(gui, guiSetPu2SweShift); }
GuiState guiSetPu2Swe = { { {"Back", &actSetPu2SweBack},
                            {"Enable", &actSetPu2SweEnable},
                            {"Period", &actSetPu2SwePeriod},
                            {"Negate", actSetPu2SweNegate},
                            {"Shift", actSetPu2SweShift} } };

void actSetNoiModeBack(Gui *gui) { gotoState(gui, guiSetNoi); }
void actSetNoiMode0(Gui *gui) { /*TODO*/; }
void actSetNoiMode1(Gui *gui) { /*TODO*/; }
GuiState guiSetNoiMode = { { {"Back", &actSetNoiModeBack},
                             {"Mode0", &actSetNoiMode0, true },
                             {"Mode1", &actSetNoiMode1} } };

void actSetNoiEnvBack(Gui *gui) { gotoState(gui, guiSetNoi); }
void actSetNoiEnvVol(Gui *gui) { gotoState(gui, guiSetNoiEnvVol); }
void actSetNoiEnvMode(Gui *gui) { gotoState(gui, guiSetNoiEnvConst); }
GuiState guiSetNoiEnv = { { {"Back", &actSetNoiEnvBack},
                            {"Vol/Period", &actSetNoiEnvVol},
                            {"Mode", &actSetNoiEnvMode} } };

void actSetPu1EnvVolBack(Gui *gui) { gotoState(gui, guiSetPu1Env); }
void actSetPu1EnvVol0(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol1(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol2(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol3(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol4(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol5(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol6(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol7(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol8(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol9(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol10(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol11(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol12(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol13(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol14(Gui *gui) { /*TODO*/; }
void actSetPu1EnvVol15(Gui *gui) { /*TODO*/; }
GuiState guiSetPu1EnvVol = { { {"Back", &actSetPu1EnvVolBack},
                               {"0", &actSetPu1EnvVol0, true},
                               {"1", &actSetPu1EnvVol1},
                               {"2", &actSetPu1EnvVol2},
                               {"3", &actSetPu1EnvVol3},
                               {"4", &actSetPu1EnvVol4},
                               {"5", &actSetPu1EnvVol5},
                               {"6", &actSetPu1EnvVol6},
                               {"7", &actSetPu1EnvVol7},
                               {"8", &actSetPu1EnvVol8},
                               {"9", &actSetPu1EnvVol9},
                               {"10", &actSetPu1EnvVol10},
                               {"11", &actSetPu1EnvVol11},
                               {"12", &actSetPu1EnvVol12},
                               {"13", &actSetPu1EnvVol13},
                               {"14", &actSetPu1EnvVol14},
                               {"15", &actSetPu1EnvVol15} } };

void actSetPu1EnvConstBack(Gui *gui) { gotoState(gui, guiSetPu1Env); }
void actSetPu1EnvConstUse(Gui *gui) { /*TODO*/; }
void actSetPu1EnvConstSaw(Gui *gui) { /*TODO*/; }
GuiState guiSetPu1EnvConst = { { {"Back", &actSetPu1EnvConstBack},
                                 {"ConstVol", &actSetPu1EnvConstUse, true},
                                 {"Saw", &actSetPu1EnvConstSaw} } };

void actSetPu2EnvVolBack(Gui *gui) { gotoState(gui, guiSetPu1Env); }
void actSetPu2EnvVol0(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol1(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol2(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol3(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol4(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol5(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol6(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol7(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol8(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol9(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol10(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol11(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol12(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol13(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol14(Gui *gui) { /*TODO*/; }
void actSetPu2EnvVol15(Gui *gui) { /*TODO*/; }
GuiState guiSetPu2EnvVol = { { {"Back", &actSetPu2EnvVolBack},
                               {"0", &actSetPu2EnvVol0, true},
                               {"1", &actSetPu2EnvVol1},
                               {"2", &actSetPu2EnvVol2},
                               {"3", &actSetPu2EnvVol3},
                               {"4", &actSetPu2EnvVol4},
                               {"5", &actSetPu2EnvVol5},
                               {"6", &actSetPu2EnvVol6},
                               {"7", &actSetPu2EnvVol7},
                               {"8", &actSetPu2EnvVol8},
                               {"9", &actSetPu2EnvVol9},
                               {"10", &actSetPu2EnvVol10},
                               {"11", &actSetPu2EnvVol11},
                               {"12", &actSetPu2EnvVol12},
                               {"13", &actSetPu2EnvVol13},
                               {"14", &actSetPu2EnvVol14},
                               {"15", &actSetPu2EnvVol15} } };

void actSetPu2EnvConstBack(Gui *gui) { gotoState(gui, guiSetPu2Env); }
void actSetPu2EnvConstUse(Gui *gui) { /*TODO*/; }
void actSetPu2EnvConstSaw(Gui *gui) { /*TODO*/; }
GuiState guiSetPu2EnvConst = { { {"Back", &actSetPu2EnvConstBack},
                                 {"ConstVol", &actSetPu2EnvConstUse, true},
                                 {"Saw", &actSetPu2EnvConstSaw} } };

void actSetNoiEnvVolBack(Gui *gui) { gotoState(gui, guiSetNoiEnv); }
void actSetNoiEnvVol0(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol1(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol2(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol3(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol4(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol5(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol6(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol7(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol8(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol9(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol10(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol11(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol12(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol13(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol14(Gui *gui) { /*TODO*/; }
void actSetNoiEnvVol15(Gui *gui) { /*TODO*/; }
GuiState guiSetNoiEnvVol = { { {"Back", &actSetNoiEnvVolBack},
                               {"0", &actSetNoiEnvVol0, true},
                               {"1", &actSetNoiEnvVol1},
                               {"2", &actSetNoiEnvVol2},
                               {"3", &actSetNoiEnvVol3},
                               {"4", &actSetNoiEnvVol4},
                               {"5", &actSetNoiEnvVol5},
                               {"6", &actSetNoiEnvVol6},
                               {"7", &actSetNoiEnvVol7},
                               {"8", &actSetNoiEnvVol8},
                               {"9", &actSetNoiEnvVol9},
                               {"10", &actSetNoiEnvVol10},
                               {"11", &actSetNoiEnvVol11},
                               {"12", &actSetNoiEnvVol12},
                               {"13", &actSetNoiEnvVol13},
                               {"14", &actSetNoiEnvVol14},
                               {"15", &actSetNoiEnvVol15} } };

void actSetNoiEnvConstBack(Gui *gui) { gotoState(gui, guiSetNoiEnv); }
void actSetNoiEnvConstUse(Gui *gui) { /*TODO*/; }
void actSetNoiEnvConstSaw(Gui *gui) { /*TODO*/; }
GuiState guiSetNoiEnvConst = { { {"Back", &actSetNoiEnvConstBack},
                                 {"ConstVol", &actSetNoiEnvConstUse, true},
                                 {"Saw", &actSetNoiEnvConstSaw} } };

void actSetPu1SweEnabledBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SweEnabledEn(Gui *gui) { /*TODO*/; }
void actSetPu1SweEnabledDis(Gui *gui) { /*TODO*/; }
GuiState guiSetPu1SweEnabled = { { {"Back", &actSetPu1SweEnabledBack},
                                   {"Enabled", &actSetPu1SweEnabledEn},
                                   {"Disabled", &actSetPu1SweEnabledDis, true} } };

void actSetPu1SwePeriodBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SwePeriod0(Gui *gui) { /*TODO*/; }
void actSetPu1SwePeriod1(Gui *gui) { /*TODO*/; }
void actSetPu1SwePeriod2(Gui *gui) { /*TODO*/; }
void actSetPu1SwePeriod3(Gui *gui) { /*TODO*/; }
void actSetPu1SwePeriod4(Gui *gui) { /*TODO*/; }
void actSetPu1SwePeriod5(Gui *gui) { /*TODO*/; }
void actSetPu1SwePeriod6(Gui *gui) { /*TODO*/; }
void actSetPu1SwePeriod7(Gui *gui) { /*TODO*/; }
GuiState guiSetPu1SwePeriod = { { {"Back", &actSetPu1SwePeriodBack},
                                  {"0", &actSetPu1SwePeriod0, true},
                                  {"1", &actSetPu1SwePeriod1},
                                  {"2", &actSetPu1SwePeriod2},
                                  {"3", &actSetPu1SwePeriod3},
                                  {"4", &actSetPu1SwePeriod4},
                                  {"5", &actSetPu1SwePeriod5},
                                  {"6", &actSetPu1SwePeriod6},
                                  {"7", &actSetPu1SwePeriod7} } };

void actSetPu1SweNegateBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SweNegateNo(Gui *gui) { /*TODO*/; }
void actSetPu1SweNegateYes(Gui *gui) { /*TODO*/; }
GuiState guiSetPu1SweNegate = { { {"Back", &actSetPu1SweNegateBack},
                                  {"Positive", &actSetPu1SweNegateNo, true},
                                  {"Negative", &actSetPu1SweNegateYes} } };

void actSetPu1SweShiftBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SweShift0(Gui *gui) { /*TODO*/; }
void actSetPu1SweShift1(Gui *gui) { /*TODO*/; }
void actSetPu1SweShift2(Gui *gui) { /*TODO*/; }
void actSetPu1SweShift3(Gui *gui) { /*TODO*/; }
void actSetPu1SweShift4(Gui *gui) { /*TODO*/; }
void actSetPu1SweShift5(Gui *gui) { /*TODO*/; }
void actSetPu1SweShift6(Gui *gui) { /*TODO*/; }
void actSetPu1SweShift7(Gui *gui) { /*TODO*/; }
GuiState guiSetPu1SweShift = { { {"Back", &actSetPu1SweShiftBack},
                                 {"0", &actSetPu1SweShift0, true},
                                 {"1", &actSetPu1SweShift1},
                                 {"2", &actSetPu1SweShift2},
                                 {"3", &actSetPu1SweShift3},
                                 {"4", &actSetPu1SweShift4},
                                 {"5", &actSetPu1SweShift5},
                                 {"6", &actSetPu1SweShift6},
                                 {"7", &actSetPu1SweShift7} } };


void actSetPu2SweEnabledBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SweEnabledEn(Gui *gui) { /*TODO*/; }
void actSetPu2SweEnabledDis(Gui *gui) { /*TODO*/; }
GuiState guiSetPu2SweEnabled = { { {"Back", &actSetPu2SweEnabledBack},
                                   {"Enabled", &actSetPu2SweEnabledEn},
                                   {"Disabled", &actSetPu2SweEnabledDis, true} } };

void actSetPu2SwePeriodBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SwePeriod0(Gui *gui) { /*TODO*/; }
void actSetPu2SwePeriod1(Gui *gui) { /*TODO*/; }
void actSetPu2SwePeriod2(Gui *gui) { /*TODO*/; }
void actSetPu2SwePeriod3(Gui *gui) { /*TODO*/; }
void actSetPu2SwePeriod4(Gui *gui) { /*TODO*/; }
void actSetPu2SwePeriod5(Gui *gui) { /*TODO*/; }
void actSetPu2SwePeriod6(Gui *gui) { /*TODO*/; }
void actSetPu2SwePeriod7(Gui *gui) { /*TODO*/; }
GuiState guiSetPu2SwePeriod = { { {"Back", &actSetPu2SwePeriodBack},
                                  {"0", &actSetPu2SwePeriod0, true},
                                  {"1", &actSetPu2SwePeriod1},
                                  {"2", &actSetPu2SwePeriod2},
                                  {"3", &actSetPu2SwePeriod3},
                                  {"4", &actSetPu2SwePeriod4},
                                  {"5", &actSetPu2SwePeriod5},
                                  {"6", &actSetPu2SwePeriod6},
                                  {"7", &actSetPu2SwePeriod7} } };

void actSetPu2SweNegateBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SweNegateNo(Gui *gui) { /*TODO*/; }
void actSetPu2SweNegateYes(Gui *gui) { /*TODO*/; }
GuiState guiSetPu2SweNegate = { { {"Back", &actSetPu2SweNegateBack},
                                  {"Positive", &actSetPu2SweNegateNo, true},
                                  {"Negative", &actSetPu2SweNegateYes} } };

void actSetPu2SweShiftBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SweShift0(Gui *gui) { /*TODO*/; }
void actSetPu2SweShift1(Gui *gui) { /*TODO*/; }
void actSetPu2SweShift2(Gui *gui) { /*TODO*/; }
void actSetPu2SweShift3(Gui *gui) { /*TODO*/; }
void actSetPu2SweShift4(Gui *gui) { /*TODO*/; }
void actSetPu2SweShift5(Gui *gui) { /*TODO*/; }
void actSetPu2SweShift6(Gui *gui) { /*TODO*/; }
void actSetPu2SweShift7(Gui *gui) { /*TODO*/; }
GuiState guiSetPu2SweShift = { { {"Back", &actSetPu2SweShiftBack},
                                 {"0", &actSetPu2SweShift0, true},
                                 {"1", &actSetPu2SweShift1},
                                 {"2", &actSetPu2SweShift2},
                                 {"3", &actSetPu2SweShift3},
                                 {"4", &actSetPu2SweShift4},
                                 {"5", &actSetPu2SweShift5},
                                 {"6", &actSetPu2SweShift6},
                                 {"7", &actSetPu2SweShift7} } };

Gui gui;

volatile bool even;
volatile float cyclesToProcess = 0;
int32 index = 0;
Uint32 wait = 0;
int32 sample = 0;

void displayGuiState(Gui *gui) {
    if(!gui->unchanged) {
        clear_screen();
        if(gui->bottomCursor) {
            write_string("  ");
        }
        else {
            write_string("> ");
        }
        write_string(gui->state.choices[gui->choice].name);
        if (gui->state.choices[gui->choice].selected) {
            write_string(" *");
        }
        if (gui->choice < 4 && gui->state.choices[gui->choice+1].name[0] != '\0') {
            shift_screen();
            if(gui->bottomCursor) {
                write_string("> ");
            }
            else {
                write_string("  ");
            }
            write_string(gui->state.choices[gui->choice+1].name);
            if (gui->state.choices[gui->choice+1].selected) {
                write_string(" *");
            }
        }
    }
    gui->unchanged = true;
}

int main(void) {

    // inits
    InitSysCtrl();
    setup_lcd();
    init_io_gpio(); //TODO: JUST FOR TEST
    InitBigBangedCodecSPI();
    InitAIC23(SR48);
    InitMcBSPb();
    Interrupt_initModule();
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_enableMaster();


    // TODO: TESTING!
    EALLOW;
    GpioCtrlRegs.GPCMUX2.bit.GPIO95 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO95 = 1;
    EDIS;



    struct Apu apu = {};
    initApu(&apu);

    /*
    const char square[] = {0xB4, 0x00, 0x30, 0xB4, 0x01, 0x08, 0xB4, 0x02, 0x00,
                           0xB4, 0x03, 0x00, 0xB4, 0x04, 0x30, 0xB4, 0x05, 0x08,
                           0xB4, 0x06, 0x00, 0xB4, 0x07, 0x00, 0xB4, 0x08, 0x80,
                           0xB4, 0x09, 0x00, 0xB4, 0x0A, 0x00, 0xB4, 0x0B, 0x00,
                           0xB4, 0x0C, 0x30, 0xB4, 0x0D, 0x00, 0xB4, 0x0E, 0x00,
                           0xB4, 0x0F, 0x00, 0xB4, 0x10, 0x00, 0xB4, 0x11, 0x00,
                           0xB4, 0x12, 0x00, 0xB4, 0x13, 0x00, 0xB4, 0x15, 0x0F,
                           0xB4, 0x17, 0x40,
                           0xB4, 0x02, 0x79, 0xB4, 0x03, 0x02, 0xB4, 0x00, 0xBF, 0x62, 0x66};
                           */

    const char *file = intro;

    gui.state = guiMain;

    while (1) {


        displayGuiState(&gui);

        if (!GpioDataRegs.GPADAT.bit.GPIO16 && !gui.upPressed) {
            // Scroll Up
            gui.upPressed = 0xFFFF;
            if (gui.choice+gui.bottomCursor > 0) {
                if (gui.bottomCursor) {
                    gui.bottomCursor = false;
                }
                else {
                    gui.choice--;
                }
                gui.unchanged = false;
            }
        }
        else if (!GpioDataRegs.GPADAT.bit.GPIO15 && !gui.selPressed) {
            // Select
            gui.selPressed = 0xFFFF;
            gui.state.choices[gui.choice+gui.bottomCursor].select(&gui);
        }
        else if (!GpioDataRegs.GPADAT.bit.GPIO14 && !gui.downPressed) {
            // Scroll Down
            gui.downPressed = 0xFFFF;
            if (gui.choice+gui.bottomCursor < 4 && gui.state.choices[gui.choice+gui.bottomCursor+1].name[0] != '\0') {
                if (gui.bottomCursor) {
                    gui.choice++;
                }
                else {
                    gui.bottomCursor = true;
                }
                gui.unchanged = false;
            }
        }

        if (GpioDataRegs.GPADAT.bit.GPIO16 && gui.upPressed) {
            gui.upPressed--;
        }
        if (GpioDataRegs.GPADAT.bit.GPIO15 && gui.selPressed) {
            gui.selPressed--;
        }
        if (GpioDataRegs.GPADAT.bit.GPIO14 && gui.downPressed) {
            gui.downPressed--;
        }


        if (cyclesToProcess > 0) {
            // PLAY FROM KEYBOARD MODE
            //get from gpio
            //if different, store to array


            // PLAY FROM FILE MODE
            if (wait == 0) {
                while (wait == 0) {
                    Uint16 command = file[index++];
                    if (command == 0xB4) {
                        Uint16 address = file[index++];
                        Uint16 data = file[index++];
                        writeRegister(&apu, address, data);
                    }
                    else if (command & 0xF0 == 0x70) {
                        wait = (command & 0x0F) + 1;
                    }
                    else if (command == 0x61) {
                        Uint16 waitLo = file[index++];
                        Uint16 waitHi = file[index++];
                        wait = waitHi << 8 | waitLo;
                    }
                    else if (command == 0x62) {
                        wait = 735;
                    }
                    else if (command == 0x66) {
                        // END OF FILE;
                        initApu(&apu);
                        wait=1;
                        index--;
                    }
                 }
            }
            else {
                wait--;
            }
            sample = processCycles(&apu, cyclesToProcess);
            cyclesToProcess = 0;
        }

    }

}

interrupt void mcBspRx() {
    if(even) {
        even = false;
    }
    else {
        even = true;
        cyclesToProcess += cyclesPerSample;
    }
    // dummy read
    Uint16 lo = McbspbRegs.DRR1.all;
    Uint16 hi = McbspbRegs.DRR2.all;
    // output the current sample
    McbspbRegs.DXR1.all = sample & 0xFFFF;
    McbspbRegs.DXR2.all = (sample >> 16) & 0xFFFF;
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
