#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define DEBUG false

#define RANK_6_LONG    "six"
#define RANK_6_SHORT    "6"
#define RANK_7_LONG    "seven"
#define RANK_7_SHORT    "7"
#define RANK_8_LONG    "eight"
#define RANK_8_SHORT    "8"
#define RANK_9_LONG    "nine"
#define RANK_9_SHORT    "9"
#define RANK_10_LONG    "ten"
#define RANK_10_SHORT    "10"
#define RANK_J_LONG    "jack"
#define RANK_J_SHORT    "j"
#define RANK_Q_LONG    "queen"
#define RANK_Q_SHORT    "q"
#define RANK_K_LONG    "king"
#define RANK_K_SHORT    "k"
#define RANK_A_LONG    "ace"
#define RANK_A_SHORT    "a"

#define SUIT_HEARTS "hearts"
#define SUIT_DIAMONDS "diamonds"
#define SUIT_CLUBS "clubs"
#define SUIT_SPADES "spades"
#define SUIT_HEARTS_SHORT "h"
#define SUIT_DIAMONDS_SHORT "d"
#define SUIT_CLUBS_SHORT "c"
#define SUIT_SPADES_SHORT "s"

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int stack[36], drop[36], uhand[36], chand[36], table[36], cadd[36];
int _stack, _drop, _uhand, _chand, _table, _cadd, fav;
// для хранения карт используются целые числа от 0 до 35. 
// Достоинства кодируются от 0 до 8 (смотрите сравнение с k в функции dumpv()
// Масти - от 0 до 3. Итого - целочисленное деление на 4 для числа-карты показывает её масть (дробных частей у целых переменных нет, поэтомо оно выглядит так странно, например 15/4=3).
// Остаток от деления числа-карты на 9 показывает её достоинство.
//
// Есть группа массивов, которые хранят карты для каких-то конкреных задач. Зовутся они NAME, количество актуальных значений в них хранится в переменной _NAME. Например: drop[36], _drop
//
// stack[36] - это колода карт, из котой после перемешивания берутся карты
// drop[36] - сюда "сбрасываются" "отбитые" карты со "стола"
// uhand[36] - User HAND - карты в руке пользователя, отсюда карты "ходят" на "стол"
// chand[36] - Computer HAND - карты в руке компьютера, аналогичен uhand, только у другого участника. Хорошая мысль из этих двух массивов собрать один, двумерный, но для начала у нас 
//             всего два участника, и сильно уходить в абстракции пока не хочется.
// cadd[36] - понадобился для более удобной реализации "подбрасывания" по одной карте ПК, после того, как он говорит "беру". Если как-то по другому организовать "стол", он будет не нужен.
// table[36] - "стол". карты лежат в массиве в пордяке "карта которой ходили" - "карта которой её побили" - [ и так далее до бесконечности ]. В таком варианте карта "наверху" массива, если
//             число элеменов нечётное - это карта которую нужно отбивать. Если число элеменов "стола" чётное - верхняя карта - это карта, которой побили вторую карту сверху.
//             Проблема такого подхода проявилась при реализации механизма "подбрасывания" в момент, когда подбрасываемы карты точно не будут отбиваться. В результате в массиве появляются
//             "дырки" вместо карт, которыми оппонент отбивается. При этом сам массив не содержит местоположений этих "дырок" и становится сложно определить количество актуальных элеменов.
//             В будущем я бы сделал массив таким образом, чтобы он содержал отдельно пары отбитых карт и отдельно - ещё не отбитые, которые пермещались бы в "отбитые" карты стола.
// fav - favorite. Это козырь. Он находится в начале массива stack[] (на "дне"). Ввиду того, что рано или поздно stack[] опустеет (его же раздают!), его нужно помнить отдельно.
// dumpv, dumps - показывает дамп передаваемого массива. v-verbose, подробно (можно подглядывать, что приходилось делать при отладке), s - silent, "тихо" - показывает только "квадратики".
// draw() - отрисовывает окружение и "стол", поменяв тут dumps на dumpv, можно подглядывать карты в колоде и у противника. Полезно для отладки.
// showtable() - отрисовывает только "стол". Сначала хотел отрисовывать по-другому, "ниже", "выше", но пока-что отображает парами слева направо. 
// decode() - из двух введённых слов (либо первых букв) определяет число-карту, о которой говорит пользователь = достоинство + (масть*9)
//              (6,7,8,9,10,j,q,k,a)=(6,7,8,9,10,jack,queen,king,ace) = достоинство (0..8)
//              (h,d,c,s)=(hearts,diamonds,clubs,spades) = масть (0..3)
//
int whonext; // в этой переменной будет храниться тот, чей сейчас ход


