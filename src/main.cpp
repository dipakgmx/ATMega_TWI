#include <avr/io.h>
#include "TWI.h"
#include "util/delay.h"

int main()
{
    TWI twi;
    sei();

    while(true) {
        uint8_t data[6];
        _delay_ms(100);
        twi.TWIRead(1,data,6);
    }
    return 0;

}