#include "Tone.h"
#include "LiquidCrystal_I2C.h"

constexpr byte encPinA{6};  // CLK
constexpr byte encPinB{5};  // DT
constexpr byte Speaker_pin{11}; // Вывод звукового сигнала
constexpr byte enc_button{2}; // Кнопка энкодера (Вкл/Выкл настройку частоты)
constexpr byte switch_button{3}; // Кнопка переключения между сигналами
constexpr int freq_down{50}; // Нижняя граница частотного диапазона
constexpr int freq_up{5000}; // Верхняя граница частотного диапазона

volatile bool adj_flag = false; // Флаг включения режима регулировки
volatile bool adj_print = true; 
volatile byte switch_mode = 0; // Режим работы 
// 0 - Вопроизведение первого сигнала. 1 - Воспроизведение второго сигнала. 2 - Режим сирены.
volatile bool cursor_switch = true; 

LiquidCrystal_I2C lcd(0x27,20,4); // Инициализация дисплея. SCL(A5), SDA(A4).
Tone speaker; // Создание объекта speaker класса Tone

int Freq1 = 100; // Частота первого сигнала
int Freq2 = 100; // Частота второго сигнала
int siren_duration = 100; // Продолжительность изменения сигнала сирены



int enc_val = 0;
int enc_val_increment = 1;
int acceleration_coef = 1;
uint32_t increment_timer = 0;

void setup(){
    speaker.begin(Speaker_pin);    // Инициализация и назначение вывода для динамика
    pinMode(enc_button, INPUT_PULLUP);  // Назначение вывода для кнопки энкодера с подтяжкой к "+"
    pinMode(switch_button, INPUT_PULLUP);  // Назначение вывода для кнопки с подтяжкой к "+"
    attachInterrupt(0, adj_func, FALLING); // Подключение прерывания на кнопку энкодера по спаду
    attachInterrupt(1, switch_func, FALLING); // Подключение прерывания на кнопку переключения сигнала по спаду

    lcd.init(); // Инициализация дисплея
    lcd.backlight();
    
    start_screen();
    delay(2000);
    lcd.clear();
    display_info();
}




void loop(){

  print_adj();

  if (adj_flag){  // Работа в режиме регулировки частоты и времени
    adjustment();
  } 

  play_mode_tone();  // Функция воспроизведения выбранного сигнала

    cursor(); // Вывод курсора
  }




void adjustment(){ // Функция регулировки частоты

  encoder(); // Считываем вращение энкодера
  
  if (enc_val != 0){ // Проверка на поворот энкодера
    acceleration_enc();
    if (switch_mode == 0){    
      Freq1 = check_freq_range(Freq1 + enc_val*acceleration_coef);
      print_value(3, 1, Freq1);
      }

    else if (switch_mode == 1){
      Freq2 = check_freq_range(Freq2 + enc_val*acceleration_coef);
      print_value(3, 2, Freq2);
      }
    else{
      siren_duration += enc_val;
      print_value(16, 3, siren_duration);
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
        

void play_mode_tone(){
  if (switch_mode == 0){    
    speaker.play(Freq1);
    }

  else if (switch_mode == 1){
    speaker.play(Freq2);
    }
  else{
    siren_mode();
  }
}

int Freq_increment;
int freq_duration;
int start_freq;
int end_freq;
int siren_timer{0};
bool freq_up_or_down = true;  // True - Частота сигнала увеличивается, False - уменьшается
int current_freq = 100; // Частота, воспроизводимая в моменте режима сирены

void siren_mode(){
  if (cursor_switch || (enc_val >= 1 && adj_flag)){  // Выполняется 1 раз при переключении на этот режим, либо изменении продолжительности сирены
    Freq_increment = abs(Freq1 - Freq2);
    if (Freq_increment == 0){
    speaker.play(Freq1);
    }
    else {
      if (Freq1 > Freq2){
        start_freq = Freq2;
        end_freq = Freq1;
      }
      else{
        start_freq = Freq1;
        end_freq = Freq2;
      }
      siren_timer = millis();
      freq_duration = siren_duration / Freq_increment;  // Расчёт длительности изменения сигнала на 1 Гц
      current_freq = start_freq;
    }
  }

  if (millis() - siren_timer >= freq_duration) {
    if(freq_up_or_down){
      current_freq += 1;
      if (current_freq > end_freq){
        current_freq = start_freq;
        freq_up_or_down = false;
      }
    }
    else{

    }
    debounce = millis();
    switch_mode += 1;
  }

}

void start_screen(){  // Вывод на дисплей названия устройства
    lcd.setCursor(5,1);
    lcd.print("Two-tone");
    lcd.setCursor(6,2);
    lcd.print("Emitter");
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
    lcd.setCursor(0,3);
    lcd.print("Siren. Time:");
    lcd.setCursor(16,3);
    lcd.print(siren_duration);
    lcd.setCursor(8,1);
    lcd.print("Hz");
    lcd.setCursor(8,2);
    lcd.print("Hz");
    cursor();
}

void cursor(){  // Вывод на дисплей курсора, указывающего на выбранный сигнал
  if (cursor_switch){
    if (switch_mode == 0){    
      lcd.setCursor(12,1);
      lcd.print("<<<");
      lcd.setCursor(12,3);
      lcd.print("   ");
      }

    else if (switch_mode == 1){
      lcd.setCursor(12,2);
      lcd.print("<<<");
      lcd.setCursor(12,1);
      lcd.print("   ");
    }
    else{
      lcd.setCursor(12,3);
      lcd.print("<<<");
      lcd.setCursor(12,2);
      lcd.print("   ");
    }
  }
  
  cursor_switch = false;
}

void print_adj(){ // Вывод на дисплей сообщения о том, что включён режим регулировки
  if (adj_print){ // Проверка: выводилось ли сообщение ранее

    if (adj_flag){  // Режим регулирования включён
      lcd.setCursor(17,3);
      lcd.print("ADJ");
    }

    else{ // Режим регулирования выключен
      lcd.setCursor(17,3);
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
    switch_mode += 1;
    if (switch_mode == 3){
      switch_mode = 0;
    }
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
  enc_val_increment +=abs(enc_val);
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