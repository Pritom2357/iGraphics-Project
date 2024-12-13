#include "iGraphics.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

// Window size
int winWidth = 800;
int winHeight = 600;

// Player structure
typedef struct {
    double x, y;        // Position
    double angle;       // Rotation angle
    int speed;          // Movement speed
    int level;          // Player level
    int health;         // Player health
    int shapeType;      // Shape type: 0=Triangle, 1=Rocket
    int score;          // Player score
} Player;

// Bullet structure
typedef struct {
    double x, y;        // Position
    double vx, vy;      // Velocity
    bool active;        // Active flag
} Bullet;

// Enemy structure
typedef struct {
    double x, y;        // Position
    double vx, vy;      // Velocity
    int health;         // Enemy health
    int type;           // Enemy type
    bool active;        // Active flag
} Enemy;

// Constants
#define MAX_BULLETS 50
#define MAX_ENEMIES 20
#define PI 3.14159265358979323846

// Globals
Player player;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;
int mouseX, mouseY;

// Function Prototypes
void initGame();
void spawnEnemy();
void movePlayer();
void moveBullets();
void moveEnemies();
void checkCollisions();
void drawPlayer();
void drawBullets();
void drawEnemies();
void drawScore();
void checkLevelUp();
bool isPointInCircle(double px, double py, double cx, double cy, double radius);

// Initialize the game
void initGame() {
    player.x = winWidth / 2;
    player.y = winHeight / 2;
    player.angle = 0;
    player.speed = 5;
    player.level = 1;
    player.health = 3;
    player.shapeType = 0;
    player.score = 0;

    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
}

// Spawn an enemy from the screen edges
void spawnEnemy() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            int side = rand() % 4;
            if (side == 0) { // Top
                enemies[i].x = rand() % winWidth;
                enemies[i].y = winHeight;
            } else if (side == 1) { // Bottom
                enemies[i].x = rand() % winWidth;
                enemies[i].y = 0;
            } else if (side == 2) { // Left
                enemies[i].x = 0;
                enemies[i].y = rand() % winHeight;
            } else { // Right
                enemies[i].x = winWidth;
                enemies[i].y = rand() % winHeight;
            }

            double angle = atan2(player.y - enemies[i].y, player.x - enemies[i].x);
            enemies[i].vx = 0.02 * cos(angle);
            enemies[i].vy = 0.02 * sin(angle);
            enemies[i].health = 1;
            enemies[i].type = 0;
            enemies[i].active = true;
            break;
        }
    }
}

// Move the player
void movePlayer() {
    if (moveUp && player.y + player.speed < winHeight)
    {
        player.y += player.speed;
        moveUp=false;
    } 
    if (moveDown && player.y - player.speed > 0){
        player.y -= player.speed;
        moveDown=false;
    } 
    if (moveLeft && player.x - player.speed > 0){
        player.x -= player.speed;
        moveLeft=false;
    } 
    if (moveRight && player.x + player.speed < winWidth){
        player.x += player.speed;
        moveRight=false;
    } 
}

// Move bullets
void moveBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].x += bullets[i].vx;
            bullets[i].y += bullets[i].vy;

            if (bullets[i].x < 0 || bullets[i].x > winWidth || bullets[i].y < 0 || bullets[i].y > winHeight) {
                bullets[i].active = false;
            }
        }
    }
}

// Move enemies
void moveEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].x += enemies[i].vx;
            enemies[i].y += enemies[i].vy;

            if (enemies[i].x < 0 || enemies[i].x > winWidth || enemies[i].y < 0 || enemies[i].y > winHeight) {
                enemies[i].active = false;
            }
        }
    }
}

// Check collisions
void checkCollisions() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].active) {
                    if (isPointInCircle(bullets[i].x, bullets[i].y, enemies[j].x, enemies[j].y, 15)) {
                        bullets[i].active = false;
                        enemies[j].health--;
                        if (enemies[j].health <= 0) {
                            enemies[j].active = false;
                            player.score += 10;
                            checkLevelUp();
                        }
                        break;
                    }
                }
            }
        }
    }
}

// Check if point is in a circle
bool isPointInCircle(double px, double py, double cx, double cy, double radius) {
    double dx = px - cx;
    double dy = py - cy;
    return (dx * dx + dy * dy <= radius * radius);
}

// Check if the player levels up
void checkLevelUp() {
    if (player.score >= 50 && player.level == 1) {
        player.level = 2;
        player.shapeType = 1; // Rocket
        player.speed += 1;   // Increase speed
    }
}

// Draw the player
void drawPlayer() {
    if (player.shapeType == 0) { 
        double size = 20;
        double x1 = player.x + size * cos(player.angle);
        double y1 = player.y + size * sin(player.angle);

        double x2 = player.x + size * cos(player.angle + PI / 2);
        double y2 = player.y + size * sin(player.angle + PI / 2);

        double x3 = player.x + size * cos(player.angle - PI / 2);
        double y3 = player.y + size * sin(player.angle - PI / 2);

        double xCoords[3] = {x1, x2, x3};
        double yCoords[3] = {y1, y2, y3};

        iSetColor(0, 255, 0);
        iFilledPolygon(xCoords, yCoords, 3);
    } else if (player.shapeType == 1) { // Rocket
        iSetColor(0, 0, 255);
        iFilledRectangle(player.x - 10, player.y - 10, 20, 40);
    }
}

// Draw bullets
void drawBullets() {
    iSetColor(255, 255, 0);
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            iFilledCircle(bullets[i].x, bullets[i].y, 3);
        }
    }
}

// Draw enemies
void drawEnemies() {
    iSetColor(255, 0, 0);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            iFilledCircle(enemies[i].x, enemies[i].y, 15);
        }
    }
}

// Draw score
void drawScore() {
    char scoreStr[20];
    sprintf(scoreStr, "Score: %d", player.score);
    iSetColor(255, 255, 255);
    iText(10, winHeight - 20, scoreStr, GLUT_BITMAP_HELVETICA_18);
}

// iDraw function
void iDraw() {
    iClear();
    movePlayer();
    moveBullets();
    moveEnemies();
    checkCollisions();

    drawPlayer();
    drawBullets();
    drawEnemies();
    drawScore();
}

// Keyboard input
void iKeyboard(unsigned char key) {
    if (key == 'w') moveUp = true;
    if (key == 's') moveDown = true;
    if (key == 'a') moveLeft = true;
    if (key == 'd') moveRight = true;
}

void iKeyboardUp(unsigned char key) {
    if (key == 'w') moveUp = false;
    if (key == 's') moveDown = false;
    if (key == 'a') moveLeft = false;
    if (key == 'd') moveRight = false;
}

// Mouse movement
void iMouseMove(int mx, int my) {
    mouseX = mx;
    mouseY = my;
    player.angle = atan2(mouseY - player.y, mouseX - player.x);
}

// Mouse click
void iMouse(int button, int state, int mx, int my) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].x = player.x;
                bullets[i].y = player.y;
                bullets[i].vx = 1 * cos(player.angle);
                bullets[i].vy = 1 * sin(player.angle);
                bullets[i].active = true;
                break;
            }
        }
    }
}

void iSpecialKeyboard(unsigned char key) {
    // Handle special keys if needed
}

// Main function
int main() {
    srand(time(0));
    initGame();
    iSetTimer(2000, spawnEnemy);
    iInitialize(winWidth, winHeight, "Player Evolution Game");
    return 0;
}
