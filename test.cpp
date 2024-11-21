#include "iGraphics.h"
#include "stdlib.h"
#include "time.h"
#include <math.h>
#include <windows.h>

int winWidth = 800;
int winHeight = 600;

#define PI 3.14159265358979323846
#define MAX_BULLET 100
#define MAX_ENEMIES 10

#define NUM_STARS 100 // Adjust as needed
#define NUM_LAYERS 3  // Number of layers for parallax effect

typedef struct {
    double x, y;   // Position
    double speed;  // Speed of the star
} Star;

typedef struct{
    double x, y;
    double speed;
    bool active;
}Enemy;

Star stars[NUM_LAYERS][NUM_STARS];

void initStars() {
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int i = 0; i < NUM_STARS; i++) {
            stars[layer][i].x = rand() % winWidth;
            stars[layer][i].y = rand() % winHeight;
            stars[layer][i].speed = (layer + 1) * 0.1; // Layer 1: 0.5, Layer 2: 1.0, Layer 3: 1.5
        }
    }
}

void moveStars() {
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int i = 0; i < NUM_STARS; i++) {
            // Move the star to the left
            stars[layer][i].x -= stars[layer][i].speed;

            // Reset star position when it goes off-screen
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
            // Set color based on layer
            int brightness = 150 + layer * 50; // Adjust brightness for depth
            iSetColor(brightness, brightness, brightness);

            // Draw the star
            iPoint((int)stars[layer][i].x, (int)stars[layer][i].y);
        }
    }
}

Enemy enemies[MAX_ENEMIES];
int enemyCount=0;

double bulletWidth = 5;   
double bulletLen = 30;

double rocketX = 50;
double rocketY = winHeight/2;
int rocketSpeed = 10;

int mouseX = rocketX, mouseY = rocketY+100;
int size = 15;

double angle;

bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;

// Structure to hold image data
typedef struct {
    int width;
    int height;
    int *pixels; // Assuming 32-bit ARGB format
} Image;

struct Projectile {
    double x, y;
    double vx, vy;
    double angle;
};

Projectile projectiles[MAX_BULLET];
int projectileCount = 0;




// Function to load BMP image
Image loadBMP(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("File not found: %s\n", filename);
        exit(1);
    }

    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // Read the BMP header

    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
    int bpp = *(short*)&info[28]; // Bits per pixel

    int bytesPerPixel = bpp / 8;
    int size = bytesPerPixel * width * height;

    // BMP rows are padded to multiples of 4 bytes
    int row_padded = (width * bytesPerPixel + 3) & (~3);
    unsigned char* data = (unsigned char*)malloc(row_padded * height);

    // Read the pixel data
    fseek(f, *(int*)&info[10], SEEK_SET); // Move to the pixel data offset
    fread(data, sizeof(unsigned char), row_padded * height, f);
    fclose(f);

    // Allocate memory for pixel array
    int* pixels = (int*)malloc(width * height * sizeof(int));

    // Convert pixel data to ARGB format
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int idx = (height - 1 - i) * row_padded + j * bytesPerPixel;
            unsigned char blue = data[idx];
            unsigned char green = data[idx + 1];
            unsigned char red = data[idx + 2];
            unsigned char alpha = 255; // Fully opaque

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


