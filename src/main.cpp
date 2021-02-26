/* ILI9341 interface for the NRF52832
 */

#include "mbed.h"
#include "display.h"
#include "font5x7.h"
#include "math.h"
/* Serial */
#define BAUDRATE 9600
Serial g_Serial_pc(P0_18, P0_14, BAUDRATE);

/* DigitalOut */
#define LED_ON      0
#define LED_OFF     1

DigitalOut DataLED(A5, LED_OFF);

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
class SemiCircularGauge {
public:
    SemiCircularGauge(string title, uint16_t x, uint16_t y, uint16_t width, uint16_t height, int min, int max, uint16_t ForeColour, uint16_t BackColour);
    void show();
    void hide();
    void setValue(int value);
    int getValue();
private:
    uint16_t x,y,w,h;
    uint16_t backcolour, forecolour;
    string title;
    int min,max;
    int value;
};
SemiCircularGauge::SemiCircularGauge(string title, uint16_t x, uint16_t y, uint16_t width, uint16_t height, int min, int max, uint16_t ForeColour, uint16_t BackColour)
{
    this->x = x;
    this->y = y;
    this->w = width;
    this->h = height;
    this->min = min;
    this->max = max;
    this->backcolour = BackColour;
    this->forecolour = ForeColour;
    this->title = title;
    value = min;
    show();
    
}
int mycos(int degrees)
{


}
int mysin(int degree)
{

}
void SemiCircularGauge::show()
{    
    int xlocation=x+(FONT_WIDTH * 3); // move the title 3 characters in from the left
    display.print(title.c_str(),title.length(),x,y+h+FONT_HEIGHT,forecolour, backcolour);
    display.print(min,x,y+h+FONT_HEIGHT,forecolour,backcolour);
    display.print(max,x+w-FONT_WIDTH*3,y+h+FONT_HEIGHT,forecolour,backcolour);
    int radius = w/2;
    int x0 = x+w/2;
    int y0 = y+h;
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);
    if (radius > x0)
        return; // don't draw even parially off-screen circles
    if (radius > y0)
        return; // don't draw even parially off-screen circles
        
    if ((x0+radius) > SCREEN_WIDTH)
        return; // don't draw even parially off-screen circles
    if ((y0+radius) > SCREEN_HEIGHT)
        return; // don't draw even parially off-screen circles    
    while (x >= y)
    {       
        display.putPixel(x0 - x, y0 - y, forecolour);
        display.putPixel(x0 - y, y0 - x, forecolour);
        display.putPixel(x0 + y, y0 - x, forecolour);
        display.putPixel(x0 + x, y0 - y, forecolour);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
    int xpt, ypt;
    float angle = (float)3.141592*(float)(value)/((float)max-(float)min);
    
    xpt = this->x+w/2+(float)h*cosf(angle);
    ypt = this->y+h - (float)h*sinf(angle);
    g_Serial_pc.printf("x=%d, y=%d\r\n",xpt,ypt);
    display.drawLine(this->x+w/2,this->y+h,xpt,ypt,forecolour);
}
void SemiCircularGauge::hide()
{
    display.fillRectangle(x,y,w*2,h*2,backcolour);
}
void SemiCircularGauge::setValue(int value)
{
    this->value = value;
    show();
}
int SemiCircularGauge::getValue()
{
    return value;
}
int main(void) {
  
    int count;
    display.begin();
    SemiCircularGauge gauge("Fuel",10,20,80,40,0,100,display.RGBToWord(0xff,0xff,0xff),0);
    
    while(1)    
    {
        DataLED = !DataLED;
        if (display.penDown()) 
        {
            display.print(display.readXTouch(),10,10,display.RGBToWord(0xff,0xff,0),display.RGBToWord(0,0,0));
            display.print(display.readYTouch(),80,10,display.RGBToWord(0xff,0xff,0),display.RGBToWord(0,0,0));
        }        
        gauge.hide();
        gauge.show();
        //display.drawRectangle(random(0,240),random(0,320),random(0,240),random(0,320),random(0,0xffff));                                
        //display.fillRectangle(random(0,240),random(0,320),random(0,240),random(0,320),random(0,0xffff));                   
        //display.drawCircle(random(0,240),random(0,240),random(0,320),random(0,0xffff));
        //display.fillCircle(random(0,240),random(0,320),random(0,120),random(0,0xffff));
        count++;
        if (count >= 100) 
        {
        //    display.fillRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,0);
            count = 0;
        }
        gauge.setValue(count);
        wait(0.1);
        
    }

    return 0;
}
