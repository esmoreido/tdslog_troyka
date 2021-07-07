// библиотека для работы интерфейса I²C для часов реального времени
#include <Wire.h>
// библиотека для работы с часами реального времени
#include "TroykaRTC.h"
// размер массива для времени
#define LEN_TIME 12
// размер массива для даты
#define LEN_DATE 12
// размер массива для дня недели
#define LEN_DOW 12
// создаём объект для работы с часами реального времени
RTC clock;
// массив для хранения текущего времени
char mins[LEN_TIME];
// массив для хранения текущего времени
char time[LEN_TIME];
// массив для хранения текущей даты
char date[LEN_DATE];
// массив для хранения текущего дня недели
char weekDay[LEN_DOW];

// библиотека для работы с протоколом 1-Wire датчика температуры
#include <OneWire.h>
// библиотека для работы с датчиком температуры DS18B20 
#include <DallasTemperature.h>
// сигнальный провод датчика температуры
#define ONE_WIRE_BUS 13
// создаём объект для работы с библиотекой OneWire для датчика температуры
OneWire oneWire(ONE_WIRE_BUS);
// создадим объект для работы с библиотекой DallasTemperature
DallasTemperature sensor(&oneWire);

// GPIO пин с поддержкой АЦП для датчика TDS 
constexpr auto pinSensor = A1;

// библиотека для работы с дисплеем
#include <TroykaTextLCD.h>
// создаем объект для работы с дисплеем
TroykaTextLCD lcd;

// библиотека для работы с SPI
#include <SPI.h>
// библиотека для работы с SD-картами
#include <SD.h>
// даём разумное имя для CS пина microSD-карты
#define SD_CS_PIN  8
// строка для записи на SD
String dataString = "";
// индикатор наличия SD
int sd_ok = 1;

void setup(){
  // инициализируем работу Serial-порта
  Serial.begin(9600);
  // начинаем работу с датчиком температуры
  sensor.begin();
  // устанавливаем разрешение датчика температуры от 9 до 12 бит
  sensor.setResolution(12);

  // инициализация часов
  clock.begin();
  // метод установки времени и даты в модуль вручную
  // clock.set(10,25,45,27,07,2005,THURSDAY);    
  // метод установки времени и даты автоматически при компиляции
  // clock.set(__TIMESTAMP__);
  // что бы время менялось при прошивки или сбросе питания
  // закоментируйте оба метода clock.set();

  // устанавливаем количество столбцов и строк экрана
  lcd.begin(16, 2);
  // устанавливаем контрастность в диапазоне от 0 до 63
  lcd.setContrast(25);
  // устанавливаем яркость в диапазоне от 0 до 255
  lcd.setBrightness(130);

  // выводим сообщение в Serial-порт о поиске карты памяти
  Serial.println("Initializing SD card...");
  // если microSD-карта не была обнаружена
  if (!SD.begin(SD_CS_PIN)) {
    // выводим сообщение об ошибке
    sd_ok = 0;
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  } else {
    Serial.println("Card initialized.");
  }
}
 
void loop(){
  // считываем напряжение с батареи
  float voltage = (float)(analogRead(0) * 5.0) / 1024;
  
  // переменная для хранения температуры
  float temperature;
  // отправляем запрос на измерение температуры
  sensor.requestTemperatures();
  // считываем данные из регистра датчика
  temperature = sensor.getTempCByIndex(0);
  
  // считываем данные с датчика TDS
  int valueSensor = analogRead(pinSensor);
  // переводим данные с датчика в напряжение
  float voltageSensor = valueSensor * 5 / 1024.0;
  // конвертируем напряжение в концентрацию
  float tdsSensor = (133.42 * pow(voltageSensor, 3) - 255.86 * pow(voltageSensor, 2) + 857.39 * voltageSensor) * 0.5;
  
  // запрашиваем данные с часов
  clock.read();
  // сохраняем текущее время, дату и день недели в переменные
  clock.getTimeStamp(time, date, weekDay);

  lcd.clear();
  // устанавливаем курсор в колонку 0, строку 0
  lcd.setCursor(0, 0);
  // печатаем дату
  lcd.print(date);
  // устанавливаем курсор в колонку 11, строку 0
  lcd.setCursor(11, 0);
  // печатаем время
  lcd.print(time); 
  // печатаем напряжение батареи
  lcd.setCursor(0, 1);
  lcd.print("Bat.v: ");
  lcd.setCursor(7, 1);
  lcd.print(voltage);
  if (sd_ok == 0){
    lcd.setCursor(12, 1);
    lcd.print("noSD");
  } else {
    lcd.setCursor(12, 1);
    lcd.print("sdOK");
  }
  // ждем 2 секунды обновления дисплея
  delay(2000);
  lcd.clear();
  // печатаем температуру во второй строке
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.setCursor(6, 0);
  lcd.print(temperature);
  
  
  // печатаем TDS 
  lcd.setCursor(0, 1);
  lcd.print("TDS:");
  lcd.setCursor(6, 1);
  lcd.print(tdsSensor);
  
  // выводим в serial порт текущее время, дату и день недели
  Serial.print(date);
  Serial.print(", ");
  Serial.print(time);
  Serial.print(", ");
  
  // выводим температуру в Serial-порт
  Serial.print("Temp C: ");
  Serial.print(temperature);
  Serial.print(", ");
  // выводим данные TDS в Serial-порт
  Serial.print("TDS ppm = "); 
  Serial.print(tdsSensor);
  Serial.print(", ");
  // выводим напряжение на батарее
  Serial.print("V: ");
  Serial.println(voltage);

  // формируем строку с результатами показаний датчика
  dataString = "";
  dataString += String(date);
  dataString += ", ";
  dataString += String(time);
  dataString += ", ";
  dataString += String(temperature);
  dataString += ", ";
  dataString += String(tdsSensor);
  dataString += ", ";
  dataString += String(voltage);
  // выводим результаты в serial-порт
  Serial.println(dataString);
  
  // сохраняем на microSD каждые n минут
  int n = 5;
  if (clock.getMinute() % n == 0){
    if (clock.getSecond() <= 15){
      saveSD();          
    }
  }  
  // ждём 2 секунды обновления дисплея
  delay(2000);
}

void saveSD()
{
  // создаём файл для записи
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // если файл доступен для записи
  if (dataFile) {
    // сохраняем данные
    dataFile.println(dataString);
    // закрываем файл
    dataFile.close();
    // выводим сообщение об удачной записи
    Serial.println("Save OK");
  } else {
    // если файл не доступен
    Serial.println("Error opening datalog.txt");
  }
}
