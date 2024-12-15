#include "iGraphics.h"
#include "stdlib.h"
#include "time.h"
#include "math.h"
#include "windows.h"
#include "stdbool.h"
#include <stdio.h>
#include<bits/stdc++.h>
using namespace std;

// Window dimensions
const int winWidth = 800;
const int winHeight = 600;
int gamestate = 0;
extern double rocketX, rocketY;
double rocketX = 50;
double rocketY = winHeight / 2;

// Constants
#define PI 3.14159265358979323846
//bullet properties
#define BULLET_WIDTH 6.0;
#define BULLET_HEIGHT 20.0;
#define MAX_BULLET 50
#define BULLET_SPEED 0.5
#define BULLET_LIFETIME 2.0
#define AUTO_FIRE_COLOR_THRESHOLD 255
#define AUTO_FIRE_DURATION 10.0f
#define COLOR_GAIN_PER_KILL 15
//star properties
#define NUM_STARS 50
#define NUM_LAYERS 3
//particle properties
#define PARTICLE_POOL_SIZE 200
#define MAX_ACTIVE_PARTICLES 50
#define PARTICLE_CLEANUP_THRESHOLD 0.05
#define MAX_TRAIL_PER_BULLET 3
#define MAX_BULLET_TRAILS (MAX_BULLET * MAX_TRAIL_PER_BULLET)
//enemy properties
#define MAX_ENEMIES 50
#define MAX_WAVES 9
//grid properties
#define GRID_CELL_SIZE 64
#define GRID_WIDTH (winWidth/GRID_CELL_SIZE)
#define GRID_HEIGHT (winHeight/GRID_CELL_SIZE)
//player properties
#define PLAYER_ACCELERATION 0.4
#define PLAYER_MAX_SPEED 15.0
#define PLAYER_FRICTION 0.98
#define MOVEMENT_SMOOTHING 0.5
#define MIN_VELOCITY 0.01
//powerups
#define MAX_POWERUPS 5
#define POWERUP_SPAWN_CHANCE 20
#define POWERUP_SIZE 15.0
#define POWERUP_DURATION 10.0
//notifications
#define NOTIFICATION_DURATION 10.0
#define NOTIFICATION_Y 550
//healt bar properties
#define HEALTH_BAR_WIDTH 40.0
#define HEALTH_BAR_HEIGHT 5.0
#define HEALTH_BAR_OFFSET 20.0
#define MAX_PLAYER_HEALTH 100
#define MAX_FLOATING_TEXTS 50
//high scores
#define MAX_HIGH_SCORES 5
#define MAX_NAME_LENGTH 32
#define READ_FILE FILE* file = fopen("highscores.txt", "r");
#define WRITE_FILE FILE* file = fopen("highscores.txt", "w");
#define FILE_CLOSE fclose(file);


double shapeLevel1[3][2] = {
    {20, 0},
    {-10, 15},
    {-10, -15}
};

double shapeLevel4[4][2] = {
    {20,   0},   
    { 0,  15},   
    {-10, 0},     
    { 0, -15}    
};

double shapeLevel7[4][2] = {
    {20,   0},
    {0,   10},
    {-10,   0},
    {0,  -10}
};

double shapeLevel10[6][2] = {
    {20, 0},
    {5, 5},
    {-10, 20},
    {0, 0},
    {-10, -20},
    {5, -5}
};

double shapeLevel13[6][2] = {
    {20, 0},
    {5, 5},
    {-10, 20},
    {0, 0},
    {-10, -20},
    {5, -5}    
};

struct ScreenShake {
    double magnitude;    
    double duration;     
    double timer;        
    double offsetX;      
    double offsetY;      
} shake = {0, 0, 0, 0, 0};

typedef struct BulletTemplate {
    double vertices[8][2]; 
    int vertexCount;
    
    BulletTemplate() : vertexCount(8) {
        for(int i = 0; i < 8; i++) {
            vertices[i][0] = 0;
            vertices[i][1] = 0;
        }
    }
}BulletTemplate;


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
    double rotationSpeed;
    double duration;
    double lifetime;
    double initialSize;
    double currentSize;
}PowerUp;

typedef struct
{
    double x, y;
    double vx, vy;
    double lifetime;
    bool active;
    int r,g,b;
}ThrustParticle;

enum EnemyType {
    BASIC,
    FAST,
    STRONG,
    SHOOTER,
    BOSS
};

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
    EnemyType type;
    bool canShoot;
    double shootInterval;
    double shootTimer;
    double speed;
    int scoreValue;
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
    double damage;
} Projectile;

double rocketSize = 30;

typedef struct {
    bool active = false;
    double radius = rocketSize * 2.0; 
    double timer = 0.0;
    // int alpha = 255; 
} Shield;

struct AutoFire {
    bool active = false;
    double timer = 0.0;
    double shootTimer = 0.0;
    const double shootInterval = 0.15;
    bool isActive = false;
} autoFire;

struct ScoreSystem {
    int score;
    float multiplier;
    float comboTimer;
    int comboCount;
    const float COMBO_TIME = 3.0f;    
    const float MAX_MULTIPLIER = 5.0f; 
} scoring = {0, 1.0f, 0.0f, 0};

struct FloatingText {
    char text[32];
    double x, y;
    double duration;
    double timer;
    bool active;
    double alpha;
    double vy;  
};

typedef struct HighScoreEntry{
    char name[MAX_NAME_LENGTH];
    int score;
}HighScoreEntry;


struct BulletTrail {
    double x, y;
    double alpha;
    bool active;
    double size;
};

struct Point {
    double x;
    double y;
};
struct Bullet {
    double x, y;
    double vx, vy;
    double damage;
    bool active;

    void initialize(double startX, double startY, double targetX, double targetY) {
        x = startX;
        y = startY;
        double angle = atan2(targetY - y, targetX - x);
        double speed = 400.0;
        vx = cos(angle) * speed;
        vy = sin(angle) * speed;
        active = true;
    }

    void update(double deltaTime) {
        x += vx * deltaTime;
        y += vy * deltaTime;
        // Deactivate if out of bounds
        if (x < 0 || x > winWidth || y < 0 || y > winHeight) {
            active = false;
        }
    }

    void draw() {
        if (active) {
            iSetColor(255, 255, 0);
            iFilledCircle(x, y, 5);
        }
    }
};

struct TriangleEnemy {
    double x, y;
    double vx, vy;
    double health;
    bool active;
    int behaviorType; // 0-2: shoot, 3-5: chase

    void initialize(double startX, double startY, int type) {
        x = startX;
        y = startY;
        behaviorType = type;
        health = 20.0;
        active = true;
        double angle = type * (PI / 3);
        double speed = 150.0;
        vx = cos(angle) * speed;
        vy = sin(angle) * speed;
    }

    void update(double deltaTime) {
        if (!active) return;
        if (behaviorType < 3) {
            // Shooting behavior
            // Implement shooting logic if necessary
        } else {
            // Chasing behavior
            double angle = atan2(rocketY - y, rocketX - x);
            double speed = 150.0;
            vx = cos(angle) * speed;
            vy = sin(angle) * speed;
        }
        x += vx * deltaTime;
        y += vy * deltaTime;
        // Check for collisions or deactivate if off-screen
        if (x < 0 || x > winWidth || y < 0 || y > winHeight) {
            active = false;
        }
    }

    void draw() {
        if (active) {
            double size = 30.0;
            double pointsX[3] = { x, x - size / 2, x + size / 2 };
            double pointsY[3] = { y + size / 2, y - size / 2, y - size / 2 };
            iSetColor(0, 0, 255);
            iFilledPolygon(pointsX, pointsY, 3);
        }
    }

    bool isDefeated() {
        return health <= 0 || !active;
    }
};

class Boss {
public:
    void initialize();
    void update(double deltaTime);
    void draw();
    bool isDefeated();
    Bullet bullets[MAX_BULLET];
    double health;
    double x, y;
    double size;
    int bulletCount;
    static const int MAX_BULLETS = 100;

private:
    bool hasSplit;
    bool active;           
    double shootCooldown;
    double timeSinceLastShot;


    static const int MAX_TRIANGLES = 6;
    TriangleEnemy triangles[MAX_TRIANGLES];
    int triangleCount;

    void shootProjectiles();
    void splitIntoTriangles();
};

void Boss::initialize() {
    health = 100.0;
    hasSplit = false;
    x = winWidth / 2;
    y = winHeight + 100;
    size = 100.0;
    shootCooldown = 2.0;
    timeSinceLastShot = 0.0;
    bulletCount = 0;
    triangleCount = 0;
}

