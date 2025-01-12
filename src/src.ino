#include "Tone.h"
#include "LiquidCrystal_I2C.h"

constexpr byte encPinA{6};  // CLK
constexpr byte encPinB{5};  // DT
constexpr byte Speaker_pin{11}; // Вывод звукового сигнала
constexpr byte enc_button{2}; // Кнопка энкодера (Вкл/Выкл настройку частоты)
constexpr byte switch_button{3}; // Кнопка переключения между сигналами
constexpr int freq_down{50}; // Нижняя граница частотного диапазона
constexpr int freq_up{5000}; // Верхняя граница частотного диапазона
constexpr int time_down{100}; // Нижняя граница периода сирены (в мс)
constexpr int time_up{60000}; // Верхняя граница периода сирены (в мс)

volatile bool adj_flag = false; // Флаг включения режима регулировки
volatile bool adj_print = true; // Флаг для вывода "ADJ"
volatile byte switch_mode = 0; // Режим работы 0 - Вопроизведение первого сигнала. 1 - Воспроизведение второго сигнала. 2 - Режим сирены.
volatile bool cursor_switch = true; // Флаг для вывода курсора "<<<"
volatile bool siren_config = true;  // Флаг активирует перезапуск сирены с изменением её конфигурации

LiquidCrystal_I2C lcd(0x27,20,4); // Инициализация дисплея. SCL(A5), SDA(A4).
Tone speaker; // Создание объекта speaker класса Tone

int Freq1 = 100; // Частота первого сигнала
int Freq2 = 100; // Частота второго сигнала
unsigned int siren_duration = 1000; // Продолжительность изменения сигнала сирены



int enc_val = 0;  // Переменная определяющая поворот энкодера
int enc_val_increment = 1; // Приращение поворотов экодера за некоторое время
int acceleration_coef = 1; // Коэффициент ускорения энкодера
uint32_t increment_timer = 0; // Переменная для таймера задающего период приращения поворотов энкодера

void setup(){
    speaker.begin(Speaker_pin);    // Инициализация и назначение вывода для динамика
    pinMode(enc_button, INPUT_PULLUP);  // Назначение вывода для кнопки энкодера с подтяжкой к "+"
    pinMode(switch_button, INPUT_PULLUP);  // Назначение вывода для кнопки с подтяжкой к "+"
    attachInterrupt(0, adj_func, FALLING); // Подключение прерывания на кнопку энкодера по спаду
    attachInterrupt(1, switch_func, FALLING); // Подключение прерывания на кнопку переключения сигнала по спаду

    lcd.init(); // Инициализация дисплея
    lcd.backlight();
    
    start_screen(); // Стартовый экран
    delay(1000);
    lcd.clear();
    display_info(); // Вывод основной информации
}




void loop(){

  print_adj();  // Вывод "ADJ" если активирован режим настройки, либо стирание "ADJ"

  cursor(); // Вывод курсора

  if (adj_flag){  // Работа в режиме регулировки частоты и времени
    adjustment();
  } 

  play_mode_tone();  // Функция воспроизведения выбранного сигнала
  }




void adjustment(){ // Функция регулировки частоты

  encoder(); // Считываем вращение энкодера
  
  if (enc_val != 0){ // Проверка на поворот энкодера
    acceleration_enc();
    if (switch_mode == 0){    // Выбрана 1 граничная частота
      Freq1 = check_freq_range(Freq1 + enc_val*acceleration_coef);  // Проверка значения частоты на превышение заданных границ  
      print_value(3, 1, Freq1);
      }

    else if (switch_mode == 1){   // Выбрана вторая граничная частота
      Freq2 = check_freq_range(Freq2 + enc_val*acceleration_coef); // Проверка значения частоты на превышение заданных границ 
      print_value(3, 2, Freq2);
      }
    else{             // Выбран режим сирены с настройкой периода сирены
      siren_duration = check_time(siren_duration+enc_val*100*acceleration_coef); // Проверка значения периода на корректность 
      siren_config = true;    // Флаг активирует перезапуск сирены с изменением её конфигурации

      // Вывод периода в секундах. Например 1100 мс переводит в 1.1 сек.
      print_value(10, 3, siren_duration / 1000);
      lcd.setCursor(14,3);
      lcd.print(".");
      lcd.setCursor(15,3);
      lcd.print((siren_duration-(siren_duration/1000)*1000)/100);
    }
    }
  }

