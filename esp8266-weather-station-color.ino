/**The MIT License (MIT)
  Copyright (c) 2017 by Daniel Eichhorn
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  See more at https://blog.squix.org
*/

/*****************************
   Important: see settings.h to configure your settings!!!
 * ***************************/
#include "settings.h"
#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
//#include "hw_timer.h"
}
#include "TM1637.h"
/***
   Install the following libraries through Arduino Library Manager
   - Mini Grafx by Daniel Eichhorn
   - ESP8266 WeatherStation by Daniel Eichhorn
   - Json Streaming Parser by Daniel Eichhorn
   - simpleDSTadjust by neptune2
 ***/

#include <JsonListener.h>
#include "HeweatherForecast.h"
#include <MiniGrafx.h>
#include <ILI9341_SPI.h>
#include "ArialRounded.h"
#include <FS.h>
#include "UTF-8toGB2312.h"

#define MINI_BLACK 0
#define MINI_WHITE 1
#define MINI_YELLOW 2
#define MINI_BLUE 3

#define MAX_FORECASTS 3

#define CLK D4 //pins definitions for TM1637 and can be changed to other ports
#define DIO D0
TM1637 tm1637(CLK, DIO);
os_timer_t TimerClockPoint;
boolean ClockPoint = true;
// defines the colors usable in the paletted 16 color frame buffer
uint16_t palette[] = {ILI9341_BLACK,  // 0
                      ILI9341_WHITE,  // 1
                      ILI9341_YELLOW, // 2
                      0x7E3C};        //3

int SCREEN_WIDTH = 240;
int SCREEN_HEIGHT = 320;
int ROTATION = 0;
// Limited to 4 colors due to memory constraints
int BITS_PER_PIXEL = 2; // 2^2 =  4 colors

ADC_MODE(ADC_VCC);
// TM1637 tm1637(CLK,DIO);
ILI9341_SPI tft = ILI9341_SPI(TFT_CS, TFT_DC);
MiniGrafx gfx = MiniGrafx(&tft, BITS_PER_PIXEL, palette);

Heweather weather;
// Setup simpleDSTadjust Library rules
simpleDSTadjust dstAdjusted(StartRule, EndRule);

bool printchs(String str, int x, int y, uint16_t color = MINI_WHITE, String font = "HZK16");
void drawchar(u16 chr, int x, int y, uint16_t color = MINI_WHITE, String font = "HZK16");
void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawForecast();
void drawLabelValue(uint8_t line, String label, String value);
void drawAbout();
void TimeShow();
//void TimerClockPoint_Callback(void *pArg);
void TimerClockPoint_Callback();
void connectWifi();

// how many different screens do we have?
int screenCount = 5;
long lastDownloadUpdate = millis();
long RefreshTime = millis();
long RefreshDisplay = millis();
String moonAgeImage = "";
int screen = 0;
long timerPress;
bool canBtnPress;
bool isDaytime = true;
long lastDrew = 0;
bool btnClick;

void setup()
{
  Serial.begin(115200);

  Serial.println(TFT_LED);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH); // HIGH to Turn on;

  gfx.init();
  gfx.setRotation(ROTATION);
  gfx.fillBuffer(MINI_BLACK);
  gfx.commit();

  connectWifi();

  // ts.begin();

  bool isFSMounted = SPIFFS.begin();
  if (!isFSMounted)
  {
    SPIFFS.format();
  }
  //SPIFFS.remove("/calibration.txt");

  // update the weather information
  updateData();
  timerPress = millis();
  canBtnPress = true;

  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL); //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

  // 123_timer_arm(&TimerClockPoint, 600, true); /** 启动该定时器 */
       /** 关闭该定时器 */
       os_timer_disarm( &TimerClockPoint );
       /** 配置该定时器回调函数 */
       os_timer_setfn( &TimerClockPoint, (ETSTimerFunc *) ( TimerClockPoint_Callback ), NULL );
       /** 启动该定时器 */
       os_timer_arm( &TimerClockPoint, 500, true );
  // hw_timer_init(FRC1_SOURCE, 1);
  // hw_timer_set_func(TimerClockPoint_Callback);
  // hw_timer_arm(50000);
}