void Boss::update(double deltaTime) {
    // Move the boss into the screen
    if (y > winHeight - size) {
        y -= 100 * deltaTime;
    }

    if (!hasSplit) {
        // Shooting logic
        timeSinceLastShot += deltaTime;
        if (timeSinceLastShot >= shootCooldown) {
            shootProjectiles();
            timeSinceLastShot = 0.0;
        }
    } else {
        // Update triangles
        for (int i = 0; i < triangleCount; i++) {
            triangles[i].update(deltaTime);
        }
    }

    // Update bullets
    for (int i = 0; i < bulletCount; i++) {
        bullets[i].update(deltaTime);
        // Deactivate bullet if off-screen
        if (!bullets[i].active) {
            bullets[i] = bullets[--bulletCount];
            i--;
        }
    }

    // Check for health half
    if (health <= 50.0 && !hasSplit) {
        splitIntoTriangles();
        hasSplit = true;
    }
}

void Boss::draw() {
    if (!hasSplit) {
        // Draw big hexagon
        const int sides = 6;
        double hexX[sides], hexY[sides];
        for (int i = 0; i < sides; i++) {
            double theta = i * (2 * PI / sides);
            hexX[i] = x + size * cos(theta);
            hexY[i] = y + size * sin(theta);
        }
        iSetColor(255, 0, 0);
        iFilledPolygon(hexX, hexY, sides);
    } else {
        // Draw triangles
        for (int i = 0; i < triangleCount; i++) {
            triangles[i].draw();
        }
    }

    // Draw bullets
    for (int i = 0; i < bulletCount; i++) {
        bullets[i].draw();
    }
}

bool Boss::isDefeated() {
    if (!hasSplit) {
        return false;
    }
    // Check if all triangles are defeated
    for (int i = 0; i < triangleCount; i++) {
        if (!triangles[i].isDefeated()) {
            return false;
        }
    }
    return true;
}

void Boss::shootProjectiles() {
    if (bulletCount < MAX_BULLETS) {
        bullets[bulletCount].initialize(x, y, rocketX, rocketY);
        bullets[bulletCount].damage = 10;
        bulletCount++;
    }
}

void Boss::splitIntoTriangles() {
    // Split into six triangles
    triangleCount = MAX_TRIANGLES;
    for (int i = 0; i < triangleCount; i++) {
        triangles[i].initialize(x, y, i);
    }
}



struct TrailTemplate {
    vector<Point> points;
    int numPoints;
    double baseSize;
};

struct ThrustFlame {
    bool active;
    double baseWidth;
    double baseLength;
    double flickerTimer;
    double fadeTimer;    
    double fadeDelay;    
} thrust = {false, 10.0, 20.0, 0.0, 0.0, 0.2};

struct ComboSystem {
    int hits;
    double timer;
    int shotsRemaining;         
    const double RESET_TIME = 3.0;
    const int DUAL_SHOT = 2;
    const int TRIPLE_SHOT = 4;
    const int LASER_SHOT = 6;
    const int SHOTS_PER_COMBO = 5;  
} combo = {0, 0.0, 0};

enum BulletType {
    NORMAL = 0,
    DUAL = 1,
    TRIPLE = 2,
    LASER = 3
};

BulletType currentBulletType = BulletType::NORMAL;

struct ComboDisplay {
    char text[32];
    double alpha;
    bool active;
    double displayTime;
    const double DISPLAY_DURATION = 5.0;    
    const double FADE_DURATION = 2.0;      
} comboDisplay = {"", 255, false, 0.0};

struct Wave {
    // int enemyCount;
    int spawnedCount;
    double spawnInterval;
    // int formation; 
    int difficulty; 
    // vector<EnemyType> enemyTypes; 
};



GridCell grid[GRID_WIDTH][GRID_HEIGHT];
Star stars[NUM_LAYERS][NUM_STARS];
Enemy enemies[MAX_ENEMIES];
Particle particles[PARTICLE_POOL_SIZE];
Projectile projectiles[MAX_BULLET];
PowerUp powerups[MAX_POWERUPS];
BulletTemplate bulletTemplate;
Shield shield;
HighScoreEntry highScores[MAX_HIGH_SCORES];
BulletTrail bulletTrails[MAX_BULLET_TRAILS];
TrailTemplate trailTemplate;
int currentLevel = 1; 
int bulletTrailCount = 0;
bool isEnteringName = false;
char playerName[MAX_NAME_LENGTH] = "";
int nameIndex = 0;
int thrustParticleCount = 0;
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
bool gameover = false;
// Performance metrics
int playerHitsTaken = 0;
int totalShotsFired = 0;  
double accuracy = 1.0;
bool gameStarted = false;
double countdownTimer = 3.0;
const double COUNTDOWN_TIME = 3.0;
int currentScore = 0;
const double MIN_SPLIT_SIZE = 20.0;
double initialEnemySpawnRate = 2.0;
// Difficulty adjustment variables
double enemySpawnRate = 1; 
double minSpawnRate = 0.5;
double maxSpawnRate = 3.0;
bool enemySpawnEnabled= false;
bool gameWon = false;

// Timer to track when to spawn enemies
double spawnTimer = 0.0;

int getGridx(double x){return (int)(x/GRID_CELL_SIZE);}
int getGridy(double y){return (int)(y/GRID_CELL_SIZE);}


// Player Properties
double bulletWidth = 5;
double bulletLen = 15;

double playerVX = 0.0;
double playerVY = 0.0;


int rocketSpeed = 10;
int mouseX = rocketX, mouseY = rocketY + 100;
double angle;
bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;

//Player Healths
int playerLives = 3;
bool isInvincible = false;
double invincibleTime = 0.0;
double invincibleDuration = 2.0;
double playerRadius = rocketSize/2.0;
const double bulletRadius = 5.0;

// Level Tracking Variables
int enemiesDestroyed = 0;
int enemiesToNextLevel = 10; 
int playerLevel = 1;
int playerXP = 0;
int nextLevelXP = 4000;

// Level Bar Dimensions

int levelBarWidth = winWidth;
int levelBarHeight = 5;
int levelBarX = 0;
int levelBarY = 0;
// Wave waves[MAX_WAVES];
int currentWave = 0;
bool waveActive = false;
double waveSpawnTimer = 0.0;
double waveBreakTimer = 0.0;
const double WAVE_BREAK_TIME = 3.0;
double gameOverTimer = 0.0; 

//mainmenu
bool isInMainMenu = true;

double lastShotTime = 0.0;
double shootInterval = 0.1;

// Blinking Variables
bool isBlinking = false;
double blinkTime = 0.0;
double blinkDuration = 0.2;
double blinkInterval = 0.2;

FloatingText floatingTexts[MAX_FLOATING_TEXTS];

// Function Prototypes
// Game State & Initialization
void initStars();
void initBulletTemplate();
double iGetTime();
// Drawing Functions
void iDraw();
void drawPlayer();
void drawStars();
void drawEnemies();
void drawParticles();
void drawPowerUps();
void drawNotification(double deltaTime);
void drawThrustParticles();
void drawBullets();
// void drawAutoFireBar();
// Movement & Updates
void moveRocket();
void moveStars();
void moveEnemies();
void moveProjectiles();
void updateParticles(double deltaTime);
void updatePowerUps(double deltaTime);
void updateAutoFire(double deltaTime);
// void updateThrustParticles(double deltaTime);
void updateProjectiles(double deltaTime);
// Collision & Combat
void checkEnemies();
void checkParticleCollisions();
void checkProjectileCollisions();
bool isPointInPolygon(double px, double py, Enemy* e);
bool isPlayerCollidingWithEnemy(Enemy e);
void damageEnemy(int index);
void damagePlayer();
void createBullet(double x, double y, double angle);
void createSingleBullet(double x, double y, double angle);
// Spawning & Creation
void spawnEnemy(EnemyType type);
void spawnPowerUp(double x, double y);
void spawnEnemy();
void breakEnemy(int index);
void spawnRandomEnemyGroup();
// UI & Notifications
void showNotification(const char* text, double time);
// void onEnemyDestroyed();
// Input Handlers
void iMouse(int button, int state, int x, int y);
void iMouseMove(int x, int y);
void iKeyboard(unsigned char key);
void iSpecialKeyboard(unsigned char key);
// Grid Management
void updateGrid();
bool bossActive = false;
Boss boss;

void showWinningMessage() {
    gameWon = true;
    // Optionally, pause the game or stop updates
}
// Function Implementations

