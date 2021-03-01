#include <mbed.h>
#include "display.h"
#include "font5x7.h"
/*
Input/Output List:
(using Kitronic microbit header Version 1.1) 
Kitronic pin header numbering is not the same as microbit edge connector numbering.
Function    KitronicPin   NRF51822
============================================
MOSI        Pin 15        P0.21
SCK         Pin 13        P0.23
Reset       Pin 14        P0.22
DC          Pin 12        P0.20
CS          Pin 16        P0.16
BackLight   Pin 2         P0.02
*/
SPI spi(P0_21, NC, P0_23); // mosi, miso, sclk - Not using MISO
DigitalOut Reset(P0_22);
DigitalOut DC(P0_20);
DigitalOut CS(P0_16);
DigitalOut Backlight(P0_2);
int display::begin()
{
    spi.frequency(4000000); // NRF51822 has a maximum SPI rate of 4MHz (slow :-< )
    
    BacklightOn();
    Reset = 1; // Drive reset high
    wait_ms(25);     
    Reset = 0; // Drive reset low
    wait_ms(25);
    Reset = 1; // Drive reset high
    wait_ms(25); // wait    
    
        
    writeCommand(0x1);  // software reset
    wait_ms(150); // wait   
    
    writeCommand(0x11);  //exit SLEEP mode
    wait_ms(25); // wait   
    
    writeCommand(0x3A); // Set colour mode        
    writeData8(0x55); // 16bits / pixel @ 64k colors 5-6-5 format 
    wait_ms(25); // wait   
    
    writeCommand(0x36);
    writeData8(0x68);//writeData8(0x08);  // RGB Format, rows are on the long axis.
    wait_ms(25); // wait   
    
    
    writeCommand(0x51); // maximum brightness
    wait_ms(25); // wait   
    
    writeCommand(0x21);    // display inversion off (datasheet is incorrect on this point)
    writeCommand(0x13);    // partial mode off                 
    writeCommand(0x29);    // display on
    wait_ms(25); // wait   
    writeCommand(0x2c);   // put display in to write mode    
    fillRectangle(0,0,SCREEN_WIDTH , SCREEN_HEIGHT  , 0);  // black out the screen           
    return 0;
    
    
    
}
uint8_t display::transferSPI8(uint8_t data)
{
    
    CS = 0;  
    spi.write(data);    
    CS = 1;
	return 0;
}

uint16_t display::transferSPI16(uint16_t data)
{
    
    CS = 0;
    spi.write(data & 0xff);    
    spi.write(data >> 8);    
    CS = 1;
    return 0;
}

void display::openAperture(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    // open up an area for drawing on the display        
    // This particular display module has an X and Y offset which is 
    // dealt with below
    x1 = x1 + 40;
    x2 = x2 + 40;
    y1 = y1 + 53;
    y2 = y2 + 53;
    DC = 0;
    transferSPI8(0x2A);
    DC = 1;
    transferSPI8(x1>>8);
    transferSPI8(x1&0xff);        
    transferSPI8(x2>>8);
    transferSPI8(x2&0xff);
    DC = 0;
    transferSPI8(0x2B);
    DC = 1;
    transferSPI8(y1>>8);
    transferSPI8(y1&0xff);        
    transferSPI8(y2>>8);
    transferSPI8(y2&0xff);    
        
    DC = 0; // put display in to data write mode
    transferSPI8(0x2c);
    DC = 1;

} 
void display::fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t Colour)
{       
    uint32_t pixelcount = height * width;
    openAperture(x, y, x + width - 1, y + height - 1);
    DC = 1;
    CS = 0;
    uint8_t lowbyte = Colour & 0xff;
    uint8_t highbyte = Colour >> 8;
    while(pixelcount--)
    {
        spi.write(lowbyte);    
        spi.write(highbyte);
    }
    CS = 1;
}
void display::BacklightOn()
{
    Backlight = 1;
}
void display::BacklightOff()
{
    Backlight = 0;
}
void display::writeCommand(uint8_t Cmd)
{
    DC = 0;
    transferSPI8(Cmd);
}
void display::writeData8(uint8_t Data)
{
    DC = 1;
    transferSPI8(Data);
}

void display::writeData16(uint16_t Data)
{
    DC = 1;
    transferSPI16(Data);
}
void display::putPixel(uint16_t x, uint16_t y, uint16_t colour)
{
    openAperture(x, y, x + 1, y + 1);
    writeData16(colour); 
}
void display::drawLineLowSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm    
  int dx = x1 - x0;
  int dy = y1 - y0;
  int yi = 1;
  if (dy < 0)
  {
    yi = -1;
    dy = -dy;
  }
  int D = 2*dy - dx;
  
  int y = y0;

  for (int x=x0; x <= x1;x++)
  {
    putPixel(x,y,Colour);    
    if (D > 0)
    {
       y = y + yi;
       D = D - 2*dx;
    }
    D = D + 2*dy;
    
  }
}

