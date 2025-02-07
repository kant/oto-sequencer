// https://arduino.stackexchange.com/questions/22762/why-does-the-arduino-respond-so-slow-to-serial-input// 
// Example of processing incoming serial data without blocking.

// how much serial data we expect before the tab char appears
const unsigned int MAX_INPUT = 500;

//Libraries 
#include "TFT_eSPI.h" //TFT LCD library 
#include "Free_Fonts.h" //include the font library

//Initializations
TFT_eSPI tft; //Initializing TFT LCD
TFT_eSprite spr = TFT_eSprite(&tft); //Initializing buffer
int y_pos{5};
int row_h{30};
int max_height{200};
void setup() {
  Serial.begin(115200); //start serial communication 
  tft.begin(); //Start TFT LCD
  tft.setRotation(3); //Set LCD rotation
  spr.createSprite(TFT_HEIGHT,TFT_WIDTH); //Create buffer
  spr.fillSprite(tft.color565(0,0,0)); //Fill background with white color
 spr.pushSprite(0,0); //Push to LCD

}


// void setup ()
//   {
//   Serial.begin (115200);
//   } // end of setup

// here to process incoming serial data after a terminator received
void process_data (const char * data)
  {
    String str{data};
  // for now just display it
  // (but you could compare it to some value, convert to an integer, etc.)
//  Serial.println (data);
  // if (y_pos > max_height)
  // {
  //   // clear screen
  //   spr.fillSprite(tft.color565(0,0,0)); 
  //   y_pos = 0;
  // }


  spr.setTextColor(TFT_WHITE); //set text color
  spr.setFreeFont(FMB18); //set font 
 // spr.drawString(data,0,y_pos); //draw string 
//   y_pos += row_h;
  // now draw it line by line
 spr.fillSprite(tft.color565(0,0,0)); //Fill background with white color
 
  int startPos = 0;
  int endPos = 0; 
  int y = 5;
  do{
    endPos = str.indexOf("\n", startPos);
    spr.drawString(str.substring(startPos, endPos),5,y); //draw string 
    y += 25;
    startPos = endPos + 1;
  } while (endPos != -1);

  spr.pushSprite(0,0); //Push to LCD



  }  // end of process_data

void processIncomingByte (const byte inByte)
  {
  static char input_line [MAX_INPUT];
  static unsigned int input_pos = 0;

  switch (inByte)
    {

    case '\t':   // end of text
      input_line [input_pos] = 0;  // terminating null byte

      // terminator reached! process input_line here ...
      process_data (input_line);

      // reset buffer for next time
      input_pos = 0;  
      break;

    case '\r':   // discard carriage return
      break;

    default:
      // keep adding if not full ... allow for terminating null byte
      if (input_pos < (MAX_INPUT - 1))
        input_line [input_pos++] = inByte;
      break;

    }  // end of switch

  } // end of processIncomingByte  

void loop()
  {
  // if serial data available, process it
  while (Serial.available () > 0)
    processIncomingByte (Serial.read ());

  // do other stuff here like testing digital input (button presses) ...

  }  // end of loop
