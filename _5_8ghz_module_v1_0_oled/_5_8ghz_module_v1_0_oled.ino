/*
  20.07.2013
  SerPanRC@yandex.ru
  http://forum.rcdesign.ru/f90/thread327590.html
  dollop mod;
  you may use any display, which is supported by U8glib 
  */

#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include "U8glib.h"

// IMPORTANT NOTE: The following list is incomplete. The complete list of supported 
// devices with all constructor calls is here: http://code.google.com/p/u8glib/wiki/device
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);	// Fast I2C / TWI 

//#define dot_plot //uncomment if you want dots instead of bars in the specnhbv graph
#define spc_solid_line //uncomment if you want solid line while updating the peaks

#define SSP 10 //receiver pin
//BUTTONS defines
#define Bolche 9
#define Regim 7
#define Menche 8
#define Vvod 6

#define Voltage A7 //Voltage input
#define Rssiin A0  //RSSI input

#define Buzzer 2   //buzzer pin

byte column=0;        //column for spectrum
int VoltConst = 431;  //voltage divider

unsigned int n=0;
byte data0=0; 
byte data1=0;
byte data2=0;
byte data3=0;
byte modes_bits=0; 
#define RCV_start_flag 0
#define SPC_start_flag 1
#define SCN_start_flag 2
#define VLT_start_flag 3
byte LowBat_flag=0;
unsigned int Freguency=0;
unsigned int VoltDivider;
unsigned int Delitel;
byte DelitelH ;
byte DelitelL ;
byte var =0; // mode, 0-receiver, 1-spectrum, 2-scaner, 3-voltage setup
int sval=0;  //RSSI value
int RSSI = 0;//maxRSSI
int spec =0; //specnrum column height RSSI
int MAXi = 0;//i value for maxRSSI
float vval=0;//battery ADC
float volt=0;//voltage
byte nS=0;//number of battery cells
unsigned long old_millis=0;
byte cont_scn=0; //step for scanner and spectrum modes
unsigned char tmp_arr [128]; //array for spectrum graph

//5.8 Logo
static const unsigned char PROGMEM Logo [] = {
0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x1F, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8,
0x3F, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0xFC, 0x3E, 0x03, 0xE0, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xC0, 0x7C, 0x3E, 0x03, 0xE0, 0x7F, 0xFF, 0x00, 0x00, 0x1F,
0xF8, 0x07, 0xC0, 0x7C, 0x3E, 0x07, 0xE0, 0x7F, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x07, 0xE0, 0x7C,
0x7C, 0x07, 0xC0, 0xFF, 0xFF, 0x00, 0x01, 0xFF, 0xFF, 0x83, 0xE0, 0x3E, 0x7C, 0x07, 0xC0, 0xFF,
0xFF, 0x00, 0x03, 0xFF, 0xFF, 0xC3, 0xE0, 0x3E, 0x7C, 0x0F, 0xC0, 0xFF, 0x80, 0x00, 0x03, 0xFE,
0x7F, 0xC3, 0xF0, 0x3E, 0x7C, 0x0F, 0x80, 0xFF, 0x00, 0x00, 0x03, 0xFE, 0x7F, 0xC1, 0xF0, 0x3E,
0xF8, 0x0F, 0x81, 0xFF, 0xE0, 0x00, 0x03, 0xFE, 0x7F, 0xC1, 0xF0, 0x1F, 0xF8, 0x1F, 0x81, 0xFF,
0xF8, 0x00, 0x03, 0xFF, 0xFF, 0xC1, 0xF8, 0x1F, 0xF8, 0x1F, 0x01, 0xFF, 0xFC, 0x00, 0x01, 0xFF,
0xFF, 0x80, 0xF8, 0x1F, 0xF8, 0x1F, 0x01, 0xFF, 0xFC, 0x00, 0x00, 0x7F, 0xFE, 0x00, 0xF8, 0x1F,
0xF8, 0x1F, 0x03, 0xFF, 0xFE, 0x00, 0x00, 0x7F, 0xFE, 0x00, 0xF8, 0x1F, 0xF8, 0x1F, 0x02, 0x07,
0xFE, 0x07, 0xC1, 0xFF, 0xFF, 0x80, 0xF8, 0x1F, 0xF8, 0x1F, 0x00, 0x03, 0xFE, 0x0F, 0xE1, 0xFF,
0xFF, 0x80, 0xF8, 0x1F, 0xF8, 0x1F, 0x80, 0x03, 0xFE, 0x1F, 0xF3, 0xFE, 0x7F, 0xC1, 0xF8, 0x1F,
0xFC, 0x0F, 0x80, 0x03, 0xFE, 0x1F, 0xF3, 0xFE, 0x7F, 0xC1, 0xF0, 0x3F, 0x7C, 0x0F, 0x82, 0x07,
0xFC, 0x1F, 0xF3, 0xFE, 0x7F, 0xC1, 0xF0, 0x3E, 0x7C, 0x0F, 0xC3, 0xFF, 0xFC, 0x1F, 0xF3, 0xFF,
0xFF, 0xC3, 0xF0, 0x3E, 0x7C, 0x07, 0xC1, 0xFF, 0xF8, 0x1F, 0xF1, 0xFF, 0xFF, 0x83, 0xE0, 0x3E,
0x7E, 0x07, 0xE0, 0xFF, 0xF0, 0x0F, 0xE0, 0xFF, 0xFF, 0x07, 0xE0, 0x7E, 0x3E, 0x03, 0xE0, 0x1F,
0xC0, 0x07, 0xC0, 0x1F, 0xF8, 0x07, 0xC0, 0x7C, 0x3E, 0x03, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x07, 0xC0, 0x7C, 0x3E, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x7C,
0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xF8, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70
};

