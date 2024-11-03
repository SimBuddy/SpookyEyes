#include <TFT_eSPI.h> 
#include <SPI.h>
#include <string.h>
#include "ESP_NOW_NETWORK.h"
#include <iostream>
#include <time.h>
#include "EEPROM.h"

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240
#define BLACK 0x0000
#define NAVY 0x000F
#define DARKGREEN 0x03E0
#define DARKCYAN 0x03EF
#define MAROON 0x7800
#define PURPLE 0x780F
#define OLIVE 0x7BE0
#define LIGHTGREY 0xC618
#define DARKGREY 0x7BEF
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define GREENYELLOW 0xAFE5
#define PINK 0xF81F
#define CUSTOMCOLOR 0x0101

//#define SERVER			// !!!! Comment this line for Clients - CLIENT sends data to SERVER

typedef struct
{
  String command;
  int p1;
  int p2;
  int p3;
  int p4;
  int p5;
  int p6;
} dataType;
dataType toSend;

ESP_NOW_Network_Node *node;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
TFT_eSprite eye = TFT_eSprite(&tft);
TFT_eSprite eyeLid = TFT_eSprite(&tft);

double px,py;         //  centre of pupil
int psize;            //  size of pupil
int isize;            // size of iris
int icolor;           // color of iris
int whiteColor;       // color of white of eye - defaults to white, might be interesting to change
int msDelay       = 2;   // delay between sending remote command, then running locally
int loopCount     = 0;   // count loops, full reset every 1000 - just in case app goes mad



//========== Common code block - common to both CLIENT and SERVER codebase - keep synced if changing

void lookTo(double new_px,double new_py)  // moves the eye to a spot, and updates px,py
{       
 double dx = new_px - px;
 double xInterval = dx/20;
 double dy = new_py - py;
 double yInterval = dy/20;  // less than 20 is jerky
 double cnt;  //            //temporary counter
 cnt=0;
 for(int x=1;x<=21;x++){
   delay(11);
   cnt+=1;
   if (cnt>1)
   {
     eye.pushSprite((x*xInterval)+px-120  ,(x*yInterval)+py-120); 
     cnt=0;
   }
 }
 px = new_px;
 py = new_py;
 eye.pushSprite(px-120  ,py-120); 
 delay(500);
}

void eyeTest()
{
 // eye-test - use these as practical limits for std isize (103) iris/pupil
 drawEye(px,py,psize,isize);
 lookTo(164,75);    
 lookTo(164,164);    
 lookTo(75,164);    
 lookTo(75,74);       // top left
 lookTo(164,74);       // top right
 lookTo(120,55);       // straight up
 lookTo(120,184);       // straight down
 lookTo(54,120);       // fully left
 lookTo(184,120);       // fully right
 lookTo(120,120);       // centred
}

void drawEye(int tpx,int tpy,int tpsize,int tisize)
{     //  routine to draw the eye - t means temp
  // draw white
  eye.fillSprite(whiteColor); // draw the white of the eye - black looks best
  //draw iris
  eye.fillSmoothCircle(tpx, tpy,tisize,icolor,TFT_TRANSPARENT);
  //draw pupil
  eye.fillSmoothCircle(tpx, tpy,tpsize,TFT_BLACK,TFT_TRANSPARENT);
  //draw 3d sparkle
  eye.fillSmoothCircle(tpx-(tpsize/2.3), tpy-(tpsize/2.3),4,tft.color565(130,130,130),TFT_BLACK);
  eye.pushSprite(0,0); 
}

void widePupil(int targetWidth){              //makes the pupil get wider
  for(int i=psize; i<=targetWidth ; i+=3.7){
    drawEye(px,py,i,isize);
    eye.pushSprite(0,0);
  }
  psize=targetWidth;
  drawEye(px,py,psize,isize);
}

void narrowPupil(int targetWidth)           // makes the pupil get narrower
{
  for(int i=psize; i>=targetWidth ; i-=3.7){
    drawEye(px,py,i,isize);
    eye.pushSprite(0,0);
  }
  psize=targetWidth;
  drawEye(px,py,psize,isize);
}

void fastBlink()                  //  blinks by drawing big overlappied squared, then deleting
{
  eyeLid.createSprite(SCREEN_WIDTH*2,SCREEN_HEIGHT*2); 
  eyeLid.fillScreen(TFT_BLACK);
  eyeLid.pushSprite(0,0);     
  eyeLid.deleteSprite();
}

