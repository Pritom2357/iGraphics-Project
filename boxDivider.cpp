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

// Constants
#define PI 3.14159265358979323846
//bullet properties
#define BULLET_WIDTH 6.0;
#define BULLET_HEIGHT 20.0;
#define MAX_BULLET 50
#define BULLET_SPEED 0.8
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
//enemy properties
#define MAX_ENEMIES 50
//grid properties
#define GRID_CELL_SIZE 64
#define GRID_WIDTH (winWidth/GRID_CELL_SIZE)
#define GRID_HEIGHT (winHeight/GRID_CELL_SIZE)
//player properties
#define PLAYER_ACCELERATION 0.5
#define PLAYER_MAX_SPEED 15.0
#define PLAYER_FRICTION 0.98
#define MOVEMENT_SMOOTHING 0.7
#define MIN_VELOCITY 0.01
#define THRUST_PARTICLES_MAX 50
#define TRHUST_PARTICLE_LIFETIME 0.5f
#define THRUST_SPREAD_ANGLE 0.5f
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


// Add at the top with other global variables
struct ScreenShake {
    double magnitude;    // Current shake strength
    double duration;     // How long shake lasts
    double timer;        // Current shake time
    double offsetX;      // Current X offset
    double offsetY;      // Current Y offset
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
    const double shootInterval = 0.1;
    bool isActive = false;
} autoFire;

struct ScoreSystem {
    int score;
    float multiplier;
    float comboTimer;
    int comboCount;
    const float COMBO_TIME = 3.0f;    // Time window for combo
    const float MAX_MULTIPLIER = 5.0f; // Maximum score multiplier
} scoring = {0, 1.0f, 0.0f, 0};

struct FloatingText {
    char text[32];
    double x, y;
    double duration;
    double timer;
    bool active;
    double alpha;
    double vy;  // Vertical velocity
};

typedef struct HighScoreEntry{
    char name[MAX_NAME_LENGTH];
    int score;
}HighScoreEntry;


GridCell grid[GRID_WIDTH][GRID_HEIGHT];
Star stars[NUM_LAYERS][NUM_STARS];
Enemy enemies[MAX_ENEMIES];
Particle particles[PARTICLE_POOL_SIZE];
Projectile projectiles[MAX_BULLET];
PowerUp powerups[MAX_POWERUPS];
ThrustParticle thrustParticles[THRUST_PARTICLES_MAX];
BulletTemplate bulletTemplate;
Shield shield;
HighScoreEntry highScores[MAX_HIGH_SCORES];
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

inline int getGridx(double x){return (int)(x/GRID_CELL_SIZE);}
inline int getGridy(double y){return (int)(y/GRID_CELL_SIZE);}


// Player Properties
double bulletWidth = 5;
double bulletLen = 15;
int currentLevel = 1;

double playerVX = 0.0;
double playerVY = 0.0;

double rocketX = 50;
double rocketY = winHeight / 2;
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

FloatingText floatingTexts[MAX_FLOATING_TEXTS];

// Function Prototypes
// Function Prototypes
// Game State & Initialization
void initStars();
void initBulletTemplate();
double iGetTime();

// Drawing Functions
void iDraw();
// void drawPlayer();
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
void updateThrustParticles(double deltaTime);
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
void spawnEnemy();
void spawnPowerUp(double x, double y);
void createThrustParticle();
void breakEnemy(int index);

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

// Function Implementations

