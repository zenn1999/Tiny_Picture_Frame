/*
 * This uses a Adafruit 1.14" 240x135 tft and a Adafruit itsy bitsy m4 express. It is a portable picture viewer. 
 * You have to resize your pictures to 240x135 for landscape pics and 135x240 for portrait. if you resize under 
 * locked constraints so the picture isnt distorted the height wont always match up and thats ok. I plan to fix 
 * in a centering function next so mor of the picture is viewable. You also have to name the pics a number, in 
 * order, so the iteration works. So 0.bmp, 1.bmp, 2.bmp and so on. And they must be .bmp files.
 * 
 * Tim Baggs
*/


#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions

#define USE_SD_CARD

// Match the pin configuration for the controller you are using
#define SD_CS   11 // SD card select pin
#define TFT_CS  10 // TFT select pin
#define TFT_DC  7 // TFT display/command pin
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin

#if defined(USE_SD_CARD)
  SdFat                SD;         // SD card filesystem
  SdFile               file;       // Individual sdcard files for the file counter
  Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
#else
  // SPI or QSPI flash filesystem (i.e. CIRCUITPY drive)
  #if defined(__SAMD51__) || defined(NRF52840_XXAA)
    Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS,
      PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
  #else
    #if (SPI_INTERFACES_COUNT == 1)
      Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
    #else
      Adafruit_FlashTransport_SPI flashTransport(SS1, &SPI1);
    #endif
  #endif
  Adafruit_SPIFlash    flash(&flashTransport);
  FatFileSystem        filesys;
  Adafruit_ImageReader reader(filesys); // Image-reader, pass in flash filesys
#endif

Adafruit_ST7789      tft    = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;
                     
int var = 0;
int fileCount;


void setup() {
  
  Serial.begin(9600);
/*
  while(!Serial);       // Wait for Serial Monitor before continuing. Uncomment to debug
*/
  tft.init(135, 240);           // Init ST7789 135x240

  // The Adafruit_ImageReader constructor call (above, before setup())
  // accepts an uninitialized SdFat or FatFileSystem object. This MUST
  // BE INITIALIZED before using any of the image reader functions!
  Serial.print(F("Initializing filesystem..."));
#if defined(USE_SD_CARD)
  // SD card is pretty straightforward, a single call...
  if(!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }
#else
  // SPI or QSPI flash requires two steps, one to access the bare flash
  // memory itself, then the second to access the filesystem within...
  if(!flash.begin()) {
    Serial.println(F("flash begin() failed"));
    for(;;);
  }
  if(!filesys.begin(&flash)) {
    Serial.println(F("filesys begin() failed"));
    for(;;);
  }
#endif
  Serial.println(F("OK!"));
  getFileCount();             // On startup count the files on the sd card

  //tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

}

void loop() {
  //Builds the iterating filename
  String root ="/";
  String bmp = String(var) + ".bmp";  //converts the int var to a string then attaches it to .bmp
  root += bmp;    // equates to "/0.bmp" and the number iterates
  
  ImageReturnCode stat = reader.bmpDimensions(string2char(root), &width, &height);    //This returns width and height.
  //Serial.println(width);         // uncomment to test variables
  //Serial.println(var);
  //Serial.println(string2char(root));
  
  if(width == 240) {                         //The if statements judge whether the screen needs to adjust for landscape or portrait
    tft.setRotation(3);
    reader.drawBMP(string2char(root), tft, 0, 0);
  }
  if(width == 135) {
    tft.setRotation(0);
    reader.drawBMP(string2char(root), tft, 0, 0);
  }
  delay(3000);
  var += 1;
  if(var > (fileCount - 1)) {       //This is how many pictures you have on the sd card. I added the - 1 because i started names at 0 not 1.
    var = 0;
  }

}
// To convert the string i built into a char*
char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}
// this function counts the pics on the sd card 
void getFileCount()
{
  //int fileCount = 0;
  
  SD.vwd()->rewind();
  while (file.openNext(SD.vwd(), O_READ)) {
    if (!file.isHidden()) {
      fileCount++;
    }
    file.close();
  }
  Serial.print(fileCount);
}
