#ifndef LCD_H_
#define LCD_H_

extern const Uint16 initCommands[];
extern const Uint16 shiftCommand[];
extern const Uint16 clearCommand[];

Uint16 strlen(const char string[]);

void write_commands(const Uint16 commands[], const Uint16 len);

void write_string(const char string[]);

void setup_lcd(void);

inline void clear_screen() {
    write_commands(clearCommand, 1);
    DELAY_US(1E3);
}

inline void shift_screen() {
    write_commands(shiftCommand, 1);
}

#endif /* LCD_H_ */
