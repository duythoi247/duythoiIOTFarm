#include <Arduino.h>
#include <SPI.h>
#include "RF24.h"
#include <DigitalIO.h>
#include <HardwareSerial.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <Ethernet2.h>
#include "utility/w5500.h"
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h> 

#define TINY_GSM_MODEM_SIM7600
#include <TinyGsmClient.h>
//TFT
#define TFT_CS        PB12
#define TFT_DC        PC6
#define TFT_RST       PA8 
#define TFT_WIDTH  128
#define TFT_HEIGHT 160

#define ST7735_DARKGREY    0x7BEF
#define ST7735_GREENYELLOW 0xAFE5
#define ST7735_LIGHTGREY   0xC618
#define ST7735_DARKGREEN   0x03E0

#define TFT_MOSI PB15  
#define TFT_SCLK PB13  
//ngo vao va ngo ra
#define rl_bom1 PC0
#define rl_quat1 PC1
#define rl_den1 PC2
#define rl_bom2 PB11
#define rl_quat2 PB1
#define rl_den2 PB10
#define key1 PA11
#define key2 PA12
#define key3 PB9
#define key4 PB7
//Khoi tao cac lop
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
SPIClass SPI_1(PA7,PA6,PA5);
HardwareSerial Serial1(USART1);
HardwareSerial Serial_2(USART2);
TinyGsm modem(Serial_2);
TinyGsmClient client(modem);
PubSubClient mqtt(client);
// EthernetClient ethClient;
// PubSubClient client(ethClient);
RF24 radio(PA4,PC4);

