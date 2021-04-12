#include <Arduino.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WS2812FX.h>

#define LED_COUNT 3
#define LED_PIN D1

const char* ssid = "Airtel_6395973808";
const char* password = "air54501";

WiFiManager wm;

// Optional - you can define air pressure correction for your altitude
#define ALT_CORRECTION 3144
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

SoftwareSerial mySerial(D6, D5);
uint16_t temp1=0;
int16_t temp2=0;

unsigned char Re_buf[30],counter=0;
unsigned char sign=0;
int led = 13;

WiFiClient client;

unsigned long prev = millis();

// You need to use modified Adafruit library with support of small displays 
// see here: https://github.com/mcauser/Adafruit_SSD1306/tree/esp8266-64x48
void connectWiFi(){
  //---------------------------------
  Serial.println("Connecting to wifi...");
  // WiFi.begin(ssid, password);

  // while (WiFi.status() != WL_CONNECTED){
    
  //   // Blink LED when connecting to wifi
  //   pinMode(LED_BUILTIN, OUTPUT);
  //   digitalWrite(LED_BUILTIN, LOW);
  //   delay(150);
  //   digitalWrite(LED_BUILTIN, HIGH);
  //   delay(150);
  // }
  //---------------------------------
      WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP    
    // put your setup code here, to run once:
    Serial.begin(115200);
    
    //reset settings - wipe credentials for testing
    //wm.resetSettings();

    wm.setConfigPortalBlocking(false);
    wm.setWiFiAutoReconnect(true);
    wm.setConnectTimeout(15);
    //automatically connect using saved credentials if they exist
    //If connection fails it starts an access point with the specified name
    if(wm.autoConnect("VayuMon v1.0")){
        Serial.println("connected...yeey :)");
    }
    else {
        Serial.println("Configportal running");
    }
}
void setupNeo(){
    ws2812fx.init();
  ws2812fx.setBrightness(170);

  // parameters: index, start, stop, mode, color, speed, reverse
  ws2812fx.setSegment(0,  0,  0, FX_MODE_STATIC, BLUE, 1000, false); // segment 0 is leds 0 - 9
  ws2812fx.setSegment(1, 1, 1, FX_MODE_STATIC,  ORANGE, 1000, false); // segment 1 is leds 10 - 19
  ws2812fx.setSegment(2, 2, 2, FX_MODE_STATIC, GREEN, 1000, true);  // segment 2 is leds 20 - 29

  ws2812fx.start();
}
void setup()
{
  Serial.begin(115200);
  delay(1000);
setupNeo();
   connectWiFi();
  
  
  pinMode(D2, INPUT);
  pinMode(D1, OUTPUT);
  mySerial.begin(9600);
  mySerial.listen();  
  delay(5000);    
 
  mySerial.write(0XA5); 
  mySerial.write(0X55);    
  mySerial.write(0X3F);    
  mySerial.write(0X39); 
  delay(100); 

  mySerial.write(0XA5); 
  mySerial.write(0X56);    
  mySerial.write(0X02);    
  mySerial.write(0XFD);
  Serial.println("BME680 Setup complete");

  delay(100);  
}

