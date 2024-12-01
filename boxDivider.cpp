#include "iGraphics.h"
#include "stdlib.h"
#include "time.h"
#include "math.h"
#include "windows.h"
#include "stdbool.h"
#include <stdio.h>

// Window dimensions
int winWidth = 800;
int winHeight = 600;
int gamestate = 0;

// Constants
#define PI 3.14159265358979323846
#define MAX_BULLET 100
#define NUM_STARS 100
#define NUM_LAYERS 3
#define MAX_PARTICLES 200
#define MAX_ENEMIES 200

//structures
typedef struct {
    double x, y;
    double radius;
    int sides;          
    int health;
    bool active;
    double angle;
    double rotationSpeed;
    int r, g, b;
    double vx, vy;   
    int state = rand()%2; 
} Enemy;

typedef struct {
    double x, y;
    double vx, vy;
    double lifespan;
    double size;
    double r, g, b;
    bool active;
} Particle;

typedef struct {
    double x, y;
    double speed;
} Star;

typedef struct {
    double x, y;
    double vx, vy;
    double angle;
} Projectile;

Star stars[NUM_LAYERS][NUM_STARS];
Enemy enemies[MAX_ENEMIES];
Particle particles[MAX_PARTICLES];
Projectile projectiles[MAX_BULLET];
int particleCount = 0; 
int enemyCount = 0;
int projectileCount = 0;


// Player Properties
double bulletWidth = 5;
double bulletLen = 30;
int currentLevel = 1;

double rocketX = 50;
double rocketY = winHeight / 2;
int rocketSpeed = 10;
int mouseX = rocketX, mouseY = rocketY + 100;
double angle;
bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;
double rocketSize = 30;

//Player Healths
int playerLives = 3;
bool isInvincible = false;
double invincibleTime = 0.0;
double invincibleDuration = 2.0;
double playerRadius = rocketSize/2.0;

// Level Tracking Variables
int enemiesDestroyed = 0;
int enemiesToNextLevel = 10; 

// Level Bar Dimensions
double levelBarWidth = 200.0;   
double levelBarHeight = 20.0;  
double levelBarX = (winWidth - levelBarWidth) / 2.0;  
double levelBarY = 20;   

//mainmenu
bool isInMainMenu = true;

double lastShotTime = 0.0;
double shootInterval = 0.15;

// Blinking Variables
bool isBlinking = false;
double blinkTime = 0.0;
double blinkDuration = 0.2;
double blinkInterval = 0.2;

// Function Prototypes
void spawnEnemy();
void moveEnemies();
void breakEnemy(int index);
void updateParticles(double deltaTime);
void drawParticles();
void checkEnemies();
void drawEnemies();
void initStars();
void moveStars();
void drawStars();
void moveRocket();
void moveProjectiles();
void checkParticleCollisions();
void damageEnemy(int index);
bool isPointInPolygon(double px, double py, Enemy e);
void checkProjectileCollisions();
double iGetTime();
void drawHeart(double x, double y, double size);
bool isPlayerCollidingWithEnemy(Enemy e);
void iDraw();
void iMouse(int button, int state, int x, int y);
void iMouseMove(int x, int y);
void iKeyboard(unsigned char key);
void iSpecialKeyboard(unsigned char key);

// Function Implementations

void spawnEnemy() {
    if (enemyCount < MAX_ENEMIES) {
        int enemiesToSpawn = currentLevel;

        for(int i=0;i<enemiesToSpawn;i++){
            if(enemyCount<MAX_ENEMIES){
                Enemy *e = &enemies[enemyCount];
                int boundary = rand() % 4;  

                e->radius = 25 + rand() % 15;  
                e->sides = 2 + (currentLevel-1)/2+1;   
                e->health = e->sides;          
                e->active = true;
                e->angle = 0;
                e->rotationSpeed = ((rand() % 10) - 5) * 0.001;  
                e->r = 255;
                e->g = 0;
                e->b = 0;

                switch (boundary) {
                    case 0: //Left
                        e->x = -e->radius;
                        e->y = rand() % winHeight;
                        e->vx = 0.05;  
                        e->vy = ((rand() % 11) - 5) * 0.01;  
                        break;
                    case 1: // Right
                        e->x = winWidth + e->radius;
                        e->y = rand() % winHeight;
                        e->vx = -0.05;  
                        e->vy = ((rand() % 11) - 5) * 0.01;  
                        break;
                    case 2: // Top
                        e->x = rand() % winWidth;
                        e->y = winHeight + e->radius;
                        e->vx = ((rand() % 11) - 5) * 0.01;  
                        e->vy = -0.05;  
                        break;
                    case 3: // Bottom
                        e->x = rand() % winWidth;
                        e->y = -e->radius;
                        e->vx = ((rand() % 11) - 5) * 0.01;  
                        e->vy = 0.05;   
                        break;
                }

                enemyCount++;
            }
        }
    }
}

