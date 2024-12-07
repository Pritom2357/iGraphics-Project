#include "iGraphics.h"
#include "stdlib.h"
#include "time.h"
#include "math.h"
#include "windows.h"
#include "stdbool.h"
#include <stdio.h>
// #include<iostream>
// #include<vector>
#include<bits/stdc++.h>
using namespace std;

// Window dimensions
const int winWidth = 800;
const int winHeight = 600;
int gamestate = 0;

// Constants
#define PI 3.14159265358979323846
//bullet properties
#define MAX_BULLET 50
#define BULLET_SPEED 0.8
#define BULLET_LIFETIME 2.0
//star properties
#define NUM_STARS 50
#define NUM_LAYERS 3
//particle properties
#define PARTICLE_POOL_SIZE 300
#define MAX_ACTIVE_PARTICLES 50
#define PARTICLE_CLEANUP_THRESHOLD 0.05
//enemy properties
#define MAX_ENEMIES 100
//grid properties
#define GRID_CELL_SIZE 64
#define GRID_WIDTH (winWidth/GRID_CELL_SIZE)
#define GRID_HEIGHT (winHeight/GRID_CELL_SIZE)
//player properties
#define PLAYER_ACCELERATION 0.5
#define PLAYER_MAX_SPEED 15.0
#define PLAYER_FRICTION 0.98
#define MOVEMENT_SMOOTHING 0.8
#define MIN_VELOCITY 0.01
//powerups
#define MAX_POWERUPS 5
#define POWERUP_SPAWN_CHANCE 20
#define POWERUP_SIZE 15.0
#define POWERUP_DURATION 10.0
//notifications
#define NOTIFICATION_DURATION 10.0
#define NOTIFICATION_Y 500
//healt bar properties
#define HEALTH_BAR_WIDTH 40.0
#define HEALTH_BAR_HEIGHT 5.0
#define HEALTH_BAR_OFFSET 20.0
#define MAX_PLAYER_HEALTH 100


//structures
typedef struct{
    vector<int> enemies;
    vector<int> particles;
    vector<int> projectiles;
}GridCell;

typedef struct {
    char text[50];
    double timer;
    bool active;
    int alpha; 
} Notification;

enum PowerupType{
    TRIPLE_SHOT,
    SPEED_BOOST,
    SHIELD,
    RAPID_FIRE,
    HEALTH_BOOST
};

typedef struct{
    double x, y;
    double angle;
    bool active;
    PowerupType type;
    double duration;
    double rotationSpeed;
}PowerUp;

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
    int gridX, gridY;
} Enemy;

typedef struct {
    double x, y;
    double vx, vy;
    double lifespan;
    double size;
    double r, g, b;
    bool active;
    int gridX, gridY;
} Particle;

typedef struct {
    double x, y;
    double speed;
} Star;

typedef struct {
    double x, y;
    double vx, vy;
    double angle;
    bool active;
    double lifetime;
} Projectile;

GridCell grid[GRID_WIDTH][GRID_HEIGHT];
Star stars[NUM_LAYERS][NUM_STARS];
Enemy enemies[MAX_ENEMIES];
Particle particles[PARTICLE_POOL_SIZE];
Projectile projectiles[MAX_BULLET];
PowerUp powerups[MAX_POWERUPS];
int powerupCount = 0;
bool hasTripleShot = false;
bool hasSpeedBoost = false;
bool hasShield = false;
bool hasRapidFire = false;
double powerUpTimer = 0.0;
int particleCount = 0; 
int enemyCount = 0;
int projectileCount = 0;
double shieldTimer = 0.0;
bool shieldActive = false;
Notification currentNotification = {"", 0.0, false, 255};
int playerHealth = MAX_PLAYER_HEALTH;

inline int getGridx(double x){return (int)(x/GRID_CELL_SIZE);}
inline int getGridy(double y){return (int)(y/GRID_CELL_SIZE);}


// Player Properties
double bulletWidth = 5;
double bulletLen = 30;
int currentLevel = 1;

