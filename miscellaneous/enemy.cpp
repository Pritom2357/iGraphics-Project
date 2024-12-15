#include "enemy.h"

Enemy enemies[MAX_ENEMY];

void initializeEnemy() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        enemies[i].isActive = false;
    }
}

void spawnEnemy(int centerX, int centerY) {
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (!enemies[i].isActive) {
            double angle = (rand() % 360) * (PI / 180.0);
            enemies[i].posX = centerX + SPAWN_RADIUS * cos(angle);
            enemies[i].posY = centerY + SPAWN_RADIUS * sin(angle);
            enemies[i].isActive = true;
            enemies[i].angle = angle;
            break;
        }
    }
}

void moveEnemies(int winWidth, int winHeight) {
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (enemies[i].isActive) {
            enemies[i].posX -= enemySpeed * cos(enemies[i].angle);
            enemies[i].posY -= enemySpeed * sin(enemies[i].angle);

            if (enemies[i].posX <= 0 || enemies[i].posX >= winWidth || enemies[i].posY <= 0 || enemies[i].posY >= winHeight) {
                enemies[i].isActive = false;
            }
        }
    }
}

void drawEnemies() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (enemies[i].isActive) {
            iSetColor(255, 125, 0);
            iFilledRectangle(enemies[i].posX, enemies[i].posY, 50, 50);
        }
    }
}

