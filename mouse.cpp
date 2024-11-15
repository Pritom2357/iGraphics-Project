#include "iGraphics.h"

// int mouseX, mouseY;  // Global variables to keep track of the mouse position

// // This function will be triggered every time the mouse moves.
// void iMouseMove(int x, int y) {
//     mouseX = x;
//     mouseY = y;
//     printf("Mouse Position: X = %d, Y = %d\n", x, y);  // Print mouse position
// }

// void iDraw(){
//     iClear();
//     // Optional: Display the mouse coordinates on the graphics window.
//     char mousePos[100];
//     sprintf(mousePos, "Mouse Position: X = %d, Y = %d", mouseX, mouseY);
//     iText(10, 20, mousePos, GLUT_BITMAP_HELVETICA_18);
// }

// void iMouse(int button, int state, int x, int y) {
//     // This function is called when the mouse button is pressed or released
// }

void iKeyboard(unsigned char key) {
    // This function is called whenever a keyboard key is pressed
}

void iSpecialKeyboard(unsigned char key) {
    // This function is called for special key inputs like arrow keys
}

void iMouseMove(int x, int y) {
    // mouseX = x;
    // mouseY = y;
    // printf("Mouse Position: X = %d, Y = %d\n", x, y);  // Print mouse position
}


// // void iSpecialKeyboard(unsigned char key) {
// //   // This function is called for special key inputs like arrow keys
// // }

int shootX=100, shootY=100;
int projectileX, projectileY;
int shootInterval=50;
int projectileSpeed=10;
bool isShooting=false;
int lineLength=20;
int lineThickenss = 5;

void iMouse(int button, int state, int x, int y){
    if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
        if(!isShooting){
            projectileX=shootX;
            projectileY=shootY;
            isShooting=true;
        }
    }
}

// void timer(){
//     if(!isShooting){
//         projectileX=shootX;
//         projectileY=shootY;
//         isShooting=true;
//     }
// }

void moveProjectile(){
    if(isShooting){
        projectileX+=projectileSpeed;
        // projectileY+=projectileSpeed;
        if(projectileX>800){
            isShooting=false;
        }
    }
}

void iDraw(){
    iClear();
    iFilledCircle(shootX, shootY, 5);

    if(isShooting){
        iSetColor(255, 0, 0);
        // iLine(shootX, shootY, projectileX, projectileY);
        // iFilledCircle(projectileX, projectileY, 5);
        // iLine(projectileX, projectileY, projectileX+lineLength, projectileY);
        iFilledRectangle(projectileX, projectileY, lineLength, lineThickenss);
    }
}


int main(){
    // iSetTimer(shootInterval, timer);
    iSetTimer(20, moveProjectile);
    iInitialize(800, 800, "Mouse Position Demo");
    // iStart();
    return 0;
}
