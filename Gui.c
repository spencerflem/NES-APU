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

#pragma DATA_SECTION(guiRecording, "gui");
#pragma DATA_SECTION(guiMain, "gui");
#pragma DATA_SECTION(guiFile, "gui");
#pragma DATA_SECTION(guiLoop, "gui");
#pragma DATA_SECTION(guiLoopPlay, "gui");
#pragma DATA_SECTION(guiRecord, "gui");
#pragma DATA_SECTION(guiPulse1, "gui");
#pragma DATA_SECTION(guiPulse2, "gui");
#pragma DATA_SECTION(guiTriangle, "gui");
#pragma DATA_SECTION(guiNoise, "gui");
#pragma DATA_SECTION(guiRecPu1, "gui");
#pragma DATA_SECTION(guiRecPu2, "gui");
#pragma DATA_SECTION(guiRecTri, "gui");
#pragma DATA_SECTION(guiRecNoi, "gui");
#pragma DATA_SECTION(guiSetPu1, "gui");
#pragma DATA_SECTION(guiSetPu2, "gui");
#pragma DATA_SECTION(guiSetNoi, "gui");
#pragma DATA_SECTION(guiSetPu1Duty, "gui");
#pragma DATA_SECTION(guiSetPu1Env, "gui");
#pragma DATA_SECTION(guiSetPu1Swe, "gui");
#pragma DATA_SECTION(guiSetPu2Duty, "gui");
#pragma DATA_SECTION(guiSetPu2Env, "gui");
#pragma DATA_SECTION(guiSetPu2Swe, "gui");
#pragma DATA_SECTION(guiSetNoiMode, "gui");
#pragma DATA_SECTION(guiSetNoiEnv, "gui");
#pragma DATA_SECTION(guiSetPu1EnvVol, "gui");
#pragma DATA_SECTION(guiSetPu1EnvConst, "gui");
#pragma DATA_SECTION(guiSetPu2EnvVol, "gui");
#pragma DATA_SECTION(guiSetPu2EnvConst, "gui");
#pragma DATA_SECTION(guiSetNoiEnvVol, "gui");
#pragma DATA_SECTION(guiSetNoiEnvConst, "gui");
#pragma DATA_SECTION(guiSetPu1SweEnabled, "gui");
#pragma DATA_SECTION(guiSetPu1SwePeriod, "gui");
#pragma DATA_SECTION(guiSetPu1SweNegate, "gui");
#pragma DATA_SECTION(guiSetPu1SweShift, "gui");
#pragma DATA_SECTION(guiSetPu2SweEnabled, "gui");
#pragma DATA_SECTION(guiSetPu2SwePeriod, "gui");
#pragma DATA_SECTION(guiSetPu2SweNegate, "gui");
#pragma DATA_SECTION(guiSetPu2SweShift, "gui");

const char clearBuff[] = {0xB4, 0x00, 0x30, 0xB4, 0x01, 0x08, 0xB4, 0x02, 0x00,
                                 0xB4, 0x03, 0x00, 0xB4, 0x04, 0x30, 0xB4, 0x05, 0x08,
                                 0xB4, 0x06, 0x00, 0xB4, 0x07, 0x00, 0xB4, 0x08, 0x80,
                                 0xB4, 0x09, 0x00, 0xB4, 0x0A, 0x00, 0xB4, 0x0B, 0x00,
                                 0xB4, 0x0C, 0x30, 0xB4, 0x0D, 0x00, 0xB4, 0x0E, 0x00,
                                 0xB4, 0x0F, 0x00, 0xB4, 0x10, 0x00, 0xB4, 0x11, 0x00,
                                 0xB4, 0x12, 0x00, 0xB4, 0x13, 0x00, 0xB4, 0x15, 0x0F,
                                 0xB4, 0x17, 0x40, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF,
                                 0x61, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0x66}; //INIT ALL INSTRUMENTS TO OFF, WAIT