void dumpv(int *arr, int count, const char *caption) {
    int i, k;
    printf("\n================%s=============%i=\n", caption, count);
    for (i = 0; i < count; ++i) {
        char c1 = 3 + arr[i] / 9;
        k = arr[i] % 9;
        if (k == 0) printf("6");
        if (k == 1) printf("7");
        if (k == 2) printf("8");
        if (k == 3) printf("9");
        if (k == 4) printf("10");
        if (k == 5) printf("J");
        if (k == 6) printf("Q");
        if (k == 7) printf("K");
        if (k == 8) printf("A");
        printf("%c\t", c1);
        if (i % 8 == 7) printf("\n");
    }
}

void dumps(int *arr, int count, const char *caption) {
    int i, k;
    printf("\n================%s=============%i=\n", caption, count);
    for (i = 0; i < count; ++i) {
        char c1 = 3 + arr[i] / 9;
        k = arr[i] % 9;
        if (k == 0) printf("\xb1");
        if (k == 1) printf("\xb1");
        if (k == 2) printf("\xb1");
        if (k == 3) printf("\xb1");
        if (k == 4) printf("\xb1");
        if (k == 5) printf("\xb1");
        if (k == 6) printf("\xb1");
        if (k == 7) printf("\xb1");
        if (k == 8) printf("\xb1");
        printf(" ");
    }
}

void showcard(int i) {
    int k = i % 9;
    if (k == 0) printf(" 6");
    if (k == 1) printf(" 7");
    if (k == 2) printf(" 8");
    if (k == 3) printf(" 9");
    if (k == 4) printf("10");
    if (k == 5) printf(" J");
    if (k == 6) printf(" Q");
    if (k == 7) printf(" K");
    if (k == 8) printf(" A");
    printf("%c", 3 + i / 9);
}

void showtable(void) {
    int i, k;
    printf("\n=========================TABLE==============================\n");
    for (i = 0; i < _table; ++i) {
        showcard(table[i]);
        printf(" ");
        if (i % 8 == 7) printf("\n");
    }
    printf("\n=============================/TABLE===========================\n");
}

int decode(char *rank, char *suit) // преобразует текст в номера карты
{
    int n = -1;
    int r = -1;
    if (strcmp(rank, RANK_6_LONG) == 0) n = 0; // везде сравнение строк с оригиналом, сначала в длинном формате
    if (strcmp(rank, RANK_7_LONG) == 0) n = 1;
    if (strcmp(rank, RANK_8_LONG) == 0) n = 2;
    if (strcmp(rank, RANK_9_LONG) == 0) n = 3;
    if (strcmp(rank, RANK_10_LONG) == 0) n = 4;
    if (strcmp(rank, RANK_J_LONG) == 0) n = 5;
    if (strcmp(rank, RANK_Q_LONG) == 0) n = 6;
    if (strcmp(rank, RANK_K_LONG) == 0) n = 7;
    if (strcmp(rank, RANK_A_LONG) == 0) n = 8;
    if (strcmp(rank, RANK_6_SHORT) == 0) n = 0; // потом в коротком
    if (strcmp(rank, RANK_7_SHORT) == 0) n = 1;
    if (strcmp(rank, RANK_8_SHORT) == 0) n = 2;
    if (strcmp(rank, RANK_9_SHORT) == 0) n = 3;
    if (strcmp(rank, RANK_10_SHORT) == 0) n = 4;
    if (strcmp(rank, RANK_J_SHORT) == 0) n = 5;
    if (strcmp(rank, RANK_Q_SHORT) == 0) n = 6;
    if (strcmp(rank, RANK_K_SHORT) == 0) n = 7;
    if (strcmp(rank, RANK_A_SHORT) == 0) n = 8;
    if (n == -1) return (-1);
    if (strcmp(suit, SUIT_HEARTS) == 0)
        r = n; // масти "добавляют" соответствующее "смещение" в число, кратное 9 (черви 0-8, бубны 9-17 и т.д.)
    if (strcmp(suit, SUIT_DIAMONDS) == 0) r = 9 + n;
    if (strcmp(suit, SUIT_CLUBS) == 0) r = 18 + n;
    if (strcmp(suit, SUIT_SPADES) == 0) r = 27 + n;
    if (strcmp(suit, SUIT_HEARTS_SHORT) == 0) r = n;
    if (strcmp(suit, SUIT_DIAMONDS_SHORT) == 0) r = 9 + n;
    if (strcmp(suit, SUIT_CLUBS_SHORT) == 0) r = 18 + n;
    if (strcmp(suit, SUIT_SPADES_SHORT) == 0) r = 27 + n;
    if (r == -1) return (-1);
    return r;
}