// Function to rotate image
Image rotateImage(Image src, double angle) {
    int w = src.width;
    int h = src.height;
    double rad = angle;
    double cosA = cos(rad);
    double sinA = sin(rad);

    // Compute dimensions of the rotated image
    int newW = (int)(fabs(w * cosA) + fabs(h * sinA));
    int newH = (int)(fabs(w * sinA) + fabs(h * cosA));

    int *newPixels = (int *)malloc(newW * newH * sizeof(int));
    for (int i = 0; i < newW * newH; i++) {
        newPixels[i] = 0; // Initialize with transparent pixels
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

// Global variable to hold the sprite image
Image rocketImage;
void spawnEnemy(){
    if(enemyCount < MAX_ENEMIES){
        enemies[enemyCount].x = winWidth+50;
        enemies[enemyCount].y = rand()%winHeight;
        enemies[enemyCount].speed = 0.1;
        enemies[enemyCount].active = true;
        enemyCount++;
    }
}

void moveEnemies() {
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active) {
            // Calculate direction towards the player
            double dx = rocketX - enemies[i].x;
            double dy = rocketY - enemies[i].y;
            double distance = sqrt(dx * dx + dy * dy);

            // Avoid division by zero
            if (distance == 0) distance = 1;

            // Normalize direction vector
            double dirX = dx / distance;
            double dirY = dy / distance;

            // Update enemy position
            enemies[i].x += enemies[i].speed * dirX;
            enemies[i].y += enemies[i].speed * dirY;

            // Check if enemy has crossed the leftmost boundary
            if (enemies[i].x < 0) {
                enemies[i].active = false;
            }
        }
    }

    // Remove inactive enemies
    int i = 0;
    while (i < enemyCount) {
        if (!enemies[i].active) {
            enemies[i] = enemies[enemyCount - 1];
            enemyCount--;
        } else {
            i++;
        }
    }
}

void drawEnemies() {
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active) {
            iSetColor(255, 0, 0); // Red color
            iFilledCircle(enemies[i].x, enemies[i].y, 15); // Radius 15
        }
    }
}



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
    // iShowBMP2(0, 0, "space.bmp", 1);

    moveStars();
    drawStars();

    // Calculate the angle between the rocket and the mouse pointer
    angle = atan2(mouseY - rocketY, mouseX - rocketX);

    // Rotate the rocket image
    Image rotatedRocket = rotateImage(rocketImage, angle);

    // Draw the rotated rocket image
    int drawX = (int)(rocketX - rotatedRocket.width / 2);
    int drawY = (int)(rocketY - rotatedRocket.height / 2);

    // Draw the image pixel by pixel
    for (int y = 0; y < rotatedRocket.height; y++) {
        for (int x = 0; x < rotatedRocket.width; x++) {
            int pixel = rotatedRocket.pixels[y * rotatedRocket.width + x];
            int alpha = (pixel >> 24) & 0xFF;
            int red = (pixel >> 16) & 0xFF;
            int green = (pixel >> 8) & 0xFF;
            int blue = pixel & 0xFF;

            if (alpha > 0) { // Skip transparent pixels
                iSetColor(red, green, blue);
                iPoint(drawX + x, drawY + y);
            }
        }
    }

    for (int i = 0; i < projectileCount; i++) {
        iSetColor(255, 255, 255);

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

    // Free the rotated image pixel data
    free(rotatedRocket.pixels);

    moveRocket();
    moveProjectiles();
    drawEnemies();
    moveEnemies();

    // Display mouse position (optional)
    char mousePos[100];
    sprintf(mousePos, "X position: %d and Y position: %d", mouseX, mouseY);
    iText(10, winHeight - 20, mousePos, GLUT_BITMAP_HELVETICA_18);
}

// void enemySpawner(){
//     spawnEnemy();
// }

void iMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

        // Correctly calculate the angle between the rocket and the mouse click position
        angle = atan2(y - rocketY, x - rocketX);

        if (projectileCount < MAX_BULLET) {
            // Offset from the center to the tip in the unrotated image
            double dx = (bulletLen - bulletWidth);
            double dy = 0.0; // Negative because tip is above the center

            // Rotate the offset vector by the angle
            double rotated_dx = dx * cos(angle) - dy * sin(angle);
            double rotated_dy = dx * sin(angle) + dy * cos(angle);

            // Calculate the tip position
            double tipX = rocketX + rotated_dx;
            double tipY = rocketY + rotated_dy;

            // Set the bullet's speed
            double bulletSpeed = 0.5; // Adjust as needed

            // Initialize the projectile at the tip of the rocket
            projectiles[projectileCount].x = tipX;
            projectiles[projectileCount].y = tipY;

            // Set the projectile's velocity components
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

    // Load the rocket sprite image
    rocketImage = loadBMP("r.bmp"); // Ensure "rocket.bmp" exists
    iSetTimer(2000, spawnEnemy);
    initStars();
    iInitialize(winWidth, winHeight, "Space Shooter - Rocket Movement");
    return 0;
}