double playerVX = 0.0;
double playerVY = 0.0;

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
double shootInterval = 0.1;

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
bool isPointInPolygon(double px, double py, Enemy* e);
void checkProjectileCollisions();
double iGetTime();
// void drawHeart(double x, double y, double size);
bool isPlayerCollidingWithEnemy(Enemy e);
void iDraw();
void iMouse(int button, int state, int x, int y);
void iMouseMove(int x, int y);
void iKeyboard(unsigned char key);
void iSpecialKeyboard(unsigned char key);

// Function Implementations

// Modify drawPlayerHealth to drawPlayerHeart
void drawPlayerHearts() {
    double heartSize = 8.0;  // Size of each heart
    double spacing = 2.0;   // Spaace between hearts
    double baseY = rocketY + rocketSize + 10;  // Base Y position above player
    
    double totalWidth = (playerLives*heartSize*2) + ((playerLives-1)*spacing);
    double startX = rocketX - totalWidth/2;

    for(int i = 0; i < playerLives; i++) {
        // Calculate position for each heart
        double heartX = startX + i*(heartSize*2 + spacing);  // Center hearts above player
        double heartY = baseY;
        
        // Draw heart shape
        int points = 30;
        double angleStep = (2 * PI) / points;
        double verticesX[30];
        double verticesY[30];
        
        for(int j = 0; j < points; j++) {
            double theta = j * angleStep;
            double scale = 16 * pow(sin(theta), 3);
            double px = heartSize * scale / 16;
            double py = heartSize * (13 * cos(theta) - 5 * cos(2 * theta) - 2 * cos(3 * theta) - cos(4 * theta)) / 16;
            verticesX[j] = heartX + px;
            verticesY[j] = heartY + py;
        }
        
        // Draw filled heart
        iSetColor(255, 0, 0);  // Red hearts
        iFilledPolygon(verticesX, verticesY, points);
    }
}

void damagePlayer() {
    if (!isInvincible) {
        playerHealth -= 20;  // Damage amount
        
        if (playerHealth <= 0) {
            playerHealth = 0;
            playerLives--;
            
            if (playerLives > 0) {
                // Reset health if lives remaining
                playerHealth = MAX_PLAYER_HEALTH;
            } else {
                // Game over
                printf("Game Over! Restarting...\n");
                playerLives = 3;
                playerHealth = MAX_PLAYER_HEALTH;
                currentLevel = 1;
                enemiesDestroyed = 0;
                enemyCount = 0;
                projectileCount = 0;
                particleCount = 0;
                rocketX = 50;
                rocketY = winHeight / 2;
            }
        }
        
        isInvincible = true;
        invincibleTime = 0.0;
    }
}

void spawnPowerUp(double x, double y) {
    if (powerupCount >= MAX_POWERUPS) return;
    if (rand() % 100 >= POWERUP_SPAWN_CHANCE) return;
    
    PowerUp* p = &powerups[powerupCount++];
    p->x = x;
    p->y = y;
    p->active = true;
    p->type = (PowerupType)(rand() % 5);
    p->angle = 0;
    p->rotationSpeed = ((rand() % 10) - 5) * 0.001;
    p->duration = POWERUP_DURATION;
}


void showNotification(const char* text) {
    strcpy(currentNotification.text, text);
    currentNotification.timer = NOTIFICATION_DURATION;
    currentNotification.active = true;
    currentNotification.alpha = 255;
}