void draw(void) {
    int i, j, k, n;
    system("cls");
    printf("Cards in stack: %i\t\tCards in drop: %i\n", _stack, _drop);
    printf("          \xda\xc4\xc4\xc4\xbf\nFavorite: \xb3");
    showcard(fav);
    printf("\xb3\n          \xc0\xc4\xc4\xc4\xd9\n", 3 + fav / 9);
    printf("CPU: ");
    for (i = 0; i < _chand; ++i) printf("\xb1 ");
    printf("\n\xda");
    for (i = 0; i < 40; ++i) printf("\xc4");
    printf("\xbf\n");
    if (((whonext == 0) && (_table % 2 == 1)) || ((whonext == 1) && (_table % 2 == 0))) {
        for (i = 0; i < 3; ++i) {
            printf("\xb3");
            k = 0;
            for (j = 0; j < _table; j += 2) {
                if (i == 0) printf("\xda\xc4\xc4\xc4\xbf");
                if (i == 1) {
                    printf("\xb3");
                    showcard(table[j]);
                    printf("\xb3", 3 + table[j] / 9);
                }
                if (i == 2) printf("\xc0\xc4\xc4\xc4\xd9");
                k += 5;
            }
            for (; k < 40; ++k) printf(" ");
            printf("\xb3\n");
        }
    }
    if (((whonext == 0) && (_table % 2 == 1)) || ((whonext == 1) && (_table % 2 == 0))) {
        for (i = 0; i < 3; ++i) {
            printf("\xb3");
            k = 0;
            for (j = 1; j < _table; j += 2) {
                if (i == 0) printf("\xda\xc4\xc4\xc4\xbf");
                if (i == 1) {
                    printf("\xb3");
                    showcard(table[j]);
                    printf("\xb3", 3 + table[j] / 9);
                }
                if (i == 2) printf("\xc0\xc4\xc4\xc4\xd9");
                k += 5;
            }
            for (; k < 40; ++k) printf(" ");
            printf("\xb3\n");
        }
    }

    if (((whonext == 0) && (_table % 2 == 0)) || ((whonext == 1) && (_table % 2 == 1))) {
        for (i = 0; i < 3; ++i) {
            printf("\xb3");
            k = 0;
            for (j = 1; j < _table; j += 2) {
                if (i == 0) printf("\xda\xc4\xc4\xc4\xbf");
                if (i == 1) {
                    printf("\xb3");
                    showcard(table[j]);
                    printf("\xb3", 3 + table[j] / 9);
                }
                if (i == 2) printf("\xc0\xc4\xc4\xc4\xd9");
                k += 5;
            }
            for (; k < 40; ++k) printf(" ");
            printf("\xb3\n");
        }
    }
    if (((whonext == 0) && (_table % 2 == 0)) || ((whonext == 1) && (_table % 2 == 1))) {
        for (i = 0; i < 3; ++i) {
            printf("\xb3");
            k = 0;
            for (j = 0; j < _table; j += 2) {
                if (i == 0) printf("\xda\xc4\xc4\xc4\xbf");
                if (i == 1) {
                    printf("\xb3");
                    showcard(table[j]);
                    printf("\xb3", 3 + table[j] / 9);
                }
                if (i == 2) printf("\xc0\xc4\xc4\xc4\xd9");
                k += 5;
            }
            for (; k < 40; ++k) printf(" ");
            printf("\xb3\n");
        }
    }
    printf("\xc0");
    for (i = 0; i < 40; ++i) printf("\xc4");
    printf("\xd9\n");
    for (i = 0; i <= _uhand /
                     15; ++i) // 15 карт в строке (одна карта - 5 симоволов:|10*|, в консоли обычно 80 столбцов, 80/5=16, но мы делаем с запасом)
    {
        for (n = 0; n < 3; ++n) // аккуратно выводим каждую "строчку" карт по 15 штук в каждой в три "прохода"
        {
            for (j = i * 15; (j < _uhand) && (j < i * 15 +
                                                  15); ++j) // сложное условие для проверки - цикл длится пока оба условия истинны. Это позволяет "отрезать" цикл до 15 элеменов (правая часть)
// и в то же время сделать его меньше 15, если это рисуется какя-то уже не первая по счёту строка карт
            {
                if (n == 0) printf("\xda\xc4\xc4\xc4\xbf"); // верхняя строчка псевдографикой ASCII, что-то вроде +---+
                if (n == 1) // |<достоинство><масть>|
                {
                    printf("\xb3");
                    showcard(uhand[j]);
                    printf("\xb3", 3 + uhand[j] / 9);
                }
                if (n == 2) printf("\xc0\xc4\xc4\xc4\xd9"); // верхняя строчка псевдографикой ASCII, что-то вроде +---+
            }
            printf("\n");
        }
    }
//  dumpv(&uhand[0],_uhand," YOUR HAND "); // показать содержимое рук пользователя

}

