#include <avr/io.h>
#include "TWI.h"
#include "util/delay.h"

int main()
{
    TWI twi;
    DDRB |= (1 << PINB7);
    twi.TWISetMode(TWIMode::Slave, 0x01);
    sei();

    while(true) {
        _delay_ms(50);
        twi.Write("hello");
    }
    return 0;
}