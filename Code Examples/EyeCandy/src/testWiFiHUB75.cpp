// /*
//  * Portions of this code are adapted from Aurora: https://github.com/pixelmatix/aurora
//  * Copyright (c) 2014 Jason Coon
//  *
//  * Portions of this code are adapted from LedEffects Plasma by Robert Atkins: https://bitbucket.org/ratkins/ledeffects/src/26ed3c51912af6fac5f1304629c7b4ab7ac8ca4b/Plasma.cpp?at=default
//  * Copyright (c) 2013 Robert Atkins
//  *
//  * Permission is hereby granted, free of charge, to any person obtaining a copy of
//  * this software and associated documentation files (the "Software"), to deal in
//  * the Software without restriction, including without limitation the rights to
//  * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
//  * the Software, and to permit persons to whom the Software is furnished to do so,
//  * subject to the following conditions:
//  *
//  * The above copyright notice and this permission notice shall be included in all
//  * copies or substantial portions of the Software.
//  *
//  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
//  * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
//  * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
//  * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//  */
 

// // HUB75E pinout
// // R1 | G1
// // B1 | GND
// // R2 | G2
// // B2 | E
// //  A | B
// //  C | D
// // CLK| LAT
// // OE | GND

// /*  Default library pin configuration for the reference
//   you can redefine only ones you need later on object creation

// #define R1 25
// #define G1 26
// #define BL1 27
// #define R2 14
// #define G2 12
// #define BL2 13
// #define CH_A 23
// #define CH_B 19
// #define CH_C 5
// #define CH_D 17
// #define CH_E -1 // assign to any available pin if using two panels or 64x64 panels with 1/32 scan
// #define CLK 16
// #define LAT 4
// #define OE 15

// */


// #include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
// #include <FastLED.h>
// #include <Arduino.h>
// #include <WiFi.h>
// #include <WebServer.h>
// // Configure for your panel(s) as appropriate!
// #define PANEL_WIDTH 64
// #define PANEL_HEIGHT 64  	// Panel height of 64 will required PIN_E to be defined.
// #define PANELS_NUMBER 1 	// Number of chained panels, if just a single panel, obviously set to 1

// #define PANE_WIDTH PANEL_WIDTH * PANELS_NUMBER
// #define PANE_HEIGHT PANEL_HEIGHT

// WebServer server(80);

// // placeholder for the matrix object
// MatrixPanel_I2S_DMA *dma_display = nullptr;

// String text;

// const PROGMEM char html_index[] = R"rawliteral(
// <!DOCTYPE html>
// <html>
// <head>
//   <title>ESP32 HUB75 MatrixPanel I2S DMA</title>
//   <style>
//     body {
//       font-family: Arial, sans-serif;
//       margin: 0;
//       padding: 0;
//     }
//     h1 {
//       text-align: center;
//       margin-top: 50px;
//     }
//     input {
//       display: block;
//       margin: 0 auto;
//       margin-top: 20px;
//       text-align: center;
//     }
//   </style>
// </head>
// <body>
//   <h1>ESP32 HUB75 MatrixPanel</h1>
//   <br>
//   <form action="/" method="post">
//     <input type="text" name="text" placeholder="Enter text to display">
//     <input type="submit" value="Display">
//   </form>
// </body>
// </html>
// )rawliteral";

// void serve() {
//   server.send(200, "text/html", html_index);
//   if (server.hasArg("text")) {
//     text = server.arg("text");
//     dma_display->fillScreen(0);
//     dma_display->setTextSize(1);
//     //server.send(200, "text/plain", "Displaying: " + text);
    
//   }
// }

// uint16_t CRGBtoRGB565(const CRGB &color) {
//   uint16_t r = (color.r >> 3) & 0x1F;
//   uint16_t g = (color.g >> 2) & 0x3F;
//   uint16_t b = (color.b >> 3) & 0x1F;
//   return (r << 11) | (g << 5) | b;
// }

// // Function to convert CHSV to RGB565
// uint16_t CHSVtoRGB565(const CHSV &hsv) {
//   CRGB rgb;
//   hsv2rgb_rainbow(hsv, rgb);
//   return CRGBtoRGB565(rgb);
// }

// void setup() {
  