void loop(){
  wm.process();
    ws2812fx.service();

  float Temperature ,Humidity;
  unsigned char i=0,sum=0;
  uint32_t Gas;
  uint32_t Pressure;
  uint16_t IAQ;
  int16_t  Altitude;
  uint8_t IAQ_accuracy;
  //Serial.println("Checking serial...");
  while (mySerial.available()) {
    //Serial.println("mySerial available");   
    Re_buf[counter]=(unsigned char)mySerial.read();
    
    if(counter==0&&Re_buf[0]!=0x5A) {
      Serial.println("Nothing received");  
    }         
    if(counter==1&&Re_buf[1]!=0x5A)
  {
    counter=0;
     return;
   };           
    counter++;       
    if(counter==20)               
    {    
       counter=0;                  
       sign=1;
    }      
  }
  if(sign)
  {  
    sign=0;
    if(Re_buf[0]==0x5A&&Re_buf[1]==0x5A )        
     {      
       for(i=0;i<19;i++)
         sum+=Re_buf[i]; 
       if(sum==Re_buf[i] ) 
         {
           temp2=(Re_buf[4]<<8|Re_buf[5]);   
           Temperature=(float)temp2/100;
           temp1=(Re_buf[6]<<8|Re_buf[7]);
           Humidity=(float)temp1/100; 
           Pressure=((uint32_t)Re_buf[8]<<16)|((uint16_t)Re_buf[9]<<8)|Re_buf[10];
           float altPressure=(float(Pressure)+ALT_CORRECTION)/100;
           IAQ_accuracy= (Re_buf[11]&0xf0)>>4;
           IAQ=((Re_buf[11]&0x0F)<<8)|Re_buf[12];
           Gas=((uint32_t)Re_buf[13]<<24)|((uint32_t)Re_buf[14]<<16)|((uint16_t)Re_buf[15]<<8)|Re_buf[16];
           Altitude=(Re_buf[17]<<8)|Re_buf[18]; 
           Serial.print("Temperature:");
           Serial.print(Temperature); 
           Serial.print(" ,Humidity:"); 
           Serial.print(Humidity); 
           Serial.print(" ,Pressure:"); 
           Serial.print(Pressure);
           Serial.print("|");
           Serial.print(altPressure);      
           Serial.print("  ,IAQ:");
           Serial.print(IAQ); 
           Serial.print(" ,Gas:"); 
           Serial.print(Gas); 
           Serial.print("  ,Altitude:"); 
           Serial.print(Altitude);                       
           Serial.print("  ,IAQ_accuracy:"); 
           Serial.println(IAQ_accuracy);
           if (IAQ > 301 && IAQ < 500)
           {
             ws2812fx.setColor(2, MAGENTA);
            //  Serial.println("Magenta");
           }
           if (IAQ > 201 && IAQ < 300)
           {
             ws2812fx.setColor(2, 0x8A2BE2); //purple
            //  Serial.println("Purple/");
           }
           if (IAQ > 151 && IAQ < 200)
           {
             ws2812fx.setColor(2, 0xFF0000);
            //  Serial.println("RED");
           }
           if (IAQ > 101 && IAQ < 150)
           {
             ws2812fx.setColor(2, ORANGE);
            //  Serial.println("orange");
           }
           if (IAQ > 51 && IAQ < 100)
           {
             ws2812fx.setColor(2, YELLOW);
            //  Serial.println("Yellow");
           }
           if (IAQ < 50)
           {
             ws2812fx.setColor(2, 0x00FF00);
            //  Serial.println("Green");
           }

           //For Temp
           if (Temperature > 51 && Temperature < 85)
           {
             ws2812fx.setColor(1, RED);
            //  Serial.println("orange");
           }

           if (Temperature > 31 && Temperature < 50)
           {
             ws2812fx.setColor(1, ORANGE);
            //  Serial.println("orange");
           }
           if (Temperature > 21 && Temperature < 30)
           {
             ws2812fx.setColor(1, YELLOW);
            //  Serial.println("Yellow");
           }
           if (Temperature < 20)
           {
             ws2812fx.setColor(1, BLUE);
            //  Serial.println("Green");
           }

               //For Humidity
           if (Humidity > 51 && Humidity < 85)
           {
             ws2812fx.setColor(0, RED);
            //  Serial.println("orange");
           }

           if (Humidity > 31 && Humidity < 50)
           {
             ws2812fx.setColor(0, ORANGE);
            //  Serial.println("orange");
           }
           if (Humidity > 21 && Humidity < 30)
           {
             ws2812fx.setColor(0, YELLOW);
            //  Serial.println("Yellow");
           }
           if (Humidity < 20)
           {
             ws2812fx.setColor(0, BLUE);
            //  Serial.println("Green");
           }

            ws2812fx.show();
          
         }            
         delay(1000);           
     }
  } 
} 
