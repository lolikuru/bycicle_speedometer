const int hall = A0;
const int sun_sensor = A1;
#define VT_PIN A2 
#define AT_PIN A3

int RawValue= 0;
float Current = 0;
long fix_millis;
long k = 0;
long r_time = 0;
boolean bound = false;
boolean pulse_on = false;

int brightness = 0;
int fadeAmount = 5;

byte InterruptPin = 2;
volatile bool f = 0;

#include <avr/wdt.h>
#include <avr/sleep.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const unsigned char PROGMEM x_logo_bmp[] = 
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

void setup() {
  pinMode(hall, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(InterruptPin, INPUT);
  
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
  //Serial.println(F("SSD1306 allocation failed"));
  for(;;); // Don't proceed, loop forever
  }
  //display.display();
  delay(2000); // Pause for 2 seconds
  pinMode(9, OUTPUT);

  // Clear the buffer
  display.clearDisplay();
}

void loop() {
  fix_millis = millis();
  int sum_hall = 0;
  int sum_sunh = 0;
  const float R = 2068; //окружность колеса 26x2.1 в мм
  for(int i = 0; i < 10; i++){
      int value0 = analogRead(hall);
      int value1 = analogRead(sun_sensor);
      sum_hall += value0;
      sum_sunh += value1;
  }
  sum_hall = sum_hall / 10;
  sum_sunh = sum_sunh / 10;

  if (fix_millis-r_time>5000) {//если долго не крутили колесо, сбрасываем
    r_time=0; k=0;
    display.clearDisplay();//выключить дисплей, если нет движения
    display.display();
    //delay(200);
    pulse_on = false;
    analogWrite(9, 0);
    attachInterrupt(digitalPinToInterrupt(InterruptPin), myISR, HIGH); // Прерывание по низкому уровню
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Устанавливаем интересующий нас режим
    sleep_mode(); // Переводим МК в спящий режим
    detachInterrupt(digitalPinToInterrupt(InterruptPin));
    }
    
  if(sum_hall > 440 && bound == false){ 
    bound = true;
    pulse_on = true;
   if(r_time!=0){
    k=fix_millis-r_time;
    r_time=fix_millis;
   }else 
    if(r_time==0) {//первый прокрут
    r_time=fix_millis;
   }
  }

  if(sum_hall < 440 && bound == true){
    bound = false;
  }

  if(fix_millis%10==0 && pulse_on == true && sum_sunh < 500){//шим на 9 пине
    analogWrite(9, brightness);
  
    // change the brightness for next time through the loop:
    brightness = brightness + fadeAmount;
  
    // reverse the direction of the fading at the ends of the fade:
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
  }
//  if(fix_millis%200==0)


  
  if(fix_millis%100==0)
  { if (pulse_on==true)
    pulse(brightness);    
   float speed_r = R/k*3.6;
   if (speed_r > 120) speed_r=0;
   
  //Serial.print("hall = ");
  //Serial.print(sum_hall);
  //Serial.print("\n");
  //delay(100);
  display.display();
  //delay(1000);
  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  //display.setCursor(0,0);
  
  //display.drawLine(0, 0, 5, 5, WHITE);
     //display.drawBitmap(0, 0, x_logo_bmp, 16, 16, 1);
  //display.display();
  
  display.setCursor(0,0);
  //display.print("hall=" + String(sum_hall));
  //display.setCursor(0,16);
  display.print(String(speed_r) + "kmH");
  //display.setCursor(0,16);
  //display.print(sum_sunh);//датчик света
  //Serial.println(String(speed_r) + "km\/h");
  power_control();
  }
}

void pulse (int br){
  byte n = map(br,0,255,1,6);
  for(int16_t i=0; i<n; i++) 
  display.drawCircle(112-15/2, 15/2, i, WHITE);
}

void power_control() {
  int vt_read = analogRead(VT_PIN);
  int at_read = analogRead(AT_PIN);
  String output;

  float voltage = vt_read * (5.0 / 1024.0) * 5.0;
  float current = at_read * (5.0 / 1024.0);
  float watts = voltage * current;
  display.setTextSize(1);
  display.setCursor(0,16);
  output+=(String(voltage).substring(0,4)+ "V "+ 
           String(current).substring(0,4)+ "A "+
           String(watts).substring(0,4)+ "W");
  display.print(output);  
}

void myISR() {
  f = !f;
}