//   Serial.begin(115200);
//   pinMode(0, INPUT);
//   while(digitalRead(0) == HIGH){
//     delay(10);
//   }
//   Serial.println(F("*****************************************************"));
//   Serial.println(F("*        ESP32-HUB75-MatrixPanel-I2S-DMA DEMO       *"));
//   Serial.println(F("*****************************************************"));

//   /*
//     The configuration for MatrixPanel_I2S_DMA object is held in HUB75_I2S_CFG structure,
//     pls refer to the lib header file for full details.
//     All options has it's predefined default values. So we can create a new structure and redefine only the options we need 

//     // those are the defaults
//     mxconfig.mx_width = 64;                   // physical width of a single matrix panel module (in pixels, usually it is always 64 ;) )
//     mxconfig.mx_height = 32;                  // physical height of a single matrix panel module (in pixels, usually almost always it is either 32 or 64)
//     mxconfig.chain_length = 1;                // number of chained panels regardless of the topology, default 1 - a single matrix module
//     mxconfig.gpio.r1 = R1;                    // pin mappings
//     mxconfig.gpio.g1 = G1;
//     mxconfig.gpio.b1 = B1;                    // etc
//     mxconfig.driver = HUB75_I2S_CFG::SHIFT;   // shift reg driver, default is plain shift register
//     mxconfig.double_buff = false;             // use double buffer (twice amount of RAM required)
//     mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;// I2S clock speed, better leave as-is unless you want to experiment
//   */

//   /*
//     For example we have two 64x64 panels chained, so we need to customize our setup like this

//   */
//   //HUB75_I2S_CFG mxconfig;
//   // mxconfig.mx_height = PANEL_HEIGHT;      // we have 64 pix heigh panels
//   // mxconfig.chain_length = PANELS_NUMBER;  // we have 2 panels chained
//   // mxconfig.gpio.e = PIN_E;                // we MUST assign pin e to some free pin on a board to drive 64 pix height panels with 1/32 scan
//   //mxconfig.driver = HUB75_I2S_CFG::FM6126A;     // in case that we use panels based on FM6126A chip, we can change that

//   /*
//     //Another way of creating config structure
//     //Custom pin mapping for all pins*/
//     HUB75_I2S_CFG::i2s_pins _pins={r1, r2, g1, g2, b1, b2, A, B, C, D, E, LAT, OE, CLK};
//     HUB75_I2S_CFG mxconfig(
//                             64,   // width
//                             64,   // height
//                              1,   // chain length
//                          _pins,   // pin mapping
//       HUB75_I2S_CFG::ICN2038S    // driver chip
//     );
    

//     mxconfig.clkphase = 0; // 0 or 180 degrees`


//   // OK, now we can create our matrix object
//   dma_display = new MatrixPanel_I2S_DMA(mxconfig);

//   // let's adjust default brightness to about 75%
//   dma_display->setBrightness8(255);    // range is 0-255, 0 - 0%, 255 - 100%

//   // Allocate memory and start DMA display
//   if( not dma_display->begin() )
//       Serial.println("****** !KABOOM! I2S memory allocation failed ***********");
 

//   dma_display->fillScreen(0xF800);
//   dma_display->setRotation(0);
//   WiFi.softAP("HUB75 Display");
//   WiFi.mode(WIFI_AP);
//   Serial.println("WiFi AP started");
//   Serial.println(WiFi.softAPIP());
//   server.on("/", serve);
//   server.begin();
//   log_d("Total heap: %d", ESP.getHeapSize());
//   log_d("Free heap: %d", ESP.getFreeHeap());
//   log_d("Total PSRAM: %d", ESP.getPsramSize());
//   log_d("Free PSRAM: %d", ESP.getFreePsram());
// }
// int h = 0;
// void loop() {
//   server.handleClient();
//   h++;
//   dma_display->setCursor(0, 0);
//   for(int i = 0; i < text.length(); i++) {
//     dma_display->print(text.charAt(i));
//     if(text.charAt(i) == ' ') continue;
//     dma_display->setTextColor(CHSVtoRGB565(CHSV((h+(i * 50))/5, 255, 255)));
//   }
  
    
//     if(h == 255*5) h = 0;
// } // end loop