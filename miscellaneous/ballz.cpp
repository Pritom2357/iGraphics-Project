#include "iGraphics.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define GRAVITY 0.5  // Gravity acceleration
#define BOUNCE_FACTOR .95// Elastic collision factor (reduced energy loss)
#define BULLET_SPEED 10
#define PLAYER_SPEED 20
#define MAX_BULLETS 10

int winWidth = 800, winHeight = 600;

// Player variables
double playerX = 350, playerY = 50, playerWidth = 100, playerHeight = 20;

// Enemy ball variables
double ballX, ballY, ballRadius = 30, ballVX = 0, ballVY = 0;
int ballHealth = 100;
bool ballActive = true;

// Bullet variables
struct Bullet {
    double x, y;
    bool isActive;
} bullets[MAX_BULLETS];
int bulletCount = 0;

// Player movement
bool moveLeft = false, moveRight = false;

void initializeBall() {
    ballX = rand() % (winWidth - 2 * (int)ballRadius) + ballRadius;
    ballY = winHeight - ballRadius;
    ballVX = 0;
    ballVY = 0; // Starts with no vertical velocity
    ballHealth = 100;
    ballActive = true;
}

void movePlayer() {
    if (moveLeft && playerX > 0) {
        playerX -= PLAYER_SPEED;
        moveLeft = false;
    }
    if (moveRight && playerX + playerWidth < winWidth) {
        playerX += PLAYER_SPEED;
        moveRight = false;
    }
}

void moveBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].isActive) {
            bullets[i].y += BULLET_SPEED;
            if (bullets[i].y > winHeight) {
                bullets[i].isActive = false; // Deactivate if out of screen
            }
        }
    }
}

void moveBall() {
    if (ballActive) {
        ballY += ballVY; // Update vertical position
        ballX += ballVX; // Update horizontal position

        ballVY -= GRAVITY; // Apply gravity

        // Bounce off the platform
        if (ballY - ballRadius <= playerY + playerHeight && 
            ballX >= playerX && ballX <= playerX + playerWidth) {
            ballY = playerY + playerHeight + ballRadius; // Adjust position to avoid sinking
            ballVY = -ballVY * BOUNCE_FACTOR; // Reverse and reduce speed due to bounce
        }

        // Bounce off the walls
        if (ballX - ballRadius <= 0 || ballX + ballRadius >= winWidth) {
            ballVX = -ballVX * BOUNCE_FACTOR;
        }

        // Prevent the ball from going below the platform
        if (ballY - ballRadius <= 0) {
            ballY = ballRadius;
            ballVY = -ballVY * BOUNCE_FACTOR;
        }
    }
}

void checkCollisions() {
    // Check bullet-ball collisions
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].isActive) {
            double dx = bullets[i].x - ballX;
            double dy = bullets[i].y - ballY;
            double distance = sqrt(dx * dx + dy * dy);

            if (distance <= ballRadius) {
                bullets[i].isActive = false; // Deactivate bullet
                ballHealth -= 20;           // Decrease ball health
                if (ballHealth <= 0) {
                    ballActive = false; // Destroy the ball
                    initializeBall();   // Spawn a new ball
                }
            }
        }
    }
}

void iDraw() {
    iClear();

    // Draw the ground
    iSetColor(100, 200, 100);
    iFilledRectangle(0, 0, winWidth, 70);

    // Draw the platform
    iSetColor(50, 50, 150);
    iFilledRectangle(playerX, playerY, playerWidth, playerHeight);

    // Draw the player
    iSetColor(0, 0, 255);
    iFilledRectangle(playerX, playerY, playerWidth, playerHeight);

    // Draw the ball
    if (ballActive) {
        iSetColor(255, 0, 0);
        iFilledCircle(ballX, ballY, ballRadius);

        // Draw ball health
        char healthText[20];
        sprintf(healthText, "Health: %d", ballHealth);
        iSetColor(255, 255, 255);
        iText(ballX - ballRadius, ballY + ballRadius + 10, healthText, GLUT_BITMAP_HELVETICA_18);
    }

    // Draw bullets
    iSetColor(255, 255, 0);
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].isActive) {
            iFilledRectangle(bullets[i].x - 2, bullets[i].y, 4, 10);
        }
    }
}

void iMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Shoot a bullet
        if (bulletCount < MAX_BULLETS) {
            bullets[bulletCount].x = playerX + playerWidth / 2;
            bullets[bulletCount].y = playerY + playerHeight;
            bullets[bulletCount].isActive = true;
            bulletCount = (bulletCount + 1) % MAX_BULLETS; // Loop through bullet array
        }
    }
}

void iKeyboard(unsigned char key) {
    if (key == 'a') moveLeft = true;
    if (key == 'd') moveRight = true;
    if (key == 'r') initializeBall(); // Restart the ball
}

void iKeyboardUp(unsigned char key) {
    if (key == 'a') moveLeft = false;
    if (key == 'd') moveRight = false;
}

void iSpecialKeyboard(unsigned char key) {}

void iSpecialKeyboardUp(unsigned char key) {}
void iMouseMove(int mx, int my) {
    // Do nothing (you can leave this empty if mouse movement is not needed)
}

int main() {
    srand(time(0));
    initializeBall();
    iSetTimer(10, moveBullets);
    iSetTimer(10, moveBall);
    iSetTimer(10, movePlayer);
    iSetTimer(10, checkCollisions);
    iInitialize(winWidth, winHeight, "Ball Bouncing Shooter");
    return 0;
}
