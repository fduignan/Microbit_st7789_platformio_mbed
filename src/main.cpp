#include <mbed.h>
#include "display.h"
Serial pc(P0_24, P0_25);
display Display;
int main() {

  int Count = 0;
  // put your setup code here, to run once:
  Display.begin();
  Display.drawRectangle(0, 0 , 239, 134, RGBToWord(0xff,0xff,0xff));    
  Display.print(DISP_STR("Hello World"),10,10,RGBToWord(0xff,0xff,0xff),0);    
  Display.drawRectangle(50,50,10,10,RGBToWord(0xff,0,0));
  Display.fillRectangle(70,50,10,10,RGBToWord(0xf,0xf,0xff));
  Display.drawCircle(100, 70, 10 , RGBToWord(0xff,0xff,0));
  Display.fillCircle(130, 70, 10 , RGBToWord(0,0xff,0));
  while(1) {
    // put your main code here, to run repeatedly:
    pc.printf("H\r\n");
    Display.print(DISP_STR("Count="),10,25,RGBToWord(0,0xff,0xff),0);
    Display.print(Count++,55,25,RGBToWord(0,0xff,0xff),0);

  
  }
}