void updatePowerUps(double deltaTime) {
    int speed;
    // Update active effects
    if (powerUpTimer > 0) {
        powerUpTimer -= deltaTime;
        if (powerUpTimer <= 0) {
            hasTripleShot = false;
            hasSpeedBoost = false;
            hasRapidFire = false;
            
            // Reset affected values
            shootInterval = 0.15;
            speed=PLAYER_MAX_SPEED;
        }
    }

    if (shieldActive && shieldTimer > 0) {
        shieldTimer -= deltaTime;
        if (shieldTimer <= 0) {
            shieldActive = false;
            isInvincible = false;
            hasShield = false;
        }
    }

    // Update powerups
    for (int i = 0; i < powerupCount; i++) {
        if (!powerups[i].active) continue;
        
        powerups[i].angle += powerups[i].rotationSpeed;
        
        // Check collection
        double dx = powerups[i].x - rocketX;
        double dy = powerups[i].y - rocketY;
        double distance = sqrt(dx * dx + dy * dy);
        
        if (distance < (POWERUP_SIZE + playerRadius)) {
            // Apply effect
            switch (powerups[i].type) {
                case TRIPLE_SHOT:
                    hasTripleShot = true;
                    showNotification("Triple Shot Activated!");
                    break;
                case SPEED_BOOST:
                    hasSpeedBoost = true;
                    showNotification("Speed Boost Activated!");
                    speed=25;
                    break;
                case SHIELD:
                    shieldActive = true;
                    hasShield = true;
                    isInvincible = true;
                    shieldTimer = POWERUP_DURATION;
                    showNotification("Shield Activated!");
                    break;
                case RAPID_FIRE:
                    hasRapidFire = true;
                    showNotification("Rapid Fire Activated!");
                    shootInterval = 0.05;
                    break;
                case HEALTH_BOOST:
                    playerLives = min(playerLives + 1, 5);
                    showNotification("Health Restored!");
                    break;
            }
            
            powerUpTimer = POWERUP_DURATION;
            powerups[i] = powerups[--powerupCount];
            i--;
        }
    }
}

// Modify drawNotification function
void drawNotification(double deltaTime) {
    if (currentNotification.active) {
        currentNotification.timer -= deltaTime;
        
        // Calculate fade based on timer
        int colorValue = (int)(255 * fmin(1.0, currentNotification.timer / 0.5));
        
        if (currentNotification.timer <= 0) {
            currentNotification.active = false;
        } else {
            // Draw notification text with fading
            iSetColor(colorValue, colorValue, colorValue);
            iText(winWidth/2 - 100, NOTIFICATION_Y, currentNotification.text, GLUT_BITMAP_HELVETICA_18);
            
            // Draw power-up timer if active
            if (powerUpTimer > 0) {
                char timerText[20];
                sprintf(timerText, "%.1f", powerUpTimer);
                iSetColor(colorValue, colorValue, colorValue);
                iText(winWidth/2 - 20, NOTIFICATION_Y - 25, timerText, GLUT_BITMAP_HELVETICA_12);
            }
        }
    }
}

void drawPowerUps() {
    for (int i = 0; i < powerupCount; i++) {
        if (!powerups[i].active) continue;
        
        // Set color based on type
        switch (powerups[i].type) {
            case TRIPLE_SHOT:
                iSetColor(255, 255, 0);  // Yellow
                break;
            case SPEED_BOOST:
                iSetColor(0, 255, 0);    // Green
                break;
            case SHIELD:
                iSetColor(0, 255, 255);  // Cyan
                break;
            case RAPID_FIRE:
                iSetColor(255, 0, 255);  // Magenta
                break;
            case HEALTH_BOOST:
                iSetColor(255, 0, 0);    // Red
                break;
        }
        
        // Draw powerup
        double size = POWERUP_SIZE;
        int sides = 6;  // Hexagon
        double xCoords[6], yCoords[6];
        
        for (int j = 0; j < sides; j++) {
            double theta = 2 * PI * j / sides + powerups[i].angle;
            xCoords[j] = powerups[i].x + size * cos(theta);
            yCoords[j] = powerups[i].y + size * sin(theta);
        }
        
        iFilledPolygon(xCoords, yCoords, sides);
    }
}

