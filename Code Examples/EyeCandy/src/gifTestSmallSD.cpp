// // ESP32_LEDMatrix_I2S
// //
// // Example sketch which shows how to display a 64x64
// // animated GIF image stored in FLASH memory
// // on a 64x64 LED matrix

// // Written for the I2S DMA Matrix Library for the ESP32
// // https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA

// // To display a GIF from memory, a single callback function
// // must be provided - GIFDRAW
// // This function is called after each scan line is decoded
// // and is passed the 8-bit pixels, RGB565 palette and info
// // about how and where to display the line. The palette entries
// // can be in little-endian or big-endian order; this is specified
// // in the begin() method.
// //
// // The AnimatedGIF class doesn't allocate or free any memory, but the
// // instance data occupies about 22.5K of RAM.
// //


// // This reference doesn't work on Windows anyways
// #include <Arduino.h>
// #include <SD_MMC.h>

// // If it doesn't work:
// // - Comment it out
// // - Create a new tab (little arrow top right)
// // - Call it "homer_tiny.h"
// // - copy in this text: https://raw.githubusercontent.com/bitbank2/AnimatedGIF/master/test_images/homer_tiny.h
// // - uncomment the following

// //#include "homer_tiny.h"


// // ----------------------------
// // Additional Libraries - each one of these will need to be installed.
// // ----------------------------

// #include <AnimatedGIF.h>

// #include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
// // This is the library for interfacing with the display

// // Can be installed from the library manager (Search for "ESP32 MATRIX DMA")
// // https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA

// // -------------------------------------
// // -------   Matrix Config   ------
// // -------------------------------------

// const int panelResX = 64;  // Number of pixels wide of each INDIVIDUAL panel module.
// const int panelResY = 64;  // Number of pixels tall of each INDIVIDUAL panel module.
// const int panel_chain = 2; // Total number of panels chained one to another

// // See the "displaySetup" method for more display config options

// //------------------------------------------------------------------------------------------------------------------

// MatrixPanel_I2S_DMA *dma_display = nullptr;
// AnimatedGIF gif;
// File f;

// // Draw a line of image directly on the LCD
// void GIFDraw(GIFDRAW *pDraw)
// {
//   uint8_t *s;
//   uint16_t *d, *usPalette, usTemp[320];
//   int x, y, iWidth;

//   usPalette = pDraw->pPalette;
//   y = pDraw->iY + pDraw->y; // current line

//   s = pDraw->pPixels;
//   if (pDraw->ucDisposalMethod == 2) // restore to background color
//   {
//     for (x = 0; x < iWidth; x++)
//     {
//       if (s[x] == pDraw->ucTransparent)
//         s[x] = pDraw->ucBackground;
//     }
//     pDraw->ucHasTransparency = 0;
//   }
//   // Apply the new pixels to the main image
//   if (pDraw->ucHasTransparency) // if transparency used
//   {
//     uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
//     int x, iCount;
//     pEnd = s + pDraw->iWidth;
//     x = 0;
//     iCount = 0; // count non-transparent pixels
//     while (x < pDraw->iWidth)
//     {
//       c = ucTransparent - 1;
//       d = usTemp;
//       while (c != ucTransparent && s < pEnd)
//       {
//         c = *s++;
//         if (c == ucTransparent) // done, stop
//         {
//           s--; // back up to treat it like transparent
//         }
//         else // opaque
//         {
//           *d++ = usPalette[c];
//           iCount++;
//         }
//       }           // while looking for opaque pixels
//       if (iCount) // any opaque pixels?
//       {
//         for (int xOffset = 0; xOffset < iCount; xOffset++)
//         {
//           dma_display->drawPixel(x + xOffset + pDraw->iX, y, usTemp[xOffset]);
//         }
//         x += iCount;
//         iCount = 0;
//       }
//       // no, look for a run of transparent pixels
//       c = ucTransparent;
//       while (c == ucTransparent && s < pEnd)
//       {
//         c = *s++;
//         if (c == ucTransparent)
//           iCount++;
//         else
//           s--;
//       }
//       if (iCount)
//       {
//         x += iCount; // skip these
//         iCount = 0;
//       }
//     }
//   }
//   else
//   {
//     s = pDraw->pPixels;
//     // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
//     for (x = 0; x < pDraw->iWidth; x++)
//     {
//       dma_display->drawPixel(x + pDraw->iX, y, usPalette[*s++]);
//     }
//   }
// } /* GIFDraw() */

