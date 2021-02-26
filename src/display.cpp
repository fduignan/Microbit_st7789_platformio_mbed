#include "mbed.h"
#include "display.h"
#include "font5x7.h"
extern Serial g_Serial_pc;
//mosi,miso,clk,ss
SPI spi(P0_25,P0_26,P0_27);
DigitalOut Rst(P0_7);
DigitalOut DC(P0_6);
DigitalOut D_CS(P0_28);
DigitalOut T_CS(P0_5);
DigitalIn PenIRQ(P0_4);
void Display::begin()
{

    spi.format(8, 0);
    spi.frequency(8000000);
    T_CS = 1; // de-assert the touch interface for now
    D_CS = 1;

    resetDisplay();
    
    LCD_Write_Data(0x21);   // Set GVDD  (varies contrast)
    
    LCD_Write_Cmd(0xC0);    // Power control 
    LCD_Write_Data(0x21);   // Set GVDD  (varies contrast)

    LCD_Write_Cmd(0xC1);    // Power control 
    LCD_Write_Data(0x10);   // default value

    LCD_Write_Cmd(0xC5);    //VCM control 
    LCD_Write_Data(0x31);   // default values
    LCD_Write_Data(0x3c);   // 

    LCD_Write_Cmd(0xC7);    //VCM control2 
    LCD_Write_Data(0xc0);   // default value

    LCD_Write_Cmd(0x36);    // Memory Access Control 
    LCD_Write_Data(0x48);   // Set display orientation and RGB colour order

    LCD_Write_Cmd(0x3A);    // Set Pixel format
    LCD_Write_Data(0x55);   // To 16 bits per pixel

    LCD_Write_Cmd(0xB1);    // Frame rate control
    LCD_Write_Data(0x00);   // Use Fosc without divisor
    LCD_Write_Data(0x1B);   // set 70Hz refresh rate

    LCD_Write_Cmd(0xB6);    // Display Function Control 
    LCD_Write_Data(0x00);   // Use default values
    LCD_Write_Data(0x82);
    LCD_Write_Data(0x27);      
    LCD_Write_Cmd(0x11);    //Exit Sleep 
    wait(0.120); 

    LCD_Write_Cmd(0x29);    //Display on 
    LCD_Write_Cmd(0x2c); 
    wait(0.005);
    
    fillRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,RGBToWord(0x00,0,0));
    D_CS = 1;

}

void Display::CommandMode()
{
    DC = 0;
}
void Display::DataMode()
{
    DC = 1;
}
void Display::LCD_Write_Cmd(uint8_t cmd)
{
    
    CommandMode();
    D_CS = 0;
    spi.write(cmd);
    D_CS = 1;
}
void Display::LCD_Write_Data(uint8_t data)
{
    
    DataMode();
    D_CS = 0;
    spi.write(data);
    D_CS = 1;
}
void Display::resetDisplay()
{
    Rst = 0;
    wait(0.01);
    Rst = 1;
    wait(0.12);
}
void Display::openAperture(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{   
    LCD_Write_Cmd(0x2a); 
    LCD_Write_Data(x1>>8);
    LCD_Write_Data(x1);
    LCD_Write_Data(x2>>8);
    LCD_Write_Data(x2);
    LCD_Write_Cmd(0x2b); 
    LCD_Write_Data(y1>>8);
    LCD_Write_Data(y1);
    LCD_Write_Data(y2>>8);
    LCD_Write_Data(y2);
    LCD_Write_Cmd(0x2c); 
}
// Simple Pixel drawing routines
void Display::putPixel(uint16_t x, uint16_t y, uint16_t Colour)
{
    uint16_t rx;
    
    openAperture(x,y,x+1,y+1);
    DataMode();
    D_CS = 0;
    spi.write((const char *) &Colour,2,(char *)&rx,0);
    D_CS = 1;
}
void Display::putImage(uint16_t xofs, uint16_t yofs, uint16_t width, uint16_t height, const uint16_t *Image)
{
    uint16_t rx;
    uint16_t Colour;
    
    uint16_t x,y;
    for (y = 0; y < height; y++) {
        for (x=0; x < width; x++)
        {
            Colour = *(Image++);            
            putPixel(x+xofs,y+yofs,Colour);            
        }    
    }
}
void Display::fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t Colour)
{
    /* in an effort to reduce overhead here a large word buffer is created.  If the number of words to 
    to write is greater than or equal to the size of this buffer then it is filled with the relevant colour
    and used as the source of data in the spi.write function.  This greatly speeds up the writing of 
    large arease of the screen.  The choice of buffer size is interesting.
    */ 
    #define BUF_SIZE 32
    uint16_t rx;
    uint16_t txBuffer[BUF_SIZE];
    int count;    
    openAperture(x,y,width,height);
    DataMode();
    D_CS = 0;
    count = (width*height)/BUF_SIZE;
    if (count)
    { // big area
        for (int i=0;i<BUF_SIZE;i++)
        {
            txBuffer[i]=Colour;
        }
        while(count--)
        {
            spi.write((const char *)txBuffer,2*BUF_SIZE,(char *)&rx,0);
        }
        count = (width*height)%BUF_SIZE; // write remainder of block
        while (count--)
        {
            spi.write((const char *) &Colour,2,(char *)&rx,0); // write a block of colour
        }   
        
    }        
    else
    {
        // small area so write 1 word at a time.
        count = (width*height);
        while(count--)
        {
            spi.write((const char *) &Colour,2,(char *)&rx,0);
        }
        
    }
    D_CS = 1;
}