void moveEnemies() {
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active) {
            enemies[i].x += enemies[i].vx;
            enemies[i].y += enemies[i].vy;
            enemies[i].angle += enemies[i].rotationSpeed;

            
            if (enemies[i].x + enemies[i].radius < 0 || enemies[i].x - enemies[i].radius > winWidth ||
                enemies[i].y + enemies[i].radius < 0 || enemies[i].y - enemies[i].radius > winHeight) {
                
                enemies[i] = enemies[enemyCount - 1];
                enemyCount--;
                i--;  
            }
        }
    }
}

void breakEnemy(int index) {
    if (enemies[index].active && enemies[index].health <= 0) {
        // PlaySound(TEXT("explosion.wav"), NULL, 1);

        Enemy *e = &enemies[index];

        int numParticlesPerVertex = 5; 
        int numVertices = e->sides;

        double xCoords[6];  
        double yCoords[6];

        for (int j = 0; j < numVertices; j++) {
            double theta = 2 * PI * j / numVertices + e->angle;
            xCoords[j] = e->x + e->radius * cos(theta);
            yCoords[j] = e->y + e->radius * sin(theta);
        }

        if (e->radius >= 30) { 
            for (int i = 0; i < 2; i++) {
                if (enemyCount < MAX_ENEMIES) {
                    Enemy *newEnemy = &enemies[enemyCount];
                    newEnemy->radius = e->radius / 2;  
                    newEnemy->sides = e->sides;  
                    newEnemy->health = e->sides-2;
                    newEnemy->active = true;
                    newEnemy->angle = rand() % 360;
                    newEnemy->rotationSpeed = ((rand() % 10) - 5) * 0.001;

                    newEnemy->x = e->x + (rand() % 21 - 10);  
                    newEnemy->y = e->y + (rand() % 21 - 10);

                    newEnemy->vx = ((rand() % 11) - 5) * 0.01;
                    newEnemy->vy = ((rand() % 11) - 5) * 0.01;

                    newEnemy->r = rand()%200 + 56;
                    newEnemy->g = rand()%100;
                    newEnemy->b = rand()%100;
                    
                    enemyCount++; 
                }
            }
        }

        for (int v = 0; v < numVertices; v++) {
            for (int i = 0; i < numParticlesPerVertex && particleCount < MAX_PARTICLES; i++) {
                Particle *p = &particles[particleCount];
                p->x = xCoords[v];
                p->y = yCoords[v];

                double angle = ((double)rand() / RAND_MAX) * 2 * PI;
                double speed = ((double)rand() / RAND_MAX) * 0.05 + 0.02; 
                p->vx = speed * cos(angle);
                p->vy = speed * sin(angle);

                p->lifespan = 10.0; 
                p->size = 2 + rand() % 3;
                p->r = 1.0;  
                p->g = 0.0;
                p->b = 0.0;
                p->active = true;
                particleCount++;  
            }
        }

        enemies[index] = enemies[enemyCount - 1];
        enemyCount--;
    }
}

void updateParticles(double deltaTime) {
    for (int i = 0; i < particleCount; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;

            double friction = 0.99; 
            particles[i].vx *= friction;
            particles[i].vy *= friction;

            particles[i].lifespan -= deltaTime;

            particles[i].size *= friction;

            if (particles[i].lifespan <= 0) {
                
                particles[i] = particles[particleCount - 1];
                particleCount--;
                i--;  
            }
        }
    }

    // printf("Active Particles: %d\n", particleCount);
}

void drawParticles() {
    for (int i = 0; i < particleCount; i++) {
        if (particles[i].active) {

            double lifespanRatio = particles[i].lifespan / 10.0; 

            if (lifespanRatio > 1.0) lifespanRatio = 1.0;
            if (lifespanRatio < 0.0) lifespanRatio = 0.0;

            double g_lifespan = (1.0 - lifespanRatio);
            double b_lifespan = (1.0 - lifespanRatio);

            double g_total = g_lifespan + particles[i].g;
            double b_total = b_lifespan + particles[i].b;

            if (g_total > 1.0) g_total = 1.0;
            if (b_total > 1.0) b_total = 1.0;

            int r = 255;
            int g = (int)(g_total * 255);
            int b = (int)(b_total * 255);

            if (g > 255) g = 255;
            if (g < 0) g = 0;
            if (b > 255) b = 255;
            if (b < 0) b = 0;

            iSetColor(r, g, b);
            iFilledCircle(particles[i].x, particles[i].y, particles[i].size);
        }
    }
}

