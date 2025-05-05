// ESP32_LEDMatrix_I2S
//
// Example sketch which shows how to display a 64x64
// animated GIF image stored in FLASH memory
// on a 64x64 LED matrix

// Written for the I2S DMA Matrix Library for the ESP32
// https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA

// To display a GIF from memory, a single callback function
// must be provided - GIFDRAW
// This function is called after each scan line is decoded
// and is passed the 8-bit pixels, RGB565 palette and info
// about how and where to display the line. The palette entries
// can be in little-endian or big-endian order; this is specified
// in the begin() method.
//
// The AnimatedGIF class doesn't allocate or free any memory, but the
// instance data occupies about 22.5K of RAM.
//


// This reference doesn't work on Windows anyways
#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <time.h>
#include <AnimatedGIF.h>

long timezone = -5; 
byte daysavetime = 1;


//#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <weatherlib.h>
// This is the library for interfacing with the display

// Can be installed from the library manager (Search for "ESP32 MATRIX DMA")
// https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA

// -------------------------------------
// -------   Matrix Config   ------
// -------------------------------------



MatrixPanel_I2S_DMA *dma_display = nullptr;



AnimatedGIF gif;
File f;
//THIS IS FOR THE BIG DISPLAY
//Each panel is "rotated" 90 degrees from what the library expects
//This will translate the coordinates to the correct ones
//Always use virtualDisp->... isntead of dma_display->...

const int panelResX = 64;  // Number of pixels wide of each INDIVIDUAL panel module.
const int panelResY = 64;  // Number of pixels tall of each INDIVIDUAL panel module.
const int panel_chain = 6; // Total number of panels chained one to another

//------------------------------------------------------------------------------------------------------------------
struct CustomMirrorScanTypeMapping {

  static VirtualCoords apply(VirtualCoords coords, int vy, int pb) {

    // coords are the input coords for adjusting

    int width  = 64*6;
    int height = 64;	
    int x = coords.x;
    coords.x = (x/64) * 64 + coords.y;
    coords.y = 64 - 1 - x%64;   
    
    return coords;

  }
};

VirtualMatrixPanel_T<CHAIN_NONE, CustomMirrorScanTypeMapping> *virtualDisp = nullptr;



// Draw a line of image directly on the LCD
void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y; // current line

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) // restore to background color
  {
    for (x = 0; x < iWidth; x++)
    {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency) // if transparency used
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + pDraw->iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < pDraw->iWidth)
    {
      c = ucTransparent - 1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent) // done, stop
        {
          s--; // back up to treat it like transparent
        }
        else // opaque
        {
          *d++ = usPalette[c];
          iCount++;
        }
      }           // while looking for opaque pixels
      if (iCount) // any opaque pixels?
      {
        for (int xOffset = 0; xOffset < iCount; xOffset++)
        {
          //log_d("before draw pix");
          virtualDisp->drawPixel(x + xOffset + pDraw->iX, y, usTemp[xOffset]);
          //log_d("after draw pix");
        }
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent)
          iCount++;
        else
          s--;
      }
      if (iCount)
      {
        x += iCount; // skip these
        iCount = 0;
      }
    }
  }
  else
  {
    s = pDraw->pPixels;
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    for (x = 0; x < pDraw->iWidth; x++)
    {
      //log_d("before draw pix");
      virtualDisp->drawPixel(x + pDraw->iX, y, usPalette[*s++]);
      //log_d("after draw pix");
    }
  }
  //virtualDisp->flipDMABuffer();
} /* GIFDraw() */

void * GIFOpenFile(const char *fname, int32_t *pSize)
{
  f = SPIFFS.open(fname);
  if (f)
  {
    *pSize = f.size();
    return (void *)&f;
  }
  return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
     f->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < iLen)
       iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (iBytesRead <= 0)
       return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{ 
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
//  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
} /* GIFSeekFile() */