void handleGameOver() {
    if(gamestate != 1) return;
    gamestate = 7;  
    gameOverTimer = 0.0;

    for(int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
    for(int i = 0; i < MAX_BULLET; i++) {
        projectiles[i].active = false;
    }
    for(int i = 0; i < MAX_ACTIVE_PARTICLES; i++) {
        particles[i].active = false;
    }
}

void drawPlayer() {
    double (*shape)[2];
    int vertexCount = 0;

    // Select shape based on playerLevel
    if (playerLevel <= 3) {
        shape = shapeLevel1;
        vertexCount = 3;
    } else if (playerLevel <= 6) {
        shape = shapeLevel4;
        vertexCount = 4;
    } else if (playerLevel <= 9) {
        shape = shapeLevel7;
        vertexCount = 4;
    } else if (playerLevel <= 12) {
        shape = shapeLevel10;
        vertexCount = 6;
    } else {
        shape = shapeLevel13;
        vertexCount = 6;
    }

    // Rotate and translate
    double rotatedX[20], rotatedY[20];
    double cosA = cos(angle);
    double sinA = sin(angle);

    for (int i = 0; i < vertexCount; i++) {
        double x = shape[i][0];
        double y = shape[i][1];

        double rx = x * cosA - y * sinA;
        double ry = x * sinA + y * cosA;

        rotatedX[i] = rocketX + rx;
        rotatedY[i] = rocketY + ry;
    }

    if (isInvincible && isBlinking) {
        iSetColor(255, 0, 0);
    } else {
        if (playerLevel <= 3) {
            iSetColor(255, 255, 255); 
        } else if (playerLevel <= 6) {
            iSetColor(0, 200, 255); 
        } else if (playerLevel <= 9) {
            iSetColor(0, 255, 0); 
        } else if (playerLevel <= 12) {
            iSetColor(255, 128, 0); 
        } else {
            iSetColor(255, 255, 0); 
        }
    }

    iFilledPolygon(rotatedX, rotatedY, vertexCount);

    iSetColor(0, 0, 0); 
    iPolygon(rotatedX, rotatedY, vertexCount);
}


void drawLevelBar() {
    iSetColor(50, 50, 50); 
    iFilledRectangle(levelBarX, levelBarY, levelBarWidth, levelBarHeight);

    double fillPercentage = (double)playerXP / nextLevelXP;
    int filledWidth = (int)(levelBarWidth * fillPercentage);

    iSetColor(0, 255, 0); 
    iFilledRectangle(levelBarX, levelBarY, filledWidth, levelBarHeight);

    char levelText[50];
    sprintf(levelText, "Level: %d", playerLevel);
    iSetColor(255, 255, 255);
    iText(levelBarX + 10, levelBarY + levelBarHeight + 5, levelText, GLUT_BITMAP_HELVETICA_18);
}

void addPlayerXP(int points) {
    playerXP += points;
    if (playerXP >= nextLevelXP) {
        playerLevel++;
        playerXP -= nextLevelXP; 
        nextLevelXP *= 1.5;   
    }
}

void updateCountdown(double deltaTime) {
    if(gamestate != 1) return;
    
    if (!gameStarted) {
        countdownTimer -= deltaTime;
     
        char countdownText[20];
        if (countdownTimer > 2.0) {
            sprintf(countdownText, "Ready...");
        } else if (countdownTimer > 1.0) {
            sprintf(countdownText, "3");
        } else if (countdownTimer > 0.0) {
            sprintf(countdownText, "2");
        } else if (countdownTimer > -1.0) {
            sprintf(countdownText, "1");
        } else if(countdownTimer > -2.0) {
            sprintf(countdownText, "Survive!");
        }else {
            gameStarted = true;
            return;
        }

        if (!gameStarted) {
            iSetColor(255, 178, 0);  
            int textX = winWidth/2 - 50;  
            int textY = winHeight/2;     
            iText(textX, textY, countdownText, GLUT_BITMAP_TIMES_ROMAN_24);
        }
    }
}

void adjustDifficulty() {
    if(gamestate != 1) return;
    if(!gameStarted) return;
    if (totalShotsFired == 0) return; 

    accuracy = (double)enemiesDestroyed / totalShotsFired;

    if (accuracy > 0.7 && playerHitsTaken == 0) {
        enemySpawnRate = max(minSpawnRate, enemySpawnRate - 0.2);
        // currentDifficulty += 0.1;
    } else if (accuracy < 0.3 || playerHitsTaken > 3) {
        enemySpawnRate = min(maxSpawnRate, enemySpawnRate + 0.2);
        // currentDifficulty -= 0.1;
    }
    
    // if (currentDifficulty < minDifficulty) 
    //     currentDifficulty = minDifficulty;
    // if (currentDifficulty > maxDifficulty) 
    //     currentDifficulty = maxDifficulty;

    if (enemiesDestroyed % 10 == 0 && enemiesDestroyed > 0) {
        totalShotsFired = 0;
        enemiesDestroyed = 0;
        playerHitsTaken = 0;
    }
    // enemySpawnRate = initialEnemySpawnRate / currentDifficulty;
}

void updateEnemySpawning(double deltaTime) {
    if(gamestate != 1) return;
    if(!gameStarted) return;
    spawnTimer += deltaTime;
    if (spawnTimer >= enemySpawnRate) {
        // printf("Spawning enemies now.\n");
        spawnRandomEnemyGroup(); 

        double randomAdjustment = ((rand() % 100) / 100.0) - 0.5; 
        enemySpawnRate += randomAdjustment;
        enemySpawnRate = max(minSpawnRate, min(enemySpawnRate, maxSpawnRate));

        spawnTimer = 0.0;
    }
}

void spawnRandomEnemyGroup() {
    if(gamestate != 1) return;
    if(!gameStarted) return;
    int groupSize = rand() % 3 + 3; 

    for (int i = 0; i < groupSize; i++) {
        EnemyType type;
        int typeRoll = rand() % 100;

        if (typeRoll < 50) type = BASIC;
        else if (typeRoll < 80) type = FAST;
        else type = STRONG;

        spawnEnemy(type); 
    }
}


void resetGameState() {
    enemyCount = 0;
    projectileCount = 0;
    particleCount = 0;
    bulletTrailCount = 0;

    gameStarted = false;
    countdownTimer = COUNTDOWN_TIME;
    
    rocketX = winWidth / 2;
    rocketY = 50;
    playerHealth = 100;
    playerLives = 3; 

    combo.hits = 0;
    combo.timer = 0;
    comboDisplay.active = false;
    autoFire.active = false;
    hasTripleShot = false;
 
    spawnTimer = 0.0;
    enemySpawnRate = 2.0;

    playerLevel = 1;
    playerXP = 0;
}

void updateComboDisplay(double deltaTime) {
    if (comboDisplay.active) {
        comboDisplay.displayTime += deltaTime;
  
        if (comboDisplay.displayTime > comboDisplay.DISPLAY_DURATION - comboDisplay.FADE_DURATION) {
            double fadeProgress = (comboDisplay.displayTime - (comboDisplay.DISPLAY_DURATION - comboDisplay.FADE_DURATION)) 
                                / comboDisplay.FADE_DURATION;
            comboDisplay.alpha = 255 * (1.0 - fadeProgress);
        }

        if (comboDisplay.displayTime >= comboDisplay.DISPLAY_DURATION) {
            comboDisplay.active = false;
        }
    }
}

void gameOver() {
    gameWon = true; // You'll need to manage this variable
    // Optionally, display a "Game Over" message
    iSetColor(255, 0, 0);
    iText(winWidth / 2 - 50, winHeight / 2, "Game Over", GLUT_BITMAP_TIMES_ROMAN_24);
    // Stop any timers or game updates if necessary
}

void drawThrustFlame(double angle) {
    double backX = rocketX - cos(angle) * (rocketSize * 0.4);
    double backY = rocketY - sin(angle) * (rocketSize * 0.4);

    double fadeAlpha = min(1.0, thrust.fadeTimer/thrust.fadeDelay);
    
    double flameLength = thrust.baseLength * (0.8 + 0.4 * sin(thrust.flickerTimer * 10));
    double flameWidth = thrust.baseWidth * (0.8 + 0.3 * cos(thrust.flickerTimer * 8)); 

    double x1 = backX - cos(angle) * flameLength;
    double y1 = backY - sin(angle) * flameLength;
    
    double x2 = backX + cos(angle + PI/2) * (flameWidth * 0.7); 
    double y2 = backY + sin(angle + PI/2) * (flameWidth * 0.7);
    
    double x3 = backX + cos(angle - PI/2) * (flameWidth * 0.7);
    double y3 = backY + sin(angle - PI/2) * (flameWidth * 0.7);

    for(int i = 0; i < 3; i++) {
        double scale = 1.0 - (i * 0.3);
        iSetColor((int)(255 * scale), (int)(255 * scale * 0.7), 0);  
        
        double px1 = x1;
        double py1 = y1;
        double px2 = backX + (x2 - backX) * scale;
        double py2 = backY + (y2 - backY) * scale;
        double px3 = backX + (x3 - backX) * scale;
        double py3 = backY + (y3 - backY) * scale;
        
        double flameX[] = {px1, px2, px3};
        double flameY[] = {py1, py2, py3};
        iFilledPolygon(flameX, flameY, 3);
    }
}

void initTrailTemplate() {
    trailTemplate.numPoints = 20;
    trailTemplate.baseSize = 3.0;
    trailTemplate.points.resize(trailTemplate.numPoints);

    for(int i = 0; i < trailTemplate.numPoints; i++) {
        double angle = (2 * PI * i) / trailTemplate.numPoints;
        trailTemplate.points[i].x = cos(angle);
        trailTemplate.points[i].y = sin(angle);
    }
}

void createBulletTrail(double x, double y) {
    if (bulletTrailCount >= MAX_BULLET_TRAILS) return;
    
    BulletTrail* trail = &bulletTrails[bulletTrailCount++];
    trail->x = x;
    trail->y = y;
    trail->alpha = 255;
    trail->active = true;
	int s = BULLET_WIDTH;
    trail->size = s/4.0;
}

void updateBulletTrails() {
    int activeCount = 0;
    for (int i = 0; i < bulletTrailCount; i++) {
        if (!bulletTrails[i].active) continue;
        
        bulletTrails[i].alpha -= 15;  // Faster fade
        
        if (bulletTrails[i].alpha <= 0) {
            bulletTrails[i].active = false;
        } else {
            if (i != activeCount) {
                bulletTrails[activeCount] = bulletTrails[i];
            }
            activeCount++;
        }
    }
    bulletTrailCount = activeCount;
}

void drawBulletTrails() {
    for (int i = 0; i < bulletTrailCount; i++) {
        if (!bulletTrails[i].active) continue;
        
        iSetColor(255, 138, 0);
        double fadeSize = bulletTrails[i].size * (bulletTrails[i].alpha / 255.0);

        double* xPoints = new double[trailTemplate.numPoints];
        double* yPoints = new double[trailTemplate.numPoints];
        
        for(int j = 0; j < trailTemplate.numPoints; j++) {
            xPoints[j] = bulletTrails[i].x + trailTemplate.points[j].x * fadeSize;
            yPoints[j] = bulletTrails[i].y + trailTemplate.points[j].y * fadeSize;
        }
        
        iFilledPolygon(xPoints, yPoints, trailTemplate.numPoints);
        delete[] xPoints;
        delete[] yPoints;
    }
}

// void handleGameOver() {
//     if(gamestate != 1) return;
//     resetGameState();

//     isEnteringName = true;
//     nameIndex = 0;
//     playerName[0] = '\0';
//     gamestate = 6;  

//     for(int i = 0; i < MAX_ENEMIES; i++) {
//         enemies[i].active = false;
//     }
//     for(int i = 0; i < MAX_BULLET; i++) {
//         projectiles[i].active = false;
//     }
//     for(int i = 0; i < MAX_ACTIVE_PARTICLES; i++) {
//         particles[i].active = false;
//     }
// }

void saveHighScores(){
	WRITE_FILE
	if(file){
		for(int i=0;i<MAX_HIGH_SCORES;i++){
			if (strlen(highScores[i].name) > 0) {
                fprintf(file, "%s %d\n", highScores[i].name, highScores[i].score);
            }
		}
		FILE_CLOSE
	}
}

void sortHighScores() {
    for (int i = 0; i < MAX_HIGH_SCORES - 1; i++) {
        for (int j = 0; j < MAX_HIGH_SCORES - i - 1; j++) {
            if (highScores[j].score < highScores[j + 1].score) {
                HighScoreEntry temp = highScores[j];
                highScores[j] = highScores[j + 1];
                highScores[j + 1] = temp;
            }
        }
    }
    saveHighScores();
}

void loadHighScores(){
    READ_FILE
    if(file){
		char line[100];
		int index = 0;

		while(fgets(line, sizeof(line), file) && index<MAX_HIGH_SCORES){
			int len = strlen(line);
			int lastSpace = len-1;
			while(lastSpace>=0 && line[lastSpace] != ' '){
				lastSpace--;
			}

			if(lastSpace > 0){
				strncpy(highScores[index].name, line, lastSpace);
				highScores[index].name[lastSpace] = '\0';
				highScores[index].score = atoi(line+lastSpace+1);
				index++;
			}
		}
		
		fclose(file);
		sortHighScores();
    }
}

void insertHighScore(const char* name, int score){
	HighScoreEntry newEntry;
	strcpy(newEntry.name, name);
	newEntry.score = score;
    currentScore = newEntry.score;
    printf("Current Score: %d\n", currentScore);

	int i;
	for(int i=0;i<MAX_HIGH_SCORES;i++){
		if(score>highScores[i].score){
			for(int j=MAX_HIGH_SCORES-1; j>i; j--){
				highScores[j] = highScores[j-1];
			}
			highScores[i] =  newEntry;
			break;
		}
	}
	saveHighScores();
}

void handleNameInput(unsigned char key){
    // iShowBMP(0, 0, "scorebar.bmp");
	if(isEnteringName){
		if(key == '\r'){
			playerName[nameIndex] = '\0';
			insertHighScore(playerName, scoring.score);
			isEnteringName = false;
			gamestate = 3;
		}
		else if(key == '\b' && nameIndex>0){
			nameIndex--;
			playerName[nameIndex] = '\0';
		}
		else if(nameIndex<MAX_NAME_LENGTH-1 && isalnum(key) || key==' '){
			playerName[nameIndex] = key;
			nameIndex++;
			playerName[nameIndex] = '\0';
		}
        // scoring.score = 0;
	}
}

void formatScore(int score, char* buffer) {
    char temp[32];
    sprintf(temp, "%d", score);
    int len = strlen(temp);
    int j = 0;
    int comma = len % 3;
    
    for(int i = 0; i < len; i++) {
        if(i != 0 && i % 3 == comma) {
            buffer[j++] = ',';
        }
        buffer[j++] = temp[i];
    }
    buffer[j] = '\0';
}

void drawHighScore(){
    iShowBMP(0, 0, "scorebar.bmp");
    iSetColor(255, 255, 255);
    char scoreText[64];
    sprintf(scoreText, "Your Score: %d", currentScore);
    iText(winWidth/2-100, 150, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);
    // printf("Your Score was: %d\n", scoring.score);
	iSetColor(0, 0, 0);
	iText(winWidth/2-100, winHeight-120, "HIGH SCORES", GLUT_BITMAP_TIMES_ROMAN_24);
    iLine(winWidth/2-100, winHeight-130, winWidth/2+80, winHeight-130);
    iLine(winWidth/2-100, winHeight-140, winWidth/2+80, winHeight-140);

	for(int i=0;i<MAX_HIGH_SCORES; i++){
		char scoreText[64];
		char formattedScore[44];
		formatScore(highScores[i].score, formattedScore);
		sprintf(scoreText, "%d. %s: %s", i+1, highScores[i].name, formattedScore);
		iText(winWidth/2-100, winHeight-200-i*50, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);
        iLine(winWidth/2-100, winHeight-210-i*50, winWidth/2+100, winHeight-210-i*50);
	}
    
    iSetColor(0, 51, 0);
	iText(winWidth/2-200, 100, "Press ESC to return to main menu", GLUT_BITMAP_TIMES_ROMAN_24);
    scoring.score = 0;
}

void drawNamePrompt(){
	iSetColor(255, 255, 255);
	iText(winWidth/2-150, winHeight/2+50, "Enter your name:", GLUT_BITMAP_TIMES_ROMAN_24);
	iText(winWidth/2-100, winHeight/2, playerName, GLUT_BITMAP_TIMES_ROMAN_24);
}



void drawScoreboard() {
    char scoreText[64];
    char formattedScore[32];
    formatScore(scoring.score, formattedScore);
    sprintf(scoreText, "Score: %s", formattedScore);
    
    iSetColor(255, 255, 255);
    iText(winWidth - 200, winHeight - 30, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);
}


void showFloatingText(double x, double y, const char* text) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!floatingTexts[i].active) {
            strcpy(floatingTexts[i].text, text);
            floatingTexts[i].x = x;
            floatingTexts[i].y = y;
            floatingTexts[i].duration = 3; 
            floatingTexts[i].timer = 3;    
            floatingTexts[i].active = true;
            floatingTexts[i].alpha = 255;
            floatingTexts[i].vy = 15;      
            break;
        }
    }
}

