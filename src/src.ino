#include "Tone.h"
//#include "LiquidCrystal_I2C.h"

constexpr byte Speaker_pin{11};
constexpr byte Potentiom_pin{A1};
constexpr byte Sound_on_pin{A1};

//LiquidCrystal_I2C lcd(0x27,20,4); // Инициализация дисплея
Tone tone1; // Инициализация источника звука

void setup(){
    tone1.begin(Speaker_pin);    // Назначение вывода для динамика
    pinMode(Potentiom_pin, INPUT); // Назначение вывода для потенциометра
    pinMode(Sound_on_pin, INPUT); 
    Serial.begin(115200); 
}


void loop(){
    tone1.play(map(analogRead(Potentiom_pin), 0, 1023, 200, 1200));
    Serial.println(map(analogRead(Potentiom_pin), 0, 1023, 20, 1200));
    }


