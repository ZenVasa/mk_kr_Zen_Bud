#include "Tone.h"
#include "LiquidCrystal_I2C.h"

constexpr byte encPinA{6};  // CLK
constexpr byte encPinB{5};  // DT
constexpr byte Speaker_pin{11};
constexpr byte Potentiom_pin{A1}; // Аналоговый вход для потенциометра
constexpr byte enc_button{2}; // Кнопка энкодера 
constexpr byte switch_button{3}; // Кнопка переключения между сигналами

volatile bool adj_flag = false; 
volatile bool switch_flag = false; 
volatile bool cursor_switch = true; 

LiquidCrystal_I2C lcd(0x27,20,4); // Инициализация дисплея. SCL(A5), SDA(A4).
Tone speaker; // Инициализация источника звука

int Freq1 = 100; // Частота первого сигнала
int Freq2 = 100; // Частота второго сигнала

int enc_val = 0;

void setup(){
    speaker.begin(Speaker_pin);    // Назначение вывода для динамика
    pinMode(enc_button, INPUT_PULLUP);
    pinMode(switch_button, INPUT_PULLUP);
    Serial.begin(115200); 
    attachInterrupt(0, adj_func, FALLING); // Подключение прерывания на кнопку энкодера
    attachInterrupt(1, switch_func, FALLING); // Подключение прерывания на кнопку переключения сигнала

    lcd.init(); // Инициализация дисплея
    lcd.backlight();
    
    start_screen();
    display_info();
}




void loop(){

  if (adj_flag){  // Работа в режиме настройки частоты
    lcd.setCursor(0,3);
    lcd.print("ADJ");
    adjustment();
  } 

  else{ // Работа в режиме обычного воспроизведения
    cursor();
    play_tone();
    lcd.setCursor(0,3);
    lcd.print("   ");
  }

  
  speaker.play(Freq1);
    }




void adjustment(){
  while (adj_flag)
    {
      cursor();
      encoder(); // Считываем вращение энкодера

      if (enc_val != 0){ // Проверка на поворот энкодера

        if (switch_flag){    
          Freq1 += enc_val;
          print_value(3, 1, Freq1);
          }

        else{
          Freq2 += enc_val;
          print_value(3, 2, Freq2);
          }
        }
      play_tone();
      }        
}

void play_tone(){
  if (switch_flag){    
    speaker.play(Freq1);
    }

  else{
    speaker.play(Freq2);
    }
}

void start_screen(){
    lcd.setCursor(7,1);
    lcd.print("Hello");
    lcd.setCursor(7,2);
    lcd.print("World");
    delay(2000);
    lcd.clear();
}

void display_info(){
    lcd.setCursor(0,0);
    lcd.print("Tones:");
    lcd.setCursor(0,1);
    lcd.print("1.");
    lcd.setCursor(0,2);
    lcd.print("2.");
    lcd.setCursor(4,1);
    lcd.print(Freq1);
    lcd.setCursor(4,2);
    lcd.print(Freq2);
    lcd.setCursor(8,1);
    lcd.print("Hz");
    lcd.setCursor(8,2);
    lcd.print("Hz");
    cursor();
}

void cursor(){
  if (cursor_switch){
    if (switch_flag){    
            lcd.setCursor(12,1);
            lcd.print("<<<");
            lcd.setCursor(12,2);
            lcd.print("   ");
            }

          else{
            lcd.setCursor(12,2);
            lcd.print("<<<");
            lcd.setCursor(12,1);
            lcd.print("   ");
            }
  }
  cursor_switch = false;
}

volatile uint32_t debounce;

void adj_func(){ // Немедленно выполняется при нажатии кнопки энкодера
  if (millis() - debounce >= 250) {
    debounce = millis();
    adj_flag = !adj_flag;
    }
}

void switch_func(){ // Немедленно выполняется при нажатии кнопки переключения сигнала
  if (millis() - debounce >= 250) {
    debounce = millis();
    switch_flag = !switch_flag;
    cursor_switch = !cursor_switch;
    }
}


byte prevState; // Предыдущие значения на выводах энкодера
// Переменные для значений на выводах энкодера
bool encA;
bool encB;

void encoder(){ // Программа для считывания вращения энкодера
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

void print_value(int col, int line, int value){ // Корректно печатает значения до 9999
  if ((value < 1000) && (value >= 100)){
    lcd.setCursor(col, line);
    lcd.print(" ");
    lcd.setCursor(col+1, line);
    lcd.print(value);
  }
  else if ((value < 100) && (value >= 10)){
    lcd.setCursor(col, line);
    lcd.print("  ");
    lcd.setCursor(col+2, line);
    lcd.print(value);
  }
  else if (value < 10) {
    lcd.setCursor(col, line);
    lcd.print("   ");
    lcd.setCursor(col+3, line);
    lcd.print(value);
    }
  else {
    lcd.setCursor(col, line);
    lcd.print(value);
  }
}