//Khoi tao cac bien
const char apn[]  = "internet";
const char user[] = "";
const char pass[] = "";
const uint64_t pipe = 0xAABBCCDDEE11; 
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 123);
IPAddress myDns(192, 168, 0, 1);
char str[100];
String StrVal_Tram, StrVal_nd, StrVal_da, StrVal_dad, StrVal_, StrVal_as;
int i;
unsigned int manhinhhientai = 1,vitrinhapnhay=35,vitrinhapnhay2=45,vitrinhapnhay3=45, snd1=20,snd2=20,sda1=0,sda2=0,sdad1=0,sdad2=0,sas1=0,sas2=0;
bool BM1_State=0,BM2_State=0,bom1=0,bom2=0,den1=0,den2=0,quat1=0,quat2=0;
bool AuOrManual = 1;
unsigned int val_nd1, val_da1, val_dad1, val_as1,val_nd2, val_da2, val_dad2, val_as2;
bool check_client= false;
bool check_net=0;
//Dinh nghia ham
// void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void publishMQTT(const char* topic,  const String& payload);
void drawButton(int x, int y, int width, int height, const char* buttonText, uint16_t textColor, uint16_t bgColor);
void drawButton2(int x, int y, int width, int height, const char* buttonText, uint16_t textColor, uint16_t bgColor);
void displayText(int x, int y, int textSize, uint16_t textColor, const char* text);
char* randomTextFrom1To100();
char* randomTextFrom1To10000();
void drawBlinkingTick(int x, int y, int size, uint16_t color);
void drawON(int x, int y);
void drawOFF(int x, int y);
void drawMain();
void drawHT1();
void drawHT2();
void drawDK1();
void drawDK2();
void drawBM1();
void drawBM2();
void drawRBM();
void drawAutoOrManual();
void mqttCallback(char* topic, byte* payload, unsigned int len);
boolean mqttConnect();
//Chay setup 1 lan
void setup() {
  //Khoi tao cac chan in out
  pinMode(key1, INPUT_PULLUP);
  pinMode(key2 , INPUT_PULLUP);
  pinMode(key3, INPUT_PULLUP);
  pinMode(key4, INPUT_PULLUP);  
  pinMode(rl_bom1, OUTPUT);
  digitalWrite(rl_bom1,HIGH);
  pinMode(rl_bom2, OUTPUT);
  digitalWrite(rl_bom2,HIGH);
  pinMode(rl_den1, OUTPUT);
  digitalWrite(rl_den1,HIGH);
  pinMode(rl_den2, OUTPUT);
  digitalWrite(rl_den2,HIGH);
  pinMode(rl_quat1, OUTPUT);
  digitalWrite(rl_quat1,HIGH);
  pinMode(rl_quat2, OUTPUT);
  digitalWrite(rl_quat2,HIGH);
  // Khoi tao uart de dubug
  Serial1.setRx(PA10);
  Serial1.setTx(PA9);  
  Serial1.begin(115200);
   while (!Serial1) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial_2.setRx(PA3);
  Serial_2.setTx(PA2);  
  Serial_2.begin(115200); 
  //Khoi tao TFT  
  tft.initR(INITR_BLACKTAB); 
  tft.setRotation(3);
  tft.fillScreen(ST7735_DARKGREY);
  tft.drawRect(0, 0, 160, 128, ST7735_WHITE);
  displayText(2,10,2,ST7735_GREENYELLOW,"Initialize...");
  displayText(2,50,1,ST7735_GREENYELLOW,"Init SIM:...");
  delay(500);
  //Khoi tao SPI giao tiep W5500
  // SPI_2.setMOSI(PB5);
  // SPI_2.setMISO(PB4);
  // SPI_2.setSCLK(PB3);
  // SPI_2.begin(PC12);
  // Ethernet.init(PC12);
  //Khoi tao SPI giao tiep NRF
  SPI_1.begin(PA4);  
  radio.begin(); 
  radio.setAutoAck(1);              
  radio.setDataRate(RF24_2MBPS);    // Tốc độ dữ liệu
  radio.setChannel(10);               // Đặt kênh
  radio.openReadingPipe(1,pipe);     
  radio.startListening();        
  //Khoi tao MQTT
  Serial1.println("Initialize SIM:");
  modem.restart();
  Serial1.println("Modem: " + modem.getModemInfo());
  Serial1.println("Searching for telco provider.");
  if(!modem.waitForNetwork())
  {
    Serial1.println("fail");
    while(true);
  }
  Serial1.println("Connected to telco.");
  Serial1.println("Signal Quality: " + String(modem.getSignalQuality()));

  Serial1.println("Connecting to GPRS network.");
  if (!modem.gprsConnect(apn, user, pass))
  {
    Serial1.println("fail");
    while(true);
  }
  Serial1.println("Connected to GPRS: " + String(apn));
  
  mqtt.setServer("broker.hivemq.com", 1883);
  mqtt.setCallback(mqttCallback);
  Serial1.println("Connecting to MQTT Broker: " + String("broker.hivemq.com"));
  // while(mqttConnect()==false) continue;
  Serial1.println();
  displayText(80,50,1,ST7735_GREENYELLOW,"Success"); 
  delay(400);
  // if(Ethernet.begin(mac)==1){
  //   check_net=2;
  //   displayText(80,50,1,ST7735_GREENYELLOW,"Success");
  //   delay(2000);
  //   Serial1.print("IP Address: ");
  //   Serial1.println(Ethernet.localIP());
  //   client.setServer("broker.hivemq.com", 1883);
  //   client.setCallback(callback);
  // }
  drawMain();
  delay(200);
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  if (radio.available()){
    mqtt.loop();
      char str[100] = "";
      radio.read(&str, sizeof(str));
      Serial1.println(str);
      String inputString = str;
      int spaceIndex1 = inputString.indexOf(' ');
      int spaceIndex2 = inputString.indexOf(' ', spaceIndex1 + 1);
      int spaceIndex3 = inputString.indexOf(' ', spaceIndex2 + 1);
      int spaceIndex4 = inputString.indexOf(' ', spaceIndex3 + 1);
      
      StrVal_Tram = inputString.substring(0, spaceIndex1);
      StrVal_nd = inputString.substring(spaceIndex1 + 1, spaceIndex2);
      StrVal_da = inputString.substring(spaceIndex2 + 1, spaceIndex3);
      StrVal_as = inputString.substring(spaceIndex3 + 1, spaceIndex4);
      StrVal_dad = inputString.substring(spaceIndex4 + 1);
      
      if(StrVal_Tram=="Tram1") {
        val_nd1=StrVal_nd.toInt();
        val_da1=StrVal_da.toInt();
        val_as1=StrVal_as.toInt();
        val_dad1=StrVal_dad.toInt();
        publishMQTT("duythoi092701/test/public/val_nd1", StrVal_nd);
        publishMQTT("duythoi092701/test/public/val_da1", StrVal_da);
        publishMQTT("duythoi092701/test/public/val_as1", StrVal_as);
        publishMQTT("duythoi092701/test/public/val_dad1", StrVal_dad);
        //publishMQTT("duythoi092701/test/public/var5", var9);
      }
      else if(StrVal_Tram=="Tram2") {
        val_nd2=StrVal_nd.toInt();
        val_da2=StrVal_da.toInt();
        val_as2=StrVal_as.toInt();
        val_dad2=StrVal_dad.toInt();
        publishMQTT("duythoi092701/test/public/val_nd2", StrVal_nd);
        publishMQTT("duythoi092701/test/public/val_da2", StrVal_da);
        publishMQTT("duythoi092701/test/public/val_as2", StrVal_as);
        publishMQTT("duythoi092701/test/public/val_dad2", StrVal_dad);
        //publishMQTT("duythoi092701/test/public/var5", var9);        
      }
      delay(200);
  }
  if (digitalRead(key1) == LOW && manhinhhientai!=1) {
    drawMain();
    delay(500);
  }  
   if ((digitalRead(key1) == LOW && manhinhhientai==1)||(digitalRead(key4) == LOW && manhinhhientai==3)) {
    drawHT1();
    delay(500);
  }
   if ((digitalRead(key4) == LOW && manhinhhientai==2)) {
    drawHT2();
    delay(500);
  }  
   if (digitalRead(key2) == LOW && manhinhhientai==1||(digitalRead(key4) == LOW && manhinhhientai==5)) {
    drawDK1();
    if(bom1==1) drawON(80,35); else drawOFF(80,35);
    if(den1==1) drawON(80,54); else drawOFF(80,54);
    if(quat1==1) drawON(90,73); else drawOFF(90,73);
    delay(500);
  }
   if ((digitalRead(key4) == LOW && manhinhhientai==4)) {
    drawDK2();
    if(bom2==1) drawON(80,35); else drawOFF(80,35);
    if(den2==1) drawON(80,54); else drawOFF(80,54);
    if(quat2==1) drawON(90,73); else drawOFF(90,73);  
    delay(500);
  }    
   if (digitalRead(key3) == LOW && manhinhhientai==1||(digitalRead(key4) == LOW && manhinhhientai==7)) {
    drawBM1();
    delay(500);
  }
   if ((digitalRead(key4) == LOW && manhinhhientai==6)) {
    drawBM2();
    delay(500);
  }  
   if ((digitalRead(key4) == LOW && manhinhhientai==1)||(digitalRead(key4) == LOW && manhinhhientai==8)) {
    drawAutoOrManual();
    delay(500);
  }
   if ((digitalRead(key4) == LOW && manhinhhientai==9)) {
    drawRBM();
    delay(500);
  }   
  if(manhinhhientai==1){
    char textBuffer1 []="";
    if(BM1_State==1) itoa(1, textBuffer1, 10);
    if(BM2_State==1) itoa(2, textBuffer1, 10);
    if(AuOrManual==0) drawButton(82,45,40,20," Auto ",ST7735_MAGENTA,ST7735_DARKGREEN);
    if(AuOrManual==1) drawButton(82,45,40,20,"Manual",ST7735_MAGENTA,ST7735_DARKGREEN);
    drawButton(140,70,15,20,textBuffer1,ST7735_MAGENTA,ST7735_DARKGREEN); 
    delay(200);
  }
  if(manhinhhientai==2){
    char buffer[10];
    drawButton2(80,35,20,10,itoa(val_nd1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
    drawButton2(80,45,20,10,itoa(val_da1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
    drawButton2(80,55,20,10,itoa(val_dad1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
    drawButton2(80,65,40,10,itoa(val_as1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);            
    drawButton2(80,75,20,10,randomTextFrom1To100(),ST7735_MAGENTA,ST7735_DARKGREEN);
    delay(20); 
    } 
  if(manhinhhientai==3){
    char buffer[10];
    drawButton2(80,35,20,10,itoa(val_nd2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
    drawButton2(80,45,20,10,itoa(val_da2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
    drawButton2(80,55,20,10,itoa(val_dad2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
    drawButton2(80,65,40,10,itoa(val_as2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);            
    drawButton2(80,75,20,10,randomTextFrom1To100(),ST7735_MAGENTA,ST7735_DARKGREEN);
    delay(20); 
    }     
  if(manhinhhientai==4||manhinhhientai==5){
   drawBlinkingTick(145,vitrinhapnhay,2,ST7735_ORANGE);
   if(digitalRead(key3) == LOW){
    vitrinhapnhay=vitrinhapnhay+19;
    if(vitrinhapnhay>73) vitrinhapnhay=35;
    drawBlinkingTick(145,vitrinhapnhay,2,ST7735_ORANGE);
   }
   if(digitalRead(key2) == LOW){
    switch (vitrinhapnhay)
    {
    case 35:
    if(manhinhhientai==4){   
        bom1= !bom1;
        if(bom1==1) drawON(80,35); else drawOFF(80,35);
      }
    if(manhinhhientai==5){   
        bom2= !bom2;
        if(bom2==1) drawON(80,35); else drawOFF(80,35);
      }  
      break;
    case 54:
    if(manhinhhientai==4){
        den1= !den1;
        if(den1==1) drawON(80,54); else drawOFF(80,54);
    }
    if(manhinhhientai==5){
        den2= !den2;
        if(den2==1) drawON(80,54); else drawOFF(80,54);
    }        
      break;
    case 73:
    if(manhinhhientai==4){
        quat1= !quat1;
        if(quat1==1) drawON(90,73); else drawOFF(90,73);
    }
    if(manhinhhientai==5){
        quat2= !quat2;
        if(quat2==1) drawON(90,73); else drawOFF(90,73);
    }      
      break;      
    default:
      break;
    }
   }
  }
 if(manhinhhientai==6||manhinhhientai==7){
   drawBlinkingTick(150,vitrinhapnhay2,1,ST7735_ORANGE);
   if(digitalRead(key3) == LOW){
    vitrinhapnhay2=vitrinhapnhay2+14;
    if(vitrinhapnhay2>77) vitrinhapnhay2=35;
    drawBlinkingTick(150,vitrinhapnhay2,1,ST7735_ORANGE);
   }
   if(digitalRead(key2) == LOW && manhinhhientai==6){
      if (vitrinhapnhay2==35)
      {
        snd1=snd1+5;
        if(snd1>70) snd1=20;
        char buffer[4]="";
        drawButton2(90,35,30,10,itoa(snd1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
      }
      if (vitrinhapnhay2==49)
      {
        sda1=sda1+5;;
        if(sda1>100) sda1=0;        
        char buffer[4]="";
        drawButton2(90,49,30,10,itoa(sda1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
      }
      if (vitrinhapnhay2==63)
      {
        sdad1=sdad1+5;
        if(sdad1>100) sdad1=0;
        char buffer[4]="";
        drawButton2(90,63,30,10,itoa(sdad1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);           
      }
      if (vitrinhapnhay2==77)
      {
        sas1=sas1+1000;
        if(sas1>10000) sas1=0;
        char buffer[4]="";
        drawButton2(90,77,35,10,itoa(sas1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);        
      }      
   }
     if(digitalRead(key2) == LOW && manhinhhientai==7){
      if (vitrinhapnhay2==35)
      {
        snd2=snd2+5;
        if(snd2>70) snd2=20;
        char buffer[4]="";
        drawButton2(90,35,30,10,itoa(snd2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
      }
      if (vitrinhapnhay2==49)
      {
        sda2=sda2+5;
        if(sda2>100) sda2=0;        
        char buffer[4]="";
        drawButton2(90,49,30,10,itoa(sda2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
      }
      if (vitrinhapnhay2==63)
      {
        sdad2=sdad2+5;
        if(sdad2>100) sdad2=0;
        char buffer[4]="";
        drawButton2(90,63,30,10,itoa(sdad2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);           
      }
      if (vitrinhapnhay2==77)
      {
        sas2=sas2+1000;
        if(sas2>10000) sas2=0;
        char buffer[4]="";
        drawButton2(90,77,35,10,itoa(sas2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);        
      }      
   } 
  }
  if(manhinhhientai==8){
    drawBlinkingTick(145,vitrinhapnhay2,2,ST7735_ORANGE);
    if(digitalRead(key3) == LOW){
      vitrinhapnhay2=vitrinhapnhay2+25;
    if(vitrinhapnhay2>70) vitrinhapnhay2=45;
      drawBlinkingTick(145,vitrinhapnhay2,2,ST7735_ORANGE);
   }
    if(digitalRead(key2) == LOW){
    switch (vitrinhapnhay2)
    {
    case 45:
      BM1_State= !BM1_State;
      if(BM1_State==1) BM2_State=0;
      if(BM1_State==0) BM2_State=1;
      if(BM1_State==1) drawON(80,45); else drawOFF(80,45);
      if(BM2_State==1) drawON(80,70); else drawOFF(80,70);
      break;
    case 70:
      BM2_State= !BM2_State;
      if(BM2_State==1) BM1_State=0;
      if(BM2_State==0) BM1_State=1;
      if(BM2_State==1) drawON(80,70); else drawOFF(80,70);
      if(BM1_State==1) drawON(80,45); else drawOFF(80,45);
      break;  
    default:
      break;
    }
    }
  }
  if(manhinhhientai==9){
    drawBlinkingTick(145,vitrinhapnhay2,2,ST7735_ORANGE);
    if(digitalRead(key3) == LOW){
      vitrinhapnhay2=vitrinhapnhay2+25;
    if(vitrinhapnhay2>70) vitrinhapnhay2=45;
      drawBlinkingTick(145,vitrinhapnhay2,2,ST7735_ORANGE);
   }
    if(digitalRead(key2) == LOW){
    switch (vitrinhapnhay2)
    {
    case 45:
      AuOrManual= !AuOrManual;
      if(AuOrManual==1) {
        drawON(80,70);
        drawOFF(80,45);
      } 
      else {
        drawOFF(80,70);
        drawON(80,45);
      }
      break;
    case 70:
      AuOrManual= !AuOrManual;
      if(AuOrManual==1) {
        drawON(80,70);
        drawOFF(80,45);
      } 
      else {
        drawOFF(80,70);
        drawON(80,45);
      }
      break;  
    default:
      break;
    }
    }
  }
  if ((AuOrManual==1&&quat1==1)||(AuOrManual==0 && BM1_State==1 && (val_nd1 > snd1 || val_da1 > sda1))||(AuOrManual==1 && BM2_State==1 && (val_nd1 > snd1 || val_da1 > sda2)))
  {
    digitalWrite(rl_quat1,LOW);
    publishMQTT("duythoi092701/iot/pub/quat1","on");
  }
  if (AuOrManual==1&&quat1==0||(AuOrManual==0&&BM1_State==1 && (val_nd1 < snd1 && val_da1 < sda1))||(AuOrManual==1 && BM2_State==1 && (val_nd1 < snd1 || val_da1 < sda2)))
  {  
    digitalWrite(rl_quat1,HIGH);
    publishMQTT("duythoi092701/iot/pub/quat1","off");
  } 
  if ((AuOrManual==1&&bom1==1)||(AuOrManual==0 && BM1_State==1 && val_dad1 < sdad1)|| (AuOrManual==0 && BM2_State==1 && val_dad1 < sdad2))
  {
    digitalWrite(rl_bom1,LOW);
    publishMQTT("duythoi092701/iot/pub/bom1","on");
  }
  if (AuOrManual==1&&bom1==0||(AuOrManual==0 &&BM1_State==1 &&  val_dad1> sdad1)|| (AuOrManual==0 && BM2_State==1 && val_dad1 > sdad2))
  {
    digitalWrite(rl_bom1,HIGH);
    publishMQTT("duythoi092701/iot/pub/bom1","off");
  }
  if ((AuOrManual==1&&den1==1)||(AuOrManual==0 && BM1_State==1 && val_as1 < sas1 )||(AuOrManual==0 && BM2_State==1 && val_as1 < sas2 ))
  {
    digitalWrite(rl_den1,LOW);
    publishMQTT("duythoi092701/iot/pub/den1","on");
  }
  if (AuOrManual==1&&den1==0||(AuOrManual==0 &&BM1_State==1 && val_as1 > sas1)||(AuOrManual==0 && BM2_State==1 && val_as1 > sas2 ))
  {
    digitalWrite(rl_den1,HIGH);
    publishMQTT("duythoi092701/iot/pub/den1","off");
  } 
  //2
  
 if ((AuOrManual==1&&quat2==1)||(AuOrManual==0 && BM1_State==1 && (val_nd2 > snd1 || val_da2 > sda1))||(AuOrManual==1 && BM2_State==1 && (val_nd2 > snd1 || val_da2 > sda2)))
  {
    digitalWrite(rl_quat2,LOW);
    publishMQTT("duythoi092701/iot/pub/quat2","on");
  }
  if (AuOrManual==1&&quat2==0||(AuOrManual==0&&BM1_State==1 && (val_nd2 < snd1 && val_da2 < sda1))||(AuOrManual==1 && BM2_State==1 && (val_nd2 < snd1 || val_da2 < sda2)))
  {  
    digitalWrite(rl_quat2,HIGH);
    publishMQTT("duythoi092701/iot/pub/quat2","off");
  } 
  if ((AuOrManual==1&&bom2==1)||(AuOrManual==0 && BM1_State==1 && val_dad2 < sdad1)|| (AuOrManual==0 && BM2_State==1 && val_dad2 < sdad2))
  {
    digitalWrite(rl_bom2,LOW);
    publishMQTT("duythoi092701/iot/pub/bom2","on");
  }
  if (AuOrManual==1&&bom2==0||(AuOrManual==0 &&BM1_State==1 &&  val_dad2 > sdad1)|| (AuOrManual==0 && BM2_State==1 && val_dad2 > sdad2))
  {
    digitalWrite(rl_bom2,HIGH);
    publishMQTT("duythoi092701/iot/pub/bom2","off");
  }
  if ((AuOrManual==1&&den2==1)||(AuOrManual==0 && BM1_State==1 && val_as2 < sas1 )||(AuOrManual==0 && BM2_State==1 && val_as2 < sas2 ))
  {
    digitalWrite(rl_den2,LOW);
    publishMQTT("duythoi092701/iot/pub/den2","on");
  }
  if (AuOrManual==1&&den2==0||(AuOrManual==0 &&BM1_State==1 && val_as2 > sas1)||(AuOrManual==0 && BM2_State==1 && val_as2 > sas2 ))
  {
    digitalWrite(rl_den2,HIGH);
    publishMQTT("duythoi092701/iot/pub/den2","off");
  }   
delay(50);     
}

// void callback(char* topic, byte* payload, unsigned int length) {
//   String incommingMessage = "";
//   for(int i=0; i<length;i++) incommingMessage += (char)payload[i];
//   Serial1.println("Massage arived ["+String(topic)+"]"+incommingMessage);
//   if(String (topic)=="duythoi092701/iot/sub/quat1"){
//     if(incommingMessage=="on") {
//       digitalWrite(rl_quat1,LOW);
//     }
//     if(incommingMessage=="off") {
//       digitalWrite(rl_quat1,HIGH);
//     }
//   }
//   if(String (topic)=="duythoi092701/iot/sub/quat2"){
//     if(incommingMessage=="on") {
//       digitalWrite(rl_quat2,LOW);
//     }
//     if(incommingMessage=="off") {
//       digitalWrite(rl_quat2,HIGH);
//     }
//   }
//   if(String (topic)=="duythoi092701/iot/sub/bom1"){
//     if(incommingMessage=="on") {
//       digitalWrite(rl_bom1,LOW);
//     }
//     if(incommingMessage=="off") {
//       digitalWrite(rl_bom1,HIGH);
//     }
//   }
//   if(String (topic)=="duythoi092701/iot/sub/bom2"){
//     if(incommingMessage=="on") {
//       digitalWrite(rl_bom2,LOW);
//     }
//     if(incommingMessage=="off") {
//       digitalWrite(rl_bom2,HIGH);
//     }
//   }
//   if(String (topic)=="duythoi092701/iot/sub/den1"){
//     if(incommingMessage=="on") {
//       digitalWrite(rl_den1,LOW);
//     }
//     if(incommingMessage=="off") {
//       digitalWrite(rl_den1,HIGH);
//     }
//   }
//   if(String (topic)=="duythoi092701/iot/sub/den2"){
//     if(incommingMessage=="on") {
//       digitalWrite(rl_den2,LOW);
//     }
//     if(incommingMessage=="off") {
//       digitalWrite(rl_den2,HIGH);
//     }
//   }          
// }
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial1.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "SIMA7680C";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str())) {
      Serial1.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      mqtt.subscribe("duythoi092701/iot/sub/quat1");
      mqtt.subscribe("duythoi092701/iot/sub/quat2");
      mqtt.subscribe("duythoi092701/iot/sub/den1");
      mqtt.subscribe("duythoi092701/iot/sub/den2");
      mqtt.subscribe("duythoi092701/iot/sub/bom1");
      mqtt.subscribe("duythoi092701/iot/sub/bom2");
    } else {
      Serial1.print("failed, rc=");
      Serial1.print(mqtt.state());
      Serial1.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishMQTT(const char* topic,  const String& payload) {
  char payloadBuffer[payload.length() + 1];
  payload.toCharArray(payloadBuffer, payload.length() + 1);
  if (mqtt.connected()) {
    mqtt.publish(topic, payloadBuffer);
  } else {
    Serial1.println("MQTT client is not connected");
  }
}
void drawButton(int x, int y, int width, int height, const char* buttonText, uint16_t textColor, uint16_t bgColor) {
  tft.fillRect(x, y, width, height, bgColor);
  tft.drawRect(x, y, width, height, ST7735_WHITE);
  tft.setCursor(x + 3, y + 6);
  tft.setTextColor(textColor);
  tft.setTextSize(1);
  tft.print(buttonText);
}

void drawButton2(int x, int y, int width, int height, const char* buttonText, uint16_t textColor, uint16_t bgColor) {
  tft.fillRect(x, y, width, height, bgColor);
  tft.drawRect(x, y, width, height, ST7735_WHITE);
  tft.setCursor(x+2, y+2);
  tft.setTextColor(textColor);
  tft.setTextSize(1);
  tft.print(buttonText);
}
void displayText(int x, int y, int textSize, uint16_t textColor, const char* text) {
  tft.setCursor(x, y);
  tft.setTextColor(textColor);
  tft.setTextSize(textSize);
  tft.print(text);
}
char* randomTextFrom1To100() {
  int randomNumber = random(1, 101); 
  static char textBuffer[4]; 
  itoa(randomNumber, textBuffer, 10); 
  return textBuffer; 
}
char* randomTextFrom1To10000() {
  int randomNumber = random(1, 10001); 
  static char textBuffer[6]; 
  itoa(randomNumber, textBuffer, 10); 
  return textBuffer; 
}
void drawBlinkingTick(int x, int y, int size, uint16_t color) {
    tft.drawChar(x, y, 'X', color, ST7735_DARKGREY, size);
    delay(100);
    tft.fillRect(x, y, size*5, size*9, ST7735_DARKGREY);
    delay(100);
}
void drawON(int x, int y){
  tft.fillRect(x,y,36,12, ST7735_BLACK);
  tft.fillRect(x,y,15,12, ST7735_RED);
  tft.drawRect(x, y,36, 12, ST7735_WHITE);
  tft.drawLine(x+15,y,x+15,y+11,ST7735_WHITE);
  tft.setCursor(x+2,y+2);
  tft.setTextColor(ST7735_ORANGE);
  tft.setTextSize(1);
  tft.print("ON");
  tft.setCursor(x+17,y+2);
  tft.print("OFF");  
}
void drawOFF(int x, int y){
  tft.fillRect(x,y,36,12, ST7735_BLACK);
  tft.fillRect(x+15,y,21,12, ST7735_RED);
  tft.drawRect(x, y,36, 12, ST7735_WHITE);
  tft.drawLine(x+15,y,x+15,y+11,ST7735_WHITE);
  tft.setCursor(x+2,y+2);
  tft.setTextColor(ST7735_ORANGE);
  tft.setTextSize(1);
  tft.print("ON");
  tft.setCursor(x+17,y+2);
  tft.print("OFF");  
}
void drawMain(){
  manhinhhientai=1;
  tft.fillScreen(ST7735_DARKGREY);
  tft.drawRect(0, 0, 160, 128, ST7735_WHITE);
  tft.drawLine(0,92,160,92,ST7735_WHITE);
  tft.drawLine(80,0,80,92,ST7735_WHITE);
  displayText(8,8,3,ST7735_GREENYELLOW,"IOT");
  displayText(4,40,3,ST7735_GREENYELLOW,"FARM");
  drawButton(130,0,30,20,"HOME",ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton(8,100,30,20,"1.HT",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38,100,30,20,"2.DK",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38,100,30,20,"3.BM",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38+38,100,30,20,"4.RU",ST7735_WHITE,ST7735_BLUE);
  displayText(82,25,2,ST7735_GREENYELLOW,"Status");
  displayText(82,70,2,ST7735_GREENYELLOW,"Bmau:");
}

void drawHT1(){
  manhinhhientai=2;
  tft.fillScreen(ST7735_DARKGREY);
  tft.drawRect(0, 0, 160, 128, ST7735_WHITE);
  tft.drawLine(0,92,160,92,ST7735_WHITE);
  displayText(10,15,2,ST7735_GREENYELLOW,"Thiet Bi 1");
  displayText(8,35,1,ST7735_GREENYELLOW,"Nhiet do kk:");
  displayText(110,35,1,ST7735_GREENYELLOW,"oC");
  displayText(8,45,1,ST7735_GREENYELLOW,"Do am kk:");
  displayText(110,45,1,ST7735_GREENYELLOW,"%");
  displayText(8,55,1,ST7735_GREENYELLOW,"Do am dat:");
  displayText(110,55,1,ST7735_GREENYELLOW,"%");
  displayText(8,65,1,ST7735_GREENYELLOW,"Anh sang:");
  displayText(125,65,1,ST7735_GREENYELLOW,"LUX");
  displayText(8,75,1,ST7735_GREENYELLOW,"Dinh duong:");
  drawButton(130,0,30,20,"HT.1",ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton(8,100,30,20,"Home",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38,100,30,20," NC ",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38,100,30,20," NC ",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38+38,100,30,20,"Next",ST7735_WHITE,ST7735_BLUE); 
}
void drawHT2(){
  manhinhhientai=3;
  tft.fillScreen(ST7735_DARKGREY);
  tft.drawRect(0, 0, 160, 128, ST7735_WHITE);
  tft.drawLine(0,92,160,92,ST7735_WHITE);
  displayText(10,15,2,ST7735_GREENYELLOW,"Thiet Bi 2");
  displayText(8,35,1,ST7735_GREENYELLOW,"Nhiet do kk:");
  displayText(110,35,1,ST7735_GREENYELLOW,"oC");
  displayText(8,45,1,ST7735_GREENYELLOW,"Do am kk:");
  displayText(110,45,1,ST7735_GREENYELLOW,"%");
  displayText(8,55,1,ST7735_GREENYELLOW,"Do am dat:");
  displayText(110,55,1,ST7735_GREENYELLOW,"%");
  displayText(8,65,1,ST7735_GREENYELLOW,"Anh sang:");
  displayText(125,65,1,ST7735_GREENYELLOW,"LUX");
  displayText(8,75,1,ST7735_GREENYELLOW,"Dinh duong:");
  drawButton(130,0,30,20,"HT.2",ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton(8,100,30,20,"Home",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38,100,30,20," NC ",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38,100,30,20," NC ",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38+38,100,30,20,"Next",ST7735_WHITE,ST7735_BLUE); 
}
void drawDK1(){
  manhinhhientai=4;
  tft.fillScreen(ST7735_DARKGREY);
  tft.drawRect(0, 0, 160, 128, ST7735_WHITE);
  tft.drawLine(0,92,160,92,ST7735_WHITE);
  displayText(10,15,2,ST7735_GREENYELLOW,"DieuKhien1");
  displayText(8,35,2,ST7735_GREENYELLOW,"Bom 1:");
  displayText(8,54,2,ST7735_GREENYELLOW,"Den 1:");
  displayText(8,73,2,ST7735_GREENYELLOW,"Quat 1:");
  drawButton(130,0,30,20,"DK.1",ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton(8,100,30,20,"Home",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38,100,30,20," SW ",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38,100,30,20,"vvvv",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38+38,100,30,20,"Next",ST7735_WHITE,ST7735_BLUE); 
}
void drawDK2(){
  manhinhhientai=5;
  tft.fillScreen(ST7735_DARKGREY);
  tft.drawRect(0, 0, 160, 128, ST7735_WHITE);
  tft.drawLine(0,92,160,92,ST7735_WHITE);
  displayText(10,15,2,ST7735_GREENYELLOW,"DieuKhien2");
  displayText(8,35,2,ST7735_GREENYELLOW,"Bom 2:");
  displayText(8,54,2,ST7735_GREENYELLOW,"Den 2:");
  displayText(8,73,2,ST7735_GREENYELLOW,"Quat 2:");
  drawButton(130,0,30,20,"DK.2",ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton(8,100,30,20,"Home",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38,100,30,20," SW ",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38,100,30,20,"vvvv",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38+38,100,30,20,"Next",ST7735_WHITE,ST7735_BLUE); 
}
void drawBM1(){
  manhinhhientai=6;
  tft.fillScreen(ST7735_DARKGREY);
  tft.drawRect(0, 0, 160, 128, ST7735_WHITE);
  tft.drawLine(0,92,160,92,ST7735_WHITE);
  drawButton(130,0,30,20,"BM.1",ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton(8,100,30,20,"Home",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38,100,30,20,"++++",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38,100,30,20,"vvvv",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38+38,100,30,20,"Next",ST7735_WHITE,ST7735_BLUE); 
  displayText(10,15,2,ST7735_GREENYELLOW,"BieuMau1");
  displayText(8,35,1,ST7735_GREENYELLOW,"Nhiet do kk <");
  displayText(130,35,1,ST7735_GREENYELLOW,"oC");  
  displayText(8,49,1,ST7735_GREENYELLOW,"Do am kk <");
  displayText(130,49,1,ST7735_GREENYELLOW,"%");
  displayText(8,63,1,ST7735_GREENYELLOW,"Do am dat >");
  displayText(130,63,1,ST7735_GREENYELLOW,"%"); 
  displayText(8,77,1,ST7735_GREENYELLOW,"Anh sang >");
  displayText(130,77,1,ST7735_GREENYELLOW,"LUX");
  char buffer[4]=""; 
  drawButton2(90,35,30,10,itoa(snd1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton2(90,49,30,10,itoa(sda1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton2(90,63,30,10,itoa(sdad1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);  
  drawButton2(90,77,35,10,itoa(sas1, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);  
}
void drawBM2(){
  manhinhhientai=7;
  tft.fillScreen(ST7735_DARKGREY);
  tft.drawRect(0, 0, 160, 128, ST7735_WHITE);
  tft.drawLine(0,92,160,92,ST7735_WHITE);
  drawButton(130,0,30,20,"BM.2",ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton(8,100,30,20,"Home",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38,100,30,20,"++++",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38,100,30,20,"vvvv",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38+38,100,30,20,"Next",ST7735_WHITE,ST7735_BLUE); 
  displayText(10,15,2,ST7735_GREENYELLOW,"BieuMau2");
  displayText(8,35,1,ST7735_GREENYELLOW,"Nhiet do kk <");
  displayText(130,35,1,ST7735_GREENYELLOW,"oC");   
  displayText(8,49,1,ST7735_GREENYELLOW,"Do am kk <");
  displayText(130,49,1,ST7735_GREENYELLOW,"%");
  displayText(8,63,1,ST7735_GREENYELLOW,"Do am dat >");
  displayText(130,63,1,ST7735_GREENYELLOW,"%"); 
  displayText(8,77,1,ST7735_GREENYELLOW,"Anh sang >");
  displayText(130,77,1,ST7735_GREENYELLOW,"LUX");
  char buffer[4]=""; 
  drawButton2(90,35,30,10,itoa(snd2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton2(90,49,30,10,itoa(sda2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton2(90,63,30,10,itoa(sdad2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);  
  drawButton2(90,77,35,10,itoa(sas2, buffer, 10),ST7735_MAGENTA,ST7735_DARKGREEN);     
}
void drawRBM(){
  manhinhhientai=8;
  tft.fillScreen(ST7735_DARKGREY);  
  tft.drawRect(0, 0, 160, 128, ST7735_WHITE);
  tft.drawLine(0,92,160,92,ST7735_WHITE);
  drawButton(130,0,30,20,"RBM",ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton(8,100,30,20,"Home",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38,100,30,20," SW ",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38,100,30,20,"vvvv",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38+38,100,30,20,"Next",ST7735_WHITE,ST7735_BLUE); 
  displayText(10,15,3,ST7735_GREENYELLOW,"Run BMau");
  displayText(8,45,2,ST7735_GREENYELLOW,"BMau1:");
  displayText(8,70,2,ST7735_GREENYELLOW,"BMau2:");
  if(BM2_State==1) drawON(80,70); else drawOFF(80,70);
  if(BM1_State==1) drawON(80,45); else drawOFF(80,45);
}
void drawAutoOrManual(){
  manhinhhientai=9;
  tft.fillScreen(ST7735_DARKGREY);  
  tft.drawRect(0, 0, 160, 128, ST7735_WHITE);
  tft.drawLine(0,92,160,92,ST7735_WHITE);
  drawButton(130,0,30,20,"RBM",ST7735_MAGENTA,ST7735_DARKGREEN);
  drawButton(8,100,30,20,"Home",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38,100,30,20," SW ",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38,100,30,20,"vvvv",ST7735_WHITE,ST7735_BLUE);
  drawButton(8+38+38+38,100,30,20,"Next",ST7735_WHITE,ST7735_BLUE); 
  displayText(10,15,3,ST7735_GREENYELLOW,"AuOrMn");
  displayText(8,45,2,ST7735_GREENYELLOW,"Auto:");
  displayText(8,70,2,ST7735_GREENYELLOW,"Manual:");
  if(AuOrManual==1) {
    drawON(80,70);
    drawOFF(80,45);
  } 
  else {
    drawOFF(80,70);
    drawON(80,45);
  }
}
void mqttCallback(char* topic, byte* payload, unsigned int len)
{
  String incommingMessage = "";
  for(int i=0; i<len;i++) incommingMessage += (char)payload[i];
  Serial1.println("Massage arived ["+String(topic)+"]"+incommingMessage);
  if(String (topic)=="duythoi092701/iot/sub/quat1"){
    if(incommingMessage=="on") {
      quat1=1;
    }
    if(incommingMessage=="off") {
      quat1=0;
    }
  }
  if(String (topic)=="duythoi092701/iot/sub/quat2"){
    if(incommingMessage=="on") {
      quat2=1;
    }
    if(incommingMessage=="off") {
      quat2=0;
    }
  }
  if(String (topic)=="duythoi092701/iot/sub/bom1"){
    if(incommingMessage=="on") {
      bom1=1;
    }
    if(incommingMessage=="off") {
      bom1=0;
    }
  }
  if(String (topic)=="duythoi092701/iot/sub/bom2"){
    if(incommingMessage=="on") {
      bom2=1;
    }
    if(incommingMessage=="off") {
      bom2=0;
    }
  }
  if(String (topic)=="duythoi092701/iot/sub/den1"){
    if(incommingMessage=="on") {
      den1=1;
    }
    if(incommingMessage=="off") {
      den1=0;
    }
  }
  if(String (topic)=="duythoi092701/iot/sub/den2"){
    if(incommingMessage=="on") {
      den2=1;
    }
    if(incommingMessage=="off") {
      den2=0;
    }
  }          
}
boolean mqttConnect()
{
  if(!mqtt.connect("GsmClientTest"))
  {
    Serial1.print(".");
    return false;
  }
  Serial1.println("Connected to broker.");
      mqtt.subscribe("duythoi092701/iot/sub/quat1");
      mqtt.subscribe("duythoi092701/iot/sub/quat2");
      mqtt.subscribe("duythoi092701/iot/sub/den1");
      mqtt.subscribe("duythoi092701/iot/sub/den2");
      mqtt.subscribe("duythoi092701/iot/sub/bom1");
      mqtt.subscribe("duythoi092701/iot/sub/bom2");
  return mqtt.connected();
}