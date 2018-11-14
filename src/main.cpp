#include "TWI.h"
#include "util/delay.h"

int main()
{
    TWI twi;
    sei();
    uint8_t dataTx[] = "hello";
    while(true) {
        bool result = twi.TWIWrite(0x01, dataTx, 5);
        _delay_ms(50);
    }
    return 0;

}
