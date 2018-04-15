// Para la pantalla 16x2

//Librerias necesarias
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);



void setup()
{
  lcd.begin(16, 2);
  for (int i = 0; i < 3; ++i) {
    //Iniciamos el fondo retroiluminado
    lcd.backlight();
    delay(250);
    //Iniciamos la pantalla
    lcd.noBacklight();
    delay(250);
  }

  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Prueba");
  lcd.setCursor(0,1);
  lcd.print("sise");
}


void loop()
{}