void loop()
{
  gfx.fillBuffer(MINI_BLACK);

  // TimerClockPoint_Callback();

  // else if (screen == 2) {
  //   drawAbout();
  // }

  // Check if we should update weather information

  // if (SLEEP_INTERVAL_SECS && millis() - timerPress >= SLEEP_INTERVAL_SECS * 1000)
  // { // after 2 minutes go to sleep
  //   drawProgress(25, "Going to Sleep!");
  //   delay(1000);
  //   drawProgress(50, "Going to Sleep!");
  //   delay(1000);
  //   drawProgress(75, "Going to Sleep!");
  //   delay(1000);
  //   drawProgress(100, "Going to Sleep!");
  //   // go to deepsleep for xx minutes or 0 = permanently
  //   ESP.deepSleep(0, WAKE_RF_DEFAULT); // 0 delay = permanently to sleep
  // }
  // if (millis() - RefreshTime > 500)
  // {
  //   // updateData();
  //   RefreshTime = millis();
  //   TimerClockPoint_Callback();
  //   TimeShow();
  //   //drawTime();
  //  // gfx.commit();
  // }
  if (millis() - RefreshDisplay > 5000)
  {
    //updateData();
    RefreshDisplay = millis();
    drawTime();
    TimeShow();
    switch (screen)
    {
    case 0:
      drawNowWeather();
      break;
    case 1:
      drawForecast();
      break;
    case 2:
      drawNowWeather();
      break;
    case 3:
      drawForecast();
      break;
    default:
      drawNowWeather();
      break;
    }
    screen=(screen>3)?0:screen+1;
   gfx.commit();
  }
  
  if (millis() - lastDownloadUpdate > 1000 * UPDATE_INTERVAL_SECS)
  {
    updateData();
    lastDownloadUpdate = millis();
  }
  //delay(1000);
}

// Update the internet based information and update screen
void updateData()
{

  gfx.fillBuffer(MINI_BLACK);
  gfx.setFont(ArialRoundedMTBold_14);

  drawProgress(10, "Updating time...");
  configTime(UTC_OFFSET * 3600, 0, NTP_SERVERS);

  drawProgress(70, "Updating forecasts...");
  HeweatherForecast *forecastClient = new HeweatherForecast(IS_METRIC);
  //forecastClient->updateForecast(forecasts, MAX_FORECASTS, WUNDERGRROUND_API_KEY, WUNDERGROUND_CITY);
  forecastClient->updateForecast();
  weather = forecastClient->getweather();
  delete forecastClient;
  forecastClient = nullptr;

  delay(500);
}

// Progress bar helper
void drawProgress(uint8_t percentage, String text)
{
  int positionX;
  int positionY;
  positionX = (ROTATION % 2 == 1) ? SCREEN_HEIGHT : SCREEN_WIDTH;
  positionY = (ROTATION % 2 == 1) ? SCREEN_WIDTH : SCREEN_HEIGHT;
  gfx.fillBuffer(MINI_BLACK);
  //gfx.drawPalettedBitmapFromPgm(23, 30, SquixLogo);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(positionX / 2, 80, "https://www.miaodi.me");
  gfx.setColor(MINI_YELLOW);

  gfx.drawString(positionX / 2, 146, text);
  gfx.setColor(MINI_WHITE);
  gfx.drawRect(10, 168, positionX - 20, 15);
  gfx.setColor(MINI_BLUE);
  gfx.fillRect(12, 170, (positionX - 24) * percentage / 100, 11);

  gfx.commit();
}