void setup()
{
///////////////////////////////////////////////////////////////////
 
  pinMode (SSP, OUTPUT);
  pinMode (Buzzer, OUTPUT);
  
  // flip screen, if required
  // u8g.setRot180();
  
  // set SPI backup if required
  //u8g.setHardwareBackup(u8g_backup_avr_spi);

  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }

  pinMode(Bolche, INPUT);
  digitalWrite(Bolche, HIGH);
  pinMode(Menche, INPUT);
  digitalWrite(Menche, HIGH);
  pinMode(Vvod, INPUT);
  digitalWrite(Vvod, HIGH);
  pinMode(Regim, INPUT);
  digitalWrite(Regim, HIGH);

  DelitelL= EEPROM.read(0);
  DelitelH= EEPROM.read(1);
  VoltDivider=EEPROM.read(5);

  Freguency= (DelitelH *32 + DelitelL)*2 +479;
  if (Freguency>6000 || Freguency<5500) Freguency=5645;

///////////////////////////////////////////////////////////////////

  u8g.setDefaultForegroundColor();
  u8g.setFontPosTop();
  u8g.setFont(u8g_font_6x10);
  u8g.firstPage(); 
  do{
     
    u8g.drawStr(35,10,"SerPanRC");
    u8g.drawStr(11,25,"modified by dollop");   
    u8g.drawBitmapP( 16, 32, 12, 32,  Logo);             
  }
  while( u8g.nextPage() );
  delay (1000);

  // initialize SPI:
    SPI.begin();
    SPI.setBitOrder(LSBFIRST);  
    digitalWrite(SSP,LOW);
    SPI.transfer(0x10);
    SPI.transfer(0x01);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    digitalWrite(SSP,HIGH);
    prog_freg();
} //SETUP END//