void displaySetup()
{
    HUB75_I2S_CFG::i2s_pins _pins={r1, r2, g1, g2, b1, b2, A, B, C, D, E, LAT, OE, CLK};
    HUB75_I2S_CFG mxconfig(
                            64,   // width
                            64,   // height
                             6,   // chain length
                         _pins,   // pin mapping
      HUB75_I2S_CFG::FM6124    // driver chip
    );
    

    mxconfig.clkphase = 1; // 0 or 180 degrees`
    mxconfig.i2sspeed = mxconfig.HZ_20M;
    mxconfig.setPixelColorDepthBits(1);
    mxconfig.double_buff = true;
    //mxconfig.double_buff
  // OK, now we can create our matrix object
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  // let's adjust default brightness to about 75%
  //dma_display->setBrightness8(255);

  // If you are using a 64x64 matrix you need to pass a value for the E pin
  // The trinity connects GPIO 18 to E.
  // This can be commented out for any smaller displays (but should work fine with it)

  // May or may not be needed depending on your matrix
  // Example of what needing it looks like:
  // https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/issues/134#issuecomment-866367216
  //mxconfig.clkphase = false;

  // Some matrix panels use different ICs for driving them and some of them have strange quirks.
  // If the display is not working right, try this.
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();

  virtualDisp = new VirtualMatrixPanel_T<CHAIN_NONE, CustomMirrorScanTypeMapping>(1, 6, 64, 64); 
  virtualDisp->setDisplay(*dma_display);
  dma_display->setBrightness(100);
}

Weatherlib weather("REDACTED", "Middletown", "US", "PA");