void checkEnemies() {
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active && enemies[i].health <= 0) {
            breakEnemy(i);
            enemiesDestroyed++; 

            if (enemiesDestroyed >= enemiesToNextLevel) {
                currentLevel++;
                enemiesDestroyed = 0; 

                enemiesToNextLevel += 5; 

                printf("Level Up! Now at Level %d\n", currentLevel); 
            }

            i--;
        }
    }
}

void drawEnemies() {
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active) {
            iSetColor(enemies[i].r, enemies[i].g, enemies[i].b);

            int numSides = enemies[i].sides;
            double xCoords[6];
            double yCoords[6];

            for (int j = 0; j < numSides; j++) {
                double theta = 2 * PI * j / numSides + enemies[i].angle;
                xCoords[j] = enemies[i].x + enemies[i].radius * cos(theta);
                yCoords[j] = enemies[i].y + enemies[i].radius * sin(theta);
            }
            int x =rand()%2;
            if(enemies[i].state){
                iFilledPolygon(xCoords, yCoords, numSides);
            }else{
                iPolygon(xCoords, yCoords, numSides);
            }
        }
    }
}

void initStars() {
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int i = 0; i < NUM_STARS; i++) {
            stars[layer][i].x = rand() % winWidth;
            stars[layer][i].y = rand() % winHeight;
            stars[layer][i].speed = (layer + 1) * 0.08;
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

void checkParticleCollisions() {
    for (int i = 0; i < particleCount; i++) {
        if (particles[i].active) {
            for (int j = 0; j < projectileCount; j++) {
                double dx = particles[i].x - projectiles[j].x;
                double dy = particles[i].y - projectiles[j].y;
                double distanceSquared = dx * dx + dy * dy;
                double collisionDistance = particles[i].size + bulletWidth / 2.0; // Sum of radii

                if (distanceSquared <= collisionDistance * collisionDistance) {
                    particles[i].g += 0.2;
                    particles[i].b += 0.2;

                    if (particles[i].g > 1.0) particles[i].g = 1.0;
                    if (particles[i].b > 1.0) particles[i].b = 1.0;

                    projectiles[j] = projectiles[projectileCount - 1];
                    projectileCount--;
                    j--; 
                }
            }
        }
    }
}

void damageEnemy(int index) {
    if (enemies[index].active) {
        enemies[index].health--;
        
        double damageRatio = (double)(enemies[index].sides - enemies[index].health) / enemies[index].sides;
        
        enemies[index].g = (int)(damageRatio * 255.0);
        enemies[index].b = (int)(damageRatio * 255.0);
        
        if (enemies[index].g > 255) enemies[index].g = 255;
        if (enemies[index].b > 255) enemies[index].b = 255;
        if (enemies[index].g < 0) enemies[index].g = 0;
        if (enemies[index].b < 0) enemies[index].b = 0;
    }
}


bool isPointInPolygon(double px, double py, Enemy e) {
    int numSides = e.sides;
    double *xCoords = (double *)malloc(numSides * sizeof(double));
    double *yCoords = (double *)malloc(numSides * sizeof(double));

    for (int j = 0; j < numSides; j++) {
        double theta = 2 * PI * j / numSides + e.angle;
        xCoords[j] = e.x + e.radius * cos(theta);
        yCoords[j] = e.y + e.radius * sin(theta);
    }

    bool inside = false;
    for (int i = 0, j = numSides - 1; i < numSides; j = i++) {
        if (((yCoords[i] > py) != (yCoords[j] > py)) &&
            (px < (xCoords[j] - xCoords[i]) * (py - yCoords[i]) / (yCoords[j] - yCoords[i]) + xCoords[i])) {
            inside = !inside;
        }
    }

    free(xCoords);
    free(yCoords);

    return inside;
}

void checkProjectileCollisions() {
    for (int i = 0; i < projectileCount; i++) {
        for (int j = 0; j < enemyCount; j++) {
            if (enemies[j].active && isPointInPolygon(projectiles[i].x, projectiles[i].y, enemies[j])) {
                damageEnemy(j);
                projectiles[i] = projectiles[projectileCount - 1];
                projectileCount--;
                i--;
                break;
            }
        }
    }
}

double iGetTime() {
    static LARGE_INTEGER frequency;
    static bool initialized = false;
    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = true;
    }

    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    return (double)currentTime.QuadPart / frequency.QuadPart;
}