void updateFloatingTexts(double deltaTime) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (floatingTexts[i].active) {
            floatingTexts[i].timer -= deltaTime;
            floatingTexts[i].y += floatingTexts[i].vy * deltaTime;
            floatingTexts[i].alpha = (floatingTexts[i].timer / floatingTexts[i].duration) * 255;
            
            if (floatingTexts[i].timer <= 0) {
                floatingTexts[i].active = false;
            }
        }
    }
}

int calculatePoints(Enemy* enemy) {
    int basePoints = 100;

    double distance = sqrt(pow(enemy->x - rocketX, 2) + pow(enemy->y - rocketY, 2));
    int distanceBonus = (int)(distance * 0.1);

    int speedBonus = (int)(enemy->rotationSpeed * 50);
    
    int points = (int)((basePoints + distanceBonus + speedBonus) * scoring.multiplier);

    scoring.comboCount++;
    scoring.comboTimer = scoring.COMBO_TIME;
    scoring.multiplier = min(scoring.MAX_MULTIPLIER, 1.0f + (scoring.comboCount * 0.1f));
    
    return points;
}

void updateScoring(double deltaTime) {
    if (scoring.comboTimer > 0) {
        scoring.comboTimer -= deltaTime;
        if (scoring.comboTimer <= 0) {
            scoring.comboCount = 0;
            scoring.multiplier = 1.0f;
        }
    }
}

