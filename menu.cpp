#include "iGraphics.h"

int buttonWidth = 180, buttonHeight = 40;
int winWidth = 800, winHeight = 600;
int gamestate = 0; // 0 = Main Menu, 1 = Game, 2 = About

void iDraw() {
    iClear();

    if (gamestate == 0) { // Main Menu
        // Background
        iClear();

        iShowBMP2(0, 0, "C:/Users/prito/Desktop/projects/space shooter/mainmenu.bmp", -1);
    }
    else if (gamestate == 1) { // Game
        iClear();
        iText(50, winHeight - 50, "Game Starting...");
    }
    else if (gamestate == 2) { // About
        iClear();
        iShowBMP2(0, 0, "C:/Users/prito/Desktop/projects/space shooter/about.bmp", -1);
    }
    else if(gamestate==5){
        iClear();
        iShowBMP2(0, 0, "C:/Users/prito/Desktop/projects/space shooter/esc.bmp", -1);
    }
    else if(gamestate==4){
        iClear();
        iShowBMP2(0, 0, "C:/Users/prito/Desktop/projects/space shooter/controls.bmp", -1);
    }
}

void iMouse(int button, int state, int mx, int my) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Start Button Click
        if (mx >= 30 && mx<=210 &&
            my >= 380 && my <= 420) {
            gamestate = 1; // Start Game
        }

        // About Button Click
        if (mx >= 100 && mx <= 280 &&
            my >= 260 && my <= 300) {
            gamestate = 2; // About Section
        }

        // Exit Button Click
        if (mx >= 300 && mx <= 450 &&
            my >= 190 && my <= 230) {
            exit(0); // Exit Game
        }
        if (mx >= 520 && mx <= 700 &&
            my >= 280 && my <= 320) {
            gamestate = 3;
            // exit(0);
        }
        if (mx >= 570 && mx <= 750 &&
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

void iKeyboard(unsigned char key) {
    // Handle regular keypresses
    if(key == 27 && GLUT_KEY_DOWN){
        gamestate=5;
    }
}

void iSpecialKeyboard(unsigned char key) {
    // if(key==GLUT_KEY_DOWN){
    //     gamestate=3;
    // }
}

void iMouseMove(int mx, int my) {
    // Handle mouse movement
}


int main() {
    PlaySound(TEXT("C:/Users/prito/Desktop/projects/space shooter/menusound.wav"), NULL, SND_ASYNC | SND_LOOP);
    iInitialize(winWidth, winHeight, "Main Menu Example");
    return 0;
}
