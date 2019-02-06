void setup() {
  // Enable LED (switch it on to show user that setup begins)
  led_init();
  led_on();

  // Enable serial
  delay(3000); // Give user some time to connect USB serial
  Serial.begin(115200);
  Serial.printf("\n\nsetup: CCS811 to ThingSpeak %s (lib v%d)\n", VERSION, CCS811_VERSION);

  // Print some version info
  Serial.printf("setup: core=%s, sdk=%s, freq=%d\n", ESP.getCoreVersion().c_str(), ESP.getSdkVersion(), ESP.getCpuFreqMHz() );

  // I2C bus clear
  Serial.printf("setup: I2C bus: %s\n", I2Cbus_statusstr(I2Cbus_clear(SDA, SCL)));

  // Enable I2C, e.g. for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)
  Wire.begin();
  Serial.printf("setup: I2C up\n");

  bool ok;

  // Enable CCS811
  // The ESP8266 does not handle I2C clock stretch correctly.
  // See the readme for a patch in the ESP8266 si2c driver to fix this.
  // Alternatively enable the below line, but it is less robust.
  //ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  ok = ccs811.begin();
  Serial.printf("setup: CCS811 %s\n", ok ? "up" : "FAILED" );

  // Start CCS811 (measure every 1 second)
  ok = ccs811.start(CCS811_MODE_1SEC);
  Serial.printf("setup: CCS811 start %s\n", ok ? "up" : "FAILED" );
  // Print CCS811 versions
  Serial.printf("setup: version: hw=0x%02x, boot=0x%04x, app=0x%04x\n", ccs811.hardware_version(), ccs811.bootloader_version(), ccs811.application_version());

  // Enable WiFi
  Serial.printf("setup: MAC %s\n", WiFi.macAddress().c_str());
  Serial.print("setup: WiFi '");
  Serial.print(wifiSsid);
  Serial.print("' ..");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiPassword);
  while ( WiFi.status() != WL_CONNECTED ) {
    led_tgl();
    delay(250);
    Serial.printf(".");
  }
  Serial.printf(" up (%s)\n", WiFi.localIP().toString().c_str());

  // Enable ThingSpeak
  ThingSpeak.begin(client);
  Serial.println("setup: ThingSpeak up");

  // End of setup() - delay helps distinguishing LED flashes
  led_off();

  delay(500);//Delay to let system boot
  Serial.println("DHT11 Humidity & temperature Sensor\n\n");
  delay(1000);//Wait before accessing Sensor
  dht.begin();

  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  //delayMS = sensor.min_delay / 1000;

  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

  Blynk.config(auth);  // in place of Blynk.begin(auth, ssid, pass);
  Blynk.connect(3333);  // timeout set to 10 seconds and then continue without Blynk
  while (Blynk.connect() == false) {
    // Wait until connected
  }
  Serial.println("Connected to Blynk server");
  delayMS = 2000;
}