void updateGrid(){
    for(int x=0;x<GRID_WIDTH;x++){
        for(int y=0;y<GRID_HEIGHT;y++){
            grid[x][y].enemies.clear();
            grid[x][y].particles.clear();
            grid[x][y].particles.clear();
        }
    }

    for(int i=0;i<enemyCount;i++){
        if(enemies[i].active){
            int gx = getGridx(enemies[i].x);
            int gy = getGridy(enemies[i].y);

            if(gx>=0 && gx<GRID_WIDTH && gy>=0 && gy<GRID_HEIGHT){
                enemies[i].gridX=gx;
                enemies[i].gridY=gy;
                grid[gx][gy].enemies.push_back(i);
            }
        }
    }

    for(int i = 0; i < particleCount; i++) {
        if(particles[i].active) {
            int gx = getGridx(particles[i].x);
            int gy = getGridy(particles[i].y);
            if(gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
                particles[i].gridX = gx;
                particles[i].gridY = gy;
                grid[gx][gy].particles.push_back(i);
            }
        }
    }
}

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

        spawnPowerUp(enemies[index].x, enemies[index].y);

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
            for (int i = 0; i < numParticlesPerVertex && particleCount < MAX_ACTIVE_PARTICLES; i++) {
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
    static double cleanupTImer = 0;
    cleanupTImer+=deltaTime;
    
    if (cleanupTImer >= PARTICLE_CLEANUP_THRESHOLD) {
        int activeCount = 0;
        for(int i=0; i<particleCount; i++){
            if(particles[i].active){
                if(i!=activeCount){
                    particles[activeCount] = particles[i];
                }
                activeCount++;
            }
        }
        particleCount = activeCount;
        cleanupTImer = 0;
    }

    int activeParticles = 0;
    for(int i=0; i<particleCount && activeParticles<MAX_ACTIVE_PARTICLES; i++){
        if(particles[i].active){
            activeParticles++;

            Particle* p = &particles[i];

            p->x += p->vx;
            p->y += p->vy;

            const double friction = 0.99; 
            p->vx *= friction;
            p->vy *= friction;
            p->size *= friction;

            p->lifespan -= deltaTime;

            if(p->lifespan<=0){
                p->active=false;
            }
        }
    }

    // for (int i = 0; i < particleCount; i++) {
    //     if (particles[i].active) {
    //         particles[i].x += particles[i].vx;
    //         particles[i].y += particles[i].vy;

    //         double friction = 0.99; 
    //         particles[i].vx *= friction;
    //         particles[i].vy *= friction;

    //         particles[i].lifespan -= deltaTime;

    //         particles[i].size *= friction; 

    //         if (particles[i].lifespan <= 0) {
                
    //             particles[i] = particles[particleCount - 1];
    //             particleCount--;
    //             i--;  
    //         }
    //     }
    // }

    // printf("Active Particles: %d\n", particleCount);
}

void createParticles(double x, double y, double vx, double vy){
    if(particleCount>=PARTICLE_POOL_SIZE || particleCount>=MAX_ACTIVE_PARTICLES){
        return;
    }

    Particle* p = &particles[particleCount++];
    p->x = x;
    p->y = y;
    p->vx = vx;
    p->vy = vy;
    p->active = true;
    p->lifespan = 10;
    p->size = 2 + rand() % 3;
    p->r = 1.0;
    p->g = 0.0;
    p->b = 0.0;
}

void drawParticles() {
    // Pre-calculate common values
    const int maxParticles = fmin(particleCount, MAX_ACTIVE_PARTICLES);
    
    // Batch similar colors
    for (int i = 0; i < maxParticles; i++) {
        if (particles[i].active) {
            double lifespanRatio = fmax(0.0, fmin(1.0, particles[i].lifespan / 10.0));
            int colorValue = (int)((1.0 - lifespanRatio) * 255);
            
            iSetColor(255, colorValue, colorValue);
            iFilledCircle(particles[i].x, particles[i].y, particles[i].size);
        }
    }
}

// void drawParticles() {
//     for (int i = 0; i < particleCount; i++) {
//         if (particles[i].active) {

//             double lifespanRatio = particles[i].lifespan / 10.0; 

//             if (lifespanRatio > 1.0) lifespanRatio = 1.0;
//             if (lifespanRatio < 0.0) lifespanRatio = 0.0;

//             double g_lifespan = (1.0 - lifespanRatio);
//             double b_lifespan = (1.0 - lifespanRatio);

//             double g_total = g_lifespan + particles[i].g;
//             double b_total = b_lifespan + particles[i].b;

//             if (g_total > 1.0) g_total = 1.0;
//             if (b_total > 1.0) b_total = 1.0;

//             int r = 255;
//             int g = (int)(g_total * 255);
//             int b = (int)(b_total * 255);

//             if (g > 255) g = 255;
//             if (g < 0) g = 0;
//             if (b > 255) b = 255;
//             if (b < 0) b = 0;

