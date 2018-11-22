#include <avr/io.h>
#include "TWI.h"
#include "util/delay.h"

int main()
{
    TWI twi;
    twi.TWISetMode(TWIMode::Slave, 0x01);
    sei();

    while(true) {
        _delay_ms(100);
        twi.TWIWrite(" ");
    }
    return 0;
}