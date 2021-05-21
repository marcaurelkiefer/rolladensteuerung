#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WifiManager.h>
#include <NTPClient.h>
#include <Timezone.h>
#include <EEPROM.h>
#include <OneButton.h>
#include <ArduinoOTA.h>

#include "config.h"

OneButton btn_up_increase(BTN_UP_INCREASE);
OneButton btn_up_decrease(BTN_UP_DECREASE);
OneButton btn_down_increase(BTN_DOWN_INCREASE);
OneButton btn_down_decrease(BTN_DOWN_DECREASE);

int hour_up = 9;
int minute_up = 0;

int hour_down = 22;
int minute_down = 00;

/**
 * Input time in epoch format and return tm time format
 * by Renzo Mischianti <www.mischianti.org> 
 */
static tm getDateTimeByParams(long time){
    struct tm *newtime;
    const time_t tim = time;
    newtime = localtime(&tim);
    return *newtime;
}
/**
 * Input tm time format and return String with format pattern
 * by Renzo Mischianti <www.mischianti.org>
 */
static String getDateTimeStringByParams(tm *newtime, char* pattern = (char *)"%d/%m/%Y %H:%M:%S"){
    char buffer[30];
    strftime(buffer, 30, pattern, newtime);
    return buffer;
}
 
/**
 * Input time in epoch format format and return String with format pattern
 * by Renzo Mischianti <www.mischianti.org> 
 */
static String getEpochStringByParams(long time, char* pattern = (char *)"%d.%m.%Y %H:%M:%S"){
//    struct tm *newtime;
    tm newtime;
    newtime = getDateTimeByParams(time);
    return getDateTimeStringByParams(&newtime, pattern);
}

void writeConfig() {
  EEPROM.begin(512);
  EEPROM.write(1,hour_up);
  EEPROM.write(2,minute_up);
  EEPROM.write(3,hour_down);
  EEPROM.write(4,minute_down);
  EEPROM.commit();
}

void readConfig() {
  EEPROM.begin(512);
  hour_up = EEPROM.read(1);
  minute_up = EEPROM.read(2);
  hour_down = EEPROM.read(3);
  minute_down = EEPROM.read(4);
}

void increase_up() {
  if (minute_up == 0) {
    minute_up = 30;
  }
  else {
    minute_up = 0;
    hour_up++;
  }
  if (hour_up > 23) {
    hour_up = 0;
  }
}

void decrease_up() {
  if (minute_up == 30) {
    minute_up = 0;
  }
  else {
    minute_up = 30;
    hour_up--;
  }
  if (hour_up < 0) {
    hour_up = 23;
  }
}

void increase_down() {
  if (minute_down == 0) {
    minute_down = 30;
  }
  else {
    minute_down = 0;
    hour_down++;
  }
  if (hour_down > 23) {
    hour_down = 0;
  }
}

void decrease_down() {
  if (minute_down == 30) {
    minute_down = 0;
  }
  else {
    minute_down = 30;
    hour_down--;
  }
  if (hour_down < 0) {
    hour_down = 23;
  }
}

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiManager wifiManager;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);

void btn_up_increase_click() {
  increase_up();
  writeConfig();
}

void btn_up_decrease_click() {
  decrease_up();
  writeConfig();
}

void btn_down_increase_click() {
  increase_down();
  writeConfig();
}

void btn_down_decrease_click() {
  decrease_down();
  writeConfig();
}

void tickButtons() {
  btn_up_increase.tick();
  btn_up_decrease.tick();
  btn_down_increase.tick();
  btn_down_decrease.tick();
}

void checkAndMoveRolladen() {
  if (hour() == hour_up && minute() == minute_up) {
    digitalWrite(OUTPUT_ROLLADEN_UP, HIGH);
  } else {
    digitalWrite(OUTPUT_ROLLADEN_UP, LOW);
  }
  if (hour() == hour_down && minute() == minute_down) {
    digitalWrite(OUTPUT_ROLLADEN_DOWN, HIGH);
  } else {
    digitalWrite(OUTPUT_ROLLADEN_DOWN, LOW);
  }
}

void initOutputs() {
  digitalWrite(OUTPUT_ROLLADEN_UP, LOW);
  digitalWrite(OUTPUT_ROLLADEN_DOWN, LOW);
  pinMode(OUTPUT_ROLLADEN_UP, OUTPUT);
  pinMode(OUTPUT_ROLLADEN_DOWN, OUTPUT);
  
}

void setup() {
  ArduinoOTA.setHostname("flashrolladen");
  ArduinoOTA.setPassword("antigua");
  ArduinoOTA.begin();
  initOutputs();
  btn_up_increase.attachClick(btn_up_increase_click);
  btn_up_decrease.attachClick(btn_up_decrease_click);
  btn_down_increase.attachClick(btn_down_increase_click);
  btn_down_decrease.attachClick(btn_down_decrease_click);
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Waiting for Wifi"));
  display.display();
  wifiManager.autoConnect("Rolladen");
  timeClient.begin();
  //writeConfig();
  readConfig();
}

void updateDisplay(){
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println("Wifi: " + wifiManager.getSSID());
  display.println(getEpochStringByParams(CE.toLocal(now())));
  display.println(" ");
  display.setTextSize(2);
  display.print((char)24);
  if (hour_up < 10) {
    display.print("0");
  } 
  display.print(hour_up);
  display.print(":");
  if (minute_up < 10) {
    display.print("0");
  } 
  display.println(minute_up);
  display.print((char) 25);
  if (hour_down < 10) {
    display.print("0");
  } 
  display.print(hour_down);
  display.print(":");
  if (minute_down < 10) {
    display.print("0");
  } 
  display.println(minute_down);
  display.setTextSize(1);
  display.println("Debug Line");
  display.display();
}

void loop() {
  ArduinoOTA.handle();
  if (timeClient.update()){
     Serial.print ( "Adjust local clock" );
     unsigned long epoch = timeClient.getEpochTime();
     setTime(epoch);
  }
  tickButtons();
  checkAndMoveRolladen();
  updateDisplay();
}

