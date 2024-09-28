// internet_clock_GC9A010_ESP8266
// platform: ESP8266 Wemos d1 mini
// display: GC9A010 driven circular display 240*240 pixels
//
// clock by Bodmer - Clock example in TFT_eSPI library
// 
// adapted and modified  
// Floris Wouterlood
// November 1, 2022   
// public domain
   
   #include "SPI.h"
   #include "Adafruit_GFX.h"
   #include "Adafruit_GC9A01A.h"
   #include <NTPClient.h>
   #include <ESP8266WiFi.h>
   #include <WiFiUdp.h>

   const char* ssid =            "DogNet";                                                            // network wifi credentials  - fill in your wifi network name
   const char* password =        "XXXX";                                                            // network wifi credentials  - fill in your wifi key

   const long utcOffsetInSeconds = -4 * 60 * 60;     // UTC to EST
   char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


   #define TFT_DC D2
   #define TFT_CS D8
   #define DEG2RAD 0.0174532925   
   
// some extra colors
   #define BLACK      0x0000
   #define BLUE       0x001F
   #define RED        0xF800
   #define GREEN      0x07E0
   #define CYAN       0x07FF
   #define MAGENTA    0xF81F
   #define YELLOW     0xFFE0
   #define WHITE      0xFFFF
   #define ORANGE     0xFBE0
   #define GREY       0x84B5
   #define BORDEAUX   0xA000

   Adafruit_GC9A01A tft(TFT_CS, TFT_DC);

   float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;                              // saved H, M, S x & y multipliers
   float sdeg = 0, mdeg= 0, hdeg = 0;
   uint16_t osx = 120, osy = 120, omx = 120, omy = 120, ohx = 120, ohy = 120;          // saved H, M, S x & y coords
   uint16_t x0=0, x1=0, yy0=0, yy1=0;
   uint32_t targetTime = 0;                                                            // for next 1 second timeout

   int hh = 0;                                                                         // hours variable
   int mm = 0;                                                                         // minutes variable
   int ss = 0;                                                                         // seconds variable

   WiFiUDP ntpUDP;                                                                     // define NTP client to get time
   NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);  

bool initial = 1;

void setup(void) {
   tft.begin (); 
   tft.setRotation (2);
   tft.fillScreen (BLACK);  
   delay (200);
   tft.fillScreen (RED);
   delay (200);
   tft.fillScreen (GREEN);
   delay (200);
   tft.fillScreen (BLUE);
   delay (200);
   tft.fillScreen (BLACK);  
   delay (200);
   tft.fillScreen (GREY);

   createDial ();

   Serial.begin (9600);
   Serial.println ();
   Serial.println ();
   Serial.print ("connecting to ");
   Serial.println (ssid);  
   WiFi.begin (ssid, password);

   while (WiFi.status() != WL_CONNECTED ) 
      {
      delay (500);
      Serial.print (".");
      }
   Serial.print ("connection with ");
   Serial.println (ssid);  
   Serial.println ("-------------------------------"); 
   
   timeClient.begin();
   timeClient.update ();
   Serial.print ("internet server time: ");   
   Serial.println(timeClient.getFormattedTime());

   hh = timeClient.getHours ();
   mm = timeClient.getMinutes ();
   ss = timeClient.getSeconds ();

}

void loop() {
   if (targetTime < millis())
      {
      targetTime += 1000;
      ss++;                                                                            // advance second
      if (ss==60)
         {
         ss=0;
         mm++;                                                                         // advance minute
         if(mm>59)
            {
            mm=0;
            hh++;                                                                      // advance hour
            if (hh>23) 
               {
               hh=0;
               timeClient.update ();                                                   // update at midnight
               }
            }
         }
          
      // pre-compute hand degrees, x & y coords for a fast screen update
      sdeg = ss*6;                                                                     // 0-59 -> 0-354
      mdeg = mm*6+sdeg*0.01666667;                                                     // 0-59 -> 0-360 - includes seconds
      hdeg = hh*30+mdeg*0.0833333;                                                     // 0-11 -> 0-360 - includes minutes and seconds
      hx = cos ((hdeg-90)*DEG2RAD);    
      hy = sin ((hdeg-90)*DEG2RAD);
      mx = cos ((mdeg-90)*DEG2RAD);    
      my = sin ((mdeg-90)*DEG2RAD);
      sx = cos ((sdeg-90)*DEG2RAD);    
      sy = sin ((sdeg-90)*DEG2RAD);

      if (ss==0 || initial) 
         {
         initial = 0;
         tft.drawLine (ohx, ohy, 120, 121, BLACK);                                     // erase hour and minute hand positions every minute
         ohx = hx*62+121;    
         ohy = hy*62+121;
         tft.drawLine (omx, omy, 120, 121, BLACK);
         omx = mx*84+120;    
         omy = my*84+121;
         }
 
      tft.drawLine (osx, osy, 120, 121, BLACK);                                      // redraw new hand positions, hour and minute hands not erased here to avoid flicker
      osx = sx*90+121;    
      osy = sy*90+121;
      tft.drawLine (osx, osy, 120, 121, RED);
      tft.drawLine (ohx, ohy, 120, 121, WHITE);
      tft.drawLine (omx, omy, 120, 121, WHITE);
      tft.drawLine (osx, osy, 120, 121, RED);
      tft.fillCircle(120, 121, 3, RED);
      }
}

void createDial (){

   tft.setTextColor (WHITE, GREY);  
   tft.fillCircle(120, 120, 118, BORDEAUX);                                           // creates outer ring
   tft.fillCircle(120, 120, 110, BLACK);   

   for (int i = 0; i<360; i+= 30)                                                     // draw 12 line segments at the outer ring 
      {                                                   
      sx = cos((i-90)*DEG2RAD);
      sy = sin((i-90)*DEG2RAD);
      x0 = sx*114+120;
      yy0 = sy*114+120;
      x1 = sx*100+120;
      yy1 = sy*100+120;
      tft.drawLine(x0, yy0, x1, yy1, GREEN);
      }
                                                             
   for (int i = 0; i<360; i+= 6)                                                      // draw 60 dots - minute markers
      {
      sx = cos((i-90)*DEG2RAD);
      sy = sin((i-90)*DEG2RAD);
      x0 = sx*102+120;
      yy0 = sy*102+120;    
      tft.drawPixel(x0, yy0, WHITE);
    
      if(i==0  || i==180) tft.fillCircle (x0, yy0, 2, WHITE);                         // draw main quadrant dots
      if(i==90 || i==270) tft.fillCircle (x0, yy0, 2, WHITE);
     }
  
   tft.fillCircle(120, 121, 3, WHITE);                                               // pivot
   targetTime = millis() + 1000;   
}
