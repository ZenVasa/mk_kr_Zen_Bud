#include <avr/io.h>
#include <avr/interrupt.h>

#define BUZZPIN1 5
#define BUZZPIN2 6

void setup()
{
    pinMode(LEDPIN, OUTPUT);
    init_timer1();
    

}

void loop()
{
    // основная программа
}

ISR(TIMER1_COMPA_vect)
{
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
}

void init_timer1(){
        cli();  // отключить глобальные прерывания
    // инициализация Timer1
    TCCR1A = 0;   // установить регистры в 0
    TCCR1B = 0;

    OCR1A = 2; // установка регистра совпадения

    TCCR1B |= (1 << WGM12);  // включить CTC режим 
    TCCR1B |= (1 << CS10); // Установить биты на коэффициент деления 1 (без деления)

    TIMSK1 |= (1 << OCIE1A);  // включить прерывание по совпадению таймера 
    sei(); // включить глобальные прерывания
}

void init_timer2(){
        cli();  // отключить глобальные прерывания
    // инициализация Timer1
    TCCR1A = 0;   // установить регистры в 0
    TCCR1B = 0;

    OCR1A = 2; // установка регистра совпадения

    TCCR1B |= (1 << WGM12);  // включить CTC режим 
    TCCR1B |= (1 << CS10); // Установить биты на коэффициент деления 1 (без деления)

    TIMSK1 |= (1 << OCIE1A);  // включить прерывание по совпадению таймера 
    sei(); // включить глобальные прерывания
}