// draws the clocktest
void drawTime()
{

  // char *dstAbbrev;
  // char time_str[11];
  // time_t now = dstAdjusted.time(&dstAbbrev);
  // struct tm *timeinfo = localtime(&now);

  // gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  // gfx.setFont(ArialRoundedMTBold_14);
  // gfx.setColor(MINI_WHITE);
  // String date = ctime(&now);
  // date = String(1900 + timeinfo->tm_year) + "-" + String(timeinfo->tm_mon + 1) + "-" + String(timeinfo->tm_mday) + " " + date.substring(0, 4);
  // gfx.drawString(160, 6, date);

  // gfx.setFont(ArialRoundedMTBold_36);
  // if (timeinfo->tm_hour > 6 && timeinfo->tm_hour < 18)
  // {
  //   isDaytime = true;
  // }
  // else
  // {
  //   isDaytime = false;
  // }
  // if (IS_STYLE_12HR)
  // {
  //   int hour = (timeinfo->tm_hour + 11) % 12 + 1; // take care of noon and midnight
  //   sprintf(time_str, "%2d:%02d:%02d\n", hour, timeinfo->tm_min, timeinfo->tm_sec);
  //   gfx.drawString(160, 20, time_str);
  // }
  // else
  // {
  //   sprintf(time_str, "%02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  //   gfx.drawString(160, 20, time_str);
  // }

  // gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  // gfx.setFont(ArialMT_Plain_10);
  // gfx.setColor(MINI_BLUE);
  // if (IS_STYLE_12HR)
  // {
  //   sprintf(time_str, "%s\n%s", dstAbbrev, timeinfo->tm_hour >= 12 ? "PM" : "AM");
  //   gfx.drawString(260, 27, time_str);
  // }
  // else
  // {
  //   sprintf(time_str, "%s", dstAbbrev);
  //   gfx.drawString(260, 27, time_str); // Known bug: Cuts off 4th character of timezone abbreviation
  // }
  drawWifiQuality();
}

// draws current weather information

void drawForecast()
{
  int positionX = 1;
  int positionY = 20;
  gfx.setTransparentColor(MINI_BLACK);
  String degreeSign = "°C";
  for (int8_t i = 0; i < 3; i++)
  {
    gfx.setFont(ArialRoundedMTBold_14);
    gfx.setColor(MINI_WHITE);
    gfx.setTextAlignment(TEXT_ALIGN_LEFT);
    gfx.drawString(positionX + 50, positionY + 5 + i * 55, weather.HeForecast[i].tmp_max + degreeSign);
    gfx.drawString(positionX + 120, positionY + 5 + i * 55, weather.HeForecast[i].tmp_min + degreeSign);
    printchs(weather.HeForecast[i].cond_txt_d, positionX + 50, positionY + 25 + i * 55);
    printchs(weather.HeForecast[i].cond_txt_n, positionX + 120, positionY + 25 + i * 55);
    gfx.drawBmpFromFile("/weather_icon/small/" + weather.HeForecast[i].cond_code_d + ".bmp", positionX, positionY + i * 55);
    gfx.drawBmpFromFile("/weather_icon/small/" + weather.HeForecast[i].cond_code_n + ".bmp", positionX + 180, positionY + i * 55);
  }
  gfx.commit();
}
void drawNowWeather()
{
  gfx.setTransparentColor(MINI_BLACK);
  gfx.drawBmpFromFile("/weather_icon/big/" + weather.now_cond_code + ".bmp", 10, 20);
  gfx.setFont(ArialRoundedMTBold_36);
  gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  String degreeSign = "°C";
  String temp = weather.now_tmp + degreeSign;
  gfx.drawString(220, 20, temp);
  printchs(weather.now_cond_txt, 135, 60);
  //printchs(weather.now_tmp, 135, 110);
  printchs("风力：" + weather.now_wind_sc + "级", 10, 130);
  printchs("风向：" + weather.now_wind_dir, 10, 150);
  printchs("相对湿度：" + weather.now_hum, 10, 170);
  printchs("能见度：" + weather.now_vis, 10, 190);
  gfx.commit();
}
void drawLabelValue(uint8_t line, String label, String value)
{
  const uint8_t labelX = 15;
  const uint8_t valueX = 150;
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_YELLOW);
  gfx.drawString(labelX, 30 + line * 15, label);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(valueX, 30 + line * 15, value);
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality()
{
  int32_t dbm = WiFi.RSSI();
  if (dbm <= -100)
  {
    return 0;
  }
  else if (dbm >= -50)
  {
    return 100;
  }
  else
  {
    return 2 * (dbm + 100);
  }
}

