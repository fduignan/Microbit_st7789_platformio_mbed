#include <stdint.h>
// Define a macro to allow easy definition of colours
// Format of colour value: <BGND 1 bit><Red 5 bits><Green 5 bits><Blue 5 bits>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define Y_TOUCH_MAX 1952
#define Y_TOUCH_MIN 88
#define X_TOUCH_MAX 1920
#define X_TOUCH_MIN 112
class Display {
public:        
    Display() {};
    void begin();
    void putPixel(uint16_t x, uint16_t y, uint16_t colour);    
    void putImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *Image);
    void fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t Colour);
    uint16_t RGBToWord(uint16_t R, uint16_t G, uint16_t B);
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Colour);
    void drawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Colour);
    void drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour);
    void fillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour);
    // Graphics text functions
    void print(const char *Text, uint16_t len, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour);
    void print(uint16_t Number, uint16_t x, uint16_t y, uint16_t ForeColour, uint16_t BackColour); 
    int penDown(void);
    uint16_t readYTouch(void);
    uint16_t readXTouch(void);
private:

    void CommandMode();
    void DataMode();
    void LCD_Write_Cmd(uint8_t cmd);
    void LCD_Write_Data(uint8_t data);
    void resetDisplay();
    void openAperture(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
    void drawLineLowSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour);
    void drawLineHighSlope(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t Colour);
    int iabs(int x) // simple integer version of abs for use by graphics functions
    {
        if (x < 0)
            return -x;
        else
            return x;
    }
    void initTouch(void);
    

};