//             iSetColor(r, g, b);
//             iFilledCircle(particles[i].x, particles[i].y, particles[i].size);
//         }
//     }
// }

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
    static double lastTime = iGetTime();
    double currentTime = iGetTime();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Scale movement by deltaTime for frame independence
    double acceleration = PLAYER_ACCELERATION * deltaTime * 60.0; // Normalized for 60fps
    double friction = pow(PLAYER_FRICTION, deltaTime * 60.0);

    // Update velocities with input
    if (moveUp)    playerVY += acceleration;
    if (moveDown)  playerVY -= acceleration;
    if (moveLeft)  playerVX -= acceleration;
    if (moveRight) playerVX += acceleration;

    // Apply speed limits
    playerVX = fmax(-PLAYER_MAX_SPEED, fmin(PLAYER_MAX_SPEED, playerVX));
    playerVY = fmax(-PLAYER_MAX_SPEED, fmin(PLAYER_MAX_SPEED, playerVY));

    // Apply friction once
    playerVX *= friction;
    playerVY *= friction;

    // Cache boundary values
    double leftBound = playerRadius;
    double rightBound = winWidth - playerRadius;
    double topBound = winHeight - playerRadius;
    double bottomBound = playerRadius;

    // Update position with boundary check
    double newX = rocketX + playerVX;
    double newY = rocketY + playerVY;

    // Boundary collision with clamping
    if (newX < leftBound) {
        newX = leftBound;
        playerVX = 0;
    } else if (newX > rightBound) {
        newX = rightBound;
        playerVX = 0;
    }

    if (newY < bottomBound) {
        newY = bottomBound;
        playerVY = 0;
    } else if (newY > topBound) {
        newY = topBound;
        playerVY = 0;
    }

    // Apply movement with smoothing
    rocketX = rocketX * MOVEMENT_SMOOTHING + newX * (1.0 - MOVEMENT_SMOOTHING);
    rocketY = rocketY * MOVEMENT_SMOOTHING + newY * (1.0 - MOVEMENT_SMOOTHING);

    // Stop if very slow
    if (fabs(playerVX) < MIN_VELOCITY) playerVX = 0;
    if (fabs(playerVY) < MIN_VELOCITY) playerVY = 0;

    // Reset movement flags
    moveUp = moveDown = moveLeft = moveRight = false;
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


bool isPointInPolygon(double px, double py, Enemy* e) {

    int intersectionCount = 0;
    int sides = e->sides;
    
    for(int i=0; i<sides; i++){
        double theta1 = 2*PI*i/sides+e->angle;
        double theta2 = 2*PI*(i+1)/sides+e->angle;

        double x1 = e->x + e->radius * cos(theta1);
        double y1 = e->y + e->radius * sin(theta1);
        double x2 = e->x + e->radius * cos(theta2);
        double y2 = e->y + e->radius * sin(theta2);

        if(((y1 > py) != (y2 > py)) &&
           (px < (x2 - x1) * (py - y1) / (y2 - y1) + x1)) {
            intersectionCount++;
        }
    }

    return (intersectionCount % 2)==1;

    // int numSides = e.sides;
    // double *xCoords = (double *)malloc(numSides * sizeof(double));
    // double *yCoords = (double *)malloc(numSides * sizeof(double));

    // for (int j = 0; j < numSides; j++) {
    //     double theta = 2 * PI * j / numSides + e.angle;
    //     xCoords[j] = e.x + e.radius * cos(theta);
    //     yCoords[j] = e.y + e.radius * sin(theta);
    // }

    // bool inside = false;
    // for (int i = 0, j = numSides - 1; i < numSides; j = i++) {
    //     if (((yCoords[i] > py) != (yCoords[j] > py)) &&
    //         (px < (xCoords[j] - xCoords[i]) * (py - yCoords[i]) / (yCoords[j] - yCoords[i]) + xCoords[i])) {
    //         inside = !inside;
    //     }
    // }

    // free(xCoords);
    // free(yCoords);

    // return inside;
}