void updateScreenShake(double deltaTime) {
    if (shake.timer > 0) {
        shake.timer -= deltaTime;

        shake.offsetX = (rand() % 100 - 50) * 0.01 * shake.magnitude;
        shake.offsetY = (rand() % 100 - 50) * 0.01 * shake.magnitude;

        shake.magnitude *= 0.9;
    } else {
        shake.offsetX = 0;
        shake.offsetY = 0;
        shake.magnitude = 0;
    }
}

void triggerScreenShake(double magnitude, double duration) {
    shake.magnitude = magnitude;
    shake.duration = duration;
    shake.timer = duration;
}

void activateShield() {
    shield.active = true;
    shield.timer = POWERUP_DURATION;
    isInvincible = true;

    for(int i = 0; i < enemyCount; i++) {
        if(!enemies[i].active) continue;
        
        double dx = enemies[i].x - rocketX;
        double dy = enemies[i].y - rocketY;
        double distance = sqrt(dx*dx + dy*dy);
        
        if(distance < shield.radius) {
            enemies[i].health = 0;
            breakEnemy(i);
            i--;
        }
    }
}

void drawShield() {
    if(!shield.active) return;

    iSetColor(100, 150, 255);
    iCircle(rocketX, rocketY, shield.radius);
}

void handleShieldCollision(Enemy* e) {
    if(!shield.active) return;
    
    double dx = e->x - rocketX;
    double dy = e->y - rocketY;
    double distance = sqrt(dx*dx + dy*dy);
    
    if(distance < shield.radius + e->radius) {
        double nx = dx/distance; 
        double ny = dy/distance;
        
        e->x = rocketX + (shield.radius + e->radius) * nx;
        e->y = rocketY + (shield.radius + e->radius) * ny;

        double dotProduct = e->vx*nx + e->vy*ny;
        e->vx = e->vx - 2*dotProduct*nx;
        e->vy = e->vy - 2*dotProduct*ny;

        double bounceMagnitude = 0.2;
        e->vx *= bounceMagnitude;
        e->vy *= bounceMagnitude;
    }
}

void updateShield(double deltaTime) {
    if (!shield.active) return;
    
    shield.timer -= deltaTime;
    
    if (shield.timer <= 0) {
        shield.active = false;
        isInvincible = false;
        showNotification("Shield Deactivated!", 10.0);
    }
}

void initBulletTemplate() {
    double length = bulletLen * 1.5;
    double width = bulletWidth * 1;
    double finSize = width * 0.8;

    bulletTemplate.vertexCount = 8;
    
    // Nose
    bulletTemplate.vertices[0][0] = length;
    bulletTemplate.vertices[0][1] = 0;
    
    // Right side
    bulletTemplate.vertices[1][0] = length * 0.7;
    bulletTemplate.vertices[1][1] = width/2;
    
    bulletTemplate.vertices[2][0] = length * 0.3;
    bulletTemplate.vertices[2][1] = width/2;
    
    // Right fin
    bulletTemplate.vertices[3][0] = 0;
    bulletTemplate.vertices[3][1] = finSize;
    
    // Left fin
    bulletTemplate.vertices[4][0] = 0;
    bulletTemplate.vertices[4][1] = -finSize;
    
    // Left side
    bulletTemplate.vertices[5][0] = length * 0.3;
    bulletTemplate.vertices[5][1] = -width/2;
    
    bulletTemplate.vertices[6][0] = length * 0.7;
    bulletTemplate.vertices[6][1] = -width/2;
    
    // Back to nose
    bulletTemplate.vertices[7][0] = length;
    bulletTemplate.vertices[7][1] = 0;
}

void drawBullets() {
    for (int i = 0; i < projectileCount; i++) {
        if (!projectiles[i].active) continue;
        
        iSetColor(255, 255, 255);
        double transformedX[8], transformedY[8];

        for (int j = 0; j < bulletTemplate.vertexCount; j++) {
            double tempX = bulletTemplate.vertices[j][0];
            double tempY = bulletTemplate.vertices[j][1];
            
            transformedX[j] = projectiles[i].x + 
                tempX * cos(projectiles[i].angle) - 
                tempY * sin(projectiles[i].angle);
                
            transformedY[j] = projectiles[i].y + 
                tempX * sin(projectiles[i].angle) + 
                tempY * cos(projectiles[i].angle);
        }
        
        iFilledPolygon(transformedX, transformedY, bulletTemplate.vertexCount);
    }
}



void updateAutoFire(double deltaTime) {
    if (!autoFire.active) return;
    
    autoFire.timer -= deltaTime;
    autoFire.shootTimer -= deltaTime;

    if (autoFire.isActive && autoFire.shootTimer <= 0) {
        double angle = atan2(mouseY - rocketY, mouseX - rocketX);
        double tipX = rocketX + (rocketSize/2) * cos(angle);
        double tipY = rocketY + (rocketSize/2) * sin(angle);
        createSingleBullet(tipX, tipY, angle);
        // printf("Auto-Fire, Bullet created\n");
        autoFire.shootTimer = autoFire.shootInterval;
    }

    if (autoFire.timer <= 0) {
        autoFire.active = false;
        showNotification("Auto-Fire Deactivated!", 3);
    }
}