void initGuiGpio() {
    setup_lcd();
    init_io_gpio();

    EALLOW;
    // UP BUTTON
    GpioCtrlRegs.GPDMUX2.bit.GPIO123 = 0;  // GPIO
    GpioCtrlRegs.GPDDIR.bit.GPIO123 = 0;  // Input
    GpioCtrlRegs.GPDPUD.bit.GPIO123 = 0; // Pull Up
    // SELECT BUTTON
    GpioCtrlRegs.GPDMUX2.bit.GPIO122 = 0;  // GPIO
    GpioCtrlRegs.GPDDIR.bit.GPIO122 = 0;  // Input
    GpioCtrlRegs.GPDPUD.bit.GPIO122 = 0; // Pull Up
    // DOWN BUTTON
    GpioCtrlRegs.GPBMUX2.bit.GPIO59 = 0;  // GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO59 = 0;  // Input
    GpioCtrlRegs.GPBPUD.bit.GPIO59 = 0; // Pull Up
    // C
    GpioCtrlRegs.GPDMUX2.bit.GPIO124 = 0;  // GPIO
    GpioCtrlRegs.GPDDIR.bit.GPIO124 = 0;  // Input
    GpioCtrlRegs.GPDPUD.bit.GPIO124 = 0; // Pull Up
    // C#
    GpioCtrlRegs.GPDMUX2.bit.GPIO125 = 0;  // GPIO
    GpioCtrlRegs.GPDDIR.bit.GPIO125 = 0;  // Input
    GpioCtrlRegs.GPDPUD.bit.GPIO125 = 0; // Pull Up
    // D
    GpioCtrlRegs.GPCMUX2.bit.GPIO95 = 0;  // GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO95 = 0;  // Input
    GpioCtrlRegs.GPCPUD.bit.GPIO95 = 0; // Pull Up
    // D#
    GpioCtrlRegs.GPEMUX1.bit.GPIO139 = 0;  // GPIO
    GpioCtrlRegs.GPEDIR.bit.GPIO139 = 0;  // Input
    GpioCtrlRegs.GPEPUD.bit.GPIO139 = 0; // Pull Up
    // E
    GpioCtrlRegs.GPBMUX2.bit.GPIO56 = 0;  // GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO56 = 0;  // Input
    GpioCtrlRegs.GPBPUD.bit.GPIO56 = 0; // Pull Up
    // F
    GpioCtrlRegs.GPDMUX1.bit.GPIO97 = 0;  // GPIO
    GpioCtrlRegs.GPDDIR.bit.GPIO97 = 0;  // Input
    GpioCtrlRegs.GPDPUD.bit.GPIO97 = 0; // Pull Up
    // F#
    GpioCtrlRegs.GPCMUX2.bit.GPIO94 = 0;  // GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO94 = 0;  // Input
    GpioCtrlRegs.GPCPUD.bit.GPIO94 = 0; // Pull Up
    // G
    GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 0;  // GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO65 = 0;  // Input
    GpioCtrlRegs.GPCPUD.bit.GPIO65 = 0; // Pull Up
    // G#
    GpioCtrlRegs.GPCMUX1.bit.GPIO66 = 0;  // GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO66 = 0;  // Input
    GpioCtrlRegs.GPCPUD.bit.GPIO66 = 0; // Pull Up
    // A
    GpioCtrlRegs.GPCMUX1.bit.GPIO67 = 0;  // GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO67 = 0;  // Input
    GpioCtrlRegs.GPCPUD.bit.GPIO67 = 0; // Pull Up
    // A#
    GpioCtrlRegs.GPDMUX1.bit.GPIO111 = 0;  // GPIO
    GpioCtrlRegs.GPDDIR.bit.GPIO111 = 0;  // Input
    GpioCtrlRegs.GPDPUD.bit.GPIO111 = 0; // Pull Up
    // B
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;  // GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO32 = 0;  // Input
    GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0; // Pull Up
    EDIS;

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
        /* TODO: GET SELECTED WORKING
        if (gui->state.choices[gui->choice].selected) {
            write_string(" *");
        }
        */
        if (gui->choice < 17 && gui->state.choices[gui->choice+1].name[0] != '\0') {
            shift_screen();
            if(gui->bottomCursor) {
                write_string("> ");
            }
            else {
                write_string("  ");
            }
            write_string(gui->state.choices[gui->choice+1].name);
            /* TODO: GET SELECTED WORKING
            if (gui->state.choices[gui->choice+1].selected) {
                write_string(" *");
            }
            */
        }
    }
    gui->unchanged = true;
}

