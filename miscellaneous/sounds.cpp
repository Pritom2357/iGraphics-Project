#include "iGraphics.h"

// Global variable to check if sound is playing
bool isSoundPlaying = false;
int soundDuration = 200;  // Approximate duration of sound in frames (e.g., ~2 seconds for 100 FPS)

// Function to play sound
void PlaySoundOnce()
{
    if (!isSoundPlaying) {
        PlaySound(TEXT("laser_gun.wav"), NULL, SND_ASYNC);  // Play sound asynchronously
        isSoundPlaying = true;  // Set flag to true to prevent further clicks from playing sound
    }
}

// Function to reset the sound playing flag after the sound has finished
void onSoundFinish()
{
    isSoundPlaying = false;
}

// iGraphics initialization function
void iDraw()
{
    // Clear the screen
    iClear();
    
    // Example graphical elements
    iText(200, 200, "Click to play sound", GLUT_BITMAP_HELVETICA_18);
}

// Mouse click event handler
void iMouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        // If left mouse button is clicked, play sound once
        if (button == GLUT_LEFT_BUTTON) {
            PlaySoundOnce();
        }
    }
}

// Timer event handler to reset the sound playing flag after a set period
void iTick()
{
    // Check if sound is playing and simulate the sound's duration
    if (isSoundPlaying) {
        static int timer = 0;
        if (++timer > soundDuration) {  // Simulate sound duration based on frames
            onSoundFinish();
            timer = 0;  // Reset the timer for the next sound
        }
    }
}

void iKeyboard(unsigned char key) {
    
}

void iSpecialKeyboard(unsigned char key) {
    // if(key==GLUT_KEY_DOWN){
    //     gamestate=3;
    // }
}

void iMouseMove(int mx, int my) {
    // Handle mouse movement
}

int main()
{
    // Initialize iGraphics
    iInitialize(500, 500);  // Window size 500x500

    // Start the drawing and event loop
    // iStart();

    return 0;
}
