#include "TWI.h"
#include "util/delay.h"

int main()
{
    TWI twi;
    uint8_t data[] = "Hello";
    while(true) {
        twi.TWIWrite(0x01, data, 6);
        _delay_ms(200);
    }
    return 0;

}
