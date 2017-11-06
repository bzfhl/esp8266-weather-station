/**The MIT License (MIT)

Copyright (c) 2015 by Daniel Eichhorn

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

See more at http://blog.squix.ch
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "HeweatherForecast.h"

HeweatherForecast::HeweatherForecast(boolean _isMetric)
{
  isMetric = _isMetric;
}

void HeweatherForecast::setMetric(bool isMetric)
{
  this->isMetric = isMetric;
}

//void HeweatherForecast::updateForecast(WGForecast *forecasts, uint8_t maxForecasts, String apiKey, String city)
void HeweatherForecast::updateForecast()
{
  //doUpdate(forecasts, maxForecasts, "https://free-api.heweather.com/s6/weather?location=" + city + " &key=" + apiKey);
  doUpdate("http://10.234.123.1/weather/forecast.json");
  doUpdate("http://10.234.123.1/weather/now.json");
  //doUpdate(forecasts, maxForecasts, "http://10.234.123.1/weather/forecast.json");
}

//void HeweatherForecast::doUpdate(WGForecast *forecasts, uint8_t maxForecasts, String url)
void HeweatherForecast::doUpdate(String url)
{
  // this->forecasts = forecasts;
  // this->maxForecasts = maxForecasts;
  currentForecastPeriod=-1;
  isArray =false;
  JsonStreamingParser parser;
  parser.setListener(this);

  HTTPClient http;

  http.begin(url);
  bool isBody = false;
  char c;
  int size;
  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  if (httpCode > 0)
  {

    WiFiClient *client = http.getStreamPtr();

    while (client->connected())
    {
      while ((size = client->available()) > 0)
      {
        c = client->read();
        if (c == '{' || c == '[')
        {

          isBody = true;
        }
        if (isBody)
        {
          parser.parse(c);
        }
      }
    }
  }
  //this->forecasts = nullptr;
}

void HeweatherForecast::whitespace(char c)
{
 // Serial.println("whitespace");
}

void HeweatherForecast::startDocument()
{
  Serial.println("start document");
 // Serial.println(String(currentForecastPeriod));
}

void HeweatherForecast::key(String key)
{
  // Serial.println("startkey:" + key);
  // Serial.println("currentForecastPeriod:" + String(currentForecastPeriod));
  currentKey = String(key);
  //  Serial.println(currentKey);
  // if (currentKey == "simpleforecast") {
  //   isSimpleForecast = true;
  // }
}

void HeweatherForecast::value(String value)
{
  // Serial.println(currentKey+ ":" + value);
//   Serial.println("currentForecastPeriod:" + String(currentForecastPeriod));
//   Serial.println("currentArray:" +  String(currentArray));  
//   Serial.println("currentKey:" +  String(currentKey));  
// Serial.println(currentKey + ":" + value);
  if (currentKey == "location")
  {
    weather.location = value;
    //Serial.println( "weather.location:" + weather.location);
  }
  if (currentKey == "cond_code" && currentParent == "now")
  {
    weather.now_cond_code = value;
   // Serial.println( "weather.now_cond_code:" + weather.now_cond_code);
  }
  if (currentKey == "cond_txt"  && currentParent == "now")
  {
    weather.now_cond_txt = value;
    //Serial.println( "weather.location:" + weather.location);
  }
  if (currentKey == "wind_dir"  && currentParent == "now")
  {
    weather.now_wind_dir = value;
    //Serial.println( "weather.location:" + weather.location);
  }
  if (currentKey == "hum"  && currentParent == "now")
  {
    weather.now_hum = value;
    //Serial.println( "weather.location:" + weather.location);
  }
  if (currentKey == "vis"  && currentParent == "now")
  {
    weather.now_vis = value;
    //Serial.println( "weather.location:" + weather.location);
  }
  if (currentKey == "tmp"  && currentParent == "now")
  {
    weather.now_tmp = value;
    //Serial.println( "weather.location:" + weather.location);
  }
  if (currentKey == "wind_sc"  && currentParent == "now")
  {
    weather.now_wind_sc = value;
    //Serial.println( "weather.location:" + weather.location);
  }

  if (currentKey == "date" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].date = value;

  }
  if (currentKey == "cond_code_d" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].cond_code_d = value;
    // weather.location = location;
  }
  if (currentKey == "cond_code_n" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].cond_code_n = value;
  }
  if (currentKey == "cond_txt_d" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].cond_txt_d = value;
    //  Serial.println("forecasts[" + (String)currentForecastPeriod + "].cond_txt_d:" + value);
    //  Serial.println("weather.HeForecast[X].cond_txt_d:" + weather.HeForecast[currentForecastPeriod].cond_txt_d);
  }
  if (currentKey == "cond_txt_n" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].cond_txt_n = value;
  }
  if (currentKey == "tmp_max" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].tmp_max = value;
  }
  if (currentKey == "tmp_min" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].tmp_min = value;
    // Serial.println("forecasts[" + (String)currentForecastPeriod + "].tmp_min:" + value);
  }
  if (currentKey == "wind_sc" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].wind_sc = value;
    // Serial.println("forecasts[" + (String)currentForecastPeriod + "].wind_sc:" + value);
  }
  if (currentKey == "wind_dir" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].wind_dir = value;
    // Serial.println("forecasts[" + (String)currentForecastPeriod + "].wind_sc:" + value);
  }
  if (currentKey == "sr" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].sr = value;
    // Serial.println("forecasts[" + (String)currentForecastPeriod + "].wind_sc:" + value);
  }
  if (currentKey == "ss" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].ss = value;
    // Serial.println("forecasts[" + (String)currentForecastPeriod + "].wind_sc:" + value);
  }
  if (currentKey == "ms" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].ms = value;
    // Serial.println("forecasts[" + (String)currentForecastPeriod + "].wind_sc:" + value);
  }
  if (currentKey == "mr" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].mr = value;
    // Serial.println("forecasts[" + (String)currentForecastPeriod + "].wind_sc:" + value);
  }
  if (currentKey == "hum" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].hum = value;
    // Serial.println("forecasts[" + (String)currentForecastPeriod + "].wind_sc:" + value);
  }
  if (currentKey == "vis" && currentForecastPeriod < maxForecasts  && currentArray == "daily_forecast")
  {
    weather.HeForecast[currentForecastPeriod].vis = value;
    // Serial.println("forecasts[" + (String)currentForecastPeriod + "].wind_sc:" + value);
  }
  if (currentKey == "loc" && currentParent == "update")
  {
    weather.update = value;
    localMillisAtUpdate = millis();
  }
}
void HeweatherForecast::startArray()
{

  //if (currentParent == "daily_forecast"){
  currentArray = currentKey;
  isArray = true;
  //Serial.println("startArray");
  //Serial.println("currentArray:" + String(currentArray));
  // }
}
void HeweatherForecast::endArray()
{

  // if (currentParent == "daily_forecast"){
  isArray = false;
//  Serial.println("endtArray");
  currentArray = "";
  //  }
}

void HeweatherForecast::startObject()
{
  currentParent = currentKey;

 // Serial.println("startObject");
  //Serial.println("currentParent:" + String(currentParent));
  if (isArray && currentArray == "daily_forecast")
  {
    currentForecastPeriod++;
   // Serial.println("currentForecastPeriod:" + String(currentForecastPeriod));
  }
}

void HeweatherForecast::endObject()
{
  // Serial.println("endObject");
  currentParent = "";
}

void HeweatherForecast::endDocument()
{
  // Serial.println("endDocument");
}

Heweather HeweatherForecast::getweather()
{ 
  // Serial.println("weather.location:" + String(weather.location));
  // Serial.println("weather.now_cond_code:" + String(weather.now_cond_code));
  // Serial.println("weather.now_cond_txt:" + String(weather.now_cond_txt));
  // Serial.println("weather.now_tmp:" + String(weather.now_tmp));
  // Serial.println("weather.HeForecast[0].wind_sc:" + String(weather.HeForecast[0].wind_sc));

  return weather;
}
