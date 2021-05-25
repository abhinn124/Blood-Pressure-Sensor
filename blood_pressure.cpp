//Author : Abhinn Verma

#include <mbed.h>
#include <USBSerial.h>
#include <stdint.h>
#include <math.h>
//Defining constant
#define COUNTS_224 (16777216L) ///< Constant: 2^24

const int addr7bit = 0x18;      // 7 bit I2C address
const int addr8bit = 0x18 << 1; // 8bit I2C address. append one bit to signifes read and write 1 read

float mmHg_min = 0;
float mmHg_max = 300;
float OUTPUT_min = 2.5;
float OUTPUT_max = 22.5;
float _OUTPUT_min = (uint32_t)((float)COUNTS_224 * (OUTPUT_min / 100.0));
float _OUTPUT_max = (uint32_t)((float)COUNTS_224 * (OUTPUT_max / 100.0));

USBSerial pc(USBTX, USBRX); // tx, rx

float convertData(uint32_t raw_mmHg)
{
  float mmHg = (raw_mmHg - _OUTPUT_min) * (mmHg_max - mmHg_min);
  mmHg /= (float)(_OUTPUT_max - _OUTPUT_min);
  mmHg += mmHg_min;
  return mmHg;
}

int main()
{

  I2C i2c_pressure_sensor(I2C_SDA, I2C_SCL);

  char status[1] = {0};
  char data[4] = {0};
  uint32_t raw_mmHg = 0;
  float mmHg = 0;
  float prev_press = 0;
  bool isDeflating = false;

  uint32_t READ_COMMAND[] = {0xAA, 0x00, 0x00};
  while (1)
  {
    i2c_pressure_sensor.write(addr8bit, (const char *)READ_COMMAND, 3);
    wait_ms(5);
    i2c_pressure_sensor.read(addr8bit, status, 1);
    wait_ms(5);
    i2c_pressure_sensor.read(addr8bit, data, 4);
    wait_ms(5);

    raw_mmHg = data[1];
    raw_mmHg <<= 8;
    raw_mmHg |= data[2];
    raw_mmHg <<= 8;
    raw_mmHg |= data[3];
    float mmHg = convertData(raw_mmHg);
    if (mmHg > 165 && isDeflating == false)
    {
      isDeflating = true;
      pc.printf("Pressure Limit Reached. Don't increase pressure further ! \n");
    }
    if (isDeflating == true)
    {
      if (abs(mmHg - prev_press) > 4)
      {
        pc.printf("Warning! Pressure decreased too quickly \n");
      }
      else if (abs(mmHg - prev_press) < 2)
      {
        pc.printf("Pressure drop rate is too slow. Try to maintain a drop rate of about 4 mmHg \n");
      }
      else
      {
        pc.printf("%f", mmHg);
        pc.printf("\n");
      }
    }
    else
    {
      pc.printf("%f", mmHg); 
      pc.printf("\n");
    }
    prev_press = mmHg;
    wait_ms(1000);
  }
}