void changeColor(int newColor)      // changes iris color
{
 int ncolor;
 if (newColor==1) {ncolor= BLUE;}
 if (newColor==2) {ncolor= GREEN;}
 if (newColor==3) {ncolor= CYAN;}
 if (newColor==4) {ncolor= RED;}
 if (newColor==5) {ncolor= MAGENTA;}
 if (newColor==6) {ncolor= YELLOW;}
 if (newColor==7) {ncolor= ORANGE;}
 if (newColor==8) {ncolor= GREENYELLOW;}
 if (newColor==9) {ncolor= PINK;}
 icolor = ncolor;
 Serial.printf("%d",icolor);
}
//  ================= END COMMON CODE BLOCK

void sendData(String command, int p1, int p2, int p3, int p4, int p5, int p6) 
{
 toSend.command = command;
 toSend.p1 = p1;
 toSend.p2 = p2;
 toSend.p3 = p3;
 toSend.p4 = p4;
 toSend.p5 = p5;
 toSend.p6 = p6;
  const char *d = (const char *)(&toSend);
  node->senddata(d, sizeof(toSend));
}

void recv(const uint8_t *addr, uint8_t pos, const uint8_t *data, int len) // this routine actually receives the data from the client
{
 esp_now_data_t *msg = (esp_now_data_t *)data;   // this routine is redundant, so left in minimal state
}

void setup(void)
{
 Serial.begin(115200);
 node = new ESP_NOW_Network_Node(EPCLIENT);
 node->onNewRecv(recv, NULL);
 srand((unsigned)time(NULL));

 px=120;
 py=120;
 psize=55;
 isize=103;            //113 max
 icolor=BLUE;
 whiteColor=TFT_BLACK;     // set to TFT_BLACK for no eye-white or TFT_WHITE
 Serial.begin(115200);
 tft.init();
 tft.setRotation(0);
 eye.setColorDepth(8);
 eye.createSprite(SCREEN_WIDTH*2,SCREEN_HEIGHT*2);
 drawEye(px,py,isize,psize);
 eye.pushSprite(0,0);
 drawEye(px,py,psize,isize);
 sendData("EYETEST",0,0,0,0,0,0);
 delay(msDelay);
 eyeTest();
 lookTo(120,120);
 
}

void loop()
{
 Serial.print("\n\rLooping...");
 int x = 45 + random(170);
 int y = 45 + random(170);
 int doStuff = random(101);   // random actions - feel free to play with probabilities below

 Serial.printf("%d",doStuff);
 if(doStuff>0 && doStuff <= 60)
 {
  Serial.print("\n\LookTo...");
  sendData("LOOKTO",x,y,0,0,0,0);
  delay(msDelay);
  lookTo(x,y);  
 }
 if(doStuff>60 && doStuff<=80)
 {
  sendData("FASTBLINK",0,0,0,0,0,0);
  delay(msDelay);
  fastBlink();
 }
 if(doStuff>80 && doStuff <= 91)
 {
  int ncolor = random(8)+1;
  sendData("CHANGECOLOR",ncolor,0,0,0,0,0);
  delay(msDelay);
  changeColor(ncolor);
 }
 if(doStuff>91 && doStuff <=101)
 {
  sendData("LOOKTO",120,120,0,0,0,0);
  delay(msDelay);
  lookTo(120,120); 
  sendData("NARROW",35,0,0,0,0,0);
  delay(msDelay);
  narrowPupil(35);
  delay(1000);
  sendData("WIDE",75,0,0,0,0,0);
  delay(msDelay);
  widePupil(75);
  delay(1000);
  sendData("NARROW",55,0,0,0,0,0);
  delay(msDelay);
  narrowPupil(55);
  sendData("LOOKTO",120,120,0,0,0,0);
  delay(msDelay);
  lookTo(120,120); 
 }
  
 loopCount++;
 Serial.printf("\n\r%d",loopCount);
 if (loopCount==1000)          // SYSTEM RESET EVERY 1000 ACTIONS - JUST IN CASE APP GOES MAD
 {
  sendData("SYSTEMRESET",0,0,0,0,0,0);  // tell server to restart
  delay(msDelay*10);  // let the server get properly restared
  lookTo(120,120);
  narrowPupil(12);      // comedy restart
  delay(1000);
  Serial.printf("\n\rRestarting ...");
  ESP.restart();    
 }
}
