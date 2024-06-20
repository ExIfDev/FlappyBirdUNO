//MADE BY AexaDev on 06/11/2023
//A basic port of FappyBird for Arduino Uno
//Uses a 128x64 OLED display and 2 push buttons
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

//the game is designed around this resolution
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BIRD_SIZE 12
#define GRAVITY 0.5 
#define JUMP_STRENGTH 3 
#define MIN_PIPE_HEIGHT 5 
#define MAX_PIPE_HEIGHT 20 
#define MIN_PIPE_SPACING 25 
#define MAX_PIPE_SPACING 45 
#define PIPE_WIDTH 10
#define PIPE_SPEED 2 
#define GAME_LOGIC_INTERVAL 0 // Game logic update tick in milliseconds 0 is as fast as the device can handle

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int birdX, birdY;
float birdVelocity; 
int pipeX;
int pipeHeight;
int pipeSpacing;
int score;
bool isGameOver;
bool hasPassedPipe;

//PUSH BUTTON PIN DEFINITIONS//
const int jumpButtonPin = 8;
const int restartButtonPin = 9;
///////////////////////////////


bool jumpButtonPressed = false;
unsigned long lastGameLogicUpdate = 0;
unsigned long frameInterval = 1000;  
unsigned long lastFrameMillis = 0;
unsigned long prevMillis = 0;
int frameCount = 0;
int fps = 0;


static const uint8_t PROGMEM newBirdSprite[] = {
   0x00,0x00,0x03,0x60,0x0e,0x70,0x06,0x68,0x7a,0x68,0x79,0x38,0x79,0xc0,0x3b,0xbe,
  0x06,0x40,0x1f,0xbc,0x07,0x80,0x00,0x00
};

void setup() {
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  prevMillis = millis();
  lastFrameMillis = millis();
  }

  
  display.clearDisplay();
  display.display();

  // Set up the pins with pull-up resistors enabled
  pinMode(jumpButtonPin, INPUT_PULLUP);
  pinMode(restartButtonPin, INPUT_PULLUP);

  // Initialize game variables
  birdX = SCREEN_WIDTH / 4;
  birdY = SCREEN_HEIGHT / 2;
  birdVelocity = 0;
  pipeX = SCREEN_WIDTH;
  score = 0;
  isGameOver = false;
  hasPassedPipe = false;

  
  randomizePipes();
}

void loop() {
  unsigned long currentMillis = millis();

  // Calculate FPS
  frameCount++;
  if (currentMillis - prevMillis >= frameInterval) {
    fps = frameCount;
    frameCount = 0;
    prevMillis = currentMillis;
  }

  
  if (currentMillis - lastGameLogicUpdate >= GAME_LOGIC_INTERVAL) {
    
    updateGameLogic();
    lastGameLogicUpdate = currentMillis;
  }

  
  if (isGameOver && digitalRead(restartButtonPin) == LOW) {
    restartGame();
  }

  // Read jump button state
  bool jumpButtonState = digitalRead(jumpButtonPin);

  // Button press method: rising edge
  if (!jumpButtonPressed && jumpButtonState == LOW) {
    jumpButtonPressed = true;
    birdVelocity = -JUMP_STRENGTH;
  }

  // Reset the jump button flag when the button is released
  if (jumpButtonState == HIGH) {
    jumpButtonPressed = false;
  }

  // Update display
  updateDisplay();
}

void updateBird() {
  // Apply gravity
  birdVelocity += GRAVITY;
  birdY += birdVelocity;

  // make the bird stay within the bounds
  birdY = constrain(birdY, 0, SCREEN_HEIGHT - BIRD_SIZE);

  // check for collision with the bottom
  if (birdY == SCREEN_HEIGHT - BIRD_SIZE) {
    isGameOver = true;
  }
}

void updatePipe() {
  pipeX -= PIPE_SPEED;

  // reset pipe position and randomize when it goes out of screen space
  if (pipeX + PIPE_WIDTH < 0) {
    pipeX = SCREEN_WIDTH;
    randomizePipes();
    hasPassedPipe = false;
  }

  // check if the bird has passed the pipe
  if (pipeX + PIPE_WIDTH < birdX && !hasPassedPipe) {
    hasPassedPipe = true;
    score++;
  }
}

void randomizePipes() {
  // randomize pipe height and spacing
  pipeHeight = random(MIN_PIPE_HEIGHT, MAX_PIPE_HEIGHT);
  pipeSpacing = random(MIN_PIPE_SPACING, MAX_PIPE_SPACING);
}

void checkCollision() {
  // check for collision with pipes
  if (birdX + BIRD_SIZE >= pipeX && birdX <= pipeX + PIPE_WIDTH) {
    if (birdY <= pipeHeight || birdY + BIRD_SIZE >= pipeHeight + pipeSpacing) {
      isGameOver = true;
    }
  }
}

void updateDisplay() {
  
  display.clearDisplay();
  display.drawBitmap(birdX, birdY, newBirdSprite, 16, 12, SSD1306_WHITE);

  // Draw pipes
  display.fillRect(pipeX, 0, PIPE_WIDTH, pipeHeight, SSD1306_WHITE);
  display.fillRect(pipeX-2, pipeHeight-6 + pipeSpacing, 14, 6, SSD1306_WHITE);
  display.fillRect(pipeX, pipeHeight + pipeSpacing, PIPE_WIDTH, SCREEN_HEIGHT - pipeHeight - pipeSpacing, SSD1306_WHITE);
  display.fillRect(pipeX-2, pipeHeight-6, 14, 6, SSD1306_WHITE);
  // black background behind the fps counter
  display.fillRect(SCREEN_WIDTH - 40, 0, 40, 8, SSD1306_BLACK);

  // Display framerate
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(SCREEN_WIDTH - 40, 0);
  display.print("FPS:");
  display.print(fps);

  // black background behind the score text
  display.fillRect(0, 0, 64, 8, SSD1306_BLACK);

  // display score
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Score: ");
  display.print(score);

  // display game over screen
  if (isGameOver) {
    display.setTextSize(2);
    display.invertDisplay(1);
    display.setCursor(13, SCREEN_HEIGHT / 2 - 8);
    display.print("Game Over");
  }

  // Update display buffer
  display.display();
}


void updateGameLogic() {
  if (!isGameOver) {
    updateBird();
    updatePipe();
    checkCollision();
  }
}


void restartGame() {
  display.invertDisplay(0);
  birdX = SCREEN_WIDTH / 4;
  birdY = SCREEN_HEIGHT / 2;
  birdVelocity = 0;
  pipeX = SCREEN_WIDTH;
  score = 0;
  isGameOver = false;
  hasPassedPipe = false;
  randomizePipes();
}