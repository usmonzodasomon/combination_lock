// кодовый замок
//  считывает три кнопки
//  выводит на экран нажатые клавиши
//  если введеный код совпадает с заданным то включет сервомотор

// ПОРТА - Порт ЖКИ инициализируется в подпрограмме инициализации
// ПОРТВ - порт клавиатуры подключен так
// верхний ряд с права на лево первые два питание
// затем 7-6-5-4 пины порта
// нижний ряд с лева на право 0-1-2-3 пины
// последние два питание
// ПОРТD - светодиоды не используются но могут пригодится для отладки
// ПОРТС - порт сервомотора Сервотор подключен к пину motor

#define F_CPU 1000000
#define number_pulse 30             // количество импульсов для сервомотора для поворота на 90 град
#define motor 0                     // номер пина порта сервомотора
#include <avr/io.h>                 // стандартная библиотека ввод-вывод
#include <avr/interrupt.h>          // стандартная библиотека прерываний
#include <util/delay.h>             // стандартная библиотека задержек
#include <string.h>                 // стандартная библиотека строки
#include "LCD.h"                    // библиотека ЖКИ
char code[4];                       // массив для введеного кода
char pass[] = {'A', '1', 'F', '9'}; // это код с которым сравниваем
unsigned char j, i, temp, row, col;

char key_code[4][4] = { // матрица клавиатуры
    {'E', 'F', '0', 'D'},
    {'9', '8', '7', 'C'},
    {'6', '5', '4', 'B'},
    {'3', '2', '1', 'A'}};

void open() // управление сервомотором поворот на 90 градусов
{           // нужно дать необходимое количество импульсов
    j = 0;
    while (j <= number_pulse)
    {
        PORTC |= (1 << motor); // начало имупульса
        _delay_ms(5);
        PORTC &= (0 << motor); // конец импульса
        _delay_ms(5);
        j++;
    }
}
void initTimer1Normal()    // Настройка таймера 1 в обычный режим
{                          // он "гоняет нолик" по строкам (столбцам)
    TCCR1B |= (1 << CS10); // prescaler 1-1
    TCNT1 = 0;             // Сбросить таймер1 в 0
    TIMSK |= (1 << TOIE1); // Включить прерывания от таймера 1 по переполнению
}

void initPorts() // Подпрограмма инициализации портов
{
    DDRD = 0xff;       // отладочный светодиодный порт
    DDRC = 0xff;       // порт для сервомотора
    DDRB = 0b11110000; // порт клавиатуры
    // старшие - для вывода бегущего нуля от таймера
    // младшие для считывания кода строки (столбца)
    PORTD = 0xff; // выключить светодиоды
}

char GetKey(void) // драйвер клавиатуры
{                 // ждет нажатия и отпускания кнопки
m1:
    temp = ~PINB;             // читаем порт клавиатуры в верх ногами
    temp = temp & 0b00001111; // убиваем старшие 4 бита
    if (temp == 0)
        goto m1; // если ничего не нажато то ждем
    else         // если нажали то останавливаем таймер
    {
        cli();            // чтобы не бегал ноль по страшим 4-м битам
        while (temp != 0) // теперь пока нажата и удерживается кнопка определяем ее код
        {
            switch (temp)
            {
            case 8:
                row = 3;
                break;
            case 4:
                row = 2;
                break;
            case 2:
                row = 1;
                break;
            case 1:
                row = 0;
                break;
            }
            switch (i)
            {
            case 0b01111111:
                col = 3;
                break;
            case 0b10111111:
                col = 2;
                break;
            case 0b11011111:
                col = 1;
                break;
            case 0b11101111:
                col = 0;
                break;
            }
            temp = ~PINB; // ждем пока кнопку отпустят
            temp = temp & 0b00001111;
        }
        sei();                       // отпустили кнопку теперь включаем таймер
        return (key_code[row][col]); // и возвращаем ее код
    }
}

int main(void)
{
    i = 0b01111111;     // это число таймер выдает в самом начале подсвечивает нулем старший пин
    initPorts();        // инициализировать нужные порты
    LCDInit();          // инициализация дисплея
    initTimer1Normal(); // настроить таймер 1 на норм. режим
    sei();              // разрешить прерывания - включить таймер теперь нолик бегает по старшим битам

    LCDCursorPosition(0);      // верхняя строчка ЖКИ
    LCDWriteByte(LCD_DR, 'P'); // напечатать этот код
    LCDWriteByte(LCD_DR, 'A'); // напечатать этот код
    LCDWriteByte(LCD_DR, 'S'); // напечатать этот код
    LCDWriteByte(LCD_DR, 'S'); // напечатать этот код
    LCDWriteByte(LCD_DR, ':'); // напечатать этот код
    LCDWriteByte(LCD_DR, ' '); // напечатать этот код

    code[0] = GetKey();            // ждать нажатия первой кнопки и получить ее код
    LCDWriteByte(LCD_DR, code[0]); // напечатать этот код

    code[1] = GetKey(); // вторая кнопка
    LCDWriteByte(LCD_DR, code[1]);

    code[2] = GetKey(); // третья кнопка
    LCDWriteByte(LCD_DR, code[2]);

    code[3] = GetKey(); // четвертая кнопка
    LCDWriteByte(LCD_DR, code[3]);

    if (strcmp(code, pass) == 0) // если введеный код совпадает с заданным
    {
        open(); // то открыть замок
        LCDCursorPosition(1);      // нижняя строчка ЖКИ
        LCDWriteByte(LCD_DR, 'O'); // напечатать этот код
        LCDWriteByte(LCD_DR, 'P'); // напечатать этот код
        LCDWriteByte(LCD_DR, 'E'); // напечатать этот код
        LCDWriteByte(LCD_DR, 'N'); // напечатать этот код

    } else {
        LCDCursorPosition(1);      // нижняя строчка ЖКИ
        LCDWriteByte(LCD_DR, 'E'); // напечатать этот код
        LCDWriteByte(LCD_DR, 'R'); // напечатать этот код
        LCDWriteByte(LCD_DR, 'R'); // напечатать этот код
        LCDWriteByte(LCD_DR, 'O'); // напечатать этот код
        LCDWriteByte(LCD_DR, 'R'); // напечатать этот код
    }
}
