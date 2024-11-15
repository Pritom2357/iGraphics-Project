#include "iGraphics.h"
#include "stdlib.h"
#include "time.h"
#include <math.h>
#include <windows.h>

int winWidth = 800;
int winHeight = 600;

#define MAX_ENEMY 10
#define PI 3.14159265358979323846
#define SPAWN_RADIUS 300
#define enemySpeed 5

#define MAX_BULLET 100

double bulletWidth = 5;   
double bulletLen = 30;

struct Projectile {
    double x, y;
    double vx, vy;
    double angle;
};

struct Enemy {
    int posX, posY;
    int eWdt, eHt;
    int dirX, dirY;
    bool isActive;
    double rotation;
    double angle;
};



Projectile projectiles[MAX_BULLET];
Enemy enemies[MAX_ENEMY];
int projectileCount = 0;

double rocketX = 300;
double rocketY = 100;
int rocketSpeed = 10;

int mouseX, mouseY;
int size = 15;

double rX[] = {rocketX, rocketX - 10, rocketX + 10};
double rY[] = {rocketY, rocketY - 20, rocketY - 20};
int centerX = rX[0], centerY = rY[0];

bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;

void playBackgroundSound(){
    PlaySound(TEXT("space_sound.wav"), NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
}

DWORD WINAPI playShootSoundThread(LPVOID lpParam) {
    PlaySound(TEXT("laser_gun.wav"), NULL, SND_ASYNC | SND_FILENAME);
    return 0;
}

void playShootSound() {
    CreateThread(NULL, 0, playShootSoundThread, NULL, 0, NULL);
}


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
            enemies[i].posX = centerX + SPAWN_RADIUS * cos(angle);
            enemies[i].posY = centerY + SPAWN_RADIUS * sin(angle);
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

double angle;
void updateRocketCoordinates() {
    rX[0] = rocketX;
    rX[1] = rocketX - 10;
    rX[2] = rocketX + 10;

    rY[0] = rocketY;
    rY[1] = rocketY - 20;
    rY[2] = rocketY - 20;

    centerX = rX[0];
    centerY = rY[0];
}

void moveRocket() {
    if (moveUp && rocketY + rocketSpeed + 20 <= winHeight) {
        rocketY += rocketSpeed;
        moveUp = false;
    }
    if (moveDown && rocketY - rocketSpeed >= 20) {
        rocketY -= rocketSpeed;
        moveDown = false;
    }
    if (moveLeft && rocketX - rocketSpeed >= 10) {
        rocketX -= rocketSpeed;
        moveLeft = false;
    }
    if (moveRight && rocketX + rocketSpeed + 10 <= winWidth) {
        rocketX += rocketSpeed;
        moveRight = false;
    }
}

void moveProjectiles() {
    for (int i = 0; i < projectileCount; i++) {
        projectiles[i].x += projectiles[i].vx;
        projectiles[i].y += projectiles[i].vy;

        if (projectiles[i].x < 0 || projectiles[i].x > winWidth || projectiles[i].y < 0 || projectiles[i].y > winHeight) {
            projectiles[i] = projectiles[projectileCount - 1];
            projectileCount--;  
            i--;
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
            if (enemies[i].posX + enemies[i].eWdt >= winWidth || enemies[i].posX <= 0) {
                enemies[i].isActive = false;
            }
            if (enemies[i].posY + enemies[i].eHt >= winHeight || enemies[i].posY <= 0) {
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

void checkCollisions() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (enemies[i].isActive) {
            for (int j = 0; j < projectileCount; j++) {
                double dist = sqrt(pow(enemies[i].posX - projectiles[j].x, 2) + 
                                   pow(enemies[i].posY - projectiles[j].y, 2));
                if (dist < enemies[i].eWdt / 2) {
                    enemies[i].isActive = false;
                    projectiles[j] = projectiles[projectileCount - 1];
                    projectileCount--;
                }
            }
        }
    }
}

void checkGameOver() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (enemies[i].isActive) {
            double dist = sqrt(pow(enemies[i].posX - rocketX, 2) + 
                               pow(enemies[i].posY - rocketY, 2));
            if (dist < enemies[i].eWdt / 2) {
                printf("Game Over!\n");
                exit(0); 
            }
        }
    }
}



void iDraw() {
    iClear();

    angle = atan2(mouseY - rocketY, mouseX - rocketX);

    double headX = rocketX + size * cos(angle);
    double headY = rocketY + size * sin(angle);
    double leftX = rocketX + size * cos(angle + 2.0944);
    double leftY = rocketY + size * sin(angle + 2.0944);
    double rightX = rocketX + size * cos(angle - 2.0944);
    double rightY = rocketY + size * sin(angle - 2.0944);

    rX[0] = headX;
    rY[0] = headY;
    rX[1] = leftX;
    rY[1] = leftY;
    rX[2] = rightX;
    rY[2] = rightY;

    iSetColor(255, 0, 0);
    iFilledPolygon(rX, rY, 3); 
    centerX = rX[0];
    centerY = rY[0];

    for (int i = 0; i < projectileCount; i++) {
        iSetColor(0, 0, 255);

        double bX[] = {projectiles[i].x + bulletWidth / 2.0, 
                       projectiles[i].x + bulletLen-bulletWidth/2.0, 
                       projectiles[i].x + bulletLen - bulletWidth / 2.0, 
                       projectiles[i].x + bulletWidth / 2.0};

        double bY[] = {projectiles[i].y - bulletWidth / 2.0, 
                       projectiles[i].y - bulletWidth / 2.0, 
                       projectiles[i].y + bulletWidth / 2.0, 
                       projectiles[i].y + bulletWidth / 2.0};

        for (int j = 0; j < 4; j++) {
            double tempX = bX[j] - projectiles[i].x; 
            double tempY = bY[j] - projectiles[i].y;

            bX[j] = projectiles[i].x + tempX * cos(projectiles[i].angle) - tempY * sin(projectiles[i].angle);
            bY[j] = projectiles[i].y + tempX * sin(projectiles[i].angle) + tempY * cos(projectiles[i].angle);
        }

        iFilledPolygon(bX, bY, 4);
    }

    moveRocket();
    moveProjectiles();

    char mousePos[100];
    sprintf(mousePos, "X position: %d and Y position: %d", mouseX, mouseY);
    iText(10, winHeight - 20, mousePos, GLUT_BITMAP_HELVETICA_18);

    drawEnemies();
    checkCollisions();
    checkGameOver();
    iCircle(centerX, centerY, SPAWN_RADIUS);
}

void iMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

        // playShootSound();aaaaaaaa
        // playBackgroundSound();
        angle = atan2(y - rY[0], x - rX[0]);

        if (projectileCount < 100) {
            projectiles[projectileCount].x = rX[0];
            projectiles[projectileCount].y = rY[0];
            projectiles[projectileCount].vx = 0.3 * cos(angle);
            projectiles[projectileCount].vy = 0.3 * sin(angle);
            projectiles[projectileCount].angle = angle;
            projectileCount++;
        }
    }
}

void iMouseMove(int x, int y) {
    mouseX = x;
    mouseY = y;
}

void iKeyboard(unsigned char key) {
    if (key == 'w') moveUp = true;
    if (key == 's') moveDown = true;
    if (key == 'a') moveLeft = true;
    if (key == 'd') moveRight = true;
}

void iSpecialKeyboard(unsigned char key) {
    // This function is called for special key inputs like arrow keys
}

int main() {
    srand(time(0));
    playBackgroundSound();
    initializeEnemy();
    iSetTimer(500, spawnEnemy);
    iSetTimer(20, moveEnemies);
    iInitialize(winWidth, winHeight, "Space Shooter - Rocket Movement");
    iSetTimer(20, moveProjectiles); 
    return 0;
}
