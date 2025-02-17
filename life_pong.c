#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// Настройки консольного «экрана»
#define SCREEN_W 80
#define SCREEN_H 25

// Параметры ракеток
#define PADDLE_HEIGHT 5

// Параметры мяча
#define BALL_START_SPEED_X 1
#define BALL_START_SPEED_Y 1

// Клеточный автомат (Game of Life)
#define LIFE_ROWS 10
#define LIFE_COLS 20

// Как часто пересчитывать Game of Life (через каждые N кадров)
#define LIFE_UPDATE_INTERVAL 5

// Структуры
typedef struct {
    int x, y;       // координаты мяча
    int vx, vy;     // скорость мяча
} Ball;

typedef struct {
    int y;          // верхняя координата ракетки
} Paddle;

// Клеточный автомат
int lifeGrid[LIFE_ROWS][LIFE_COLS];
int lifeBuffer[LIFE_ROWS][LIFE_COLS];

// Инициализация клеточного автомата
void initLife() {
    for (int r = 0; r < LIFE_ROWS; r++) {
        for (int c = 0; c < LIFE_COLS; c++) {
            // С вероятностью 1/6 клетка становится «живой»
            lifeGrid[r][c] = (rand() % 6 == 0) ? 1 : 0;
        }
    }
}

// Подсчёт соседей по правилам Conway's Game of Life
int countNeighbors(int r, int c) {
    int count = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue; // не считаем саму клетку
            int rr = r + dr;
            int cc = c + dc;
            if (rr >= 0 && rr < LIFE_ROWS && cc >= 0 && cc < LIFE_COLS) {
                count += lifeGrid[rr][cc];
            }
        }
    }
    return count;
}

// Обновление клеточного автомата
void updateLifeAutomaton() {
    for (int r = 0; r < LIFE_ROWS; r++) {
        for (int c = 0; c < LIFE_COLS; c++) {
            int neighbors = countNeighbors(r, c);
            int state = lifeGrid[r][c];
            // Правила Conway's Life
            if (state == 1 && (neighbors < 2 || neighbors > 3)) {
                lifeBuffer[r][c] = 0;   // умирает
            } else if (state == 0 && neighbors == 3) {
                lifeBuffer[r][c] = 1;   // рождается
            } else {
                lifeBuffer[r][c] = state;  // без изменений
            }
        }
    }
    // Перекладываем буфер обратно
    for (int r = 0; r < LIFE_ROWS; r++) {
        for (int c = 0; c < LIFE_COLS; c++) {
            lifeGrid[r][c] = lifeBuffer[r][c];
        }
    }
}

// Инициализация игры
void initGame(Ball *ball, Paddle *left, Paddle *right) {
    // Мяч в центре
    ball->x = SCREEN_W / 2;
    ball->y = SCREEN_H / 2;
    ball->vx = BALL_START_SPEED_X;
    ball->vy = BALL_START_SPEED_Y;

    // Ракетки (по вертикали примерно посередине)
    left->y = (SCREEN_H / 2) - (PADDLE_HEIGHT / 2);
    right->y = (SCREEN_H / 2) - (PADDLE_HEIGHT / 2);

    // Клеточный автомат
    initLife();
}

// Движение мяча и отскоки
void updateBall(Ball *ball, Paddle *left, Paddle *right) {
    // Двигаем мяч
    ball->x += ball->vx;
    ball->y += ball->vy;

    // Отскок сверху/снизу
    if (ball->y < 0) {
        ball->y = 0;
        ball->vy = -ball->vy;
    } else if (ball->y >= SCREEN_H) {
        ball->y = SCREEN_H - 1;
        ball->vy = -ball->vy;
    }

    // Выход за левый/правый край
    // Здесь нет полноценного счёта, просто возвращаем мяч в центр
    if (ball->x < 0) {
        ball->x = SCREEN_W / 2;
        ball->y = SCREEN_H / 2;
    } else if (ball->x >= SCREEN_W) {
        ball->x = SCREEN_W / 2;
        ball->y = SCREEN_H / 2;
    }

    // Координаты ракеток:
    // Левая: x=0, от y=left->y до y=left->y+PADDLE_HEIGHT-1
    // Правая: x=SCREEN_W-1, от y=right->y до y=right->y+PADDLE_HEIGHT-1

    // Столкновение с левой ракеткой
    if (ball->x == 1) {
        if (ball->y >= left->y && ball->y < left->y + PADDLE_HEIGHT) {
            // Отбиваем
            ball->x = 1;
            ball->vx = -ball->vx;
        }
    }
    // Столкновение с правой ракеткой
    if (ball->x == SCREEN_W - 2) {
        if (ball->y >= right->y && ball->y < right->y + PADDLE_HEIGHT) {
            ball->x = SCREEN_W - 2;
            ball->vx = -ball->vx;
        }
    }
}

