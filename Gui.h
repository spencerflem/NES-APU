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
} Gui;

void initGuiGpio();
void initGui(Gui *gui, Apu *apu);
void displayGuiState(Gui *gui);
void updateGuiState(Gui *gui);

#endif /* GUI_H_ */
