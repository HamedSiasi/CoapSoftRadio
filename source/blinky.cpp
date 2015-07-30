#include "mbed/mbed.h"

DigitalOut led(LED1);

void app_start(int, char**){
    while(1){
        led = !led;
        printf("LED = %d \n\r",led.read());
        wait(0.5f);
    }
}