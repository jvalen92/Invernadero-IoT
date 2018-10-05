//Este codigo se utiliza para cuadrar la hora del RTC
//despues de poner este codigo en el arduino se utiliza el codigo de relojTiempoReal para mostrar la fecha y la hora en la consola

#include "Wire.h"
#define DS1307_I2C_ADDRESS 0x68

byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}

void setDateDs1307(
  byte second,        // 0-59
  byte minute,        // 0-59
  byte hour,          // 1-23
  byte dayOfWeek,     // 1-7 1=Lunes, 7=Domingo
  byte dayOfMonth,    // 1-28/29/30/31
  byte month,         // 1-12
  byte year           // 0-99
)
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}

void setup()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  Wire.begin();
  pinMode(13, OUTPUT);

  // Cambiamos los parámetros con la fecha actual
  // Lo mejor es poner la hora del script 30 segundos antes que es lo que tarda el ordenador aproximadamente en compilar el programa
  // Sólo es necesario compilar el script una vez
  // El formato es de 24 horas
  // Días de la semana lunes es 1 a domingo que es 7
  // El formato de año son los dos últimos dígitos del año
  // Una vez cargado el programa el led 13 parpadeará cada segundo, no debemos resetear.

  second = 10;
  minute = 2;
  hour = 12;
  dayOfWeek = 6;
  dayOfMonth = 8;
  month = 9;
  year = 18;
  setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
}

void loop()
{
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
}