void drawWifiQuality()
{
  int positionX;
  int8_t quality = getWifiQuality();
  positionX = (ROTATION % 2 == 1) ? SCREEN_HEIGHT : SCREEN_WIDTH;
  gfx.setColor(MINI_WHITE);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(positionX - 11, 1, String(quality) + "%");
  for (int8_t i = 0; i < 4; i++)
  {
    for (int8_t j = 0; j < 2 * (i + 1); j++)
    {
      if (quality > i * 25 || j == 0)
      {
        gfx.setPixel(positionX - 10 + 2 * i, 10 - j);
      }
    }
  }
}

void drawAbout()
{
  gfx.fillBuffer(MINI_BLACK);
  // gfx.drawPalettedBitmapFromPgm(23, 30, SquixLogo);

  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 80, "https://blog.squix.org");

  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  drawLabelValue(7, "Heap Mem:", String(ESP.getFreeHeap() / 1024) + "kb");
  drawLabelValue(8, "Flash Mem:", String(ESP.getFlashChipRealSize() / 1024 / 1024) + "MB");
  drawLabelValue(9, "WiFi Strength:", String(WiFi.RSSI()) + "dB");
  drawLabelValue(10, "Chip ID:", String(ESP.getChipId()));
  drawLabelValue(11, "VCC: ", String(ESP.getVcc() / 1024.0) + "V");
  drawLabelValue(12, "CPU Freq.: ", String(ESP.getCpuFreqMHz()) + "MHz");
  char time_str[15];
  const uint32_t millis_in_day = 1000 * 60 * 60 * 24;
  const uint32_t millis_in_hour = 1000 * 60 * 60;
  const uint32_t millis_in_minute = 1000 * 60;
  uint8_t days = millis() / (millis_in_day);
  uint8_t hours = (millis() - (days * millis_in_day)) / millis_in_hour;
  uint8_t minutes = (millis() - (days * millis_in_day) - (hours * millis_in_hour)) / millis_in_minute;
  sprintf(time_str, "%2dd%2dh%2dm", days, hours, minutes);
  drawLabelValue(13, "Uptime: ", time_str);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_YELLOW);
  gfx.drawString(15, 250, "Last Reset: ");
  gfx.setColor(MINI_WHITE);
  gfx.drawStringMaxWidth(15, 265, 240 - 2 * 15, ESP.getResetInfo());
}