void updateProjectiles(double deltaTime){
    for(int i=0;i<projectileCount;i++){
        if(!projectiles[i].active) continue;

        projectiles[i].x += projectiles[i].vx;
        projectiles[i].y += projectiles[i].vy;

        projectiles[i].lifetime -= deltaTime;

        if(projectiles[i].lifetime <= 0 || projectiles[i].x < 0 || projectiles[i].x > winWidth || projectiles[i].y<0 || projectiles[i].y>winHeight){
            projectiles[i].active = false;
            continue;
        }
    }

    int activeCount = 0;
    for(int i = 0; i < projectileCount; i++) {
        if(projectiles[i].active) {
            if(i != activeCount) {
                projectiles[activeCount] = projectiles[i];
            }
            activeCount++;
        }
    }
    projectileCount = activeCount;
}

void checkProjectileCollisions() {
    for(int i = 0; i < projectileCount; i++) {
        if(!projectiles[i].active) continue;
        
        bool hitEnemy = false;
        for(int j = 0; j < enemyCount; j++) {
            if(!enemies[j].active) continue;
            
            if(isPointInPolygon(projectiles[i].x, projectiles[i].y, &enemies[j])) {
                damageEnemy(j);
                hitEnemy = true;
                break;
            }
        }
        
        if(hitEnemy) {
            projectiles[i].active = false;
        }
    }
}

void createSingleBullet(double startX, double startY, double angle) {
    if(projectileCount >= MAX_BULLET) return;
    
    Projectile* bullet = &projectiles[projectileCount++];
    bullet->x = startX;
    bullet->y = startY;
    bullet->angle = angle;
    bullet->vx = BULLET_SPEED * cos(angle);
    bullet->vy = BULLET_SPEED * sin(angle);
    bullet->active = true;
    bullet->lifetime = BULLET_LIFETIME;
}

void createBullet(double startX, double startY, double angle) {
    if (hasTripleShot) {
        // Create 3 bullets at different angles
        double spreadAngle = 0.2;  // About 11 degrees
        createSingleBullet(startX, startY, angle);
        createSingleBullet(startX, startY, angle + spreadAngle);
        createSingleBullet(startX, startY, angle - spreadAngle);
    } else {
        createSingleBullet(startX, startY, angle);
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


// void drawHeart(double x, double y, double size) {
//     int points = 30;
//     double angleStep = (2 * PI) / points;
//     double verticesX[30];
//     double verticesY[30];

//     for(int i = 0; i < points; i++) {
//         double theta = i * angleStep;
//         double scale = 16 * pow(sin(theta), 3);
//         double px = size * scale / 16;
//         double py = size * (13 * cos(theta) - 5 * cos(2 * theta) - 2 * cos(3 * theta) - cos(4 * theta)) / 16;
//         verticesX[i] = x + px;
//         verticesY[i] = y + py;
//     }

//     iSetColor(255, 0, 0);
//     iFilledPolygon(verticesX, verticesY, points);
// }


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
        updateProjectiles(deltaTime);
        checkProjectileCollisions();

        checkParticleCollisions();
        checkEnemies();
        updateParticles(deltaTime);
        drawParticles();
        updateGrid();
        updatePowerUps(deltaTime);
        drawPowerUps();
        drawNotification(deltaTime);
        drawPlayerHearts();

        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].active && isPlayerCollidingWithEnemy(enemies[i])) {
                if (!isInvincible) {
                    playerLives--;
                    printf("Player Hit! Lives Remaining: %d\n", playerLives);
                    isInvincible = true;
                    invincibleTime = 0.0;

                    if (playerLives <= 0) {
                        printf("Game Over! Restarting...\n");
                        playerLives = 3;  // Reset to 3 hearts
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

        // for(int i = 0; i < playerLives; i++) {
        //     drawHeart(20 + i * 40, winHeight - 40, 20); 
        // }
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

        if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        double currentTime = iGetTime();
        
            if(currentTime - lastShotTime >= shootInterval) {
                angle = atan2(my - rocketY, mx - rocketX);
                PlaySound(TEXT("bullet.wav"), NULL, SND_ASYNC);
                
                // Calculate bullet spawn position at rocket tip
                double tipX = rocketX + (rocketSize / 2) * cos(angle);
                double tipY = rocketY + (rocketSize / 2) * sin(angle);
                
                createBullet(tipX, tipY, angle);
                lastShotTime = currentTime;
            }
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
