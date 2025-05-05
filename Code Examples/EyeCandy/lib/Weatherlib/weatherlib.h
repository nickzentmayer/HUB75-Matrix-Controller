#pragma once
#ifndef WEATHERLIB_H
#define WEATHERLIB_H

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

struct WeatherData
{
    String condition;
    String description;
    float temperature;
    float feelsLike;
    float humidity;
    float pressure;
    float windSpeed;
    uint16_t icon;
    uint64_t sunrise;
    uint64_t sunset;
};

class Weatherlib
{
    public:
    Weatherlib(String apiKey, String City, String CountryCode, String StateCode);
    Weatherlib(String apiKey, String City, String CountryCode, String StateCode, WiFiClient* client);
    Weatherlib() = delete;

    int updateWeatherData();
    //bool parseWeatherData();

    WeatherData getWeather();
    WeatherData* getWeatherPtr();
    private:
    String APIKey;
    String city;
    String countryCode;
    String stateCode;
    WeatherData weatherData;
    WiFiClient* client;
    JsonDocument doc;
};

#endif