void Display::drawLineLowSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour)
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

void Display::drawLineHighSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour)
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
void Display::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    D_CS = 0;
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
    D_CS = 1;
}


uint16_t Display::RGBToWord(uint16_t R, uint16_t G, uint16_t B)
{
    uint16_t rvalue = 0;
    rvalue += G >> 5;
    rvalue += (G & (0b111)) << 13;
    rvalue += (B >> 3) << 8;
    rvalue += (R >> 3) << 3;
    return rvalue;
}
void Display::drawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Colour)
{
    drawLine(x,y,x+w,y,Colour);
    drawLine(x,y,x,y+h,Colour);
    drawLine(x+w,y,x+w,y+h,Colour);
    drawLine(x,y+h,x+w,y+h,Colour);
}

void Display::drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour)
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
void Display::fillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour)
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
void Display::print(const char *Text, uint16_t len, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour)
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
void Display::print(uint16_t Number, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour)
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
void Display::initTouch(void)
{
    // This display module has an XPT2046 touch screen controller connected over the SPI interface
    // The XPT2046 
    T_CS = 0;
    spi.write(0b10011000); // 8 bit conversion, ratiometric measurement, read XP
    //wait_us(100);
    uint16_t indata,outdata;
    outdata = 0xffff;
    spi.write((const char *) &outdata,2,(char *)&indata,2); // write a block of colour
    T_CS = 1;
}
uint16_t Display::readYTouch(void)
{
    // This display module has an XPT2046 touch screen controller connected over the SPI interface
    // The XPT2046 
    /*
    Format of command word : 
    b7 : must be 1
    b6,5,4 : A2,A1,A0
    000 : temperature

    b3 : Mode
    b2 : SER,DFR
    b1,0 : PD1,PD0
    */
    uint16_t indata,outdata;    
    spi.frequency(2000000); // Can't run the touch screen controller at high speed
    T_CS = 0;                
    spi.write((uint8_t)0b10011000); // 12 bit conversion, ratiometric measurement, reference off
                                    // read XP which tells us Y (see diagram in datasheet)
    wait_us(20);    
    outdata = 0xffff;
    spi.write((const char *) &outdata,2,(char *)&indata,2); 
    T_CS = 1;        
    spi.frequency(8000000);   
    int calibrated_return= (indata>>12)+((indata & 0xff)<<4);
    calibrated_return = (calibrated_return - Y_TOUCH_MIN); 
    if (calibrated_return < 0)
        calibrated_return = 0;
    calibrated_return = (calibrated_return * SCREEN_HEIGHT) / (Y_TOUCH_MAX - Y_TOUCH_MIN) ;
    return calibrated_return;
}
uint16_t Display::readXTouch(void)
{
    // This display module has an XPT2046 touch screen controller connected over the SPI interface
    // The XPT2046 
    /*
    Format of command word : 
    b7 : must be 1
    b6,5,4 : A2,A1,A0
    000 : temperature

    b3 : Mode
    b2 : SER,DFR
    b1,0 : PD1,PD0
    */
    uint16_t indata,outdata;     
    spi.frequency(2000000);// Can't run the touch screen controller at high speed
    T_CS = 0;                
    spi.write((uint8_t)0b11011000); // 12 bit conversion, ratiometric measurement, reference off
                                    // read YP which tells us X (see diagram in datasheet)
    wait_us(20);    
    outdata = 0xffff;
    spi.write((const char *) &outdata,2,(char *)&indata,2); 
    T_CS = 1;        
    spi.frequency(8000000);    
    int calibrated_return= (indata>>12)+((indata & 0xff)<<4);    
    calibrated_return = (calibrated_return - X_TOUCH_MIN);
    if (calibrated_return < 0)
        calibrated_return = 0;
    calibrated_return = (calibrated_return * SCREEN_WIDTH) / (X_TOUCH_MAX - X_TOUCH_MIN);
    // The X reading is backwards so need to subtract it from the screen width
    calibrated_return = SCREEN_WIDTH - calibrated_return;
    if (calibrated_return < 0)
        calibrated_return = 0;
    return calibrated_return;
}
int Display::penDown(void)
{           
    if (PenIRQ.read()==0)
        return 1;
    else
        return 0;
}