void draw(void) {
  // graphic commands to redraw the complete screen should be placed here  
    switch (var)
  {
    case 0: //Receiver mode
      u8g.setFont(u8g_font_8x13B);  //mode header
      u8g.drawStr(30,10,"RECEIVER");
      u8g.setFont(u8g_font_6x10);
      u8g.drawStr(0,20,"FREQ: ");
      //u8g.drawStr(0,30,"VOLT: "); 
      u8g.drawStr(0,40,"RSSI: ");     
      u8g.setPrintPos(36,20);
      u8g.print(Freguency);
      if(LowBat_flag & 1)  //when the battery is low...
       {       
        u8g.setFont(u8g_font_8x13B);
        u8g.drawStr(102,31,"LOW");   //blinking "LOW"
        //u8g.drawStr(0,31,"VOLT: "); 
        u8g.setFont(u8g_font_6x10);    
       }
      else
       {
        u8g.drawStr(0,30,"VOLT: "); //blinking "VOLT: "
       }
      u8g.setPrintPos(36, 30); 
      u8g.print(volt);
      u8g.setPrintPos(80, 30); 
      u8g.print(nS);
      u8g.drawStr(88, 30, "S");
       
//RSSI calculations
    if (sval>1400)
    {
      u8g.setPrintPos(36, 40);
      u8g.print(((sval-1400)/4));
      byte L=(sval-1380)/15;
      if (L>127) L=127;
      u8g.drawFrame(0, 45, 127, 15);
      u8g.drawBox(0, 45, L, 15);
    }
    else
    {
      u8g.drawStr(36, 40,"0");
      u8g.drawFrame(0, 45, 127, 15);
    };            
    break;
    
///////////////////////////////////////////////////////////////////    
    case 1: //Spectrum mode
      u8g.setFont(u8g_font_8x13B); //mode header
      u8g.drawStr(25,10,"SPECTRUM");
      u8g.setFont(u8g_font_6x10);
      u8g.drawStr(0,18,"FREQ: ");
      u8g.setPrintPos(36,18);
      u8g.print(Freguency);  
      for (byte i=0; i < 128; i++)
      {
        #if defined (dot_plot)    
         u8g.drawPixel(i, 64-tmp_arr[i]); //for dot plot
        #else
         u8g.drawVLine(i, 64-tmp_arr[i], tmp_arr[i]); //for bar plot
        #endif
      }
        #if defined (spc_solid_line)  
         u8g.drawVLine(cont_scn+1, 18, 46); //long moving line in spectrum graph
        #else
         u8g.setColorIndex(0);
         u8g.drawVLine(column, 18, 46);
         u8g.setColorIndex(1);
        #endif
    break;
    
///////////////////////////////////////////////////////////////////     
    case 2:  //Scanner mode
      u8g.setFont(u8g_font_8x13B); //mode header
      u8g.drawStr(37,10,"SCANNER");
      u8g.setFont(u8g_font_6x10);
      u8g.drawStr(0,20,"FREQ: ");
      u8g.drawStr(0,30,"RSSI: ");
      u8g.setPrintPos(36,20);
      u8g.print(Freguency);
      u8g.setPrintPos(36, 30);       
      u8g.print(sval);
      //progress bar
      u8g.drawFrame(0, 45, 127, 15);
      u8g.drawBox(0, 45, cont_scn/2, 15);
    break;
    
/////////////////////////////////////////////////////////////////// 
    case 3: //Voltage setup mode
      u8g.setFont(u8g_font_8x13B); //mode header
      u8g.drawStr(12,10, "VOLTAGE SETUP");
      u8g.setFont(u8g_font_6x10);
      u8g.drawStr(0,30,"VOLT: ");
      u8g.setPrintPos(36,30);
      u8g.print(volt); 
      u8g.setPrintPos(80,30);
      u8g.print(nS);
      u8g.print("S");
      //debug info
      u8g.setPrintPos(0,45);
      u8g.print("VoltDivider = ");
      u8g.print(VoltDivider);
    break;
  }  
}