bool isPlayerCollidingWithEnemy(Enemy e) {
    double dx = e.x - rocketX;
    double dy = e.y - rocketY;
    double distance = sqrt(dx * dx + dy * dy);
    return distance < (e.radius + playerRadius);
}


void drawHeart(double x, double y, double size) {
    int points = 30;
    double angleStep = (2 * PI) / points;
    double verticesX[30];
    double verticesY[30];

    for(int i = 0; i < points; i++) {
        double theta = i * angleStep;
        double scale = 16 * pow(sin(theta), 3);
        double px = size * scale / 16;
        double py = size * (13 * cos(theta) - 5 * cos(2 * theta) - 2 * cos(3 * theta) - cos(4 * theta)) / 16;
        verticesX[i] = x + px;
        verticesY[i] = y + py;
    }

    iSetColor(255, 0, 0);
    iFilledPolygon(verticesX, verticesY, points);
}


bool f = false;

void iDraw() {
    iClear();

    if (gamestate == 0) { // Main Menu
        iClear();
        isInMainMenu = true;
        if(!f){
            PlaySound(TEXT("menusound.wav"), NULL, SND_ASYNC | SND_LOOP);
            f = true;
        }

        iShowBMP2(0, 0, "C:/Users/prito/Desktop/projects/space shooter/mainmenu.bmp", -1);
        // system("pause");
    }
    else if (gamestate == 1) { // Game
        iClear();
        if(f){
            PlaySound(NULL, NULL, SND_ASYNC | SND_LOOP);
            f=false;
        }
        isInMainMenu = false;
        // iShowBMP2(0, 0, "menu.bmp", -1);
        static double lastFrameTime = 0;
        double currentTime = iGetTime();
        double deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        iClear();

        moveStars();
        drawStars();

        angle = atan2(mouseY - rocketY, mouseX - rocketX);

        double localX[3] = {0, -rocketSize / 2, rocketSize / 2};
        double localY[3] = {rocketSize / 2, -rocketSize / 2, -rocketSize / 2};

        double rotatedX[3], rotatedY[3];

        for (int i = 0; i < 3; i++) {
            double x = localX[i];
            double y = localY[i];
            rotatedX[i] = x * cos(angle) - y * sin(angle) + rocketX;
            rotatedY[i] = x * sin(angle) + y * cos(angle) + rocketY;
        }

        if(isInvincible){
            iSetColor(255, 0, 0); 
        }else{
            iSetColor(255, 255, 255); 
        }

        iFilledPolygon(rotatedX, rotatedY, 3);

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

        moveRocket();
        moveProjectiles();
        moveEnemies();
        drawEnemies();
        checkProjectileCollisions();

        checkParticleCollisions();
        checkEnemies();
        updateParticles(deltaTime);
        drawParticles();

        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].active && isPlayerCollidingWithEnemy(enemies[i])) {
                if (!isInvincible) {
                    playerLives--;
                    printf("Player Hit! Lives Remaining: %d\n", playerLives);
                    isInvincible = true;
                    invincibleTime = 0.0;

                    if (playerLives <= 0) {
                        printf("Game Over! Restarting...\n");
                        //Restart
                        playerLives = 3;
                        currentLevel = 1;
                        enemiesDestroyed = 0;
                        enemyCount = 0;
                        projectileCount = 0;
                        particleCount = 0;
                        rocketX = 50;
                        rocketY = winHeight / 2;
                    }
                }
            }
        }

        if(isInvincible){
            invincibleTime += deltaTime;
            blinkTime += deltaTime;

            if(blinkTime >= blinkInterval){
                isBlinking=!isBlinking;
                blinkTime=0.0;
            }

            if (invincibleTime >= invincibleDuration) {
                isInvincible = false;
            }

            if(isInvincible && isBlinking){
                // iSetColor(255, 0, 0);
                // iFilledCircle(rocketX, rocketY, 3);
            }else{
                iSetColor(255, 255, 255);
                iFilledPolygon(rotatedX, rotatedY, 3);
            }
        }

        iSetColor(255, 255, 255);
        iRectangle(levelBarX - 1, levelBarY - 1, levelBarWidth + 2, levelBarHeight + 2);
        
        double progressRatio = (double)enemiesDestroyed / enemiesToNextLevel;
        if (progressRatio > 1.0) progressRatio = 1.0;
        double filledWidth = progressRatio * levelBarWidth;

        iSetColor(255, 255, 255);
        iFilledRectangle(levelBarX, levelBarY, filledWidth, levelBarHeight);

        char levelText[20];
        sprintf(levelText, "Level: %d", currentLevel);
        iSetColor(255, 255, 255); 
        iText(levelBarX + levelBarWidth / 2.0 - 20, levelBarY + levelBarHeight + 5, levelText, GLUT_BITMAP_HELVETICA_18);

        for(int i = 0; i < playerLives; i++) {
            drawHeart(20 + i * 40, winHeight - 40, 20); 
        }
    }
    else if (gamestate == 2) { // About
        iClear();
        // isInMainMenu = false;
        iShowBMP2(0, 0, "C:/Users/prito/Desktop/projects/space shooter/about.bmp", -1);
    }
    else if(gamestate==3){
        iClear();
        // isInMainMenu = false;
        printf("High Scores\n");
    }
    else if(gamestate==4){
        iClear();
        // isInMainMenu = false;
        iShowBMP2(0, 0, "C:/Users/prito/Desktop/projects/space shooter/controls.bmp", -1);
    }
    else if(gamestate==5){
        iClear();
        // isInMainMenu = false;
        iShowBMP2(0, 0, "C:/Users/prito/Desktop/projects/space shooter/esc.bmp", -1);
    }
}

