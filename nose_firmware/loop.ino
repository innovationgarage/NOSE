void loop() {
  // Read ENS210
  bool pass_ok = true;
  int t_data, t_status, h_data, h_status;

  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
    t_status = false;
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    Field5_T = event.temperature;
    t_status = true;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
    h_status = false;
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    Field6_H = event.relative_humidity;
    h_status = true;
  }

  if (t_status == true && h_status == true)
  { pass_ok = ccs811.set_envdataDHT(t_data, h_data);

    // On error, flash led, count errors
    if ( !pass_ok ) {
      Serial.printf("pass failed, ");
      Field8_errcount++;
      led_blink(4, 100);
    }
    else
      Field8_errcount++;
  }

  // Read CCS811
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2, &etvoc, &errstat, &raw);

  // Process CCS811
  if ( errstat == CCS811_ERRSTAT_OK ) {
    // We have valid gas data, update "last known correct values"
    Field1_eCO2 = eco2;
    Field2_eTVOC = etvoc;
    Field4_Resist = (1650 * 1000.0 / 1023) * (raw % 1024) / (raw / 1024); // V/I
    // Print eCO2, eTVOC, and R
    Serial.printf("eco2=%dppm, ", Field1_eCO2);
    Serial.printf("etvoc=%dppb, ", Field2_eTVOC);
    Serial.printf("R=%.0fΩ, ", Field4_Resist);
  } else if ( errstat == CCS811_ERRSTAT_OK_NODATA ) {
    Serial.printf("waiting for (new) data, ");
    // Not considered an error
  } else if ( errstat & CCS811_ERRSTAT_I2CFAIL ) {
    Serial.printf("I2C error, ");
    // Flash led, count errors
    led_blink(2, 100);
    Field8_errcount++;
  } else {
    Serial.printf("error: %s, ", ccs811.errstat_str(errstat) );
    // Flash led, count errors
    led_blink(1, 100);
    Field8_errcount++;
  }

  // Print cumulative number of read errors
  Serial.printf("errcount=%d, ", Field8_errcount);

  // When was the last ThingSpeak update? We want them 60s apart
  static unsigned long prev;
  unsigned long now = millis();
  if ( now - prev > 60 * 1000 ) {
    // Prepare ThingSpeak package
    ThingSpeak.setField(1, Field1_eCO2);
    ThingSpeak.setField(2, Field2_eTVOC);
    ThingSpeak.setField(3, errstat + (pass_ok ? 0 : 0x0004) ); // Extra flag added by SW, the "TH pass" failed
    ThingSpeak.setField(4, Field4_Resist);
    ThingSpeak.setField(5, Field5_T);
    ThingSpeak.setField(6, Field6_H);
    ThingSpeak.setField(7, t_status * 10 + h_status); // Coded in two decimals
    ThingSpeak.setField(8, Field8_errcount);

    // Send to ThingSpeak
    led_on();
    int http = ThingSpeak.writeFields(thingspeakChannelId, thingspeakWriteApiKey);
    led_off();

    // ThingSpeak upload feedback
    if ( http != 200 ) led_blink(20, 50); // Flash to show upload failure
    Serial.printf("wifi=%d, http=%d, ", WiFi.status(), http);

    // Record upload time
    if ( http == 200 ) {
      prev = now = millis();
      Field8_errcount = 0;
    }
  }

  // If both sensors give I2C error, clear the bus and restart I2C
  if ( errstat & CCS811_ERRSTAT_I2CFAIL) {
    Serial.printf("[%s], ", I2Cbus_statusstr(I2Cbus_clear(SDA, SCL)));
    Wire.begin();
  }

  // End of the iteration
  Serial.printf("%lds\n", (now - prev + 500) / 1000);

  // Wait (match the CCS811 measurement mode, so that T/H is updated)
  delayMS = millis() + 3000;

  while (millis() < delayMS)
  {
    Blynk.run();


    if (mode >= 0)
    {
      ws2812fx.service();

      if (mode != ws2812fx.getMode())
        ws2812fx.setMode(mode);
    }
    else if (mode == -1)
    {
      // CO2
      ws2812fx.clear();
      double co2 = Field1_eCO2;
      for (int l = NUMws2812fx - 1; l >= 0; l--)
      {
        if (co2 > 1000)
        {
          ws2812fx.setPixelColor(l, 255, 0, 255);
          co2 -= 1000;
        }
        else
          break;
      }
      ws2812fx.show();

    }
    else if (mode == -2)
    {
      // Temperature (rudolph)
      ws2812fx.clear();
      for (int l = 0; l < NUMws2812fx; l++)
        ws2812fx.setPixelColor(NUMws2812fx - 1 - l, 0, max(0, min(255, (int)map(Field5_T, -10, 20, 255, 0))), 0);

      Serial.print("temp ");
      Serial.print(Field5_T);
      Serial.print(" map ");
      Serial.println( max(0, min(255, (int)map(Field5_T, -10, 20, 255, 0))) );
      ws2812fx.show();
    }
  }
}
