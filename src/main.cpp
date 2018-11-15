#include "TWI.h"
#include "util/delay.h"

int main()
{
    TWI twi;
    DDRB |= (1 << PINB7);
    sei();

    while(true) {
        //twi.TWIWrite(0x01, "hello");
        _delay_ms(100);
        twi.TWIWrite(0x01, 1);

    }
    return 0;

}