void setup()
{
  //Serial.begin(115200);
  displaySetup();
  virtualDisp->clearScreen();
  gif.begin(LITTLE_ENDIAN_PIXELS);
  
  if (!SPIFFS.begin(false))
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS initialized.");
  if(SPIFFS.open("/fireworks.gif")) Serial.println("working");
   WiFi.mode(WIFI_AP_STA);
   WiFi.softAP("REDACTED", "REDACTED"); // Create an access point
   WiFi.begin("psu-personal");
   ArduinoOTA.setHostname("gif-display"); // Set OTA hostname
   ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      //virtualDisp->fillRect(0, 0, (progress / (total / 100)*64*6), 64, 0x07e0);
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
  xTaskCreatePinnedToCore(
    [](void *pvParameters) {
      ArduinoOTA.begin();
      while (true) {
        ArduinoOTA.handle(); // Handle OTA requests
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
    },
    "ArduinoOTA",
    1024 * 5,
    NULL,
    5,
    NULL,
    ARDUINO_RUNNING_CORE); // Run on core 1, leave core 0 for the main loop
    Serial.println("OTA initialized.");
    int tmo = 0;
    while(!WiFi.isConnected() && tmo++ < 15000) delay(1);
    virtualDisp->clearScreen();
    virtualDisp->setCursor(0, 10);
    virtualDisp->setTextSize(1);
    virtualDisp->setTextColor(0x07e0);
    if(WiFi.isConnected()) {
        virtualDisp->println("Connected to WiFi");
        virtualDisp->print("IP: " + WiFi.localIP().toString());
    }
    else virtualDisp->print("No WiFi");
    virtualDisp->setCursor(0, 10);
    virtualDisp->setTextColor(0xF900);
    virtualDisp->setFont(&FreeSansBold24pt7b);
    //delay(3000);
    // byte* mac = new byte[8];
    // esp_efuse_mac_get_default(mac);
    // for(int i = 0; i < 8; i++) {
    //   virtualDisp->print(mac[i], HEX);
    //   virtualDisp->print(":");
    // }
    // while(1) delay(10000);
    configTime(3600*timezone, daysavetime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");

}
int timeout = 0;
const char* message = "More Capstone Projects Upstairs!";
int shift = 1;
void loop()
{
//   File root = SPIFFS.open("/");
//   String name = root.getNextFileName(); // Get the first file name in the directory
//   while(!name.equals("") && false) {// This is just to ensure the SPIFFS is still open and valid
//   Serial.println("Opening GIF file: " + name);
//   // put your main code here, to run repeatedly:
//    if (gif.open(String(name).c_str(), GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
//   {
//     Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
//     while (gif.playFrame(true, NULL))
//     {
//         virtualDisp->flipDMABuffer();
//     }
//   }
//   gif.close();
//   name = root.getNextFileName();
//   }
//   root.close();
  if(WiFi.isConnected()) {
    virtualDisp->setTextColor(0x0015);
    virtualDisp->clearScreen();
    virtualDisp->setTextSize(1);
    virtualDisp->setFont(&FreeSansBold24pt7b);
	struct tm tmstruct;
    static int lastMin = -1;
    tmstruct.tm_year = 0;
    getLocalTime(&tmstruct, 5000);
    if(tmstruct.tm_hour > 9) virtualDisp->setCursor(135, 47);
    else virtualDisp->setCursor(150, 47);
    if(tmstruct.tm_hour > 12)
    virtualDisp->print(tmstruct.tm_hour - 12);
    else 
    virtualDisp->print(tmstruct.tm_hour);
    virtualDisp->print(":");
    if(tmstruct.tm_min < 10) virtualDisp->print("0");
    virtualDisp->print(tmstruct.tm_min);

    if(tmstruct.tm_min != lastMin) {
      lastMin = tmstruct.tm_min;
      weather.updateWeatherData();
    }
    virtualDisp->setFont(&FreeSansBold18pt7b);
    virtualDisp->setCursor(275, 47);
    float temp = weather.getWeather().temperature;
    virtualDisp->printf("%0.1f", temp);
    virtualDisp->setFont();
    virtualDisp->setCursor(virtualDisp->getCursorX() + 3, 20);
    virtualDisp->setTextSize(1);
    virtualDisp->print("o");
    virtualDisp->setFont(&FreeSansBold18pt7b);
    virtualDisp->setCursor(virtualDisp->getCursorX(), 47);
    virtualDisp->print("F");
    switch(weather.getWeather().icon) {
        case 0x04d:
        virtualDisp->fillCircle(36, 36, 20, 0xFFFF);
        virtualDisp->fillCircle(50, 26, 20, 0xFFFF);
        virtualDisp->fillCircle(72, 36, 20, 0xFFFF);
        virtualDisp->fillRect(33, 36, 42, 21, 0xFFFF);
        break;

        case 0x03d:
        virtualDisp->fillCircle(31, 18, 15, 0xF760);
        virtualDisp->fillCircle(35, 36, 20, 0xFFFF);
        virtualDisp->fillCircle(49, 26, 20, 0xFFFF);
        virtualDisp->fillCircle(71, 36, 20, 0xFFFF);
        virtualDisp->fillRect(32, 36, 42, 21, 0xFFFF);
        virtualDisp->drawLine(17, 18, 9, 15, 0xF760);
        virtualDisp->drawLine(18, 15, 11, 10, 0xF760);
        virtualDisp->drawLine(21, 10, 14, 5, 0xF760);
        virtualDisp->drawLine(27, 7, 17, 1, 0xF760);
        virtualDisp->drawLine(28, 6, 24, 0, 0xF760);
        virtualDisp->drawLine(29, 4, 29, 0, 0xF760);
        virtualDisp->drawLine(33, 4, 33, 0, 0xF760);
        virtualDisp->drawLine(35, 4, 39, 0, 0xF760);
        virtualDisp->drawLine(17, 22, 10, 25, 0xF760);
        virtualDisp->drawLine(37, 6, 45, 0, 0xF760);
        break;

        default:
        virtualDisp->setFont();
        virtualDisp->setTextColor(0x07e0);
        virtualDisp->setCursor(0, 10);
        virtualDisp->println(weather.getWeather().condition);
        virtualDisp->println(weather.getWeather().icon, HEX);
        break;
    }
    virtualDisp->flipDMABuffer();
    delay(5000);
  }
int16_t xOne, yOne;
uint16_t w, h;
  virtualDisp->setTextSize(1);
  virtualDisp->setFont(&FreeSansBold24pt7b);
  virtualDisp->clearScreen();
  virtualDisp->flipDMABuffer();
  virtualDisp->clearScreen();
  for(int i = 64*6; i > -850; i-=shift) {
  virtualDisp->setCursor(i, 47);
  bool textColor = false;
  for(int k = 0; k < strlen(message); k++) {
    if(textColor) virtualDisp->setTextColor(0xFFFF);
    else virtualDisp->setTextColor(0x0015);
    textColor = !textColor;
    virtualDisp->print(message[k]);
  }
  //delay(1);
  virtualDisp->getTextBounds(message, i, 47, &xOne, &yOne, &w, &h);
  int arrowX = xOne + w + 30;
  if(arrowX <= 64*6) {
    virtualDisp->fillRect(arrowX, 15, 64, 32, 0xFFFF);
    virtualDisp->fillTriangle(arrowX + 62, 12, arrowX + 62, 49, arrowX + 94, 31, 0xFFFF);

  }
  virtualDisp->flipDMABuffer();
  //if(arrowX > 64*6 && arrowX < 64 * 7) 
  virtualDisp->clearScreen();
  /*else {
   virtualDisp->setTextColor(0x0);
   virtualDisp->setCursor(i + shift, 47);
   virtualDisp->print(message);
   virtualDisp->fillRect(arrowX + shift, 15, 64, 32, 0x0);
  }*/
    //virtualDisp->clearScreen();
}
}