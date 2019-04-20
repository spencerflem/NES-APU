#include <F28x_Project.h>
#include "intro.h"
#include "Gui.h"
#include "Apu.h"
#include "lcd.h"
#include "io.h"

#pragma DATA_SECTION(pingBuff, "pingloop");
char pingBuff[0x6000] = { 0 };
#pragma DATA_SECTION(pongBuff, "pongloop");
char pongBuff[0x6000] = { 0 };

void initGuiGpio() {
    setup_lcd();
    init_io_gpio();
}

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
        if (gui->choice < 17 && gui->state.choices[gui->choice+1].name[0] != '\0') {
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

void updateGuiState(Gui *gui) {
    if (!GpioDataRegs.GPADAT.bit.GPIO16 && !gui->upPressed) {
        // Scroll Up
        gui->upPressed = 0xFFFF;
        if (gui->choice+gui->bottomCursor > 0) {
            if (gui->bottomCursor) {
                gui->bottomCursor = false;
            }
            else {
                gui->choice--;
            }
            gui->unchanged = false;
        }
    }
    else if (!GpioDataRegs.GPADAT.bit.GPIO15 && !gui->selPressed) {
        // Select
        gui->selPressed = 0xFFFF;
        gui->state.choices[gui->choice+gui->bottomCursor].select(gui);
    }
    else if (!GpioDataRegs.GPADAT.bit.GPIO14 && !gui->downPressed) {
        // Scroll Down
        gui->downPressed = 0xFFFF;
        if (gui->choice+gui->bottomCursor < 17 && gui->state.choices[gui->choice+gui->bottomCursor+1].name[0] != '\0') {
            if (gui->bottomCursor) {
                gui->choice++;
            }
            else {
                gui->bottomCursor = true;
            }
            gui->unchanged = false;
        }
    }

    if (GpioDataRegs.GPADAT.bit.GPIO16 && gui->upPressed) {
        gui->upPressed--;
    }
    if (GpioDataRegs.GPADAT.bit.GPIO15 && gui->selPressed) {
        gui->selPressed--;
    }
    if (GpioDataRegs.GPADAT.bit.GPIO14 && gui->downPressed) {
        gui->downPressed--;
    }
}

#pragma DATA_SECTION(guiMain, "gui");
static const GuiState guiMain;
#pragma DATA_SECTION(guiFile, "gui");
static const GuiState guiFile;
#pragma DATA_SECTION(guiLoop, "gui");
static const GuiState guiLoop;
#pragma DATA_SECTION(guiLoopPlay, "gui");
static const GuiState guiLoopPlay;
#pragma DATA_SECTION(guiRecord, "gui");
static const GuiState guiRecord;
#pragma DATA_SECTION(guiPulse1, "gui");
static const GuiState guiPulse1;
#pragma DATA_SECTION(guiPulse2, "gui");
static const GuiState guiPulse2;
#pragma DATA_SECTION(guiTriangle, "gui");
static const GuiState guiTriangle;
#pragma DATA_SECTION(guiNoise, "gui");
static const GuiState guiNoise;
#pragma DATA_SECTION(guiRecPu1, "gui");
static const GuiState guiRecPu1;
#pragma DATA_SECTION(guiRecPu2, "gui");
static const GuiState guiRecPu2;
#pragma DATA_SECTION(guiRecTri, "gui");
static const GuiState guiRecTri;
#pragma DATA_SECTION(guiRecNoi, "gui");
static const GuiState guiRecNoi;
#pragma DATA_SECTION(guiSetPu1, "gui");
static const GuiState guiSetPu1;
#pragma DATA_SECTION(guiSetPu2, "gui");
static const GuiState guiSetPu2;
#pragma DATA_SECTION(guiSetNoi, "gui");
static const GuiState guiSetNoi;
#pragma DATA_SECTION(guiSetPu1Duty, "gui");
static const GuiState guiSetPu1Duty;
#pragma DATA_SECTION(guiSetPu1Env, "gui");
static const GuiState guiSetPu1Env;
#pragma DATA_SECTION(guiSetPu1Swe, "gui");
static const GuiState guiSetPu1Swe;
#pragma DATA_SECTION(guiSetPu2Duty, "gui");
static const GuiState guiSetPu2Duty;
#pragma DATA_SECTION(guiSetPu2Env, "gui");
static const GuiState guiSetPu2Env;
#pragma DATA_SECTION(guiSetPu2Swe, "gui");
static const GuiState guiSetPu2Swe;
#pragma DATA_SECTION(guiSetNoiMode, "gui");
static const GuiState guiSetNoiMode;
#pragma DATA_SECTION(guiSetNoiEnv, "gui");
static const GuiState guiSetNoiEnv;
#pragma DATA_SECTION(guiSetPu1EnvVol, "gui");
static const GuiState guiSetPu1EnvVol;
#pragma DATA_SECTION(guiSetPu1EnvConst, "gui");
static const GuiState guiSetPu1EnvConst;
#pragma DATA_SECTION(guiSetPu2EnvVol, "gui");
static const GuiState guiSetPu2EnvVol;
#pragma DATA_SECTION(guiSetPu2EnvConst, "gui");
static const GuiState guiSetPu2EnvConst;
#pragma DATA_SECTION(guiSetNoiEnvVol, "gui");
static const GuiState guiSetNoiEnvVol;
#pragma DATA_SECTION(guiSetNoiEnvConst, "gui");
static const GuiState guiSetNoiEnvConst;
#pragma DATA_SECTION(guiSetPu1SweEnabled, "gui");
static const GuiState guiSetPu1SweEnabled;
#pragma DATA_SECTION(guiSetPu1SwePeriod, "gui");
static const GuiState guiSetPu1SwePeriod;
#pragma DATA_SECTION(guiSetPu1SweNegate, "gui");
static const GuiState guiSetPu1SweNegate;
#pragma DATA_SECTION(guiSetPu1SweShift, "gui");
static const GuiState guiSetPu1SweShift;
#pragma DATA_SECTION(guiSetPu2SweEnabled, "gui");
static const GuiState guiSetPu2SweEnabled;
#pragma DATA_SECTION(guiSetPu2SwePeriod, "gui");
static const GuiState guiSetPu2SwePeriod;
#pragma DATA_SECTION(guiSetPu2SweNegate, "gui");
static const GuiState guiSetPu2SweNegate;
#pragma DATA_SECTION(guiSetPu2SweShift, "gui");
static const GuiState guiSetPu2SweShift;

void resetLoop(Gui *gui) {
    // TODO! CLEAR PINGBUFF!!!
    gui->oldBuff = pingBuff;
    gui->newBuff = pongBuff;
}

void initGui(Gui *gui, Apu *apu) {
    gui->state = guiMain;
    gui->apu = apu;
    resetLoop(gui);
}

void gotoState(Gui *gui, GuiState state) {
    gui->state = state;
    gui->bottomCursor = false;
    gui->choice = 0;
    gui->unchanged = 0;
}

void actFile(Gui *gui) {
    gotoState(gui, guiFile);
    //play the file
    gui->wait = 0;
    gui->index = 0;
    gui->file = intro;
    gui->playFile = true;
    gui->fileLoopIndex = introLoopIndex;
}
void actLoop(Gui *gui) { gotoState(gui, guiLoop); }
static const GuiState guiMain = { { {"File", &actFile},
                       {"Loop", &actLoop} } };


void actFileBack(Gui *gui) {
    gotoState(gui, guiMain);
    gui->playFile = false;
}
static const GuiState guiFile = { { {"Back", actFileBack} } };

void actLoopBack(Gui *gui) { gotoState(gui, guiMain); }
void actLoopPlay(Gui *gui) { gotoState(gui, guiLoopPlay); }
void actRecord(Gui *gui) { gotoState(gui, guiRecord); }
void actReset(Gui *gui) { resetLoop(gui); }
static const GuiState guiLoop = { { {"Back", &actLoopBack},
                       {"Play", &actLoopPlay},
                       {"Record", &actRecord},
                       {"Reset", &actReset} } };

void actLoopPlayBack(Gui *gui) { gotoState(gui, guiLoop); }
static const GuiState guiLoopPlay = { { {"Back", &actLoopPlayBack} } };

void actRecordBack(Gui *gui) { gotoState(gui, guiLoop); }
void actPulse1(Gui *gui) { gotoState(gui, guiPulse1); }
void actPulse2(Gui *gui) { gotoState(gui, guiPulse2); }
void actTriangle(Gui *gui) { gotoState(gui, guiTriangle); }
void actNoise(Gui *gui) { gotoState(gui, guiNoise); }
static const GuiState guiRecord = { { { "Back", &actRecordBack},
                         {"Pulse1", &actPulse1},
                         {"Pulse2", &actPulse2},
                         {"Triangle", &actTriangle},
                         {"Noise", &actNoise} } };

void actPulse1Back(Gui *gui) { gotoState(gui, guiRecord); }
void actPulse1Record(Gui *gui) {
    gotoState(gui, guiRecPu1);
    gui->recordLoop = 1;
}
void actPulse1Settings(Gui *gui) { gotoState(gui, guiSetPu1); }
static const GuiState guiPulse1 = { { {"Back", &actPulse1Back},
                         {"Record", &actPulse1Record},
                         {"Settings", &actPulse1Settings} } };

void actPulse2Back(Gui *gui) { gotoState(gui, guiRecord); }
void actPulse2Record(Gui *gui) {
    gotoState(gui, guiRecPu2);
    gui->recordLoop = 2;
}
void actPulse2Settings(Gui *gui) { gotoState(gui, guiSetPu2); }
static const GuiState guiPulse2 = { { {"Back", &actPulse2Back},
                         {"Record", &actPulse2Record},
                         {"Settings", &actPulse2Settings} } };

void actTriangleBack(Gui *gui) { gotoState(gui, guiRecord); }
void actTriangleRecord(Gui *gui) {
    gotoState(gui, guiRecTri);
    gui->recordLoop = 3;
}
static const GuiState guiTriangle = { { {"Back", &actTriangleBack},
                           {"Record", &actTriangleRecord} } };

void actNoiseBack(Gui *gui) { gotoState(gui, guiRecord); }
void actNoiseRecord(Gui *gui) {
    gotoState(gui, guiRecNoi);
    gui->recordLoop = 4;
}
void actNoiseSettings(Gui *gui) { gotoState(gui, guiSetNoi); }
static const GuiState guiNoise = { { {"Back", &actNoiseBack},
                        {"Record", &actNoiseRecord},
                        {"Settings", &actNoiseSettings}  } };

void actRecPu1Save(Gui *gui) {
    gotoState(gui, guiPulse1);
    char *tempBuff = gui->oldBuff;
    gui->oldBuff = gui->newBuff;
    gui->newBuff = tempBuff;
}
void actRecPu1Disc(Gui *gui) { gotoState(gui, guiPulse1); }
static const GuiState guiRecPu1 = { { {"Save", &actRecPu1Save},
                                      {"Discard", &actRecPu1Disc} } };

void actRecPu2Save(Gui *gui) {
    gotoState(gui, guiPulse2);
    char *tempBuff = gui->oldBuff;
    gui->oldBuff = gui->newBuff;
    gui->newBuff = tempBuff;
}
void actRecPu2Disc(Gui *gui) { gotoState(gui, guiPulse2); }
static const GuiState guiRecPu2 = { { {"Save", &actRecPu2Save},
                                      {"Discard", &actRecPu2Disc} } };

void actRecTriSave(Gui *gui) {
    gotoState(gui, guiTriangle);
    char *tempBuff = gui->oldBuff;
    gui->oldBuff = gui->newBuff;
    gui->newBuff = tempBuff;
}
void actRecTriDisc(Gui *gui) { gotoState(gui, guiTriangle); }
static const GuiState guiRecTri = { { {"Save", &actRecTriSave},
                                      {"Discard", &actRecTriDisc} } };

void actRecNoiSave(Gui *gui) {
    gotoState(gui, guiNoise);
    char *tempBuff = gui->oldBuff;
    gui->oldBuff = gui->newBuff;
    gui->newBuff = tempBuff;
}
void actRecNoiDisc(Gui *gui) { gotoState(gui, guiNoise); }
static const GuiState guiRecNoi = { { {"Save", &actRecNoiSave},
                                      {"Discard", &actRecNoiDisc} } };

void actSetPu1Back(Gui *gui) { gotoState(gui, guiPulse1); }
void actSetPu1Duty(Gui *gui) { gotoState(gui, guiSetPu1Duty); }
void actSetPu1Env(Gui *gui) { gotoState(gui, guiSetPu1Env); }
void actSetPu1Swe(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
static const GuiState guiSetPu1 = { { {"Back", &actSetPu1Back},
                         {"Duty", &actSetPu1Duty},
                         {"Envelope", &actSetPu1Env},
                         {"Sweep", &actSetPu1Swe} } };

void actSetPu2Back(Gui *gui) { gotoState(gui, guiPulse2); }
void actSetPu2Duty(Gui *gui) { gotoState(gui, guiSetPu2Duty); }
void actSetPu2Env(Gui *gui) { gotoState(gui, guiSetPu2Env); }
void actSetPu2Swe(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
static const GuiState guiSetPu2 = { { {"Back", &actSetPu2Back},
                         {"Duty", &actSetPu2Duty},
                         {"Envelope", &actSetPu2Env},
                         {"Sweep", &actSetPu2Swe} } };

void actSetNoiBack(Gui *gui) { gotoState(gui, guiNoise); }
void actSetNoiMode(Gui *gui) { gotoState(gui, guiSetNoiMode); }
void actSetNoiEnv(Gui *gui) { gotoState(gui, guiSetNoiEnv); }
static const GuiState guiSetNoi = { { {"Back", &actSetNoiBack},
                         {"Mode", &actSetNoiMode},
                         {"Envelope", &actSetNoiEnv} } };

void actSetPu1DutyBack(Gui *gui) { gotoState(gui, guiSetPu1); }
void actSetPu1Duty0(Gui *gui) { setPulse1Duty(gui->apu, 0); }
void actSetPu1Duty1(Gui *gui) { setPulse1Duty(gui->apu, 1); }
void actSetPu1Duty2(Gui *gui) { setPulse1Duty(gui->apu, 2); }
void actSetPu1Duty3(Gui *gui) { setPulse1Duty(gui->apu, 3); }
static const GuiState guiSetPu1Duty = { { {"Back", &actSetPu1DutyBack},
                             {"12.5%", &actSetPu1Duty0, true},
                             {"25%", &actSetPu1Duty1},
                             {"50%", &actSetPu1Duty2},
                             {"75%", &actSetPu1Duty3} } };

void actSetPu1EnvBack(Gui *gui) { gotoState(gui, guiSetPu1); }
void actSetPu1EnvVol(Gui *gui) { gotoState(gui, guiSetPu1EnvVol); }
void actSetPu1EnvMode(Gui *gui) { gotoState(gui, guiSetPu1EnvConst); }
static const GuiState guiSetPu1Env = { { {"Back", &actSetPu1EnvBack},
                            {"Vol/Period", &actSetPu1EnvVol},
                            {"Mode", &actSetPu1EnvMode} } };

void actSetPu1SweBack(Gui *gui) { gotoState(gui, guiSetPu1); }
void actSetPu1SweEnable(Gui *gui) { gotoState(gui, guiSetPu1SweEnabled); }
void actSetPu1SwePeriod(Gui *gui) { gotoState(gui, guiSetPu1SwePeriod); }
void actSetPu1SweNegate(Gui *gui) { gotoState(gui, guiSetPu1SweNegate); }
void actSetPu1SweShift(Gui *gui) { gotoState(gui, guiSetPu1SweShift); }
static const GuiState guiSetPu1Swe = { { {"Back", &actSetPu1SweBack},
                            {"Enable", &actSetPu1SweEnable},
                            {"Period", &actSetPu1SwePeriod},
                            {"Negate", &actSetPu1SweNegate},
                            {"Shift", &actSetPu1SweShift} } };

void actSetPu2DutyBack(Gui *gui) { gotoState(gui, guiSetPu2); }
void actSetPu2Duty0(Gui *gui) { setPulse2Duty(gui->apu, 0); }
void actSetPu2Duty1(Gui *gui) { setPulse2Duty(gui->apu, 1); }
void actSetPu2Duty2(Gui *gui) { setPulse2Duty(gui->apu, 2); }
void actSetPu2Duty3(Gui *gui) { setPulse2Duty(gui->apu, 3); }
static const GuiState guiSetPu2Duty = { { {"Back", &actSetPu2DutyBack},
                             {"12.5%", &actSetPu2Duty0, true},
                             {"25%", &actSetPu2Duty1},
                             {"50%", &actSetPu2Duty2},
                             {"75%", &actSetPu2Duty3} } };

void actSetPu2EnvBack(Gui *gui) { gotoState(gui, guiSetPu2); }
void actSetPu2EnvVol(Gui *gui) { gotoState(gui, guiSetPu2EnvVol); }
void actSetPu2EnvMode(Gui *gui) { gotoState(gui, guiSetPu2EnvConst); }
static const GuiState guiSetPu2Env = { { {"Back", &actSetPu2EnvBack},
                            {"Vol/Period", &actSetPu2EnvVol},
                            {"Mode", &actSetPu2EnvMode} } };

void actSetPu2SweBack(Gui *gui) { gotoState(gui, guiSetPu2); }
void actSetPu2SweEnable(Gui *gui) { gotoState(gui, guiSetPu2SweEnabled); }
void actSetPu2SwePeriod(Gui *gui) { gotoState(gui, guiSetPu2SwePeriod); }
void actSetPu2SweNegate(Gui *gui) { gotoState(gui, guiSetPu2SweNegate); }
void actSetPu2SweShift(Gui *gui) { gotoState(gui, guiSetPu2SweShift); }
static const GuiState guiSetPu2Swe = { { {"Back", &actSetPu2SweBack},
                            {"Enable", &actSetPu2SweEnable},
                            {"Period", &actSetPu2SwePeriod},
                            {"Negate", actSetPu2SweNegate},
                            {"Shift", actSetPu2SweShift} } };

void actSetNoiModeBack(Gui *gui) { gotoState(gui, guiSetNoi); }
void actSetNoiMode0(Gui *gui) { setNoiseMode(gui->apu, false); }
void actSetNoiMode1(Gui *gui) { setNoiseMode(gui->apu, true); }
static const GuiState guiSetNoiMode = { { {"Back", &actSetNoiModeBack},
                             {"Mode0", &actSetNoiMode0, true },
                             {"Mode1", &actSetNoiMode1} } };

void actSetNoiEnvBack(Gui *gui) { gotoState(gui, guiSetNoi); }
void actSetNoiEnvVol(Gui *gui) { gotoState(gui, guiSetNoiEnvVol); }
void actSetNoiEnvMode(Gui *gui) { gotoState(gui, guiSetNoiEnvConst); }
static const GuiState guiSetNoiEnv = { { {"Back", &actSetNoiEnvBack},
                            {"Vol/Period", &actSetNoiEnvVol},
                            {"Mode", &actSetNoiEnvMode} } };

void actSetPu1EnvVolBack(Gui *gui) { gotoState(gui, guiSetPu1Env); }
void actSetPu1EnvVol0(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 0); }
void actSetPu1EnvVol1(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 1); }
void actSetPu1EnvVol2(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 2); }
void actSetPu1EnvVol3(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 3); }
void actSetPu1EnvVol4(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 4); }
void actSetPu1EnvVol5(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 5); }
void actSetPu1EnvVol6(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 6); }
void actSetPu1EnvVol7(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 7); }
void actSetPu1EnvVol8(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 8); }
void actSetPu1EnvVol9(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 9); }
void actSetPu1EnvVol10(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 10); }
void actSetPu1EnvVol11(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 11); }
void actSetPu1EnvVol12(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 12); }
void actSetPu1EnvVol13(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 13); }
void actSetPu1EnvVol14(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 14); }
void actSetPu1EnvVol15(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 15); }
static const GuiState guiSetPu1EnvVol = { { {"Back", &actSetPu1EnvVolBack},
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
void actSetPu1EnvConstUse(Gui *gui) { setPulse1EnvelopeUseConstantVolume(gui->apu, true); }
void actSetPu1EnvConstSaw(Gui *gui) { setPulse1EnvelopeUseConstantVolume(gui->apu, false); }
static const GuiState guiSetPu1EnvConst = { { {"Back", &actSetPu1EnvConstBack},
                                 {"ConstVol", &actSetPu1EnvConstUse, true},
                                 {"Saw", &actSetPu1EnvConstSaw} } };

void actSetPu2EnvVolBack(Gui *gui) { gotoState(gui, guiSetPu1Env); }
void actSetPu2EnvVol0(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 0); }
void actSetPu2EnvVol1(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 1); }
void actSetPu2EnvVol2(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 2); }
void actSetPu2EnvVol3(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 3); }
void actSetPu2EnvVol4(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 4); }
void actSetPu2EnvVol5(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 5); }
void actSetPu2EnvVol6(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 6); }
void actSetPu2EnvVol7(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 7); }
void actSetPu2EnvVol8(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 8); }
void actSetPu2EnvVol9(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 9); }
void actSetPu2EnvVol10(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 10); }
void actSetPu2EnvVol11(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 11); }
void actSetPu2EnvVol12(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 12); }
void actSetPu2EnvVol13(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 13); }
void actSetPu2EnvVol14(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 14); }
void actSetPu2EnvVol15(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 15); }
static const GuiState guiSetPu2EnvVol = { { {"Back", &actSetPu2EnvVolBack},
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
void actSetPu2EnvConstUse(Gui *gui) { setPulse2EnvelopeUseConstantVolume(gui->apu, true); }
void actSetPu2EnvConstSaw(Gui *gui) { setPulse2EnvelopeUseConstantVolume(gui->apu, false); }
static const GuiState guiSetPu2EnvConst = { { {"Back", &actSetPu2EnvConstBack},
                                 {"ConstVol", &actSetPu2EnvConstUse, true},
                                 {"Saw", &actSetPu2EnvConstSaw} } };

void actSetNoiEnvVolBack(Gui *gui) { gotoState(gui, guiSetNoiEnv); }
void actSetNoiEnvVol0(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 0); }
void actSetNoiEnvVol1(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 1); }
void actSetNoiEnvVol2(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 2); }
void actSetNoiEnvVol3(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 3); }
void actSetNoiEnvVol4(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 4); }
void actSetNoiEnvVol5(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 5); }
void actSetNoiEnvVol6(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 6); }
void actSetNoiEnvVol7(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 7); }
void actSetNoiEnvVol8(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 8); }
void actSetNoiEnvVol9(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 9); }
void actSetNoiEnvVol10(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 10); }
void actSetNoiEnvVol11(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 11); }
void actSetNoiEnvVol12(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 12); }
void actSetNoiEnvVol13(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 13); }
void actSetNoiEnvVol14(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 14); }
void actSetNoiEnvVol15(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 15); }
static const GuiState guiSetNoiEnvVol = { { {"Back", &actSetNoiEnvVolBack},
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
void actSetNoiEnvConstUse(Gui *gui) { setNoiseEnvelopeUseConstantVolume(gui->apu, true); }
void actSetNoiEnvConstSaw(Gui *gui) { setNoiseEnvelopeUseConstantVolume(gui->apu, false); }
static const GuiState guiSetNoiEnvConst = { { {"Back", &actSetNoiEnvConstBack},
                                 {"ConstVol", &actSetNoiEnvConstUse, true},
                                 {"Saw", &actSetNoiEnvConstSaw} } };

void actSetPu1SweEnabledBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SweEnabledEn(Gui *gui) { setPulse1SweepEnabled(gui->apu, true); }
void actSetPu1SweEnabledDis(Gui *gui) { setPulse1SweepEnabled(gui->apu, false); }
static const GuiState guiSetPu1SweEnabled = { { {"Back", &actSetPu1SweEnabledBack},
                                   {"Enabled", &actSetPu1SweEnabledEn},
                                   {"Disabled", &actSetPu1SweEnabledDis, true} } };

void actSetPu1SwePeriodBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SwePeriod0(Gui *gui) { setPulse1SweepPeriod(gui->apu, 0); }
void actSetPu1SwePeriod1(Gui *gui) { setPulse1SweepPeriod(gui->apu, 1); }
void actSetPu1SwePeriod2(Gui *gui) { setPulse1SweepPeriod(gui->apu, 2); }
void actSetPu1SwePeriod3(Gui *gui) { setPulse1SweepPeriod(gui->apu, 3); }
void actSetPu1SwePeriod4(Gui *gui) { setPulse1SweepPeriod(gui->apu, 4); }
void actSetPu1SwePeriod5(Gui *gui) { setPulse1SweepPeriod(gui->apu, 5); }
void actSetPu1SwePeriod6(Gui *gui) { setPulse1SweepPeriod(gui->apu, 6); }
void actSetPu1SwePeriod7(Gui *gui) { setPulse1SweepPeriod(gui->apu, 7); }
static const GuiState guiSetPu1SwePeriod = { { {"Back", &actSetPu1SwePeriodBack},
                                  {"0", &actSetPu1SwePeriod0, true},
                                  {"1", &actSetPu1SwePeriod1},
                                  {"2", &actSetPu1SwePeriod2},
                                  {"3", &actSetPu1SwePeriod3},
                                  {"4", &actSetPu1SwePeriod4},
                                  {"5", &actSetPu1SwePeriod5},
                                  {"6", &actSetPu1SwePeriod6},
                                  {"7", &actSetPu1SwePeriod7} } };

void actSetPu1SweNegateBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SweNegateNo(Gui *gui) { setPulse1SweepNegate(gui->apu, false); }
void actSetPu1SweNegateYes(Gui *gui) { setPulse1SweepNegate(gui->apu, true); }
static const GuiState guiSetPu1SweNegate = { { {"Back", &actSetPu1SweNegateBack},
                                  {"Positive", &actSetPu1SweNegateNo, true},
                                  {"Negative", &actSetPu1SweNegateYes} } };

void actSetPu1SweShiftBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SweShift0(Gui *gui) { setPulse1SweepShift(gui->apu, 0); }
void actSetPu1SweShift1(Gui *gui) { setPulse1SweepShift(gui->apu, 1); }
void actSetPu1SweShift2(Gui *gui) { setPulse1SweepShift(gui->apu, 2); }
void actSetPu1SweShift3(Gui *gui) { setPulse1SweepShift(gui->apu, 3); }
void actSetPu1SweShift4(Gui *gui) { setPulse1SweepShift(gui->apu, 4); }
void actSetPu1SweShift5(Gui *gui) { setPulse1SweepShift(gui->apu, 5); }
void actSetPu1SweShift6(Gui *gui) { setPulse1SweepShift(gui->apu, 6); }
void actSetPu1SweShift7(Gui *gui) { setPulse1SweepShift(gui->apu, 7); }
static const GuiState guiSetPu1SweShift = { { {"Back", &actSetPu1SweShiftBack},
                                 {"0", &actSetPu1SweShift0, true},
                                 {"1", &actSetPu1SweShift1},
                                 {"2", &actSetPu1SweShift2},
                                 {"3", &actSetPu1SweShift3},
                                 {"4", &actSetPu1SweShift4},
                                 {"5", &actSetPu1SweShift5},
                                 {"6", &actSetPu1SweShift6},
                                 {"7", &actSetPu1SweShift7} } };


void actSetPu2SweEnabledBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SweEnabledEn(Gui *gui) { setPulse2SweepEnabled(gui->apu, true); }
void actSetPu2SweEnabledDis(Gui *gui) { setPulse2SweepEnabled(gui->apu, false); }
static const GuiState guiSetPu2SweEnabled = { { {"Back", &actSetPu2SweEnabledBack},
                                   {"Enabled", &actSetPu2SweEnabledEn},
                                   {"Disabled", &actSetPu2SweEnabledDis, true} } };

void actSetPu2SwePeriodBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SwePeriod0(Gui *gui) { setPulse2SweepPeriod(gui->apu, 0); }
void actSetPu2SwePeriod1(Gui *gui) { setPulse2SweepPeriod(gui->apu, 1); }
void actSetPu2SwePeriod2(Gui *gui) { setPulse2SweepPeriod(gui->apu, 2); }
void actSetPu2SwePeriod3(Gui *gui) { setPulse2SweepPeriod(gui->apu, 3); }
void actSetPu2SwePeriod4(Gui *gui) { setPulse2SweepPeriod(gui->apu, 4); }
void actSetPu2SwePeriod5(Gui *gui) { setPulse2SweepPeriod(gui->apu, 5); }
void actSetPu2SwePeriod6(Gui *gui) { setPulse2SweepPeriod(gui->apu, 6); }
void actSetPu2SwePeriod7(Gui *gui) { setPulse2SweepPeriod(gui->apu, 7); }
static const GuiState guiSetPu2SwePeriod = { { {"Back", &actSetPu2SwePeriodBack},
                                  {"0", &actSetPu2SwePeriod0, true},
                                  {"1", &actSetPu2SwePeriod1},
                                  {"2", &actSetPu2SwePeriod2},
                                  {"3", &actSetPu2SwePeriod3},
                                  {"4", &actSetPu2SwePeriod4},
                                  {"5", &actSetPu2SwePeriod5},
                                  {"6", &actSetPu2SwePeriod6},
                                  {"7", &actSetPu2SwePeriod7} } };

void actSetPu2SweNegateBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SweNegateNo(Gui *gui) { setPulse2SweepNegate(gui->apu, false); }
void actSetPu2SweNegateYes(Gui *gui) { setPulse2SweepNegate(gui->apu, true); }
static const GuiState guiSetPu2SweNegate = { { {"Back", &actSetPu2SweNegateBack},
                                  {"Positive", &actSetPu2SweNegateNo, true},
                                  {"Negative", &actSetPu2SweNegateYes} } };

void actSetPu2SweShiftBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SweShift0(Gui *gui) { setPulse2SweepShift(gui->apu, 0); }
void actSetPu2SweShift1(Gui *gui) { setPulse2SweepShift(gui->apu, 1); }
void actSetPu2SweShift2(Gui *gui) { setPulse2SweepShift(gui->apu, 2); }
void actSetPu2SweShift3(Gui *gui) { setPulse2SweepShift(gui->apu, 3); }
void actSetPu2SweShift4(Gui *gui) { setPulse2SweepShift(gui->apu, 4); }
void actSetPu2SweShift5(Gui *gui) { setPulse2SweepShift(gui->apu, 5); }
void actSetPu2SweShift6(Gui *gui) { setPulse2SweepShift(gui->apu, 6); }
void actSetPu2SweShift7(Gui *gui) { setPulse2SweepShift(gui->apu, 7); }
static const GuiState guiSetPu2SweShift = { { {"Back", &actSetPu2SweShiftBack},
                                 {"0", &actSetPu2SweShift0, true},
                                 {"1", &actSetPu2SweShift1},
                                 {"2", &actSetPu2SweShift2},
                                 {"3", &actSetPu2SweShift3},
                                 {"4", &actSetPu2SweShift4},
                                 {"5", &actSetPu2SweShift5},
                                 {"6", &actSetPu2SweShift6},
                                 {"7", &actSetPu2SweShift7} } };
