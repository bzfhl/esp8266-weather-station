#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

void getweatherforecast()
{
    HTTPClient http;
    http.begin("https://free-api.heweather.com/s6/weather/now?location=CN101270102&key=5c043b56de9f4371b0c7f8bee8f5b75e");
    int httpCode = http.GET();
    // httpCode will be negative on error
    if (httpCode > 0)
    {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            Serial.println(payload);
            DynamicJsonBuffer jsonBuffer;
            JsonObject &root = jsonBuffer.parseObject(payload);
            // for(int i=100;i>=0;i--){
            //   String a=root["Traces"][i]["AcceptTime"];
            // String b=root["Traces"][i]["AcceptStation"];
            // if(a!=""){
            // Serial.println(a);
            // Serial.println(b);
            // }
            // }
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}