// void displaySetup()
// {
//     HUB75_I2S_CFG::i2s_pins _pins={r1, r2, g1, g2, b1, b2, A, B, C, D, E, LAT, OE, CLK};
//     HUB75_I2S_CFG mxconfig(
//                             64,   // width
//                             64,   // height
//                              2,   // chain length
//                          _pins,   // pin mapping
//       HUB75_I2S_CFG::FM6124    // driver chip
//     );
    

//     mxconfig.clkphase = 1; // 0 or 180 degrees`


//   // OK, now we can create our matrix object
//   dma_display = new MatrixPanel_I2S_DMA(mxconfig);

//   // let's adjust default brightness to about 75%
//   dma_display->setBrightness8(255);

//   // If you are using a 64x64 matrix you need to pass a value for the E pin
//   // The trinity connects GPIO 18 to E.
//   // This can be commented out for any smaller displays (but should work fine with it)

//   // May or may not be needed depending on your matrix
//   // Example of what needing it looks like:
//   // https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/issues/134#issuecomment-866367216
//   //mxconfig.clkphase = false;

//   // Some matrix panels use different ICs for driving them and some of them have strange quirks.
//   // If the display is not working right, try this.
//   mxconfig.driver = HUB75_I2S_CFG::FM6126A;

//   dma_display = new MatrixPanel_I2S_DMA(mxconfig);
//   dma_display->begin();
// }

// void * GIFOpenFile(const char *fname, int32_t *pSize)
// {
//   f = SD_MMC.open(fname);
//   if (f)
//   {
//     *pSize = f.size();
//     return (void *)&f;
//   }
//   return NULL;
// } /* GIFOpenFile() */

// void GIFCloseFile(void *pHandle)
// {
//   File *f = static_cast<File *>(pHandle);
//   if (f != NULL)
//      f->close();
// } /* GIFCloseFile() */

// int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
// {
//     int32_t iBytesRead;
//     iBytesRead = iLen;
//     File *f = static_cast<File *>(pFile->fHandle);
//     // Note: If you read a file all the way to the last byte, seek() stops working
//     if ((pFile->iSize - pFile->iPos) < iLen)
//        iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
//     if (iBytesRead <= 0)
//        return 0;
//     iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
//     pFile->iPos = f->position();
//     return iBytesRead;
// } /* GIFReadFile() */

// int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
// { 
//   int i = micros();
//   File *f = static_cast<File *>(pFile->fHandle);
//   f->seek(iPosition);
//   pFile->iPos = (int32_t)f->position();
//   i = micros() - i;
// //  Serial.printf("Seek time = %d us\n", i);
//   return pFile->iPos;
// } /* GIFSeekFile() */

// void setup()
// {
//   Serial.begin(115200);
//   pinMode(SD_DET, INPUT_PULLUP); // Set SD_DET pin as input with pull-up resistor
//   while(digitalRead(SD_DET) == HIGH) // Wait for SD card to be inserted
//   {
//     Serial.println("Waiting for SD card to be inserted...");
//     delay(1000);
//   }
//   displaySetup();
//   dma_display->fillScreen(dma_display->color565(0, 0, 0));
//   gif.begin(LITTLE_ENDIAN_PIXELS);
  
//   SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3); // Set the SD_MMC pins
//   if (!SD_MMC.begin())
//   {
//     Serial.println("Card Mount Failed");
//     return;
//   }
//   Serial.println("SD Card initialized.");
//   if(SD_MMC.exists("/fireworks.gif")) {
//     Serial.println("GIF file exists on SD Card");
//     //File sdGif = SD_MMC.open("/fireworks.gif", FILE_READ); // Open the GIF file
//   } else {
//     Serial.println("GIF file does not exist on SD Card");
//     while(1) delay(1000); // Wait forever if the GIF file is not found
//   }
// }

// void loop()
// {
//   // put your main code here, to run repeatedly:
//   if (gif.open("/fireworks.gif", GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
//   {
//     Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
//     while (gif.playFrame(true, NULL))
//     {
//     }
//     gif.close();
//   }
// }