void handleGameOver(){
	isEnteringName = true;
	nameIndex = 0;
	playerName[0] = '\0';
	gamestate = 6;
}

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
    // Bubble sort for simplicity
    for (int i = 0; i < MAX_HIGH_SCORES - 1; i++) {
        for (int j = 0; j < MAX_HIGH_SCORES - i - 1; j++) {
            if (highScores[j].score < highScores[j + 1].score) {
                // Swap entries
                HighScoreEntry temp = highScores[j];
                highScores[j] = highScores[j + 1];
                highScores[j + 1] = temp;
            }
        }
    }
    
    // Save sorted scores
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
	iSetColor(255, 255, 255);
	iText(winWidth/2-100, winHeight-100, "HIGH SCORES", GLUT_BITMAP_TIMES_ROMAN_24);

	for(int i=0;i<MAX_HIGH_SCORES; i++){
		char scoreText[64];
		char formattedScore[44];
		formatScore(highScores[i].score, formattedScore);
		sprintf(scoreText, "%d. %s: %s", i+1, highScores[i].name, formattedScore);
		iText(winWidth/2-150, winHeight-200-i*50, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);
	}
	iText(winWidth/2-100, 100, "Press ESC to return to main menu", GLUT_BITMAP_TIMES_ROMAN_24);
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
            floatingTexts[i].duration = 3;  // 500ms = 0.5 seconds
            floatingTexts[i].timer = 3;     // Initial timer value
            floatingTexts[i].active = true;
            floatingTexts[i].alpha = 255;
            floatingTexts[i].vy = 15;        // Increased speed for shorter duration
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
    // Base points
    int basePoints = 100;
    
    // Distance bonus (further = more points)
    double distance = sqrt(pow(enemy->x - rocketX, 2) + pow(enemy->y - rocketY, 2));
    int distanceBonus = (int)(distance * 0.1);
    
    // Speed bonus
    int speedBonus = (int)(enemy->rotationSpeed * 50);
    
    // Calculate final points with multiplier
    int points = (int)((basePoints + distanceBonus + speedBonus) * scoring.multiplier);
    
    // Update combo
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
        
        // Calculate random offset
        shake.offsetX = (rand() % 100 - 50) * 0.01 * shake.magnitude;
        shake.offsetY = (rand() % 100 - 50) * 0.01 * shake.magnitude;
        
        // Decrease magnitude over time
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
    // handleShieldCollision(d);
    
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
    
    // Store missile vertices
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
        
        // Transform template vertices for each bullet
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

void createThrustParticle() {
    if (thrustParticleCount >= THRUST_PARTICLES_MAX) return;

    // Get position behind rocket (opposite to angle)
    double backAngle = angle + PI;
    double spread = ((rand() % 100) / 100.0 - 0.5) * THRUST_SPREAD_ANGLE;
    double particleAngle = backAngle + spread;

    ThrustParticle* p = &thrustParticles[thrustParticleCount++];
    
    // Position particles at back of rocket
    p->x = rocketX - (rocketSize/2) * cos(angle);
    p->y = rocketY - (rocketSize/2) * sin(angle);

    // Higher speed for more visible effect
    double speed = 0.3 + (rand() % 100) / 200.0;
    p->vx = speed * cos(particleAngle);
    p->vy = speed * sin(particleAngle);

    p->lifetime = TRHUST_PARTICLE_LIFETIME;
    p->active = true;

    // Brighter colors
    p->r = 255;
    p->g = rand() % 128 + 64; // More orange
    p->b = 0;
}


void updateThrustParticles(double deltaTime) {
    // Create new particles when moving
    if (moveUp || moveDown || moveLeft || moveRight) {
        for (int i = 0; i < 3; i++) { // Create multiple particles per frame
            createThrustParticle();
        }
    }
    
    // Update existing particles
    for (int i = 0; i < thrustParticleCount; i++) {
        if (!thrustParticles[i].active) continue;
        
        thrustParticles[i].x += thrustParticles[i].vx;
        thrustParticles[i].y += thrustParticles[i].vy;
        thrustParticles[i].lifetime -= deltaTime;
        
        if (thrustParticles[i].lifetime <= 0) {
            thrustParticles[i] = thrustParticles[--thrustParticleCount];
            i--;
        }
    }
}

void drawThrustParticles() {
    for (int i = 0; i < thrustParticleCount; i++) {
        if (!thrustParticles[i].active) continue;

        double fade = thrustParticles[i].lifetime / TRHUST_PARTICLE_LIFETIME;
        int r = (int)(thrustParticles[i].r * fade);
        int g = (int)(thrustParticles[i].g * fade);
        int b = (int)(thrustParticles[i].b * fade);

        iSetColor(r, g, b);
        double size = 4.0 * fade; // Larger particles
        iFilledCircle(thrustParticles[i].x, thrustParticles[i].y, size);
    }
}