void updateGuiState(Gui *gui) {
    if (!GpioDataRegs.GPDDAT.bit.GPIO123 && !gui->upPressed) {
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
    else if (!GpioDataRegs.GPDDAT.bit.GPIO122 && !gui->selPressed) {
        // Select
        gui->selPressed = 0xFFFF;
        gui->state.choices[gui->choice+gui->bottomCursor].select(gui);
    }
    else if (!GpioDataRegs.GPBDAT.bit.GPIO59 && !gui->downPressed) {
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

    if (GpioDataRegs.GPDDAT.bit.GPIO123 && gui->upPressed) {
        gui->upPressed--;
    }
    if (GpioDataRegs.GPDDAT.bit.GPIO122 && gui->selPressed) {
        gui->selPressed--;
    }
    if (GpioDataRegs.GPBDAT.bit.GPIO59 && gui->downPressed) {
        gui->downPressed--;
    }
}

void resetLoop(Gui *gui) {
    for (int16 i = 0; i < 196; i++) {
        pingBuff[i] = clearBuff[i];
    }
    gui->oldBuff = pingBuff;
    gui->newBuff = pongBuff;
}

void initGui(Gui *gui, Apu *apu) {
    gui->state = guiMain;
    gui->apu = apu;
    gui->octave = 3;
    resetLoop(gui);
}

void gotoState(Gui *gui, GuiState state) {
    gui->state = state;
    gui->bottomCursor = false;
    gui->choice = 0;
    gui->unchanged = 0;
}

void saveRegs(Gui *gui) {
    gui->pu1 = getPulse1Registers(gui->apu);
    gui->pu2 = getPulse2Registers(gui->apu);
    gui->tri = getTriangleRegisters(gui->apu);
    gui->noi = getNoiseRegisters(gui->apu);
    gui->sta = getStatusRegisters(gui->apu);
}

void restoreRegs(Gui *gui) {
    writeRegister(gui->apu, 0x00, gui->pu1.reg0);
    writeRegister(gui->apu, 0x01, gui->pu1.reg1);
    writeRegister(gui->apu, 0x02, gui->pu1.reg2);
    writeRegister(gui->apu, 0x03, gui->pu1.reg3);
    writeRegister(gui->apu, 0x04, gui->pu2.reg0);
    writeRegister(gui->apu, 0x05, gui->pu2.reg1);
    writeRegister(gui->apu, 0x06, gui->pu2.reg2);
    writeRegister(gui->apu, 0x07, gui->pu2.reg3);
    writeRegister(gui->apu, 0x08, gui->tri.reg0);
    writeRegister(gui->apu, 0x0A, gui->tri.reg2);
    writeRegister(gui->apu, 0x0B, gui->tri.reg3);
    writeRegister(gui->apu, 0x0C, gui->noi.reg0);
    writeRegister(gui->apu, 0x0E, gui->noi.reg2);
    writeRegister(gui->apu, 0x0F, gui->noi.reg3);
    writeRegister(gui->apu, 0x15, gui->sta.reg1);
    writeRegister(gui->apu, 0x17, gui->sta.reg3);
}


void guiSelect(Gui *gui, int16 selected) {
    /* TODO: GET SELECTED WORKING
     * has something to do with the GuiStates being const
     * Gui Needs to Store this state, but im not sure where it should go
    for (int16 i = 0; i < 17; i++) {
        gui->state.choices[i].selected = false;
    }
    gui->state.choices[selected].selected = true;
    gui->unchanged = false;
    */
    // Backup behavior:
    // Fragile, relies on 0 always begin back
    gui->state.choices[0].select(gui);
}

void actRecording(Gui *gui) {/*Do Nothing*/;}
const GuiState guiMain = { { {"Recording", &actRecording} } };

void actFile(Gui *gui) {
    gotoState(gui, guiFile);
    saveRegs(gui);
    gui->wait = 0;
    gui->index = 0;
    gui->file = intro;
    gui->playFile = true;
    gui->fileLoopIndex = introLoopIndex;
}
void actLoop(Gui *gui) { gotoState(gui, guiLoop); }
const GuiState guiRecording = { { {"File", &actFile},
                       {"Loop", &actLoop} } };


void actFileBack(Gui *gui) {
    gotoState(gui, guiMain);
    restoreRegs(gui);
    gui->playFile = false;
}
const GuiState guiFile = { { {"Back", actFileBack} } };

void actLoopBack(Gui *gui) { gotoState(gui, guiMain); }
void actLoopPlay(Gui *gui) {
    gotoState(gui, guiLoopPlay);
    saveRegs(gui);
    gui->wait = 0;
    gui->index = 0;
    gui->playLoop = true;
}
void actRecord(Gui *gui) { gotoState(gui, guiRecord); }
void actReset(Gui *gui) { resetLoop(gui); }
const GuiState guiLoop = { { {"Back", &actLoopBack},
                       {"Play", &actLoopPlay},
                       {"Record", &actRecord},
                       {"Reset", &actReset} } };

void actLoopPlayBack(Gui *gui) {
    gotoState(gui, guiLoop);
    restoreRegs(gui);
    gui->playLoop = false;
}
const GuiState guiLoopPlay = { { {"Back", &actLoopPlayBack} } };

void actRecordBack(Gui *gui) { gotoState(gui, guiLoop); }
void actPulse1(Gui *gui) { gotoState(gui, guiPulse1); }
void actPulse2(Gui *gui) { gotoState(gui, guiPulse2); }
void actTriangle(Gui *gui) { gotoState(gui, guiTriangle); }
void actNoise(Gui *gui) { gotoState(gui, guiNoise); }
const GuiState guiRecord = { { { "Back", &actRecordBack},
                         {"Pulse1", &actPulse1},
                         {"Pulse2", &actPulse2},
                         {"Triangle", &actTriangle},
                         {"Noise", &actNoise} } };

void actPulse1Back(Gui *gui) { gotoState(gui, guiRecord); }
void actPulse1Record(Gui *gui) {
    gotoState(gui, guiRecording);
    gui->recordLoop = 1;
    gui->wait = 0;
    gui->waitTotal = 0;
    gui->index = 0;
    gui->newIndex = 0;
    saveRegs(gui);
}
void actPulse1Settings(Gui *gui) { gotoState(gui, guiSetPu1); }
const GuiState guiPulse1 = { { {"Back", &actPulse1Back},
                         {"Record", &actPulse1Record},
                         {"Settings", &actPulse1Settings} } };

void actPulse2Back(Gui *gui) { gotoState(gui, guiRecord); }
void actPulse2Record(Gui *gui) {
    gotoState(gui, guiRecording);
    gui->recordLoop = 2;
    gui->wait = 0;
    gui->waitTotal = 0;
    gui->index = 0;
    gui->newIndex = 0;
    saveRegs(gui);
}
void actPulse2Settings(Gui *gui) { gotoState(gui, guiSetPu2); }
const GuiState guiPulse2 = { { {"Back", &actPulse2Back},
                         {"Record", &actPulse2Record},
                         {"Settings", &actPulse2Settings} } };

void actTriangleBack(Gui *gui) { gotoState(gui, guiRecord); }
void actTriangleRecord(Gui *gui) {
    gotoState(gui, guiRecording);
    gui->recordLoop = 3;
    gui->wait = 0;
    gui->waitTotal = 0;
    gui->index = 0;
    gui->newIndex = 0;
    saveRegs(gui);
}
const GuiState guiTriangle = { { {"Back", &actTriangleBack},
                           {"Record", &actTriangleRecord} } };

void actNoiseBack(Gui *gui) { gotoState(gui, guiRecord); }
void actNoiseRecord(Gui *gui) {
    gotoState(gui, guiRecording);
    gui->recordLoop = 4;
    gui->wait = 0;
    gui->waitTotal = 0;
    gui->index = 0;
    gui->newIndex = 0;
    saveRegs(gui);
}
void actNoiseSettings(Gui *gui) { gotoState(gui, guiSetNoi); }
const GuiState guiNoise = { { {"Back", &actNoiseBack},
                        {"Record", &actNoiseRecord},
                        {"Settings", &actNoiseSettings}  } };

void actRecPu1Save(Gui *gui) {
    gotoState(gui, guiPulse1);
    char *tempBuff = gui->oldBuff;
    gui->oldBuff = gui->newBuff;
    gui->newBuff = tempBuff;
    gui->recordLoop = 0;
    restoreRegs(gui);
}
void actRecPu1Disc(Gui *gui) {
    gotoState(gui, guiPulse1);
    gui->recordLoop = 0;
    restoreRegs(gui);
}
const GuiState guiRecPu1 = { { {"Save", &actRecPu1Save},
                                      {"Discard", &actRecPu1Disc} } };

void actRecPu2Save(Gui *gui) {
    gotoState(gui, guiPulse2);
    char *tempBuff = gui->oldBuff;
    gui->oldBuff = gui->newBuff;
    gui->newBuff = tempBuff;
    gui->recordLoop = 0;
    restoreRegs(gui);
}
void actRecPu2Disc(Gui *gui) {
    gotoState(gui, guiPulse2);
    gui->recordLoop = 0;
    restoreRegs(gui);
}
const GuiState guiRecPu2 = { { {"Save", &actRecPu2Save},
                                      {"Discard", &actRecPu2Disc} } };

void actRecTriSave(Gui *gui) {
    gotoState(gui, guiTriangle);
    char *tempBuff = gui->oldBuff;
    gui->oldBuff = gui->newBuff;
    gui->newBuff = tempBuff;
    gui->recordLoop = 0;
    restoreRegs(gui);
}
void actRecTriDisc(Gui *gui) {
    gotoState(gui, guiTriangle);
    gui->recordLoop = 0;
    restoreRegs(gui);
}
const GuiState guiRecTri = { { {"Save", &actRecTriSave},
                                      {"Discard", &actRecTriDisc} } };

void actRecNoiSave(Gui *gui) {
    gotoState(gui, guiNoise);
    char *tempBuff = gui->oldBuff;
    gui->oldBuff = gui->newBuff;
    gui->newBuff = tempBuff;
    gui->recordLoop = 0;
    restoreRegs(gui);
}
void actRecNoiDisc(Gui *gui) {
    gotoState(gui, guiNoise);
    gui->recordLoop = 0;
    restoreRegs(gui);
}
const GuiState guiRecNoi = { { {"Save", &actRecNoiSave},
                                      {"Discard", &actRecNoiDisc} } };

void actSetPu1Back(Gui *gui) { gotoState(gui, guiPulse1); }
void actSetPu1Duty(Gui *gui) { gotoState(gui, guiSetPu1Duty); }
void actSetPu1Env(Gui *gui) { gotoState(gui, guiSetPu1Env); }
void actSetPu1Swe(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
const GuiState guiSetPu1 = { { {"Back", &actSetPu1Back},
                         {"Duty", &actSetPu1Duty},
                         {"Envelope", &actSetPu1Env},
                         {"Sweep", &actSetPu1Swe} } };

void actSetPu2Back(Gui *gui) { gotoState(gui, guiPulse2); }
void actSetPu2Duty(Gui *gui) { gotoState(gui, guiSetPu2Duty); }
void actSetPu2Env(Gui *gui) { gotoState(gui, guiSetPu2Env); }
void actSetPu2Swe(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
const GuiState guiSetPu2 = { { {"Back", &actSetPu2Back},
                         {"Duty", &actSetPu2Duty},
                         {"Envelope", &actSetPu2Env},
                         {"Sweep", &actSetPu2Swe} } };

void actSetNoiBack(Gui *gui) { gotoState(gui, guiNoise); }
void actSetNoiMode(Gui *gui) { gotoState(gui, guiSetNoiMode); }
void actSetNoiEnv(Gui *gui) { gotoState(gui, guiSetNoiEnv); }
const GuiState guiSetNoi = { { {"Back", &actSetNoiBack},
                         {"Mode", &actSetNoiMode},
                         {"Envelope", &actSetNoiEnv} } };

void actSetPu1DutyBack(Gui *gui) { gotoState(gui, guiSetPu1); }
void actSetPu1Duty0(Gui *gui) { setPulse1Duty(gui->apu, 0); guiSelect(gui, 1); }
void actSetPu1Duty1(Gui *gui) { setPulse1Duty(gui->apu, 1); guiSelect(gui, 2); }
void actSetPu1Duty2(Gui *gui) { setPulse1Duty(gui->apu, 2); guiSelect(gui, 3); }
void actSetPu1Duty3(Gui *gui) { setPulse1Duty(gui->apu, 3); guiSelect(gui, 4); }
const GuiState guiSetPu1Duty = { { {"Back", &actSetPu1DutyBack},
                             {"12.5%", &actSetPu1Duty0, true},
                             {"25%", &actSetPu1Duty1},
                             {"50%", &actSetPu1Duty2},
                             {"75%", &actSetPu1Duty3} } };

void actSetPu1EnvBack(Gui *gui) { gotoState(gui, guiSetPu1); }
void actSetPu1EnvVol(Gui *gui) { gotoState(gui, guiSetPu1EnvVol); }
void actSetPu1EnvMode(Gui *gui) { gotoState(gui, guiSetPu1EnvConst); }
const GuiState guiSetPu1Env = { { {"Back", &actSetPu1EnvBack},
                            {"Vol/Period", &actSetPu1EnvVol},
                            {"Mode", &actSetPu1EnvMode} } };

void actSetPu1SweBack(Gui *gui) { gotoState(gui, guiSetPu1); }
void actSetPu1SweEnable(Gui *gui) { gotoState(gui, guiSetPu1SweEnabled); }
void actSetPu1SwePeriod(Gui *gui) { gotoState(gui, guiSetPu1SwePeriod); }
void actSetPu1SweNegate(Gui *gui) { gotoState(gui, guiSetPu1SweNegate); }
void actSetPu1SweShift(Gui *gui) { gotoState(gui, guiSetPu1SweShift); }
const GuiState guiSetPu1Swe = { { {"Back", &actSetPu1SweBack},
                            {"Enable", &actSetPu1SweEnable},
                            {"Period", &actSetPu1SwePeriod},
                            {"Negate", &actSetPu1SweNegate},
                            {"Shift", &actSetPu1SweShift} } };

void actSetPu2DutyBack(Gui *gui) { gotoState(gui, guiSetPu2); }
void actSetPu2Duty0(Gui *gui) { setPulse2Duty(gui->apu, 0); guiSelect(gui, 1); }
void actSetPu2Duty1(Gui *gui) { setPulse2Duty(gui->apu, 1); guiSelect(gui, 2); }
void actSetPu2Duty2(Gui *gui) { setPulse2Duty(gui->apu, 2); guiSelect(gui, 3); }
void actSetPu2Duty3(Gui *gui) { setPulse2Duty(gui->apu, 3); guiSelect(gui, 4); }
const GuiState guiSetPu2Duty = { { {"Back", &actSetPu2DutyBack},
                             {"12.5%", &actSetPu2Duty0, true},
                             {"25%", &actSetPu2Duty1},
                             {"50%", &actSetPu2Duty2},
                             {"75%", &actSetPu2Duty3} } };

void actSetPu2EnvBack(Gui *gui) { gotoState(gui, guiSetPu2); }
void actSetPu2EnvVol(Gui *gui) { gotoState(gui, guiSetPu2EnvVol); }
void actSetPu2EnvMode(Gui *gui) { gotoState(gui, guiSetPu2EnvConst); }
const GuiState guiSetPu2Env = { { {"Back", &actSetPu2EnvBack},
                            {"Vol/Period", &actSetPu2EnvVol},
                            {"Mode", &actSetPu2EnvMode} } };

void actSetPu2SweBack(Gui *gui) { gotoState(gui, guiSetPu2); }
void actSetPu2SweEnable(Gui *gui) { gotoState(gui, guiSetPu2SweEnabled); }
void actSetPu2SwePeriod(Gui *gui) { gotoState(gui, guiSetPu2SwePeriod); }
void actSetPu2SweNegate(Gui *gui) { gotoState(gui, guiSetPu2SweNegate); }
void actSetPu2SweShift(Gui *gui) { gotoState(gui, guiSetPu2SweShift); }
const GuiState guiSetPu2Swe = { { {"Back", &actSetPu2SweBack},
                            {"Enable", &actSetPu2SweEnable},
                            {"Period", &actSetPu2SwePeriod},
                            {"Negate", actSetPu2SweNegate},
                            {"Shift", actSetPu2SweShift} } };

void actSetNoiModeBack(Gui *gui) { gotoState(gui, guiSetNoi); }
void actSetNoiMode0(Gui *gui) { setNoiseMode(gui->apu, false); guiSelect(gui, 1);  }
void actSetNoiMode1(Gui *gui) { setNoiseMode(gui->apu, true); guiSelect(gui, 2);  }
const GuiState guiSetNoiMode = { { {"Back", &actSetNoiModeBack},
                             {"Mode0", &actSetNoiMode0, true },
                             {"Mode1", &actSetNoiMode1} } };

void actSetNoiEnvBack(Gui *gui) { gotoState(gui, guiSetNoi); }
void actSetNoiEnvVol(Gui *gui) { gotoState(gui, guiSetNoiEnvVol); }
void actSetNoiEnvMode(Gui *gui) { gotoState(gui, guiSetNoiEnvConst); }
const GuiState guiSetNoiEnv = { { {"Back", &actSetNoiEnvBack},
                            {"Vol/Period", &actSetNoiEnvVol},
                            {"Mode", &actSetNoiEnvMode} } };

void actSetPu1EnvVolBack(Gui *gui) { gotoState(gui, guiSetPu1Env); }
void actSetPu1EnvVol0(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 0); guiSelect(gui, 1);  }
void actSetPu1EnvVol1(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 1); guiSelect(gui, 2); }
void actSetPu1EnvVol2(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 2); guiSelect(gui, 3); }
void actSetPu1EnvVol3(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 3); guiSelect(gui, 4); }
void actSetPu1EnvVol4(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 4); guiSelect(gui, 5); }
void actSetPu1EnvVol5(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 5); guiSelect(gui, 6); }
void actSetPu1EnvVol6(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 6); guiSelect(gui, 7); }
void actSetPu1EnvVol7(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 7); guiSelect(gui, 8); }
void actSetPu1EnvVol8(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 8); guiSelect(gui, 9); }
void actSetPu1EnvVol9(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 9); guiSelect(gui, 10); }
void actSetPu1EnvVol10(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 10); guiSelect(gui, 11); }
void actSetPu1EnvVol11(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 11); guiSelect(gui, 12); }
void actSetPu1EnvVol12(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 12); guiSelect(gui, 13); }
void actSetPu1EnvVol13(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 13); guiSelect(gui, 14); }
void actSetPu1EnvVol14(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 14); guiSelect(gui, 15); }
void actSetPu1EnvVol15(Gui *gui) { setPulse1EnvelopePeriodAndVolume(gui->apu, 15); guiSelect(gui, 16); }
const GuiState guiSetPu1EnvVol = { { {"Back", &actSetPu1EnvVolBack},
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
void actSetPu1EnvConstUse(Gui *gui) { setPulse1EnvelopeUseConstantVolume(gui->apu, true); guiSelect(gui, 1); }
void actSetPu1EnvConstSaw(Gui *gui) { setPulse1EnvelopeUseConstantVolume(gui->apu, false); guiSelect(gui, 2); }
const GuiState guiSetPu1EnvConst = { { {"Back", &actSetPu1EnvConstBack},
                                 {"ConstVol", &actSetPu1EnvConstUse, true},
                                 {"Saw", &actSetPu1EnvConstSaw} } };

void actSetPu2EnvVolBack(Gui *gui) { gotoState(gui, guiSetPu1Env); }
void actSetPu2EnvVol0(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 0); guiSelect(gui, 1); }
void actSetPu2EnvVol1(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 1); guiSelect(gui, 2); }
void actSetPu2EnvVol2(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 2); guiSelect(gui, 3); }
void actSetPu2EnvVol3(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 3); guiSelect(gui, 4); }
void actSetPu2EnvVol4(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 4); guiSelect(gui, 5); }
void actSetPu2EnvVol5(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 5); guiSelect(gui, 6); }
void actSetPu2EnvVol6(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 6); guiSelect(gui, 7); }
void actSetPu2EnvVol7(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 7); guiSelect(gui, 8); }
void actSetPu2EnvVol8(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 8); guiSelect(gui, 9); }
void actSetPu2EnvVol9(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 9); guiSelect(gui, 10); }
void actSetPu2EnvVol10(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 10); guiSelect(gui, 11); }
void actSetPu2EnvVol11(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 11); guiSelect(gui, 12); }
void actSetPu2EnvVol12(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 12); guiSelect(gui, 13); }
void actSetPu2EnvVol13(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 13); guiSelect(gui, 14); }
void actSetPu2EnvVol14(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 14); guiSelect(gui, 15); }
void actSetPu2EnvVol15(Gui *gui) { setPulse2EnvelopePeriodAndVolume(gui->apu, 15); guiSelect(gui, 16); }
const GuiState guiSetPu2EnvVol = { { {"Back", &actSetPu2EnvVolBack},
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
void actSetPu2EnvConstUse(Gui *gui) { setPulse2EnvelopeUseConstantVolume(gui->apu, true); guiSelect(gui, 1); }
void actSetPu2EnvConstSaw(Gui *gui) { setPulse2EnvelopeUseConstantVolume(gui->apu, false); guiSelect(gui, 2); }
const GuiState guiSetPu2EnvConst = { { {"Back", &actSetPu2EnvConstBack},
                                 {"ConstVol", &actSetPu2EnvConstUse, true},
                                 {"Saw", &actSetPu2EnvConstSaw} } };

void actSetNoiEnvVolBack(Gui *gui) { gotoState(gui, guiSetNoiEnv); }
void actSetNoiEnvVol0(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 0); guiSelect(gui, 1); }
void actSetNoiEnvVol1(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 1); guiSelect(gui, 2); }
void actSetNoiEnvVol2(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 2); guiSelect(gui, 3); }
void actSetNoiEnvVol3(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 3); guiSelect(gui, 4); }
void actSetNoiEnvVol4(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 4); guiSelect(gui, 5); }
void actSetNoiEnvVol5(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 5); guiSelect(gui, 6); }
void actSetNoiEnvVol6(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 6); guiSelect(gui, 7); }
void actSetNoiEnvVol7(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 7); guiSelect(gui, 8); }
void actSetNoiEnvVol8(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 8); guiSelect(gui, 9); }
void actSetNoiEnvVol9(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 9); guiSelect(gui, 10); }
void actSetNoiEnvVol10(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 10); guiSelect(gui, 11); }
void actSetNoiEnvVol11(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 11); guiSelect(gui, 12); }
void actSetNoiEnvVol12(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 12); guiSelect(gui, 13); }
void actSetNoiEnvVol13(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 13); guiSelect(gui, 14); }
void actSetNoiEnvVol14(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 14); guiSelect(gui, 15); }
void actSetNoiEnvVol15(Gui *gui) { setNoiseEnvelopePeriodAndVolume(gui->apu, 15); guiSelect(gui, 16); }
const GuiState guiSetNoiEnvVol = { { {"Back", &actSetNoiEnvVolBack},
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
void actSetNoiEnvConstUse(Gui *gui) { setNoiseEnvelopeUseConstantVolume(gui->apu, true); guiSelect(gui, 1); }
void actSetNoiEnvConstSaw(Gui *gui) { setNoiseEnvelopeUseConstantVolume(gui->apu, false); guiSelect(gui, 2); }
const GuiState guiSetNoiEnvConst = { { {"Back", &actSetNoiEnvConstBack},
                                 {"ConstVol", &actSetNoiEnvConstUse, true},
                                 {"Saw", &actSetNoiEnvConstSaw} } };

void actSetPu1SweEnabledBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SweEnabledEn(Gui *gui) { setPulse1SweepEnabled(gui->apu, true); guiSelect(gui, 1); }
void actSetPu1SweEnabledDis(Gui *gui) { setPulse1SweepEnabled(gui->apu, false); guiSelect(gui, 2); }
const GuiState guiSetPu1SweEnabled = { { {"Back", &actSetPu1SweEnabledBack},
                                   {"Enabled", &actSetPu1SweEnabledEn},
                                   {"Disabled", &actSetPu1SweEnabledDis, true} } };

void actSetPu1SwePeriodBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SwePeriod0(Gui *gui) { setPulse1SweepPeriod(gui->apu, 0); guiSelect(gui, 1); }
void actSetPu1SwePeriod1(Gui *gui) { setPulse1SweepPeriod(gui->apu, 1); guiSelect(gui, 2); }
void actSetPu1SwePeriod2(Gui *gui) { setPulse1SweepPeriod(gui->apu, 2); guiSelect(gui, 3); }
void actSetPu1SwePeriod3(Gui *gui) { setPulse1SweepPeriod(gui->apu, 3); guiSelect(gui, 4); }
void actSetPu1SwePeriod4(Gui *gui) { setPulse1SweepPeriod(gui->apu, 4); guiSelect(gui, 5); }
void actSetPu1SwePeriod5(Gui *gui) { setPulse1SweepPeriod(gui->apu, 5); guiSelect(gui, 6); }
void actSetPu1SwePeriod6(Gui *gui) { setPulse1SweepPeriod(gui->apu, 6); guiSelect(gui, 7); }
void actSetPu1SwePeriod7(Gui *gui) { setPulse1SweepPeriod(gui->apu, 7); guiSelect(gui, 8); }
const GuiState guiSetPu1SwePeriod = { { {"Back", &actSetPu1SwePeriodBack},
                                  {"0", &actSetPu1SwePeriod0, true},
                                  {"1", &actSetPu1SwePeriod1},
                                  {"2", &actSetPu1SwePeriod2},
                                  {"3", &actSetPu1SwePeriod3},
                                  {"4", &actSetPu1SwePeriod4},
                                  {"5", &actSetPu1SwePeriod5},
                                  {"6", &actSetPu1SwePeriod6},
                                  {"7", &actSetPu1SwePeriod7} } };

void actSetPu1SweNegateBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SweNegateNo(Gui *gui) { setPulse1SweepNegate(gui->apu, false); guiSelect(gui, 1); }
void actSetPu1SweNegateYes(Gui *gui) { setPulse1SweepNegate(gui->apu, true); guiSelect(gui, 2); }
const GuiState guiSetPu1SweNegate = { { {"Back", &actSetPu1SweNegateBack},
                                  {"Positive", &actSetPu1SweNegateNo, true},
                                  {"Negative", &actSetPu1SweNegateYes} } };

void actSetPu1SweShiftBack(Gui *gui) { gotoState(gui, guiSetPu1Swe); }
void actSetPu1SweShift0(Gui *gui) { setPulse1SweepShift(gui->apu, 0); guiSelect(gui, 1); }
void actSetPu1SweShift1(Gui *gui) { setPulse1SweepShift(gui->apu, 1); guiSelect(gui, 2); }
void actSetPu1SweShift2(Gui *gui) { setPulse1SweepShift(gui->apu, 2); guiSelect(gui, 3); }
void actSetPu1SweShift3(Gui *gui) { setPulse1SweepShift(gui->apu, 3); guiSelect(gui, 4); }
void actSetPu1SweShift4(Gui *gui) { setPulse1SweepShift(gui->apu, 4); guiSelect(gui, 5); }
void actSetPu1SweShift5(Gui *gui) { setPulse1SweepShift(gui->apu, 5); guiSelect(gui, 6); }
void actSetPu1SweShift6(Gui *gui) { setPulse1SweepShift(gui->apu, 6); guiSelect(gui, 7); }
void actSetPu1SweShift7(Gui *gui) { setPulse1SweepShift(gui->apu, 7); guiSelect(gui, 8); }
const GuiState guiSetPu1SweShift = { { {"Back", &actSetPu1SweShiftBack},
                                 {"0", &actSetPu1SweShift0, true},
                                 {"1", &actSetPu1SweShift1},
                                 {"2", &actSetPu1SweShift2},
                                 {"3", &actSetPu1SweShift3},
                                 {"4", &actSetPu1SweShift4},
                                 {"5", &actSetPu1SweShift5},
                                 {"6", &actSetPu1SweShift6},
                                 {"7", &actSetPu1SweShift7} } };


void actSetPu2SweEnabledBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SweEnabledEn(Gui *gui) { setPulse2SweepEnabled(gui->apu, true); guiSelect(gui, 1); }
void actSetPu2SweEnabledDis(Gui *gui) { setPulse2SweepEnabled(gui->apu, false); guiSelect(gui, 2); }
const GuiState guiSetPu2SweEnabled = { { {"Back", &actSetPu2SweEnabledBack},
                                   {"Enabled", &actSetPu2SweEnabledEn},
                                   {"Disabled", &actSetPu2SweEnabledDis, true} } };

void actSetPu2SwePeriodBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SwePeriod0(Gui *gui) { setPulse2SweepPeriod(gui->apu, 0); guiSelect(gui, 1); }
void actSetPu2SwePeriod1(Gui *gui) { setPulse2SweepPeriod(gui->apu, 1); guiSelect(gui, 2); }
void actSetPu2SwePeriod2(Gui *gui) { setPulse2SweepPeriod(gui->apu, 2); guiSelect(gui, 3); }
void actSetPu2SwePeriod3(Gui *gui) { setPulse2SweepPeriod(gui->apu, 3); guiSelect(gui, 4); }
void actSetPu2SwePeriod4(Gui *gui) { setPulse2SweepPeriod(gui->apu, 4); guiSelect(gui, 5); }
void actSetPu2SwePeriod5(Gui *gui) { setPulse2SweepPeriod(gui->apu, 5); guiSelect(gui, 6); }
void actSetPu2SwePeriod6(Gui *gui) { setPulse2SweepPeriod(gui->apu, 6); guiSelect(gui, 7); }
void actSetPu2SwePeriod7(Gui *gui) { setPulse2SweepPeriod(gui->apu, 7); guiSelect(gui, 8); }
const GuiState guiSetPu2SwePeriod = { { {"Back", &actSetPu2SwePeriodBack},
                                  {"0", &actSetPu2SwePeriod0, true},
                                  {"1", &actSetPu2SwePeriod1},
                                  {"2", &actSetPu2SwePeriod2},
                                  {"3", &actSetPu2SwePeriod3},
                                  {"4", &actSetPu2SwePeriod4},
                                  {"5", &actSetPu2SwePeriod5},
                                  {"6", &actSetPu2SwePeriod6},
                                  {"7", &actSetPu2SwePeriod7} } };

void actSetPu2SweNegateBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SweNegateNo(Gui *gui) { setPulse2SweepNegate(gui->apu, false); guiSelect(gui, 1); }
void actSetPu2SweNegateYes(Gui *gui) { setPulse2SweepNegate(gui->apu, true); guiSelect(gui, 2); }
const GuiState guiSetPu2SweNegate = { { {"Back", &actSetPu2SweNegateBack},
                                  {"Positive", &actSetPu2SweNegateNo, true},
                                  {"Negative", &actSetPu2SweNegateYes} } };

void actSetPu2SweShiftBack(Gui *gui) { gotoState(gui, guiSetPu2Swe); }
void actSetPu2SweShift0(Gui *gui) { setPulse2SweepShift(gui->apu, 0); guiSelect(gui, 1); }
void actSetPu2SweShift1(Gui *gui) { setPulse2SweepShift(gui->apu, 1); guiSelect(gui, 2); }
void actSetPu2SweShift2(Gui *gui) { setPulse2SweepShift(gui->apu, 2); guiSelect(gui, 3); }
void actSetPu2SweShift3(Gui *gui) { setPulse2SweepShift(gui->apu, 3); guiSelect(gui, 4); }
void actSetPu2SweShift4(Gui *gui) { setPulse2SweepShift(gui->apu, 4); guiSelect(gui, 5); }
void actSetPu2SweShift5(Gui *gui) { setPulse2SweepShift(gui->apu, 5); guiSelect(gui, 6); }
void actSetPu2SweShift6(Gui *gui) { setPulse2SweepShift(gui->apu, 6); guiSelect(gui, 7); }
void actSetPu2SweShift7(Gui *gui) { setPulse2SweepShift(gui->apu, 7); guiSelect(gui, 8); }
const GuiState guiSetPu2SweShift = { { {"Back", &actSetPu2SweShiftBack},
                                 {"0", &actSetPu2SweShift0, true},
                                 {"1", &actSetPu2SweShift1},
                                 {"2", &actSetPu2SweShift2},
                                 {"3", &actSetPu2SweShift3},
                                 {"4", &actSetPu2SweShift4},
                                 {"5", &actSetPu2SweShift5},
                                 {"6", &actSetPu2SweShift6},
                                 {"7", &actSetPu2SweShift7} } };
