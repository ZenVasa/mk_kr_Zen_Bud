#include "Tone.h"
#include "LiquidCrystal_I2C.h"

constexpr byte encPinA{6};  // CLK
constexpr byte encPinB{5};  // DT
constexpr byte Speaker_pin{11}; // Вывод звукового сигнала
constexpr byte enc_button{2}; // Кнопка энкодера (Вкл/Выкл настройку частоты)
constexpr byte switch_button{3}; // Кнопка переключения между сигналами
constexpr int freq_down{50}; // Нижняя граница частотного диапазона
constexpr int freq_up{5000}; // Верхняя граница частотного диапазона

volatile bool adj_flag = false; 
volatile bool adj_print = true; 
volatile bool switch_flag = false; 
volatile bool cursor_switch = true; 

LiquidCrystal_I2C lcd(0x27,20,4); // Инициализация дисплея. SCL(A5), SDA(A4).
Tone speaker; // Создание объекта speaker класса Tone

int Freq1 = 100; // Частота первого сигнала
int Freq2 = 100; // Частота второго сигнала


int enc_val = 0;
int enc_val_increment = 1;
int acceleration_coef = 1;
uint32_t increment_timer = 0;

void setup(){
    speaker.begin(Speaker_pin);    // Инициализация и назначение вывода для динамика
    pinMode(enc_button, INPUT_PULLUP);
    pinMode(switch_button, INPUT_PULLUP);
    attachInterrupt(0, adj_func, FALLING); // Подключение прерывания на кнопку энкодера по спаду
    attachInterrupt(1, switch_func, FALLING); // Подключение прерывания на кнопку переключения сигнала по спаду

    lcd.init(); // Инициализация дисплея
    lcd.backlight();
    
    start_screen();
    display_info();
}




void loop(){

  print_adj();
  cursor();

  if (adj_flag){  // Работа в режиме регулировки частоты
    adjustment();
  } 

  play_tone();  // Функция воспроизведения выбранного сигнала
  }




void adjustment(){ // Функция регулировки частоты

  encoder(); // Считываем вращение энкодера
  acceleration_enc();

  if (enc_val != 0){ // Проверка на поворот энкодера
    enc_val_increment += abs(enc_val);
    if (switch_flag){    
      Freq1 = check_freq_range(Freq1 + enc_val*acceleration_coef);
      print_value(3, 1, Freq1);
      }

    else{
      Freq2 = check_freq_range(Freq2 + enc_val*acceleration_coef);
      print_value(3, 2, Freq2);
      }
    }
  }

int check_freq_range(int freq){

  if (freq < freq_down){
    return freq_up;
}
  else if (freq > freq_up){
    return freq_down;
  }
  return freq;
}
        

void play_tone(){
  if (switch_flag){    
    speaker.play(Freq1);
    }

  else{
    speaker.play(Freq2);
    }
}

void start_screen(){  // Вывод на дисплей названия устройства
    lcd.setCursor(5,1);
    lcd.print("Two-tone");
    lcd.setCursor(6,2);
    lcd.print("Emitter");
    delay(2000);
    lcd.clear();
}

void display_info(){  // Вывод основной информации
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

void cursor(){  // Вывод на дисплей курсора, указывающего на выбранный сигнал
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

void print_adj(){ // Вывод на дисплей сообщения о том, что включён режим регулировки
  if (adj_print){ // Проверка: выводилось ли сообщение ранее

    if (adj_flag){  // Режим регулирования включён
      lcd.setCursor(0,3);
      lcd.print("ADJ");
    }

    else{ // Режим регулирования выключен
      lcd.setCursor(0,3);
      lcd.print("   ");
    }
  }
  adj_print = false;  // Чтобы сообщение не выводилось каждую итерацию
}

volatile uint32_t debounce{0}; // Переменнная для значения таймера

void adj_func(){ // Немедленно выполняется при нажатии кнопки энкодера
  if (millis() - debounce >= 250) {  // Защита от дребезга контактов
    debounce = millis();
    adj_flag = !adj_flag;
    adj_print = true; // Чтобы оптимизировать программу и не выводить каждую итерацию "ADJ"
    }
}

void switch_func(){ // Немедленно выполняется при нажатии кнопки переключения сигнала
  if (millis() - debounce >= 250) {  // Защита от дребезга контактов
    debounce = millis();
    switch_flag = !switch_flag;
    cursor_switch = true; // Чтобы оптимизировать программу и не выводить каждую итерацию курсор
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

void acceleration_enc(){
  if (((millis() - increment_timer) < 300) && (enc_val_increment <= 5)){
    acceleration_coef = 1;
  }

  else if (((millis() - increment_timer) < 300) && (enc_val_increment > 5)){
    acceleration_coef = enc_val_increment * 2;
  }

  else {
    increment_timer = millis();
    enc_val_increment = 1;
    acceleration_coef = 1;
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