// Проверяем пересечение мяча с живой клеткой
// Игровое поле Life: [0..LIFE_COLS-1] x [0..LIFE_ROWS-1]
void checkBallLifeCollision(Ball *ball) {
    if (ball->x >= 0 && ball->x < LIFE_COLS &&
        ball->y >= 0 && ball->y < LIFE_ROWS) {
        if (lifeGrid[ball->y][ball->x] == 1) {
            // Увеличиваем скорость
            if (ball->vx > 0) ball->vx++;
            else ball->vx--;
            if (ball->vy > 0) ball->vy++;
            else ball->vy--;
            // lifeGrid[ball->y][ball->x] = 0; // одноразовый эффект
        }
    }
}

// Обработка управляющего символа
void handleInput(char ch, Paddle *left, Paddle *right, bool *running) {
    switch(ch) {
        // Левая ракетка
        case 'w':
            if (left->y > 0) left->y--;
            break;
        case 's':
            if (left->y + PADDLE_HEIGHT < SCREEN_H) left->y++;
            break;

        // Правая ракетка
        case 'i':
            if (right->y > 0) right->y--;
            break;
        case 'k':
            if (right->y + PADDLE_HEIGHT < SCREEN_H) right->y++;
            break;

        // Выход
        case 'q':
            *running = false;
            break;

        default:
            break;
    }
}

// Отрисовка в консоль (ASCII)
void render(const Ball *ball, const Paddle *left, const Paddle *right) {
    // Очистка экрана (ANSI escape- последовательности)
    printf("\033[H\033[J");

    // Создаём буфер символов
    static char screen[SCREEN_H][SCREEN_W + 1];

    // Заполняем пробелами
    for (int r = 0; r < SCREEN_H; r++) {
        for (int c = 0; c < SCREEN_W; c++) {
            screen[r][c] = ' ';
        }
        screen[r][SCREEN_W] = '\0';
    }

    // Рисуем Game of Life (живые клетки обозначим 'o')
    for (int rr = 0; rr < LIFE_ROWS; rr++) {
        for (int cc = 0; cc < LIFE_COLS; cc++) {
            if (lifeGrid[rr][cc] == 1) {
                screen[rr][cc] = 'o';
            }
        }
    }

    // Левая ракетка (x = 0)
    for (int i = 0; i < PADDLE_HEIGHT; i++) {
        int row = left->y + i;
        if (row >= 0 && row < SCREEN_H) {
            screen[row][0] = '#';
        }
    }
    // Правая ракетка (x = SCREEN_W-1)
    for (int i = 0; i < PADDLE_HEIGHT; i++) {
        int row = right->y + i;
        if (row >= 0 && row < SCREEN_H) {
            screen[row][SCREEN_W - 1] = '#';
        }
    }

    // Мяч (обозначим '@')
    if (ball->x >= 0 && ball->x < SCREEN_W &&
        ball->y >= 0 && ball->y < SCREEN_H) {
        screen[ball->y][ball->x] = '@';
    }

    // Выводим построчно
    for (int r = 0; r < SCREEN_H; r++) {
        printf("%s\n", screen[r]);
    }

    // Подсказка
    printf("\n(W/S) - левая ракетка, (I/K) - правая ракетка, (Q) - выход\n");
}

int main() {
    srand((unsigned)time(NULL));

    Ball ball;
    Paddle left, right;
    initGame(&ball, &left, &right);

    bool running = true;
    int frameCount = 0;

    while (running) {
        // Считываем один символ (если есть). 
        // Простой способ: fgets из stdin, один символ + Enter.
        char buffer[8];
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            handleInput(buffer[0], &left, &right, &running);
        }

        // Обновляем мяч
        updateBall(&ball, &left, &right);
        // Проверяем пересечение с живыми клетками
        checkBallLifeCollision(&ball);

        // Раз в несколько кадров обновляем Game of Life
        frameCount++;
        if (frameCount % LIFE_UPDATE_INTERVAL == 0) {
            updateLifeAutomaton();
        }

        // Отрисовываем сцену
        render(&ball, &left, &right);
    }

    printf("Game Over!\n");
    return 0;
}
