/* ILI9341 interface for the NRF52832
 */

#include "mbed.h"
#include "display.h"
/* Serial */
#define BAUDRATE 9600
Serial g_Serial_pc(P0_18, P0_14, BAUDRATE);

/* DigitalOut */
#define LED_ON      0
#define LED_OFF     1

DigitalOut g_DO_LedBlue(A5, LED_OFF);
Display display;

uint32_t prbs()
{
    // This is an unverified 31 bit PRBS generator
    // It should be maximum length but this has not been verified 
    static uint32_t shift_register=0xaa551199;
    unsigned long new_bit=0;
    static int busy=0; // need to prevent re-entrancy here  
    if (!busy)
    {
        busy=1;
        new_bit= ((shift_register & (1<<27))>>27) ^ ((shift_register & (1<<30))>>30);
        new_bit= ~new_bit;
        new_bit = new_bit & 1;
        shift_register=shift_register << 1;
        shift_register=shift_register | (new_bit);
        busy=0;
    }
    return shift_register & 0x7ffffff; // return 31 LSB's 
}
uint32_t random(uint32_t lower,uint32_t upper)
{
    return (prbs()%(upper-lower))+lower;
}
int main(void) {
  
    int Count;
    display.begin();
    
    while(true) {
        g_Serial_pc.printf("LED Toggle\r\n");     
        g_DO_LedBlue = !g_DO_LedBlue;
        
        for (Count = 0; Count < 20; Count++)
        {
            display.drawRectangle(random(0,240),random(0,320),random(0,240),random(0,320),random(0,0xffff));
        }
        display.fillRectangle(0,0,240,320,0);
        for (Count = 0; Count < 20; Count++)
        {
           
           display.fillRectangle(random(0,240),random(0,320),random(0,240),random(0,320),random(0,0xffff));           

        }
        display.fillRectangle(0,0,240,320,0);
        for (Count = 0; Count < 20; Count++)
        {
            display.drawCircle(random(0,240),random(0,240),random(0,320),random(0,0xffff));

        }
        display.fillRectangle(0,0,240,320,0);
        for (Count = 0; Count < 20; Count++)
        {
            display.fillCircle(random(0,240),random(0,320),random(0,120),random(0,0xffff));

        }
        display.fillRectangle(0,0,240,320,0);
        
    }

    return 0;
}
