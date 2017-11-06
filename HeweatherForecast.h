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

#pragma once

#include <JsonListener.h>
#include <JsonStreamingParser.h>

typedef struct WGForecast {
  String date;
  String cond_code_d;
  String cond_code_n;
  String cond_txt_d;
  String cond_txt_n;
  String tmp_max;
  String tmp_min;
  String wind_sc;
  String wind_dir;
  String sr;
  String ss;
  String mr;
  String ms;
  String hum;
  String vis;
} WGForecast;

typedef struct Heweather {
  String location;
  String now_cond_code;
  String now_cond_txt;
  String now_tmp;
  String now_wind_sc;
  String now_wind_dir;
  String now_hum;
  String now_vis;
  String update;

  WGForecast HeForecast[3];
} Heweather;

class HeweatherForecast: public JsonListener {
  private:
    String currentKey;
    String currentParent = "";
    String currentArray = ""; 
    String location;
    Heweather weather;
   // WGForecast *forecasts;
    uint8_t maxForecasts = 3;
    long localEpoc = 0;
    long localMillisAtUpdate;

    boolean isMetric = true;
    bool isArray = false;

   // void doUpdate(WGForecast *forecasts, uint8_t maxForecasts, String url);
    void doUpdate(String url);  

    int currentForecastPeriod = -1;



  public:
    HeweatherForecast(boolean isMetric);
    void updateForecast();
  //  void updateForecast(WGForecast *forecasts, uint8_t maxForecasts, String apiKey, String city); 

//    void updateForecastPWS(WGForecast *forecasts, uint8_t maxForecasts, String apiKey, String language, String pws);
//    void updateForecastZMW(WGForecast *forecasts, uint8_t maxForecasts, String apiKey, String language, String zmwCode);
    void setMetric(bool isMetric);

    virtual void whitespace(char c);

    virtual void startDocument();

    virtual void key(String key);

    virtual void value(String value);

    virtual void endArray();

    virtual void endObject();

    virtual void endDocument();

    virtual void startArray();

    virtual void startObject();
    Heweather getweather();
};
