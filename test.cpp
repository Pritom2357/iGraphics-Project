#include "iGraphics.h"
#include "stdlib.h"
#include "time.h"
#include <math.h>
#include <windows.h>
#include <stdbool.h>

int winWidth = 800;
int winHeight = 600;
int speedFactor = 1;

#define PI 3.14159265358979323846
#define MAX_BULLET 100

#define NUM_STARS 100 
#define NUM_LAYERS 3  

#define MAX_RECTANGLES 10

typedef struct
{
    /* data */
};


typedef struct {
    int width;
    int height;
    int *pixels; 
} Image;

typedef struct {
    double x, y;   
    double speed;  
} Star;

Star stars[NUM_LAYERS][NUM_STARS];

void initStars() {
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int i = 0; i < NUM_STARS; i++) {
            stars[layer][i].x = rand() % winWidth;
            stars[layer][i].y = rand() % winHeight;
            stars[layer][i].speed = (layer + 1 + speedFactor) * 0.08; 
        }
    }
}

void moveStars() {
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int i = 0; i < NUM_STARS; i++) {
            stars[layer][i].x -= stars[layer][i].speed;

            if (stars[layer][i].x < 0) {
                stars[layer][i].x = winWidth;
                stars[layer][i].y = rand() % winHeight;
            }
        }
    }
}

void drawStars() {
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int i = 0; i < NUM_STARS; i++) {
            int brightness = 155 + layer * 50; 
            iSetColor(brightness, brightness, brightness);
            iPoint((int)stars[layer][i].x, (int)stars[layer][i].y);
        }
    }
}

double bulletWidth = 5;   
double bulletLen = 30;

double rocketX = 50;
double rocketY = winHeight / 2;
int rocketSpeed = 10;

int mouseX = rocketX, mouseY = rocketY + 100;

double angle;

bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;

struct Projectile {
    double x, y;
    double vx, vy;
    double angle;
};

Projectile projectiles[MAX_BULLET];
int projectileCount = 0;

Image loadBMP(const char *filename) {
    FILE *f = fopen(filename, "rb");
    printf("Opening file: %s\n", filename);
    if (!f) {
        printf("File not found: %s\n", filename);
        exit(1);
    }

    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f);

    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
    int bpp = *(short*)&info[28];
    printf("Width: %d, Height: %d, BPP: %d\n", width, height, bpp);

    int bytesPerPixel = bpp / 8;
    int row_padded = (width * bytesPerPixel + 3) & (~3);
    unsigned char* data = (unsigned char*)malloc(row_padded * height);

    fseek(f, *(int*)&info[10], SEEK_SET);
    fread(data, sizeof(unsigned char), row_padded * height, f);
    fclose(f);

    int* pixels = (int*)malloc(width * height * sizeof(int));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int idx = (height - 1 - i) * row_padded + j * bytesPerPixel;
            unsigned char blue = data[idx];
            unsigned char green = data[idx + 1];
            unsigned char red = data[idx + 2];
            unsigned char alpha = 255;
            if (red == 0 && green == 0 && blue == 0) {
                alpha = 0;
            }

            int pixelIdx = i * width + j;
            pixels[pixelIdx] = (alpha << 24) | (red << 16) | (green << 8) | blue;
        }
    }
    free(data);

    Image img;
    img.width = width;
    img.height = height;
    img.pixels = pixels;
    return img;
}

Image rotateImage(Image src, double angle) {
    int w = src.width;
    int h = src.height;
    double rad = angle;
    double cosA = cos(rad);
    double sinA = sin(rad);

    int newW = (int)(fabs(w * cosA) + fabs(h * sinA));
    int newH = (int)(fabs(w * sinA) + fabs(h * cosA));

    int *newPixels = (int *)malloc(newW * newH * sizeof(int));
    for (int i = 0; i < newW * newH; i++) {
        newPixels[i] = 0; 
    }

    int x0 = w / 2.0;
    int y0 = h / 2.0;
    int newX0 = newW / 2.0;
    int newY0 = newH / 2.0;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int srcIdx = y * w + x;
            int pixel = src.pixels[srcIdx];

            int dx = x - x0;
            int dy = y - y0;

            int newX = (int)(dx * cosA - dy * sinA) + newX0;
            int newY = (int)(dx * sinA + dy * cosA) + newY0;

            if (newX >= 0 && newX < newW && newY >= 0 && newY < newH) {
                int destIdx = newY * newW + newX;
                newPixels[destIdx] = pixel;
            }
        }
    }

    Image rotatedImg;
    rotatedImg.width = newW;
    rotatedImg.height = newH;
    rotatedImg.pixels = newPixels;
    return rotatedImg;
}

Image rocketImage;

void moveRocket() {
    if (moveUp && rocketY + rocketSpeed <= winHeight) {
        rocketY += rocketSpeed;
        moveUp = false;
    }
    if (moveDown && rocketY - rocketSpeed >= 0) {
        rocketY -= rocketSpeed;
        moveDown = false;
    }
    if (moveLeft && rocketX - rocketSpeed >= 0) {
        rocketX -= rocketSpeed;
        moveLeft = false;
    }
    if (moveRight && rocketX + rocketSpeed <= winWidth) {
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

void iDraw() {
    iClear();

    moveStars();
    drawStars();

    angle = atan2(mouseY - rocketY, mouseX - rocketX);

    Image rotatedRocket = rotateImage(rocketImage, angle);

    int drawX = (int)(rocketX - rotatedRocket.width / 2);
    int drawY = (int)(rocketY - rotatedRocket.height / 2);

    for (int y = 0; y < rotatedRocket.height; y++) {
        for (int x = 0; x < rotatedRocket.width; x++) {
            int pixel = rotatedRocket.pixels[y * rotatedRocket.width + x];
            int alpha = (pixel >> 24) & 0xFF;
            int red = (pixel >> 16) & 0xFF;
            int green = (pixel >> 8) & 0xFF;
            int blue = pixel & 0xFF;

            if (alpha > 0) { 
                iSetColor(red, green, blue);
                iPoint(drawX + x, drawY + y);
            }
        }
    }

    for (int i = 0; i < projectileCount; i++) {
        iSetColor(255, 255, 255);

        double bX[] = {projectiles[i].x + bulletWidth / 2.0, 
                       projectiles[i].x + bulletLen - bulletWidth / 2.0, 
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

    free(rotatedRocket.pixels);

    moveRocket();
    moveProjectiles();

    char mousePos[100];
    sprintf(mousePos, "X position: %d and Y position: %d", mouseX, mouseY);
    iText(10, winHeight - 20, mousePos, GLUT_BITMAP_HELVETICA_18);
}

void iMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        angle = atan2(y - rocketY, x - rocketX);

        if (projectileCount < MAX_BULLET) {
            double dx = (bulletLen - bulletWidth);
            double dy = 0.0; 

            double rotated_dx = dx * cos(angle) - dy * sin(angle);
            double rotated_dy = dx * sin(angle) + dy * cos(angle);

            double tipX = rocketX + rotated_dx;
            double tipY = rocketY + rotated_dy;

            double bulletSpeed = 0.5;

            projectiles[projectileCount].x = tipX;
            projectiles[projectileCount].y = tipY;

            projectiles[projectileCount].vx = bulletSpeed * cos(angle);
            projectiles[projectileCount].vy = bulletSpeed * sin(angle);

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
    // Handle special keys if needed
}

int main() {
    srand(time(0));

    rocketImage = loadBMP("r.bmp");
    initStars();
    iInitialize(winWidth, winHeight, "Space Shooter - Rocket Movement");
    return 0;
}