void loop()
{
/////////////////////////////////////////////////////////////////// 
  BatMeasure();
  //modes switch and display
  while (digitalRead(Regim)==LOW)
  {
    var=var++;
    cont_scn=0;
    if (var>3) var=0 ;
//    short_bip();
    switch (var)
    {
     case 0:
     u8g.firstPage(); 
     do{
      u8g.setFont(u8g_font_8x13B);
      u8g.drawStr(30,30,"RECEIVER");
     }while( u8g.nextPage() );
     break;
     
     case 1:
     u8g.firstPage(); 
     do{
      u8g.setFont(u8g_font_8x13B);
      u8g.drawStr(30,30,"SPECTRUM");
     }while( u8g.nextPage() );
     break;

     case 2:
     u8g.firstPage(); 
     do{
      u8g.setFont(u8g_font_8x13B);
      u8g.drawStr(37,30,"SCANNER");
     }while( u8g.nextPage() );     
     break;
     case 3:
     if (bitRead (modes_bits, SCN_start_flag)) var = 0;
     u8g.firstPage(); 
     do{
      if (var==0) //We shouldn't go to battery setup mode in normal short button press 
       {
        u8g.setFont(u8g_font_8x13B);
        u8g.drawStr(30,30,"RECEIVER");
       }
       else
       {
        u8g.setFont(u8g_font_8x13B);
        u8g.drawStr(12,30,"VOLTAGE SETUP");
       }
      }while( u8g.nextPage() );     
     break;
    }
    delay(700);
  };

    if(LowBat_flag != 0)     //for low battery indication
    {
      LowBat_flag++;
    }
  
  //modes
  switch (var)
  {
    case 0: ////////////////////receiver mode
      cont_scn=0;
      sval=0;
      for ( byte i = 0; i < 10; i++)
       {
         sval = sval + analogRead(Rssiin);
         delay (5);
       }
      if (Freguency <5500) Freguency=5999;
      if (Freguency >6000) Freguency=5501;
      if (digitalRead(Bolche)==LOW or digitalRead(Menche)==LOW)
       {
        n=n+1;
        if (digitalRead(Bolche)==LOW and n<15) Freguency=Freguency+2;
        if (digitalRead(Bolche)==LOW and n>=15) Freguency=Freguency+10;
        if (digitalRead(Menche)==LOW and n<15 )  Freguency=Freguency-2;
        if (digitalRead(Menche)==LOW and n>=15 )  Freguency=Freguency-10;
        prog_freg();
       }
       else n=0;  
 
    if (digitalRead(Vvod)==LOW) //write frequency to the EEPROM
    {
//      short_bip();
      EEPROM.write(0, DelitelL);
      EEPROM.write(1, DelitelH);
      delay(3000);
    }
    delay(100);
    modes_bits = 0;
    bitSet(modes_bits, RCV_start_flag); 
    break;
///////////////////////////////////////////////////////////////////       
    case 1: ////////////////////spectrum mode
     if (cont_scn < 128)
       //go through frequences with 4MHz step
      {
        cont_scn++;
        Freguency = 5495+4*cont_scn;
        prog_freg(); //receiver control
        delay (20);
        if (digitalRead(Bolche)==LOW) delay (1000); //you may pause the process for a while
        sval=0;
        for ( byte u = 0; u < 10; u++)  //Reads RSSI 10 times
         {
           sval = sval + analogRead(Rssiin);
           delay (5);
         } 
/*According to datashit RSSI=0.5-1.1V, that equals to 3,3V with 10bit ADC
3,3/1024=0,003V per bit
0,5/(3,3/1024)=155 bits
1,1/(3,3/1024)=341 bits
When we summarize 10 values we get 1550 min and 3410 max.
*/      
      if (sval<1280) sval=1280; //just to be safe we put 1280 instead 1550
      spec = map (sval, 1280, 3410, 1, 47); //we map walues of RSSI to spectrum diagram
      tmp_arr[cont_scn]=spec;     //store the values to the temporary array for later use in the graphics output routine
    }
    else cont_scn=0;
    modes_bits = 0;
    bitSet (modes_bits, SPC_start_flag);    
    break;
///////////////////////////////////////////////////////////////////  
    case 2:  //////////////////////scanner mode
    sval=0;
    if (cont_scn<249)
    {
      cont_scn++;
      Freguency = 5501+2*cont_scn;
      prog_freg();
      delay(40);
      sval =analogRead(Rssiin);
      if (sval > RSSI )
      {
        RSSI=sval;
        MAXi=cont_scn;
      }
    }
    else 
    {
     Freguency = 5501+2*MAXi;
     prog_freg();
     var=0;
     cont_scn=0;
     RSSI=0;
    }
    modes_bits = 0;
    bitSet (modes_bits, SCN_start_flag);
    break;
///////////////////////////////////////////////////////////////////      
    case 3: //////////////////////voltage setup mode
     cont_scn=0;
     if (VoltDivider <1) VoltDivider=255;
     if (VoltDivider >255) VoltDivider=1;  
     if (digitalRead(Bolche)==LOW or digitalRead(Menche)==LOW)
      {
       n=n+1;
       if (digitalRead(Menche)==LOW and n<15) VoltDivider=VoltDivider+1;
       if (digitalRead(Menche)==LOW and n>=15) VoltDivider=VoltDivider+10;
       if (digitalRead(Bolche)==LOW and n<15) VoltDivider=VoltDivider-1;
       if (digitalRead(Bolche)==LOW and n>=15) VoltDivider=VoltDivider-10;
      }
    else n=0;
    if (digitalRead(Vvod)==LOW)
    {
      EEPROM.write(5, VoltDivider);
      delay(2000);
    }
    modes_bits = 0;
    bitSet (modes_bits, VLT_start_flag);
    break;
  } //END case
    
    // picture loop
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
}


