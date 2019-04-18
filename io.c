#include <F28x_Project.h>

void init_io_gpio() {
    EALLOW;
    GpioCtrlRegs.GPAMUX1.all &= 0x0F000000; // Set GPIO 15:14, 11:0 pins as GPIO
    GpioCtrlRegs.GPAMUX2.all &= 0xFFFFFFFC; // Set GPIO16 as GPIO
    GpioCtrlRegs.GPADIR.all |= 0x000000FF; // Set just the GPIO7:0 pins as output
    GpioCtrlRegs.GPADIR.all &= 0xFFFE30FF; // Set just the GPIO16:14,11:8 pins as input
    GpioCtrlRegs.GPAPUD.all &= 0xFFFE30FF; //Enable PUD for GPIO 16:14,11:8W
    GpioDataRegs.GPASET.all = 0x000000FF; // Clear LEDs
    EDIS;
}
