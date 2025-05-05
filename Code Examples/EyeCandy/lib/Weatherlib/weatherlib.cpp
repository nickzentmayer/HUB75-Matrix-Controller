#include <weatherlib.h>

Weatherlib::Weatherlib(String apiKey, String City, String CountryCode, String StateCode) {
    APIKey = apiKey;
    city = City;
    countryCode = CountryCode;
    stateCode = StateCode;
    client = new WiFiClient();
}

Weatherlib::Weatherlib(String apiKey, String City, String CountryCode, String StateCode, WiFiClient* Client) {
    APIKey = apiKey;
    city = City;
    countryCode = CountryCode;
    stateCode = StateCode;
    client = Client;
}

int Weatherlib::updateWeatherData() {
    String url = "/data/2.5/weather?q=" + city + "," + stateCode + "," + countryCode + "&appid=" + APIKey;
    if(!client->connect("api.openweathermap.org", 80))
    return -1;
    client->println(String("GET ") + url + " HTTP/1.1");
    client->println("Host: api.openweathermap.org");
    client->println("Connection: close\r\n");
    /*if (client.println() == 0) {
        log_e(F("Failed to send request"));
        client.stop();
        return -2;
      }*/
    
      // Check HTTP status
      char status[32] = {0};
      client->readBytesUntil('\r', status, sizeof(status));
      // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
      if (strcmp(status + 9, "200 OK") != 0) {
        log_e("Unexpected response: ");
        Serial.println(status);
        client->stop();
        return -3;
      }
    
      // Skip HTTP headers
      char endOfHeaders[] = "\r\n\r\n";
      if (!client->find(endOfHeaders)) {
        log_e("Invalid response");
        client->stop();
        return -4;
      }
    
    while(!client->available()) {
        log_d("Waiting for data...");
        delay(100);
    }
    doc.clear();

    DeserializationError errorr = deserializeJson(doc, *client);
  if (errorr) {
    log_e("deserializeJson() failed: ");
    //log_e(errorr.f_str().c_str());
    client->stop();
    return -5;
  }
    client->stop();

    weatherData.condition = doc["weather"][0]["main"].as<String>();
    weatherData.icon = strtoul(doc["weather"][0]["icon"].as<String>().c_str(), NULL, 16);
    weatherData.description = doc["weather"][0]["description"].as<String>();
    weatherData.temperature = (doc["main"]["temp"].as<float>() - 273.15) * (9.0/5.0) + 32;
    weatherData.feelsLike = (doc["main"]["feels_like"].as<float>() - 273.15) * (9.0/5.0) + 32;
    weatherData.humidity = doc["main"]["humidity"].as<float>();
    weatherData.pressure = doc["main"]["pressure"].as<float>();
    weatherData.windSpeed = doc["wind"]["speed"].as<float>();
    weatherData.sunrise = doc["sys"]["sunrise"].as<uint64_t>();
    weatherData.sunset = doc["sys"]["sunset"].as<uint64_t>();
    return 0;
}

WeatherData Weatherlib::getWeather() {
    return weatherData;
}

WeatherData* Weatherlib::getWeatherPtr() {
    return &weatherData;
}
