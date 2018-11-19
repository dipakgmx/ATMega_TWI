#include <avr/io.h>
#include "TWI.h"
#include "util/delay.h"

int main()
{
    TWI twi;
    DDRB |= (1 << PINB7);
    sei();

    while(true) {
        twi.TWIWrite(0x01, "hello");
        _delay_ms(10);
        twi.TWIWrite(0x01, "bjlat");
        _delay_ms(10);
        twi.TWIWrite<uint8_t >(0x01, 254);
        _delay_ms(10);

    }
    return 0;

}