void display::drawLineHighSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour)
{
        // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

  int dx = x1 - x0;
  int dy = y1 - y0;
  int xi = 1;
  if (dx < 0)
  {
    xi = -1;
    dx = -dx;
  }  
  int D = 2*dx - dy;
  int x = x0;

  for (int y=y0; y <= y1; y++)
  {
    putPixel(x,y,Colour);
    if (D > 0)
    {
       x = x + xi;
       D = D - 2*dy;
    }
    D = D + 2*dx;
  }
}
void display::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    if ( iabs(y1 - y0) < iabs(x1 - x0) )
    {
        if (x0 > x1)
        {
            drawLineLowSlope(x1, y1, x0, y0, Colour);
        }
        else
        {
            drawLineLowSlope(x0, y0, x1, y1, Colour);
        }
    }
    else
    {
        if (y0 > y1) 
        {
            drawLineHighSlope(x1, y1, x0, y0, Colour);
        }
        else
        {
            drawLineHighSlope(x0, y0, x1, y1, Colour);
        }
        
    }
}
void display::drawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Colour)
{
    drawLine(x,y,x+w,y,Colour);
    drawLine(x,y,x,y+h,Colour);
    drawLine(x+w,y,x+w,y+h,Colour);
    drawLine(x,y+h,x+w,y+h,Colour);
}

void display::drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour)
{
// Reference : https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
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
        putPixel(x0 + x, y0 + y, Colour);
        putPixel(x0 + y, y0 + x, Colour);
        putPixel(x0 - y, y0 + x, Colour);
        putPixel(x0 - x, y0 + y, Colour);
        putPixel(x0 - x, y0 - y, Colour);
        putPixel(x0 - y, y0 - x, Colour);
        putPixel(x0 + y, y0 - x, Colour);
        putPixel(x0 + x, y0 - y, Colour);

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
}
void display::fillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour)
{
// Reference : https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
// Similar to drawCircle but fills the circle with lines instead
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
        drawLine(x0 - x, y0 + y,x0 + x, y0 + y, Colour);        
        drawLine(x0 - y, y0 + x,x0 + y, y0 + x, Colour);        
        drawLine(x0 - x, y0 - y,x0 + x, y0 - y, Colour);        
        drawLine(x0 - y, y0 - x,x0 + y, y0 - x, Colour);        

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
}
void display::putImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t * img) 
{
    uint16_t Colour;    
    openAperture(x, y, x + w - 1, y + h - 1);
    DC = 1;    
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            Colour = *(img++);
            transferSPI16(Colour);            
        }
    }    
}
void display::print(const char *Text, uint16_t len, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour)
{
        // This function draws each character individually.  It uses an array called TextBox as a temporary storage
    // location to hold the dots for the character in question.  It constructs the image of the character and then
    // calls on putImage to place it on the screen
    uint8_t Index = 0;
    uint8_t Row, Col;
    const uint8_t *CharacterCode = 0;    
    uint16_t TextBox[FONT_WIDTH * FONT_HEIGHT];
    for (Index = 0; Index < len; Index++)
    {
        CharacterCode = &Font5x7[FONT_WIDTH * (Text[Index] - 32)];
        Col = 0;
        while (Col < FONT_WIDTH)
        {
            Row = 0;
            while (Row < FONT_HEIGHT)
            {
                if (CharacterCode[Col] & (1 << Row))
                {
                    TextBox[(Row * FONT_WIDTH) + Col] = ForeColour;
                }
                else
                {
                    TextBox[(Row * FONT_WIDTH) + Col] = BackColour;
                }
                Row++;
            }
            Col++;
        }
        putImage(x, y, FONT_WIDTH, FONT_HEIGHT, (const uint16_t *)TextBox);
        x = x + FONT_WIDTH + 2;
    }
}
void display::print(uint16_t Number, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour)
{
     // This function converts the supplied number into a character string and then calls on puText to
    // write it to the display
    char Buffer[5]; // Maximum value = 65535
    Buffer[4] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[3] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[2] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[1] = Number % 10 + '0';
    Number = Number / 10;
    Buffer[0] = Number % 10 + '0';
    print(Buffer, 5, x, y, ForeColour, BackColour);
}