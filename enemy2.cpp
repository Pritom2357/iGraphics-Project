#include "iGraphics.h"
#include "stdlib.h"
#include "time.h"
#include <math.h>

#define MAX_ENEMY 10
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PI 3.14159265358979323846
#define SPAWN_RADIUS 300
#define enemySpeed 5

struct Enemy {
    int posX, posY;
    int eWdt, eHt;
    int dirX, dirY;
    bool isActive;
    double rotation;
    double angle;
};

int mouseX = SCREEN_WIDTH / 2, mouseY = SCREEN_HEIGHT / 2;

Enemy enemies[MAX_ENEMY];

void initializeEnemy() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        enemies[i].isActive = false;
        enemies[i].rotation = 0.0;
    }
}

void spawnEnemy() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (!enemies[i].isActive) {
            double angle = (rand() % 360) * (PI / 180.0);
            enemies[i].posX = mouseX + SPAWN_RADIUS * cos(angle);
            enemies[i].posY = mouseY + SPAWN_RADIUS * sin(angle);
            enemies[i].eWdt = 50;
            enemies[i].eHt = 50;
            enemies[i].dirX = (rand() % 2 == 0) ? 1 : -1;
            enemies[i].dirY = (rand() % 2 == 0) ? 1 : -1;
            enemies[i].rotation = 0.0;
            enemies[i].isActive = true;
            enemies[i].angle=angle;
            break;
        }
    }
}

void moveEnemies() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (enemies[i].isActive) {
            enemies[i].posX -= enemySpeed* cos(enemies[i].angle);
            enemies[i].posY -= enemySpeed* sin(enemies[i].angle);
            enemies[i].rotation += 5.0;
            if (enemies[i].rotation >= 360.0) {
                enemies[i].rotation -= 360.0;
            }
            if (enemies[i].posX + enemies[i].eWdt >= SCREEN_WIDTH || enemies[i].posX <= 0) {
                enemies[i].isActive = false;
            }
            if (enemies[i].posY + enemies[i].eHt >= SCREEN_HEIGHT || enemies[i].posY <= 0) {
                enemies[i].isActive = false;
            }
        }
    }
}

void drawEnemies() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (enemies[i].isActive) {
            double angleRad = enemies[i].rotation * (PI / 180.0);
            double hWdt = enemies[i].eWdt / 2.0;
            double hHt = enemies[i].eHt / 2.0;
            double cornersX[4] = { -hWdt, hWdt, hWdt, -hWdt };
            double cornersY[4] = { -hHt, -hHt, hHt, hHt };
            double transformedX[4], transformedY[4];
            for (int j = 0; j < 4; j++) {
                transformedX[j] = enemies[i].posX + cornersX[j] * cos(angleRad) - cornersY[j] * sin(angleRad);
                transformedY[j] = enemies[i].posY + cornersX[j] * sin(angleRad) + cornersY[j] * cos(angleRad);
            }
            iSetColor(255, 125, 0);
            iFilledPolygon(transformedX, transformedY, 4);
        }
    }
}

void iDraw() {
    iClear();
    drawEnemies();
    iCircle(mouseX, mouseY, SPAWN_RADIUS);
}

void iMouseMove(int mx, int my) { }

void iMouse(int button, int state, int mx, int my) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mouseX = mx;
        mouseY = my;
    }
}

void iKeyboard(unsigned char key) { }

void iSpecialKeyboard(unsigned char key) { }

int main() {
    srand(time(0));
    initializeEnemy();
    iSetTimer(1500, spawnEnemy);
    iSetTimer(20, moveEnemies);
    iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Enemy Spawn on Radius");
    return 0;
}