void updateAutoFire(double deltaTime) {
    if (!autoFire.active) return;
    
    autoFire.timer -= deltaTime;
    autoFire.shootTimer -= deltaTime;
    
    // Auto-shoot logic
    if (autoFire.isActive && autoFire.shootTimer <= 0) {
        double angle = atan2(mouseY - rocketY, mouseX - rocketX);
        double tipX = rocketX + (rocketSize/2) * cos(angle);
        double tipY = rocketY + (rocketSize/2) * sin(angle);
        createSingleBullet(tipX, tipY, angle);
        printf("Auto-Fire, Bullet created\n");
        autoFire.shootTimer = autoFire.shootInterval;
    }
    
    // Handle duration expiry
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

void damagePlayer() {
    if (!isInvincible) {
        playerHealth -= 20; 
        
        if (playerHealth <= 0) {
            playerHealth = 0;
            playerLives--;
            
            if (playerLives > 0) {
                playerHealth = MAX_PLAYER_HEALTH;
            } else {
                gameover = true;
				isEnteringName = true;
				nameIndex = 0;
				playerName[0] = '\0';
				gamestate = 6;

				enemyCount = 0;
				projectileCount = 0;
				particleCount = 0;
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
        
        // Calculate color based on remaining time
        int colorValue;
        if (currentNotification.timer <= 1.0) {
            // In last second, fade from 255 to 0
            colorValue = (int)(255 * (currentNotification.timer / 1.0));
        } else {
            colorValue = 255;
        }
        
        // Ensure color value stays in valid range
        colorValue = max(0, min(255, colorValue));
        
        if (currentNotification.timer <= 0) {
            currentNotification.active = false;
        } else {
            iSetColor(colorValue, colorValue, colorValue);
            iText(winWidth/2 - 100, NOTIFICATION_Y, currentNotification.text, GLUT_BITMAP_TIMES_ROMAN_24);
            
            // Also fade the timer text if showing
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

void spawnEnemy() {
    if(gamestate!=1) return;

    if (enemyCount < MAX_ENEMIES) {
        int enemiesToSpawn = 1;

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
    if(gamestate!=1) return;
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active) {
            enemies[i].x += enemies[i].vx;
            enemies[i].y += enemies[i].vy;
            enemies[i].angle += enemies[i].rotationSpeed;

            handleShieldCollision(&enemies[i]);

            
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
        int points = calculatePoints(&enemies[index]);
        scoring.score += points;
        triggerScreenShake(10.0, 0.3);
        spawnPowerUp(enemies[index].x, enemies[index].y);
        char scoreText[32];
        sprintf(scoreText, "+%d", points);
        showFloatingText(enemies[index].x, enemies[index].y, scoreText);
    

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

    // createThrustParticle();

    double acceleration = PLAYER_ACCELERATION * deltaTime * 60.0;
    double friction = pow(PLAYER_FRICTION, deltaTime * 60.0);

    if (moveUp)    playerVY += acceleration;
    if (moveDown)  playerVY -= acceleration;
    if (moveLeft)  playerVX -= acceleration;
    if (moveRight) playerVX += acceleration;

    playerVX = fmax(-PLAYER_MAX_SPEED, fmin(PLAYER_MAX_SPEED, playerVX));
    playerVY = fmax(-PLAYER_MAX_SPEED, fmin(PLAYER_MAX_SPEED, playerVY));

    playerVX *= friction;
    playerVY *= friction;

    double leftBound = playerRadius;
    double rightBound = winWidth - playerRadius;
    double topBound = winHeight - playerRadius;
    double bottomBound = playerRadius;

    double newX = rocketX + playerVX;
    double newY = rocketY + playerVY;

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

    rocketX = rocketX * MOVEMENT_SMOOTHING + newX * (1.0 - MOVEMENT_SMOOTHING);
    rocketY = rocketY * MOVEMENT_SMOOTHING + newY * (1.0 - MOVEMENT_SMOOTHING);

    if (fabs(playerVX) < MIN_VELOCITY) playerVX = 0;
    if (fabs(playerVY) < MIN_VELOCITY) playerVY = 0;

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
        double spreadAngle = 0.2; 
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

bool f = false;

void iDraw() {
    iClear();
    glTranslatef(shake.offsetX, shake.offsetY, 0);
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
        // iClear();
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
        updateFloatingTexts(1.0/60.0);
        moveStars();
        drawStars();
        updateShield(deltaTime);
        drawShield();

        for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
            if (floatingTexts[i].active) {
                // Set color with alpha based on lifetime
                int alpha = (int)floatingTexts[i].alpha;
                // Use iSetColor for RGB values 
                iSetColor(255, 255, 255); 
                
                // Draw text using iText
                iText(floatingTexts[i].x, floatingTexts[i].y, floatingTexts[i].text, GLUT_BITMAP_TIMES_ROMAN_24);
            }
        }

        angle = atan2(mouseY - rocketY, mouseX - rocketX);
        double size = rocketSize;
        double height = rocketSize*0.8660254;

        double localX[3] = {height/2, -height/2, -height/2};
        double localY[3] = {0, size / 2.0, -size / 2.0};

        // double tipX = xTipLocal * cos(angle) - yTipLocal * sin(angle) + rocketX;
        // double tipY = xTipLocal * sin(angle) + yTipLocal * cos(angle) + rocketY;


        double rotatedX[3], rotatedY[3];

        for (int i = 0; i < 3; i++) {
            double x = localX[i];
            double y = localY[i];
            rotatedX[i] = x * cos(angle) - y * sin(angle) + rocketX;
            rotatedY[i] = x * sin(angle) + y * cos(angle) + rocketY;
        }

        if(isInvincible && isBlinking) {
            iSetColor(255, 0, 0);
        } 
        // else if(rocketColor.isAutoFiring) {
        //     iSetColor(0, 0, 255); // Full blue during auto-fire
        // } 
        else {
            // Normal color transition
            // iSetColor(rocketColor.r, rocketColor.g, rocketColor.b);
            iSetColor(255, 255, 255);
        }

        iFilledPolygon(rotatedX, rotatedY, 3);
        

        drawBullets();

        moveRocket();
        // updateRapidFire(deltaTime);
        moveProjectiles();
        updateAutoFire(deltaTime);
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
        
        // drawPlayer();
        // updateThrustParticles(deltaTime);
        // drawThrustParticles();

        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].active && isPlayerCollidingWithEnemy(enemies[i])) {
                if (!isInvincible) {
                    playerLives--;
                    printf("Player Hit! Lives Remaining: %d\n", playerLives);
                    isInvincible = true;
                    invincibleTime = 0.0;

                    if (playerLives <= 0) {
                        printf("Game Over!!!\n");
                        isEnteringName = true;
						nameIndex = 0;
						playerName[0] = '\0';
						gamestate = 6;
						
						enemyCount=0;
						projectileCount=0;
						particleCount=0;
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
        iText(levelBarX + levelBarWidth / 2.0 - 20, levelBarY + levelBarHeight + 5, levelText, GLUT_BITMAP_TIMES_ROMAN_24);

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
		// sprintf("High scores\n", "%d",  scoring.score);
        // printf("High Scores\n");
		drawHighScore();
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
    }else if(gamestate==6){
		iClear();
		drawNamePrompt();
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
                
                PlaySound(TEXT("bullet.wav"), NULL, SND_ASYNC);
                
                createBullet(frontX, frontY, angle);
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

        printf("x: %d, y: %d\n", mx, my);
    }
}

void iMouseMove(int x, int y) {
    mouseX = x;
    mouseY = y;
}

void iKeyboard(unsigned char key) {
    if (key == 'w'){
        moveUp = true;
        createThrustParticle();
    } 
    if (key == 's')
    {
        moveDown = true;
        createThrustParticle();
    } 
    if (key == 'a')
    {
        moveLeft = true;
        createThrustParticle();
    } 
    if (key == 'd'){
        moveRight = true;
        createThrustParticle();
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
        printf("%c\n", key);
    } 
}

int main() {
    srand(time(0));
    initStars();
    initBulletTemplate();
	loadHighScores();
    iSetTimer(2000, spawnEnemy); 
    iInitialize(winWidth, winHeight, "Space Shooter - Multiple Enemy Types");
    return 0;
}
