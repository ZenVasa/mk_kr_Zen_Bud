#include "Tone.h"
#include "LiquidCrystal_I2C.h"

constexpr byte encPinA{6};  // CLK
constexpr byte encPinB{5};  // DT
constexpr byte Speaker_pin{11};
constexpr byte Potentiom_pin{A1}; // Аналоговый вход для потенциометра
constexpr byte encBut{2}; // Кнопка энкодера (ОК)
constexpr byte button{3}; // Кнопка переключения между сигналами

LiquidCrystal_I2C lcd(0x27,20,4); // Инициализация дисплея. SCL(A5), SDA(A4).
Tone tone1; // Инициализация источника звука

void setup(){
    tone1.begin(Speaker_pin);    // Назначение вывода для динамика
    pinMode(Potentiom_pin, INPUT); // Назначение вывода для потенциометра
    Serial.begin(115200); 
    //attachInterrupt(0, encoder_flag, FALLING); // Подключение прерывания
    //attachInterrupt(1, settings_flag, FALLING);
    lcd.init(); // Инициализация дисплея
    lcd.backlight();
}




void loop(){
    lcd.setCursor(3,2);
    lcd.print("Hello world!");
    //tone1.play(map(analogRead(Potentiom_pin), 0, 1023, 20, 1000));
    //Serial.println(map(analogRead(Potentiom_pin), 0, 1023, 20, 1000));
    //delay(100);
    }

// Переменные значений на выводах энкодера
bool encA;
bool encB;

byte prevState; // Предыдущие значения на выводах энкодера

int enc_val = 0;

volatile bool enc_flag = false; 
volatile bool set_flag = false;


void encoder(){
  enc_val = 0;
  encA = digitalRead(encPinA);
  encB = digitalRead(encPinB);
  if ((encA) && (encB)){
    if (prevState==B00000001){
      enc_val = -1;
    }
    else if (prevState==B00000010){
      enc_val = 1;
    }
    prevState=B00000011;
  }
  else if ((!encA) && (encB)){
    if (prevState==B00000000){
      enc_val = -1;
    }
    else if (prevState==B00000011){
      enc_val = 1;
    }
    prevState=B00000001;
  }
  else if ((encA) && (!encB)){
    if (prevState==B00000011){
      enc_val = -1;
    }
    else if (prevState==B00000000){
      enc_val = 1;
    }
    prevState=B00000010;
  }  
}