int check_freq_range(int freq){   // Проверка значения частоты на превышение заданных границ 

  if (freq < freq_down){
    return freq_up;
}
  else if (freq > freq_up){
    return freq_down;
  }
  return freq;
}

int check_time(unsigned int time){   // Проверка значения периода сирены на превышение заданных границ 
  if (time < time_down){
    return time_up;
  }
  else if (time > time_up){
    return time_down;
  }
  return time;
}
        

void play_mode_tone(){  // Функция, которая активирует воспроизведение сигнала в выбранном режиме
  if (switch_mode == 0){    // 1 граничная частота
    speaker.play(Freq1);
    }

  else if (switch_mode == 1){  // 2 граничная частота
    speaker.play(Freq2);
    }
  else{         // Режим сирены
    siren_mode();
  }
}


// Переменные параметров сирены
int Freq_increment;   // Разность между 
uint32_t freq_duration;
int start_freq;
int end_freq;
uint64_t siren_timer{0};
bool freq_up_or_down = true;  // True - Частота сигнала увеличивается, False - уменьшается
int current_freq; // Частота, воспроизводимая в моменте режима сирены
bool similar_freq = false;   // Граничные частоты совпадают, выводится 1 граничная частота

void siren_mode(){    // Функция воспроизведения в режиме сирены
  if (siren_config){  // Выполняется 1 раз при переключении на этот режим и изменении периода сирены
    siren_configuration();  // Формирование параметров сирены
  }

  if (!similar_freq){ // Проверка на несовпадение граничных частот
    siren_play(); // Функция воспроизведения сирены
  }
}

void siren_configuration(){   // Формирование параметров сирены
  siren_duration = check_time(siren_duration);
    siren_config = false;
    Freq_increment = abs(Freq1 - Freq2);

    if (Freq_increment == 0){  // Граничные частоты совпадают
      speaker.play(Freq1);
      similar_freq = true;
    }

    else {   // Выполянется если частоты не совпадают
      similar_freq = false;
      // Определение начальной частоты
      if (Freq1 > Freq2){
        start_freq = Freq2;
        end_freq = Freq1;
      }
      else{
        start_freq = Freq1;
        end_freq = Freq2;
      }

      siren_timer = micros();
      uint32_t siren_duration_micros{siren_duration};
      freq_duration = siren_duration_micros*1000 / Freq_increment;  // Расчёт длительности изменения сигнала на 1 Гц (в мкс)
      current_freq = start_freq;
    }
}

void siren_play(){ // Функция воспроизведения сирены
  if (micros() - siren_timer >= freq_duration) {

    if(freq_up_or_down){  // Фаза увеличения частоты сирены
      current_freq += 1;
      if (current_freq >= end_freq){
        freq_up_or_down = false;
      }
    }

    else{              // Фаза уменьшения частоты сирены
      current_freq -= 1;
      if (current_freq <= start_freq){
        freq_up_or_down = true;
      }
    }
    siren_timer = micros(); // Обновления таймера
    speaker.play(current_freq);  // Воспроизведение частоты
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
    lcd.print("Siren.Time");
    lcd.setCursor(13,3);
    lcd.print("1.0");
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
      }

    else if (switch_mode == 1){
      lcd.setCursor(12,2);
      lcd.print("<<<");
      lcd.setCursor(12,1);
      lcd.print("   ");
    }
    else{
      lcd.setCursor(12,2);
      lcd.print("   ");
    }
  }
  
  cursor_switch = false;  // Перевод флаг в false для исключения вывода курсора на том же месте каждую итерацию
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
    siren_config = true;
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

void acceleration_enc(){    // Функция расчёт ускорения энкодера
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

void print_value(int col, int line, int value){ // Корректно выводит значения до 9999 на дисплей
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