void drawPlayerHearts() {
    double heartSize = 8.0; 
    double spacing = 2.0; 
    double baseY = rocketY + rocketSize + 10; 
    
    double totalWidth = (playerLives*heartSize*2) + ((playerLives-1)*spacing);
    double startX = rocketX - totalWidth/2;

    for(int i = 0; i < playerLives; i++) {
        double heartX = startX + i*(heartSize*2 + spacing);
        double heartY = baseY;
        
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
        
        iSetColor(255, 0, 0); 
        iFilledPolygon(verticesX, verticesY, points);
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
    p->lifetime = 10.0;
    p->initialSize = POWERUP_SIZE;
    p->currentSize = POWERUP_SIZE;
}


void showNotification(const char* text, double time) {
    strcpy(currentNotification.text, text);
    currentNotification.timer = time;
    currentNotification.active = true;
    currentNotification.alpha = 255;
}


void updatePowerUps(double deltaTime) {
    int speed;
    if (powerUpTimer > 0) {
        powerUpTimer -= deltaTime;
        if (powerUpTimer <= 0) {
            hasTripleShot = false;
            hasSpeedBoost = false;
            hasRapidFire = false;
            
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

    for (int i = 0; i < powerupCount; i++) {
        if (!powerups[i].active) continue;

        powerups[i].lifetime -= deltaTime;
        double lifeRatio = powerups[i].lifetime/30.0;
        powerups[i].currentSize = powerups[i].initialSize * (0.5 + (0.5 * lifeRatio));

        if (powerups[i].lifetime <= 0) {
            powerups[i] = powerups[--powerupCount];
            i--;
            continue;
        }
        
        powerups[i].angle += powerups[i].rotationSpeed;
        
        double dx = powerups[i].x - rocketX;
        double dy = powerups[i].y - rocketY;
        double distance = sqrt(dx * dx + dy * dy);
        
        if (distance < (POWERUP_SIZE + playerRadius)) {
            switch (powerups[i].type) {
                case TRIPLE_SHOT:
                    hasTripleShot = true;
                    showNotification("Triple Shot Activated!", 10.0);
                    break;
                case SPEED_BOOST:
                    hasSpeedBoost = true;
                    showNotification("Speed Boost Activated!", 10.0);
                    speed=25;
                    break;
                case SHIELD:
                    shieldActive = true;
                    // hasShield = true;
                    isInvincible = true;
                    shieldTimer = POWERUP_DURATION;
                    activateShield();
                    showNotification("Shield Activated!", 10.0);
                    break;
                case RAPID_FIRE:
                    autoFire.active = true;
                    autoFire.timer = POWERUP_DURATION;
                    autoFire.shootTimer = 0.0;
                    autoFire.isActive = true;
                    showNotification("Auto-Fire Activated!", 10.0);
                    break;
                case HEALTH_BOOST:
                    playerLives = min(playerLives + 1, 5);
                    showNotification("Health Restored!", 2.0);
                    break;
            }

            powerups[i].active = false;
            
            powerUpTimer = POWERUP_DURATION;
            powerups[i] = powerups[--powerupCount];
            i--;
        }
    }
}

void drawNotification(double deltaTime) {
    if (currentNotification.active) {
        currentNotification.timer -= deltaTime;

        int colorValue;
        if (currentNotification.timer <= 1.0) {
            colorValue = (int)(255 * (currentNotification.timer / 1.0));
        } else {
            colorValue = 255;
        }

        colorValue = max(0, min(255, colorValue));
        
        if (currentNotification.timer <= 0) {
            currentNotification.active = false;
        } else {
            iSetColor(colorValue, colorValue, colorValue);
            iText(winWidth/2 - 100, NOTIFICATION_Y, currentNotification.text, GLUT_BITMAP_TIMES_ROMAN_24);

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
        
        double size = POWERUP_SIZE;
        int sides = 6;
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


void spawnEnemy(EnemyType type) {
    if (enemyCount >= MAX_ENEMIES) return;
    if(bossActive) return;

    // printf("Spawning enemy of type %d\n", type);

    enum SpawnEdge { TOP, BOTTOM, LEFT, RIGHT };
    SpawnEdge edge = (SpawnEdge)(rand() % 4);

    double startX, startY;
    switch (edge) {
        case TOP:
            startX = rand() % winWidth;
            startY = winHeight;
            break;
        case BOTTOM:
            startX = rand() % winWidth;
            startY = -50;
            break;
        case LEFT:
            startX = -50;
            startY = rand() % winHeight;
            break;
        case RIGHT:
            startX = winWidth;
            startY = rand() % winHeight;
            break;
    }

    Enemy* e = &enemies[enemyCount++];
    e->x = startX;
    e->y = startY;
    e->type = type;
    e->active = true;
    
    e->radius = 15.0;
    e->health = 1;
    e->speed = 2.0;

    double levelMultiplier = 1.0 + (playerLevel - 1) * 0.1;

    double speedAdjustment = ((rand() % 20) / 100.0) - 0.1;

    switch(type) {
        case BASIC:
            e->health = (int)(e->health * levelMultiplier);;
            e->speed = 0.08 + speedAdjustment;
            e->radius = 15 + (rand() % 10);
            e->sides = 3;
            e->canShoot = false;\
            e->r = 155+rand()%100; e->g = 155+rand()%100; e->b = 255;
            break;
        case FAST:
            e->health = 1;
            e->speed = 0.12 + speedAdjustment;
            e->radius = 10 + (rand() % 5);
            e->sides = 4;
            e->canShoot = false;
            e->b = 155+rand()%100; e->g = 155+rand()%100; e->r = 255;
            break;
        case STRONG:
            e->health = 3;
            e->speed = 0.05 + speedAdjustment;
            e->radius = 25 + (rand() % 10);
            e->sides = 5;
            e->canShoot = false;
            e->b = 155+rand()%100; e->r = 155+rand()%100; e->g = 255;
            break;
        case SHOOTER:
            e->health = 2;
            e->speed = 0.05 + speedAdjustment;
            e->radius = 22 + (rand() % 10);
            e->sides = 6;
            e->canShoot = true;
            e->shootInterval = 2.0;
            e->shootTimer = 0;
            e->b = 155+rand()%100; e->r = 155+rand()%100; e->g = 255+rand()%100;
            break;
    }

    double dx = rocketX - e->x;
    double dy = rocketY - e->y;
    double length = sqrt(dx*dx + dy*dy);
    e->vx = (dx/length) * e->speed;
    e->vy = (dy/length) * e->speed;

    e->rotationSpeed = ((rand() % 10) - 5) * 0.001;

    // e->r = rand() % 200 + 56;
    // e->g = rand() % 100;
    // e->b = rand() % 100;
}


void moveEnemies() {
    if(gamestate != 1) return;
    if(bossActive) return;
    for (int i = 0; i < enemyCount; i++) {

        if (enemies[i].active) {
            Enemy* e = &enemies[i];

            if (e->type == FAST) {
                double dx = rocketX - e->x;
                double dy = rocketY - e->y;
                double length = sqrt(dx*dx + dy*dy);
                if (length > 0) {
                    e->vx = (dx/length) * e->speed;
                    e->vy = (dy/length) * e->speed;
                }
            }

            e->x += e->vx;
            e->y += e->vy;
            e->angle += e->rotationSpeed;

            handleShieldCollision(e);

            if (e->x + e->radius < 0 || e->x - e->radius > winWidth ||
                e->y + e->radius < 0 || e->y - e->radius > winHeight) {
                
                enemies[i] = enemies[enemyCount - 1];
                enemyCount--;
                i--;  
            }
        }
    }
}

void breakEnemy(int index) {
    if (!enemies[index].active || enemies[index].health > 0) return;
    
    Enemy *e = &enemies[index];
 
    int points = calculatePoints(e);
    scoring.score += points;
    triggerScreenShake(10.0, 0.3);
    spawnPowerUp(e->x, e->y);
    addPlayerXP(points);

    // if (e->radius > MIN_SPLIT_SIZE) {
    //     splitEnemy(e);
    // }
    
    char scoreText[32];
    sprintf(scoreText, "+%d", points);
    showFloatingText(e->x, e->y, scoreText);

    double oldX = e->x;
    double oldY = e->y;
    double oldRadius = e->radius;
    int oldSides = e->sides;

    enemies[index] = enemies[enemyCount - 1];
    enemyCount--;

    if (oldRadius >= 30) {
        for (int i = 0; i < 2; i++) {
            if (enemyCount < MAX_ENEMIES) {
                Enemy *newEnemy = &enemies[enemyCount];
                newEnemy->radius = oldRadius / 2;
                newEnemy->sides = oldSides;
                newEnemy->health = oldSides/2;
                newEnemy->active = true;
                newEnemy->angle = rand() % 360;
                newEnemy->rotationSpeed = ((rand() % 10) - 5) * 0.001;
                newEnemy->x = oldX + (rand() % 21 - 10);
                newEnemy->y = oldY + (rand() % 21 - 10);
                newEnemy->vx = ((rand() % 11) - 5) * 0.01;
                newEnemy->vy = ((rand() % 11) - 5) * 0.01;
                newEnemy->r = rand()%200 + 56;
                newEnemy->g = rand()%100;
                newEnemy->b = rand()%100;
                enemyCount++;
                // printf("enemyCount after spawning: %d\n", enemyCount);
            }
        }
    }

    double xCoords[6], yCoords[6];
    for (int j = 0; j < oldSides; j++) {
        double theta = 2 * PI * j / oldSides + e->angle;
        xCoords[j] = oldX + oldRadius * cos(theta);
        yCoords[j] = oldY + oldRadius * sin(theta);
    }

    int numParticlesPerVertex = 5;
    for (int v = 0; v < oldSides; v++) {
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

    // enemies[index].active = false;
    // if (index != enemyCount - 1) {
    //     enemies[index] = enemies[enemyCount - 1];
    // }
    // enemyCount--;
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
    const int maxParticles = fmin(particleCount, MAX_ACTIVE_PARTICLES);
    
    for (int i = 0; i < maxParticles; i++) {
        if (particles[i].active) {
            double lifespanRatio = fmax(0.0, fmin(1.0, particles[i].lifespan / 10.0));
            int colorValue = (int)((1.0 - lifespanRatio) * 255);
            
            iSetColor(255, colorValue, colorValue);
            iFilledCircle(particles[i].x, particles[i].y, particles[i].size);
        }
    }
}

void checkEnemies() {
    if(gamestate!=1) return;
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active && enemies[i].health <= 0) {
            breakEnemy(i);
            enemiesDestroyed++; 

            if (enemiesDestroyed >= enemiesToNextLevel) {
                currentLevel++;
                enemiesDestroyed = 0; 
                enemiesToNextLevel += 5; 
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

    double acceleration = PLAYER_ACCELERATION * deltaTime * 60.0;
    double friction = pow(PLAYER_FRICTION, deltaTime * 60.0);

    thrust.active = (moveUp || moveDown || moveLeft || moveRight);

    if (moveUp || moveDown || moveLeft || moveRight) {
        thrust.active = true;
        thrust.fadeTimer = thrust.fadeDelay;
    } else if (thrust.fadeTimer > 0) {
        thrust.fadeTimer -= deltaTime;
        thrust.active = true;
    } else {
        thrust.active = false;
    }

    if (thrust.active) {
        thrust.flickerTimer += deltaTime * 5.0; 
    }

    if (moveUp)    playerVY += acceleration;
    if (moveDown)  playerVY -= acceleration;
    if (moveLeft)  playerVX -= acceleration;
    if (moveRight) playerVX += acceleration;

    playerVX = fmax(-PLAYER_MAX_SPEED, fmin(PLAYER_MAX_SPEED, playerVX));
    playerVY = fmax(-PLAYER_MAX_SPEED, fmin(PLAYER_MAX_SPEED, playerVY));

    playerVX *= friction;
    playerVY *= friction;

    double newX = rocketX + playerVX;
    double newY = rocketY + playerVY;

    // First do the smoothing step
    double finalX = rocketX * MOVEMENT_SMOOTHING + newX * (1.0 - MOVEMENT_SMOOTHING);
    double finalY = rocketY * MOVEMENT_SMOOTHING + newY * (1.0 - MOVEMENT_SMOOTHING);

    // Now apply wrap-around AFTER smoothing
    if (finalX < 0) {
        finalX += winWidth;
    } else if (finalX > winWidth) {
        finalX -= winWidth;
    }

    if (finalY < 0) {
        finalY += winHeight;
    } else if (finalY > winHeight) {
        finalY -= winHeight;
    }

    rocketX = finalX;
    rocketY = finalY;

    moveUp = moveDown = moveLeft = moveRight = false;
}

void moveProjectiles() {
    for (int i = 0; i < projectileCount; i++) {
        projectiles[i].x += projectiles[i].vx;
        projectiles[i].y += projectiles[i].vy;
		
		createBulletTrail(projectiles[i].x, projectiles[i].y);

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
        combo.hits++;
        combo.timer = 0.0;
        combo.shotsRemaining = combo.SHOTS_PER_COMBO;
        
        double damageRatio = (double)(enemies[index].sides - enemies[index].health) / enemies[index].sides;
        
        enemies[index].g = (int)(damageRatio * 255.0);
        enemies[index].b = (int)(damageRatio * 255.0);
        
        if (enemies[index].g > 255) enemies[index].g = 255;
        if (enemies[index].b > 255) enemies[index].b = 255;
        if (enemies[index].g < 0) enemies[index].g = 0;
        if (enemies[index].b < 0) enemies[index].b = 0;

        sprintf(comboDisplay.text, "KILL x%d", combo.hits);
        comboDisplay.active = true;
        comboDisplay.displayTime = 0.0;
        comboDisplay.alpha = 255;
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
            if(!enemies[j].active || enemies[j].health <= 0) continue;

            double dx = enemies[j].x - projectiles[i].x;
            double dy = enemies[j].y - projectiles[i].y;
            double distSq = dx*dx + dy*dy;
            double radiusCheck = enemies[j].radius + (bulletWidth * 0.5);
            
            if (distSq <= radiusCheck*radiusCheck) {
                if(isPointInPolygon(projectiles[i].x, projectiles[i].y, &enemies[j])) {
                    if(enemies[j].active && enemies[j].health > 0) {
                        damageEnemy(j);
                        hitEnemy = true;
                        break;
                    }
                }
            }
        }

        if(bossActive && boss.health > 0 && projectiles[i].active) {
            double dx = boss.x - projectiles[i].x;
            double dy = boss.y - projectiles[i].y;
            double distSq = dx*dx + dy*dy;
            double radiusCheck = boss.size + (bulletWidth * 0.5);

            if (distSq <= radiusCheck*radiusCheck) {
                // Collision detected between projectile and boss
                boss.health -= projectiles[i].damage; // Apply damage to the boss
                projectiles[i].active = false;        // Deactivate the projectile
                triggerScreenShake(5.0, 0.1);         // Optional: add screen shake
                if(boss.health <= 0) {
                    // bossDefeated();
                    showWinningMessage();
                }
            }
        }
 
        if(hitEnemy) {
            projectiles[i].active = false;
            projectiles[i] = projectiles[--projectileCount];
        }
    }
}

void checkBossBulletCollisions() {
    for(int i = 0; i < boss.bulletCount; i++) {
        if(!boss.bullets[i].active) continue;

        double dx = boss.bullets[i].x - rocketX;
        double dy = boss.bullets[i].y - rocketY;
        double distSq = dx*dx + dy*dy;
        double radiusCheck = playerRadius + bulletRadius;

        if (distSq <= radiusCheck*radiusCheck) {
            // Collision detected between boss's bullet and player
            playerHealth -= boss.bullets[i].damage; // Apply damage to the player
            boss.bullets[i].active = false;         // Deactivate the bullet
            triggerScreenShake(5.0, 0.1);           // Optional: add screen shake
            if(playerHealth <= 0) {
                gameOver();
            }
        }
    }
}

void bossDefeated() {
    bossActive = false;     // Deactivate the boss
    showWinningMessage();   // Display the winning message
    // Add any additional logic needed when the boss is defeated
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
    bullet->damage = 10.0;
}

void createBullet(double startX, double startY, double angle) {
    double tipX = rocketX + cos(angle) * rocketSize;
    double tipY = rocketY + sin(angle) * rocketSize;
    
    double leftX = rocketX + cos(angle + 2.0944) * rocketSize;
    double leftY = rocketY + sin(angle + 2.0944) * rocketSize;
    
    double rightX = rocketX + cos(angle - 2.0944) * rocketSize;
    double rightY = rocketY + sin(angle - 2.0944) * rocketSize;

    double mainAngle = atan2(mouseY - tipY, mouseX - tipX);

    if (hasTripleShot) {
        createSingleBullet(leftX, leftY, mainAngle);
        createSingleBullet(rightX, rightY, mainAngle);
        createSingleBullet(tipX, tipY, mainAngle);
        return;
    }

    if (combo.shotsRemaining > 0) {
        if (combo.hits >= combo.LASER_SHOT) {
            createSingleBullet(tipX, tipY, mainAngle);
        } 
        else if (combo.hits >= combo.TRIPLE_SHOT) {
            createSingleBullet(leftX, leftY, mainAngle);
            createSingleBullet(rightX, rightY, mainAngle);
            createSingleBullet(tipX, tipY, mainAngle);
        } 
        else if (combo.hits >= combo.DUAL_SHOT) {
            createSingleBullet(leftX, leftY, mainAngle);
            createSingleBullet(rightX, rightY, mainAngle);
        } 
        else {
            createSingleBullet(tipX, tipY, mainAngle);
        }
        combo.shotsRemaining--;
    } 
    else {
        createSingleBullet(tipX, tipY, mainAngle);
    }
}

void updateCombo(double deltaTime) {
    if (combo.hits > 0) {
        combo.timer += deltaTime;
        if (combo.timer >= combo.RESET_TIME) {
            combo.hits = 0;
            combo.timer = 0.0;
            combo.shotsRemaining = 0;
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

bool f = false;
bool isGameMusicPlaying = false;
bool gameOverSoundPlayed = false;
bool aboutSound = false;
bool controlsSound = false;

void iDraw() {
    // if(gameWon) return;
    iClear();
    glTranslatef(shake.offsetX, shake.offsetY, 0);
    if (gamestate == 0) { // Main Menu
        iClear();
        isInMainMenu = true;
        if(!f){
            PlaySound(NULL, NULL, 0);
            PlaySound(TEXT("menusound.wav"), NULL, SND_ASYNC | SND_LOOP);
            f = true;
            gameOverSoundPlayed = false;
            controlsSound = false;
            aboutSound = false;
        }

        iShowBMP2(0, 0, "mainmenu.bmp", -1);
    }
    else if (gamestate == 1) { // Game
        if (!isGameMusicPlaying) {
            PlaySound(NULL, NULL, 0); 
            PlaySound(TEXT("play.wav"), NULL, SND_ASYNC | SND_LOOP);
            isGameMusicPlaying = true;
            f = false;
            gameOverSoundPlayed = false;
            controlsSound = false;
            aboutSound = false;
        }
        isInMainMenu = false;
        // iShowBMP2(0, 0, "menu.bmp", -1);
        static double lastFrameTime = 0;
        double currentTime = iGetTime();
        double deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        iClear();
        updateFloatingTexts(1.0/60.0);
        moveStars();
        drawStars();
        updateShield(deltaTime);
        drawShield();
		drawShield();

        for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
            if (floatingTexts[i].active) {
                int alpha = (int)floatingTexts[i].alpha;
                iSetColor(255, 255, 255); 
                
                iText(floatingTexts[i].x, floatingTexts[i].y, floatingTexts[i].text, GLUT_BITMAP_TIMES_ROMAN_24);
            }
        }

        angle = atan2(mouseY - rocketY, mouseX - rocketX);
        double size = rocketSize;
        double height = rocketSize*0.8660254;

        double localX[3] = {height/2, -height/2, -height/2};
        double localY[3] = {0, size / 2.0, -size / 2.0};


        double rotatedX[3], rotatedY[3];

        for (int i = 0; i < 3; i++) {
            double x = localX[i];
            double y = localY[i];
            rotatedX[i] = x * cos(angle) - y * sin(angle) + rocketX;
            rotatedY[i] = x * sin(angle) + y * cos(angle) + rocketY;
        }

        drawPlayer();

        if(thrust.active) {
            drawThrustFlame(angle);
        }

        if (comboDisplay.active) {
            iSetColor(255, 255, 0); 
            iText(20, winHeight - 40, comboDisplay.text, GLUT_BITMAP_HELVETICA_18);
        }

    	drawBulletTrails();
        drawBullets();
		updateBulletTrails();
        updateComboDisplay(1.0/60.0);
        adjustDifficulty();
        // updateWaveSystem(1.0/60.0);
        moveRocket();
        updateEnemySpawning(deltaTime);
        // updateRapidFire(deltaTime);
        moveProjectiles();
        updateAutoFire(deltaTime);
        updateCountdown(1.0/3000.0);
        updateCombo(1.0/60.0);
        moveEnemies();
        drawEnemies();
        updateProjectiles(deltaTime);
        checkProjectileCollisions();
        drawScoreboard();

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
                        handleGameOver();
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
                // iSetColor(rocketColor.r, rocketColor.g, rocketColor.b);
                // iSetColor(rocketColor.r, rocketColor.g, rocketColor.b);
                // iFilledPolygon(rotatedX, rotatedY, 3);
            }
        }

        if (scoring.score >= 5000 && !bossActive) {
        for (int i = 0; i < enemyCount; i++) {
            enemies[i].active = false;
            }
            enemySpawnEnabled = false;
            bossActive = true;
            boss.initialize();
        }

        if (bossActive) {
            boss.update(deltaTime);
            checkProjectileCollisions(); 
            checkBossBulletCollisions();
            if (boss.isDefeated()) {
                bossActive = false;
                showWinningMessage();
            }
        }else{
            checkProjectileCollisions();
        }

        if (bossActive && boss.isDefeated()) {
            bossActive = false;
            showWinningMessage();
        }

        if (bossActive) {
            boss.draw();
        }

        if (gameWon) {
            iSetColor(0, 255, 0);
            iText(winHeight / 2 - 50, winWidth / 2, "You Win!", GLUT_BITMAP_TIMES_ROMAN_24);
        }

        drawLevelBar();

        // iSetColor(255, 255, 255);
        // iRectangle(levelBarX - 1, levelBarY - 1, levelBarWidth + 2, levelBarHeight + 2);
        
        // double progressRatio = (double)enemiesDestroyed / enemiesToNextLevel;
        // if (progressRatio > 1.0) progressRatio = 1.0;
        // double filledWidth = progressRatio * levelBarWidth;

        // iSetColor(255, 255, 255);
        // iFilledRectangle(levelBarX, levelBarY, filledWidth, levelBarHeight);

        // char levelText[20];
        // sprintf(levelText, "Level: %d", currentLevel);
        // iSetColor(255, 255, 255); 
        // iText(levelBarX + levelBarWidth / 2.0 - 20, levelBarY + levelBarHeight + 5, levelText, GLUT_BITMAP_TIMES_ROMAN_24);

        // for(int i = 0; i < playerLives; i++) {
        //     drawHeart(20 + i * 40, winHeight - 40, 20); 
        // }
    }
    else if (gamestate == 2) { // About
        iClear();
        // isInMainMenu = false;
        if(!aboutSound){
            PlaySound(NULL, NULL, 0);
            PlaySound(TEXT("about.wav"), NULL, SND_ASYNC);
            aboutSound = true;
            f=false;
            gameOverSoundPlayed = false;
            controlsSound = false;
        }
        iShowBMP2(0, 0, "about3.bmp", -1);
    }
    else if(gamestate==3){
        iClear();
        // isInMainMenu = false;
		// sprintf("High scores\n", "%d",  scoring.score);
        // printf("High Scores\n");
        // printf("%d\n", scoring.score);
		drawHighScore();
    }
    else if(gamestate==4){
        iClear();
        // isInMainMenu = false;
        if(!controlsSound){
            PlaySound(NULL, NULL, 0);
            PlaySound(TEXT("controls.wav"), NULL, SND_ASYNC);
            controlsSound = true;
            f=false;
            gameOverSoundPlayed = false;
            aboutSound = false;
        }
        iShowBMP2(0, 0, "controls.bmp", -1);
    }
    else if(gamestate==5){
        iClear();
        // isInMainMenu = false;
        iShowBMP2(0, 0, "esc.bmp", -1);
    }else if(gamestate==6){
		iClear();
		drawNamePrompt();
        // scoring.score = 0;
	}else if(gamestate == 7){
        iClear();
        if (!gameOverSoundPlayed) {
            PlaySound(NULL, NULL, 0); 
            PlaySound(TEXT("gameOver.wav"), NULL, SND_ASYNC );
            gameOverSoundPlayed = true;
            isGameMusicPlaying = false;
            f = false;
        }
        iSetColor(255, 0, 0);
        int textX = winWidth/2 - 100;
        int textY = winHeight/2;
        iText(textX, textY, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

        double currentTime = iGetTime();
        static double lastFrameTime = currentTime; 
        double deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        gameOverTimer += deltaTime;
        if (gameOverTimer >= 4.0) {
            resetGameState(); 
            isEnteringName = true;
            nameIndex = 0;
            playerName[0] = '\0';
            gamestate = 6;
        }
    }
    glLoadIdentity();
    updateScreenShake(1.0/60.0);
}

void iMouse(int button, int state, int mx, int my) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

        double currentTime = iGetTime();

        if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        double currentTime = iGetTime();
        
            if(currentTime - lastShotTime >= shootInterval) {
                angle = atan2(my - rocketY, mx - rocketX);
                double size = rocketSize;
                double height = size*0.8660254;

                double frontX = rocketX + height/2*cos(angle);
                double frontY = rocketY + height*sin(angle);
                
                // PlaySound(TEXT("bullet.wav"), NULL, SND_ASYNC);
                
                createBullet(frontX, frontY, angle);
                totalShotsFired++;
                lastShotTime = currentTime;
            }
        }

        if (gamestate == 0 && mx >= 30 && mx<=210 &&
            my >= 380 && my <= 420) {
            gamestate = 1; 
        }

        if (gamestate == 0 && mx >= 100 && mx <= 280 &&
            my >= 260 && my <= 300) {
            gamestate = 2; 
        }

        if (gamestate == 0 && mx >= 300 && mx <= 450 &&
            my >= 190 && my <= 230) {
            exit(0); // Exit Game
        }
        if (gamestate == 0 && mx >= 520 && mx <= 700 &&
            my >= 280 && my <= 320) {
            gamestate = 3;
            // exit(0);
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

        // printf("x: %d, y: %d\n", mx, my);
    }
}

void iMouseMove(int x, int y) {
    mouseX = x;
    mouseY = y;
}

void iKeyboard(unsigned char key) {
    if (key == 'w'){
        moveUp = true;
    } 
    if (key == 's')
    {
        moveDown = true;
    } 
    if (key == 'a')
    {
        moveLeft = true;
    } 
    if (key == 'd'){
        moveRight = true;
    } 
    if (key == 27 && gamestate == 1) gamestate=5;
	
	handleNameInput(key);

	if(gamestate==3 && key==27){
		gamestate=0;
	}
	// printf("%d %d\n", gamestate, key);
}

void iSpecialKeyboard(unsigned char key) {
    if(key == GLUT_KEY_DOWN){
        // printf("%c\n", key);
    } 
}

int main() {
    srand(time(0));
    initStars();
    initBulletTemplate();
    initTrailTemplate();
    gameStarted = false;
    countdownTimer = COUNTDOWN_TIME;
    // initializeWaves();
	loadHighScores();
    // iSetTimer(2000, spawnEnemy); 
    iInitialize(winWidth, winHeight, "Space Shooter");
    return 0;
}
