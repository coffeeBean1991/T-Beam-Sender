#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306.h"
#include <TinyGPS++.h>
//#include <axp20x.h>
//AXP20X_Class axp;
#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND  915E6

SSD1306 display(0x3c, 21, 22);
String rssi = "RSSI --";
String packSize = "--";
String packet ;
String timestamp;

TinyGPSPlus gps;
HardwareSerial GPS(1);

char ID[23];
void setup()
{
  uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(ID, 23, "IAT-%04X%08X", chip, (uint32_t)chipid);


  GPS.begin(9600, SERIAL_8N1, 34, 12);   //17-TX 18-RX


  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Sender Test");

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("init ok");
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  delay(1500);
}

void loop()
{
  Serial.println(ID);
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Sending packet");
  display.drawString(20, 15, String(ID));
  display.drawString(0, 30, "Latitude:");
  display.drawString(55, 30, String(gps.location.lat(), 5));
  display.drawString(0, 40, "Longitude:");
  display.drawString(55, 40, String(gps.location.lng(), 5));
  display.display();

  //Build timestamp
  struct tm  timeinfo;
  unsigned long int unixtime;
  timeinfo.tm_year =  gps.date.year() - 1900;
  timeinfo.tm_mon = gps.date.month() - 1;
  timeinfo.tm_mday =  gps.date.day();
  timeinfo.tm_hour =  gps.time.hour();
  timeinfo.tm_min =  gps.time.minute();
  timeinfo.tm_sec = gps.time.second();
  unixtime = mktime(&timeinfo);
  printf("unixtime = %u\n", unixtime);

  // send packet
  LoRa.beginPacket();
  LoRa.print(ID);
  LoRa.print(";");
  LoRa.print(gps.location.lat(), 5);
  LoRa.print(";");
  LoRa.print(gps.location.lng(), 5);
  LoRa.print(";");
  LoRa.print(unixtime);
  LoRa.endPacket();

  Serial.print("Latitude  : ");
  Serial.println(gps.location.lat(), 5);
  Serial.print("Longitude : ");
  Serial.println(gps.location.lng(), 4);
  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());
  Serial.print("Altitude  : ");
  Serial.print(gps.altitude.feet() / 3.2808);
  Serial.println("M");
  Serial.print("Time      : ");
  Serial.print(gps.time.hour());
  Serial.print(":");
  Serial.print(gps.time.minute());
  Serial.print(":");
  Serial.println(gps.time.second());
  Serial.print("Speed     : ");
  Serial.println(gps.speed.kmph());
  smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));

}
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPS.available())
      gps.encode(GPS.read());
  } while (millis() - start < ms);
}