void prog_freg (void)//***Receiver control
{  
  Delitel=(Freguency -479)/2;
  DelitelH=Delitel>>5;
  DelitelL=Delitel &0x1F;
  data0=DelitelL *32 +17;
  data1=DelitelH *16 + DelitelL /8;
  data2=DelitelH /16;

  digitalWrite(SSP,LOW);
  SPI.transfer(data0);
  SPI.transfer(data1);
  SPI.transfer(data2);
  SPI.transfer(data3);
  digitalWrite(SSP,HIGH);
}

void BatMeasure() //battery voltage measurement
{
  vval=0;
  for ( byte i = 0; i < 10; i++)
  {
    vval = vval + analogRead(Voltage);
    delay(2);
  }
  volt = vval/(VoltDivider+VoltConst); // VoltConst=431 - constant.
  //Х=3.3*(3.03/16.8)=18.297 MAX voltage on the input 10230/18.297=559 559-128=431. 
  //for the divider 100k/22k when the input is 16.8V the output is 3.03V
  if (volt > 6.4 && volt < 8.4) nS=2;
  if (volt > 9.6 && volt < 12.6) nS=3;
  if (volt > 12.8 && volt < 16.8) nS=4;

  //if the voltage is greater than 6,6B and lower than 7V with 2S battery and 30 second passed after the last beep...
  if(nS == 2 && volt > 6.6 && volt < 7 && (millis()-old_millis) > 30000)
  {
//    bip();
    old_millis = millis();
    LowBat_flag = 1;
  }
  if(nS == 3 && volt > 9.9 && volt < 10.5 && (millis()-old_millis) > 30000)
  {
//    bip();
    old_millis = millis();
    LowBat_flag = 1;
  }
  if(nS == 4 && volt > 13.2 && volt < 14 && (millis()-old_millis) > 30000)
  {
//    bip();
    old_millis = millis();
    LowBat_flag = 1;
  }
  if(nS == 2 && volt < 6.6 && (millis()-old_millis) > 10000)
  {
//    bip();
    old_millis = millis();
    LowBat_flag = 1;
  }
  if(nS == 3 && volt < 9.9 && (millis()-old_millis) > 10000)
  {
//    bip();
    old_millis = millis();
    LowBat_flag = 1;
  }
  if(nS == 4 && volt < 13.2 && (millis()-old_millis) > 10000)
  {
//    bip();
    old_millis = millis();
    LowBat_flag = 1;
  }  
}

void bip()
{
  tone(Buzzer, 3000, 300);
  delay(300);
  tone(Buzzer, 3500, 300);
  delay(300);
  tone(Buzzer, 4000, 300);
  delay(300);
}

void short_bip()
{
  tone(Buzzer, 4000, 100);
  delay(100);
}