void _draw(void) {
    system("cls");
    printf("\n*******************************************************\n");
// неизящный способ "промотать" экран вверх вместо очистки экрана. Это позволит при желании отмотать партию назад, чтобы посмотреть
    dumps(&stack[0], _stack, " STACK DUMP "); // показать содеримое колоды
    dumps(&drop[0], _drop, " DROP DUMP "); // показать содеримое отбоя
//  dump(&cadd[0],_cadd," CARD ADD TO LOSER DUMP "); // показать содеримое подброса
    dumpv(&stack[0], 1, " TRUMP "); // показать козырь
    dumps(&chand[0], _chand, " COMPUTER HAND "); // показать содеримое рук ПК
    showtable(); // показать игровой стол
    dumpv(&uhand[0], _uhand, " YOUR HAND "); // показать содержимое рук пользователя
    return;
}


int main(int argc, char **argv) {

    int i, n, a, b, c;

    srand(time(0));

    for (i = 0; i < 36; ++i) // заполняем массивы "пустыми" значениями
    {
        stack[i] = i; // в колоду записываются поочерёдно все карты-числа. После завершнения цикла, колода аккуратно рассортирована от 6 до туза, от червей до пик
        drop[i] = -1;
        uhand[i] = -1;
        chand[i] = -1;
        table[i] = -1;
        cadd[i] = -1;
    }
    _stack = 36; // описываем количество карт в каждом массиве. Всё в колоде пока-что.
    _drop = 0;
    _uhand = 0;
    _chand = 0;
    _table = 0;
    _cadd = 0;
    fav = -1; // козыря нет. Карты от 0 до 35, поэтому хорошо именть наверняка другое значение (отрицательное)
    printf("1. New game\n");
    printf("2. Quit\n");
    char ch = getch(); // начальное меню
    if (ch == '2') return 0; // нажато 2 - выход из игры.
// 	 dumpv(&stack[0],_stack," CLEAR STACK DUMP "); // раскомментировать чтобы посмотреть колоду до перемешивания
    n = rand() % 500 +
        500; // перемешиваем колоду от 500 до 1000 раз. Одно перемешивание подразумевает перестановку двух РАЗНЫХ карт в массиве между собой.
    for (i = 0; i < n; ++i) {
        a = rand() % 36; // случайная карта
        b = rand() % 36; // вторая случайная карта
        if (a ==
            b) // если вышло так, что обе карты оказались одной и той же картой, нужно повторить заново (вероятность порядка (1/36)*(1/36)=0.08%, но тем не менее, ведь это, возможно, плохой генератор случайных чисел)
        {
            i--; // если счётчик не убавить, то будет не от 500 до 1000, а меньше.
            continue;
        }
        c = stack[a]; // меняем карты между собой в три приёма с внешней переменной c.
        stack[a] = stack[b];
        stack[b] = c;
    }
// 	 dumpv(&stack[0],_stack," MIXED STACK DUMP "); //  можно подсмотреть перемешанную колоду

    uhand[0] = stack[35]; // начальная раздача. Так читабельнее. Разумеется, можно сделать почти в два раза короче в цикле.
    chand[0] = stack[34];
    uhand[1] = stack[33];
    chand[1] = stack[32];
    uhand[2] = stack[31];
    chand[2] = stack[30];
    uhand[3] = stack[29];
    chand[3] = stack[28];
    uhand[4] = stack[27];
    chand[4] = stack[26];
    uhand[5] = stack[25];
    chand[5] = stack[24];

    fav = stack[23]; // козырь - следующий.
// можно было бы обойтись и без этого, просто объявив козырем нижнюю карту stack[0]. Но мы честно раздаём и раскладываем карты!
    for (i = 35; i > 0; --i)
        stack[i] = stack[i -
                         1]; // содержимое колоды "приподнимается" на одну позицию, чтобы на дно (i=0) положить карту-козырь.
    stack[0] = fav; // вот мы её и положили на дно колоды
    _stack -= 12; // в колоде минус 12 карт
    _uhand += 6; // в руках пользователя - больше на шесть
    _chand += 6; // в руках компьютера - тоже больше на шесть
    draw(); // рисуем картинку с картами и столом

    whonext = -1; // переменная, в которой будем хранить информацию о следующем ходящем: 0 - пользователь, 1 - компьютер

    int minc = 10; // ищем саму мальнькую карту-козырь для определения первого ходящего
    for (i = 0; i < _chand; ++i) {
        if (chand[i] / 9 != fav / 9) continue;
        if (chand[i] % 9 < minc) minc = chand[i] % 9;
    }
    int minu = 10;
    for (i = 0; i < _uhand; ++i) {
        if (uhand[i] / 9 != fav / 9) continue;
        if (uhand[i] % 9 < minu) minu = uhand[i] % 9;
    }
    if ((minu == 10) && (minc == 10)) {
        printf("\nSorry, we have no trumps..."); // ни одного козыря на руках нет. Будем считать, что колода плохо тасована и прерываем игру.
        return 2;
    }
    whonext = (minu < minc) ? 0 : 1; // определяем следующего ходящего
    if (whonext == 0) printf("\nUser first!!!\n");
    if (whonext == 1) printf("\nComputer first!!!\n");
    printf("press any key");
    getch();
    while (1 == 1) // игра в бесконечном цикле
    {
        char rank[50], suit[50];
        if ((_stack == 0) && ((_uhand == 0) || (_chand == 0)))
            break; // игра закончена если в колоде нет карт и при этом их нет на руках по крайней мере у одного игрока.

        draw();
        if (whonext == 0) // ходит человек
        {
            n = 0;
            printf("\n\n(%s|%s|%s|%s|%s|%s|%s|%s|%s) ", RANK_6_LONG, RANK_7_LONG, RANK_8_LONG, RANK_9_LONG,
                   RANK_10_LONG, RANK_J_LONG, RANK_Q_LONG, RANK_K_LONG, RANK_A_LONG);
            printf("(%s|%s|%s|%s)\n\tor\t", SUIT_HEARTS, SUIT_DIAMONDS, SUIT_CLUBS, SUIT_SPADES);
            printf("(%s|%s|%s|%s|%s|%s|%s|%s|%s) ", RANK_6_SHORT, RANK_7_SHORT, RANK_8_SHORT, RANK_9_SHORT,
                   RANK_10_SHORT, RANK_J_SHORT, RANK_Q_SHORT, RANK_K_SHORT, RANK_A_SHORT);
            printf("(%s|%s|%s|%s)", SUIT_HEARTS_SHORT, SUIT_DIAMONDS_SHORT, SUIT_CLUBS_SHORT, SUIT_SPADES_SHORT);
            printf("\nEnter card to go (or enter \"i take\" or \"end turn\"""): ");
            scanf("%40s%40s", &rank[0], &suit[0]); // читаем два слова с клавиатуры
            if ((strcmp(rank, "end") == 0) && (strcmp(suit, "turn") == 0)) // если набрано end turn - это конец хода
            {
                if (_table == 0) // нельзя объявлять конец хода при пустом столе
                {
                    printf("\nNeed to make a move!!!\n\n");
                    sleep(3);
                    continue;
                }
                for (i = 0; i < _table; ++i) // карты со стола полетели в отбой
                {
                    drop[_drop] = table[i];
                    _drop++;
                }
                _table = 0; // на столе карт нет (вернее, они есть, т.е. в массиве есть их значения, но программа никогда не читает этот массив выше индекса [_table])
                while (1 == 1) // добираем в руки карты из колоды по очереди, если они там есть.
                {
                    if ((_uhand < 6) && (_stack > 0)) // пользователю берём карту
                    {
                        uhand[_uhand] = stack[_stack - 1];
                        _stack--;
                        _uhand++;
                    }
                    if ((_chand < 6) && (_stack > 0))  // компьютеру берём карту
                    {
                        chand[_chand] = stack[_stack - 1];
                        _stack--;
                        _chand++;
                    }
                    if (_stack == 0) break;
                    if ((_uhand >= 6) && (_chand >= 6)) break;
                }
                whonext = (whonext == 1) ? 0 : 1; // ходит следующий!
                continue;
            }
            if ((strcmp(rank, "i") == 0) &&
                (strcmp(suit, "take") == 0)) // пользователь решил взять карты на столе (набрал "i take")
            {
                for (i = 0; i < _chand; ++i) // ПК перебирает свои карты в поисках того, что можно подбросить вдогонку
                {
                    a = -1;
                    b = -1;
                    for (n = 0; n < _table; ++n) {
                        if (chand[i] % 9 != table[n] % 9)
                            continue; // ПК нельзя подкидывать карты с достоинствами, которых нет на столе
                        if (chand[i] / 9 == fav / 9) continue; // козыри тоже не подбрасываем
                        a = 1;
                        b = i;
                    }
                    if (a == 1) // карта для подбрасывания нашлась
                    {
                        table[_table] = chand[b];
                        _table++;
                        for (n = b; n <= _chand; ++n) {
                            chand[n] = chand[n +
                                             1]; // карты в руке перемещаются на 1 позицию "вниз", заполнив "пустое" место uhand[a]
                        }
                        _chand--; // в руке на карту меньше
                        i--; // ввиду смещения элементов массива chand[] "вниз", следующий элемент находится уже в позиции [i], а если я не уменьшу i, то я его пропущу!
                    }
                }
                for (i = 0; i < _table; ++i) // перекладываем со стола пользователю
                {
                    uhand[_uhand] = table[i];
                    _uhand++;
                }
                _table = 0;
                while (1 == 1) // досдаём колоду - сначало ПК
                {
                    if ((_chand < 6) && (_stack > 0)) {
                        chand[_chand] = stack[_stack - 1];
                        _stack--;
                        _chand++;
                    }
                    if ((_uhand < 6) && (_stack > 0)) {
                        uhand[_uhand] = stack[_stack - 1];
                        _stack--;
                        _uhand++;
                    }
                    if (_stack == 0) break;
                    if ((_uhand >= 6) && (_chand >= 6)) break;
                }
                whonext = (whonext == 1) ? 0 : 1;
                continue;
            }
            n = decode(rank, suit);
            if (n == -1) {
                printf("\nI don't understand you!!!\n");
                sleep(3);
                continue;
            }
            if ((_table > 0) &&
                (_table % 2 == 0)) // если карты лежат на столе, нельзя ходить с карты прочего достоинства
            {
                a = 0;
                for (i = 0; i < _table; ++i) {
                    if (table[i] % 9 == n % 9) a = 1;
                }
                if (a == 0) // на столе нет карты того же достоинства - ходить так нельзя!!!
                {
                    printf("\nYou can't do this move!!! Cart with this rank is not on table!!!\n");
                    sleep(3);
                    continue;
                }
            }
            a = -1;
            for (i = 0; i < _uhand; ++i) {
                if (uhand[i] == n) a = i;
            }
            if (a == -1) {
                printf("\nYou don't have this card :)!!!\n");
                sleep(3);
                continue;
            }
            if (_table % 2 == 1) // пользователь отбивается - его ход и карт на столе нечётное количество
            {
                if (table[_table - 1] / 9 == uhand[a] / 9) // бьётся одинаковой мастью
                {
                    if (table[_table - 1] % 9 <
                        uhand[a] % 9) { // карта той же масти у пользователя подходит для отбивания
                        table[_table] = uhand[a]; // кладём карту на стол
                        _table++; // на столе +1 карта
                        for (i = a; i <= _uhand; ++i) {
                            uhand[i] = uhand[i +
                                             1]; // карты в руке перемещаются на 1 позицию "вниз", заполнив "пустое" место uhand[a]
                        }
                        _uhand--; // в руке на карту меньше
                        whonext = (whonext == 1) ? 0 : 1;
                        continue;
                    } else { // карта той же мастью но младше - отбиваться нельзя!
                        printf("\nYou can't beat by this card!!!\n'");
                        sleep(3);
                        continue;
                    }
                } else // пользователь отбивается другой мастью, чем лежит на столе
                {
                    if (uhand[a] / 9 != fav / 9) {
                        printf("\nYou can't beat by this card, only the same suit or trump suit!!!\n'");
                        sleep(3);
                        continue;
                    }
// сюда можно попасть только с козырной картой большего достоинства.
                    table[_table] = uhand[a]; // кладём карту на стол
                    _table++; // на столе +1 карта
                    for (i = a; i <= _uhand; ++i) {
                        uhand[i] = uhand[i +
                                         1]; // карты в руке перемещаются на 1 позицию "вниз", заполнив "пустое" место uhand[a]
                    }
                    _uhand--; // в руке на карту меньше
                    whonext = (whonext == 1) ? 0 : 1;
                    continue;

                }
            }
            table[_table] = uhand[a]; // кладём карту на стол
            _table++; // на столе +1 карта
            for (i = a; i <= _uhand; ++i) {
                uhand[i] = uhand[i +
                                 1]; // карты в руке перемещаются на 1 позицию "вниз", заполнив "пустое" место uhand[a]
            }
            _uhand--; // в руке на карту меньше
            whonext = (whonext == 1) ? 0 : 1;
            continue;
        } else if (whonext == 1) // ходит ПК
        {
            if (_table > 0) // на столе есть карты!
            {
                if (_table % 2 == 1) // если на столе нечётное число карт то я отбиваюсь...
                {
                    a = -1;
                    b = 10;
                    for (i = 0; i <
                                _chand; ++i) // ищем той же масти, что последняя карта на столе. Верхняя (небитая "верхняя" карта на столе - table[_table-1])
                    {
                        if (chand[i] / 9 == table[_table - 1] / 9) // моя карта той же масти, что и на столе
                        {
                            if (chand[i] % 9 < table[_table - 1] % 9)
                                continue; // эта карта младше той, что на столе, поехали смотреть следующую
                            if ((chand[i] % 9 < b) && (chand[i] % 9 > table[_table - 1] %
                                                                      9)) // мы нашли карту старше, чем на столе, но в цикле найдём из них самую маленькую.
                            {
                                b = chand[i] % 9;
                                a = i;
                            }
                        }
                    }
                    if (a == -1) // карты той же масти не нашли, давайте искать козыри
                    {
                        b = 10;
                        for (i = 0; i <
                                    _chand; ++i) // ищем козыри. Верхняя (небитая "верхняя" карта на столе - table[_table-1])
                        {
                            if (chand[i] / 9 != fav / 9) continue; // эта карта не козырь
//			 	 	   	 	 	if ((chand[i]%9 < b)&&(chand[i]%9 > table[_table-1]%9)) // мы нашли карту-козырь, но в цикле найдём из них самую маленькую.
                            if (chand[i] % 9 < b) // мы нашли карту-козырь, но в цикле найдём из них самую маленькую.
                            {
                                if (chand[i] / 9 == table[_table - 1] / 9)
                                    if (chand[i] % 9 < table[_table - 1] % 9)
                                        continue; // если на столе козырь его нужно отбивать как минимум, бОльшим козырем, а не меньшим.
                                b = chand[i] % 9;
                                a = i;
                            }
                        }
                    }
                    if (a != -1) // есть чем отбиться
                    {
                        table[_table] = chand[a]; // кладём карту на стол
                        _table++; // на столе +1 карта
                        for (i = a; i <= _chand; ++i) {
                            chand[i] = chand[i +
                                             1]; // карты в руке перемещаются на 1 позицию "вниз", заполнив "пустое" место uhand[a]
                        }
                        _chand--; // в руке на карту меньше
                        whonext = (whonext == 1) ? 0 : 1;
                        continue;
                    } else {
                        for (i = _uhand; i >=
                                         0; i--) // отдавать карты ПК будем в цикле. Цикл сможет продолжиться столько раз, сколько сейчас карт в руке пользователя.
                        {
                            draw();
                            printf("BERU!!!");
                            printf("\nEnter card to drop (\"end drop\" to end dropping): ");
                            scanf("%40s%40s", &rank[0], &suit[0]);
                            n = decode(rank, suit);
                            if ((strcmp(rank, "end") == 0) &&
                                (strcmp(suit, "drop") == 0)) // end drop, т.е. больше чем подкинули не подкидываем
                            {
                                for (i = 0; i < _cadd; ++i) // перекиываем компьютеру в руку из массива для подкидывания
                                {
                                    chand[_chand] = cadd[i];
                                    _chand++;
                                }
                                for (i = 0; i < 36; i++)
                                    cadd[i] = -1; // на всякий случай почистим массив для подкидвания
                                _cadd = 0; // и обнулим число элементов в нём
                                for (i = 0; i < _table; ++i) // содержимое стола тоже сбрасываем в руку компьютера
                                {
                                    chand[_chand] = table[i];
                                    _chand++;
                                }
                                _table = 0; // карт на "столе" как бы больше нет (=0)
                                while (1 == 1) // сдаём карты из колоды, если кому надо
                                {
                                    if ((_uhand < 6) && (_stack > 0)) {
                                        uhand[_uhand] = stack[_stack - 1];
                                        _stack--;
                                        _uhand++;
                                    }
                                    if ((_chand < 6) && (_stack > 0)) {
                                        chand[_chand] = stack[_stack - 1];
                                        _stack--;
                                        _chand++;
                                    }
                                    if (_stack == 0) break;
                                    if ((_uhand >= 6) && (_chand >= 6)) break;
                                }
                                whonext = (whonext == 1) ? 0 : 1;
                                break; // прерываем цикл подкидывания компьютеру
                            }
                            if (n ==
                                -1) // decode() вернуло -1. Это означает что мы ничего не смогли разобрать в вводе пользователя.
                            {
                                printf("\nI don't understand you!!!\n");
                                sleep(3);
                                i++;
                                continue;
                            }
                            a = -1;
                            for (i = 0; i < _uhand; ++i) {
                                if (uhand[i] == n) a = i;
                            }
                            if (a == -1) {
                                printf("\nYou don't have this card :)!!!\n");
                                sleep(3);
                                i++;
                                continue;
                            }
                            cadd[_cadd] = uhand[a];
                            _cadd++;
                            for (i = a; i <= _uhand; ++i) {
                                uhand[i] = uhand[i +
                                                 1]; // карты в руке перемещаются на 1 позицию "вниз", заполнив "пустое" место uhand[a]
                            }
                            _uhand--; // в руке на карту меньше
                        }
                    }

                } else // а если на столе чётное число карт - то я, компьютер, подкидываю (если есть что подкидывать...)
                {
                    n = -1;
                    b = 10;
                    for (i = 0; i < _chand; ++i) {
                        if (chand[i] / 9 == fav / 9) continue; // козыри сначала не подбрасываем
                        if (chand[i] % 9 < b) // возможно, мы нашли нужную карту
                        {
                            a = 0;
                            for (c = 0; c < _table; ++c) {
                                if (table[c] % 9 == chand[i] % 9) {
                                    a = 1;
                                    break;
                                }
                            }
                            if (a == 1) { // ПК нашёл некозырную карту для подброса
                                b = chand[i] % 9;
                                n = i;
                            }
                        }
                    }
                    if (n == -1) // нечего подбросить. Не нашёл ПК у себя достойной карты
                    {
                        for (i = 0; i < _table; ++i) {
                            drop[_drop] = table[i];
                            _drop++;
                        }
                        _table = 0;
                        while (1 == 1) {
                            if ((_uhand < 6) && (_stack > 0)) {
                                uhand[_uhand] = stack[_stack - 1];
                                _stack--;
                                _uhand++;
                            }
                            if ((_chand < 6) && (_stack > 0)) {
                                chand[_chand] = stack[_stack - 1];
                                _stack--;
                                _chand++;
                            }
                            if (_stack == 0) break;
                            if ((_uhand >= 6) && (_chand >= 6)) break;
                        }
                        whonext = (whonext == 1) ? 0 : 1;
                        continue;
                    }
                    table[_table] = chand[n];
                    _table++;
                    for (i = n; i <= _chand; ++i) {
                        chand[i] = chand[i +
                                         1]; // карты в руке перемещаются на 1 позицию "вниз", заполнив "пустое" место uhand[a]
                    }
                    _chand--; // в руке на карту меньше
                    sleep(5);
                    whonext = (whonext == 1) ? 0 : 1;
                }
            } else // стол пуст и ПК нужно ходить первому!!!
            {
                if (_table % 2 == 0) // если на столе чётное количество карт то ходит ПК...
                {
                    n = -1;
                    b = 10;
                    for (i = 0; i < _chand; ++i) {
                        if (chand[i] / 9 == fav / 9) continue; // пока козырей не берём для ходы
                        if (chand[i] % 9 < b) {
                            b = chand[i] % 9;
                            n = i;
                        }
                    }
                    if (n == -1) // у меня одни козыри, нужно выбрать поменьше
                    {
                        b = 10;
                        for (i = 0; i < _chand; ++i) {
                            if (chand[i] / 9 != fav / 9) continue;
                            if (chand[i] % 9 < b) {
                                b = chand[i] % 9;
                                n = i;
                            }
                        }
                    }
                    table[_table] = chand[n];
                    _table++;
                    for (i = n; i <= _chand; ++i) {
                        chand[i] = chand[i +
                                         1]; // карты в руке перемещаются на 1 позицию "вниз", заполнив "пустое" место uhand[a]
                    }
                    _chand--; // в руке на карту меньше
                    whonext = (whonext == 1) ? 0 : 1;
                } else {
                    printf("Table is empty and odd count carts on it in the same time... Say hello to Schrodinger! \n");
                    getch();
                }
            }

        }
    }
    draw();
    if (_chand == 0) printf("\n\nComputer WIN!!!\n");
    if (_uhand == 0) printf("\n\nUser WIN!!!\n");
    return 0;
}
