#ifndef ENEMY_H
#define ENEMY_H

#include "iGraphics.h"
#include <math.h>

#define MAX_ENEMY 10
#define PI 3.14159265358979323846
#define SPAWN_RADIUS 300
#define enemySpeed 5

struct Enemy {
    int posX, posY;
    bool isActive;
    double angle;
};

extern Enemy enemies[MAX_ENEMY];

void initializeEnemy();
void spawnEnemy(int centerX, int centerY);
void moveEnemies(int winWidth, int winHeight);
void drawEnemies();
void iSpecialKeyboard(unsigned char key);
void iMouseMove(int x, int y);


#endif