void iMouse(int button, int state, int mx, int my) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

        double currentTime = iGetTime();

        if(currentTime - lastShotTime >= shootInterval){
            angle = atan2(my - rocketY, mx - rocketX);
            PlaySound(TEXT("bullet.wav"), NULL, SND_ASYNC);

            if (projectileCount < MAX_BULLET) {
                double tipX = rocketX + (rocketSize / 2) * cos(angle);
                double tipY = rocketY + (rocketSize / 2) * sin(angle);

                double bulletSpeed = 0.8; 

                projectiles[projectileCount].x = tipX;
                projectiles[projectileCount].y = tipY;

                projectiles[projectileCount].vx = bulletSpeed * cos(angle);
                projectiles[projectileCount].vy = bulletSpeed * sin(angle);

                projectiles[projectileCount].angle = angle;
                projectileCount++;
            }
            lastShotTime = currentTime;
        }

        if (gamestate == 0 && mx >= 30 && mx<=210 &&
            my >= 380 && my <= 420) {
            gamestate = 1; // Start Game
        }

        if (gamestate == 0 && mx >= 100 && mx <= 280 &&
            my >= 260 && my <= 300) {
            gamestate = 2; // About Section
        }

        if (gamestate == 0 && mx >= 300 && mx <= 450 &&
            my >= 190 && my <= 230) {
            exit(0); // Exit Game
        }
        if (gamestate == 0 && mx >= 520 && mx <= 700 &&
            my >= 280 && my <= 320) {
            gamestate = 3;
            exit(0);
        }
        if (gamestate == 0 && mx >= 570 && mx <= 750 &&
            my >= 380 && my <= 420) {
            gamestate=4;
        }

        if(gamestate==2 && mx >= 630 && mx <= 770 && my >= 530 && my <= 560){
            gamestate=0;
        }
        if(gamestate==4 && mx >= 620 && mx <= 750 && my >= 490 && my <= 520){
            gamestate=0;
        }

        if(gamestate == 5){
            if(mx >= 310 && mx <= 470 && my >= 400 && my <= 430){
                gamestate=1;
            }
            if(mx >= 340 && mx <= 450 && my >= 320 && my <= 350){
                gamestate=0;
            }
            if(mx >= 290 && mx <= 500 && my >= 230 && my <= 260){
                gamestate=4;
            }
            if(mx >= 350 && mx <= 440 && my >= 150 && my <= 180){
                exit(0);
            }
        }

        printf("x: %d, y: %d\n", mx, my);
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
    if (key == 27 && gamestate == 1) gamestate=5;
}

void iSpecialKeyboard(unsigned char key) {
    if(key == GLUT_KEY_DOWN){
        printf("%c\n", key);
    } 
}

int main() {
    srand(time(0));
    initStars();
    iSetTimer(2000, spawnEnemy); 
    iInitialize(winWidth, winHeight, "Space Shooter - Multiple Enemy Types");
    return 0;
}