void drawchar(u16 chr, int x, int y, uint16_t color, String font)
{
  if (chr >= 0xa1a0)
  {
    File f = SPIFFS.open("/zk/" + font + ".bin", "r");
    // if (f)
    // {
    //   Serial.println("file open secc"); // 檔案開啟失敗
    // }
    // else
    // {
    //   Serial.println("file open failed");
    // }
    gfx.setColor(color);
    u8 high, low;
    u8 buf[32];
    high = chr >> 8;
    low = chr & 0xff;
    //Serial.println(chr);
    f.seek(32 * ((high - 0xa1) * 94 + (low - 0xa1)), SeekSet);
    f.read(buf, 32);
    for (int i = 0; i < 16; i++)
    {
      u8 readr = buf[i * 2];
      for (int j = 0; j < 8; j++)
      {
        if (readr & (0x80 >> j))
          gfx.setPixel(x + j, y + i);
      }
      readr = buf[i * 2 + 1];
      for (int j = 0; j < 8; j++)
        if (readr & (0x80 >> j))
          gfx.setPixel(x + j + 8, y + i);
      //  Serial.print((int)(buf[i * 2]), HEX);
      // Serial.print((int)(buf[i * 2 + 1]), HEX);
    }
    //Serial.println();
    f.close();
  }
}
bool printchs(String str, int x, int y, uint16_t color, String font)
{
  int positionX;
  int num = str.length();
  int n = num / 3 + 10;
  positionX = (ROTATION % 2 == 1) ? SCREEN_HEIGHT : SCREEN_WIDTH;
  u16 s[n + 1];
  bool pd = false;
  Utf8ToGb2312(str.c_str(), num, s);
  int j = 0;
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setColor(color);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  for (int i = 0; i < n; i++)
  {
    if (s[i] == 0xffff)
      break;

    if (x + i * 16 + 16 < positionX + 1)
      if (s[i] < 0x8000)
      {
        //  s[i] = string((s[i] | 0x8000));

        // gfx.drawString(x + i * 16, y, s[i]);
      }
      else
      {
        drawchar(s[i], x + i * 16, y, color);
      }
    else
    {
      if (s[i] < 0x8000)
      {
        // s[i] = string((s[i] | 0x8000));
        //gfx.drawString(j * 16, y + 16, s[i]);
      }
      else
      {
        drawchar(s[i], j * 16, y + 16, color);
        j++;
        pd = true;
      }
    }
  }
  return pd;
}
void TimeShow()
{
  char *dstAbbrev;
  char time_str[11];
  time_t now = dstAdjusted.time(&dstAbbrev);
  struct tm *timeinfo = localtime(&now);

  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setColor(MINI_WHITE);
  String date = ctime(&now);
  date = String(1900 + timeinfo->tm_year) + "-" + String(timeinfo->tm_mon + 1) + "-" + String(timeinfo->tm_mday) + " " + date.substring(0, 4) + " " + String(timeinfo->tm_hour) + ":" + String(timeinfo->tm_min);
  gfx.drawString(120, 1, date);

  // int8_t TimeDisp[4];
  // int hour = timeinfo->tm_hour;
  // int minute = timeinfo->tm_min;
  // TimeDisp[0] = hour / 10;
  // TimeDisp[1] = hour % 10;
  // TimeDisp[2] = minute / 10;
  // TimeDisp[3] = minute % 10;
  // tm1637.display(0, TimeDisp[0]);
  // tm1637.display(1, TimeDisp[1]);
  // tm1637.display(2, TimeDisp[2]);
  // tm1637.display(3, TimeDisp[3]);
  // Update = OFF;
}
//void TimerClockPoint_Callback(void *pArg)
void TimerClockPoint_Callback()
{
  char *dstAbbrev;
  char time_str[11];
  time_t now = dstAdjusted.time(&dstAbbrev);
  struct tm *timeinfo = localtime(&now);

  if (ClockPoint)
  tm1637.point(POINT_ON);
else
  tm1637.point(POINT_OFF);
ClockPoint = !ClockPoint;
  int8_t TimeDisp[4];
  int hour = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;
  TimeDisp[0] = hour / 10;
  TimeDisp[1] = hour % 10;
  TimeDisp[2] = minute / 10;
  TimeDisp[3] = minute % 10;
  tm1637.display(0, TimeDisp[0]);
  tm1637.display(1, TimeDisp[1]);
  tm1637.display(2, TimeDisp[2]);
  tm1637.display(3, TimeDisp[3]);
  //Serial.println("Timer 1 Event");

}
void connectWifi()
{
  if (WiFi.status() == WL_CONNECTED)
    return;
  //Manual Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    if (i > 80)
      i = 0;
    drawProgress(i, "Connecting to WiFi");
    i += 10;
    Serial.print(".");
  }
}