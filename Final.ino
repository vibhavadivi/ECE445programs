#include <SPI.h>
#include <DW1000Ranging.h>
#include "BluetoothSerial.h"
#include "ICM_20948.h" // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4
#define PIN_RST 27
#define PIN_IRQ 34

#define SERIAL_PORT Serial
#define WIRE_PORT Wire // Your desired Wire port.      Used when "USE_SPI" is not defined
// The value of the last bit of the I2C address.
// On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
#define AD0_VAL 86
ICM_20948_I2C myICM; // Otherwise create an ICM_20948_I2C object
BluetoothSerial SerialBT;

float d_0;
float d_7;
float d_8;
bool receivedAddresses[3] = {false, false, false};

void setup()
{
    SERIAL_PORT.begin(115200);
    SerialBT.begin("ESP32_C");
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    DW1000Ranging.initCommunication(PIN_RST, DW_CS, PIN_IRQ);
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);
    DW1000Ranging.startAsTag("7D:00:22:EA:82:60:3B:9C", DW1000.MODE_LONGDATA_RANGE_LOWPOWER);
    WIRE_PORT.begin();
    WIRE_PORT.setClock(400000);

    bool initialized = false;
    while (!initialized)
    {

        myICM.begin(WIRE_PORT, AD0_VAL);

        SerialBT.print(F("Initialization of the sensor returned: "));
        SerialBT.println(myICM.statusString());
        if (myICM.status != ICM_20948_Stat_Ok)
        {
            SerialBT.println("Trying again...");
            delay(500);
        }
        else
        {
            initialized = true;
        }
    }
}

void loop()
{
    
    if (myICM.dataReady())
    {
        myICM.getAGMT();         // The values are only updated when you call 'getAGMT'
        printScaledAGMT(&myICM); // This function takes into account the scale settings from when the measurement was made to calculate the values with units
        delay(30);
    }
    else
    {
        SerialBT.println("Waiting for data");
        delay(500);
    }
    if (receivedAddresses[0] == true && receivedAddresses[1] == true && receivedAddresses[2] == true){
      receivedAddresses[0] = false;
      receivedAddresses[1] = false;
      receivedAddresses[2] = false;
    }
    while (!(receivedAddresses[0] == true && receivedAddresses[1] == true && receivedAddresses[2] == true)){
      DW1000Ranging.loop();
    }    
}





void newRange()
{
    uint16_t id = DW1000Ranging.getDistantDevice()->getShortAddress();
    if (id == 128){
      receivedAddresses[0] = true;
    }
    else if (id == 135){
      receivedAddresses[1] = true;
    }
    else if (id == 136){
      receivedAddresses[2] = true;
    }
    SerialBT.print("from: ");
    SerialBT.print(id, HEX);
    SerialBT.print("\t Range: ");
    SerialBT.print(DW1000Ranging.getDistantDevice()->getRange());
    SerialBT.print(" m \n");

}


void newDevice(DW1000Device *device)
{
    // Serial.print("ranging init; 1 device added ! -> ");
    // Serial.print(" short:");
    // Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device)
{
    // Serial.print("delete inactive device: ");
    // Serial.println(device->getShortAddress(), HEX);
}

void printFormattedFloat(float val, uint8_t leading, uint8_t decimals)
{
  float aval = abs(val);
  if (val < 0)
  {
    SerialBT.print("-");
  }
  else
  {
    SerialBT.print(" ");
  }
  for (uint8_t indi = 0; indi < leading; indi++)
  {
    uint32_t tenpow = 0;
    if (indi < (leading - 1))
    {
      tenpow = 1;
    }
    for (uint8_t c = 0; c < (leading - 1 - indi); c++)
    {
      tenpow *= 10;
    }
    if (aval < tenpow)
    {
      SerialBT.print("0");
    }
    else
    {
      break;
    }
  }
  if (val < 0)
  {
    SerialBT.print(-val, decimals);
  }
  else
  {
    SerialBT.print(val, decimals);
  }
}

void printScaledAGMT(ICM_20948_I2C *sensor)
{
  SerialBT.print("AccX: ");
  //printFormattedFloat(sensor->accX(), 5, 2);
  SerialBT.print(((sensor->accX() /1000) * 9.81));
  SerialBT.print(", ");
  SerialBT.print("AccY: ");
  //printFormattedFloat(sensor->accY(), 5, 2);
  SerialBT.print(((sensor->accY() /1000) * 9.81) + 3.2);
  SerialBT.print(", ");
  SerialBT.print("AccZ: ");
  //printFormattedFloat(sensor->accZ(), 5, 2);
  SerialBT.print(((sensor->accZ() /1000) * 9.81)-9.66);
  SerialBT.print(", GyrX: ");
  printFormattedFloat(sensor->gyrX(), 5, 2);
  SerialBT.print(", GyrY: ");
  printFormattedFloat(sensor->gyrY(), 5, 2);
  SerialBT.print(", GyrZ: ");
  printFormattedFloat(sensor->gyrZ(), 5, 2);
  SerialBT.print(", MagX: ");
  printFormattedFloat(sensor->magX(), 5, 2);
  SerialBT.print(", MagY: ");
  printFormattedFloat(sensor->magY(), 5, 2);
  SerialBT.print(", MagZ: ");
  printFormattedFloat(sensor->magZ(), 5, 2);
  SerialBT.print(", Tmp: ");
  printFormattedFloat(sensor->temp(), 5, 2);
  SerialBT.println();
}
