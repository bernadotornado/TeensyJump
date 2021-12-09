#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>
#include "qmath.h"
//#include "Test.hpp"

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET 4        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define NUMFLAKES 10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16

char tmp_str[20]; // temporary variable used in convert function

char *convert_int16_to_str(int16_t i)
{ // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}
void printStr(char *string, int size)
{
  display.setTextSize(size); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(string);
}
class v2
{
public:
  int x;
  int y;
  v2(int _x, int _y)
  {
    x = _x;
    y = _y;
  }
  float magnitude()
  {
    return sqrt(x + y);
  }
  v2 subtract(v2 a, v2 b)
  {
    return v2(a.x - b.x, a.y - b.y);
  }
  v2 add(v2 a, v2 b)
  {
    return v2(a.x + b.x, a.y + b.y);
  }
};

static const unsigned char PROGMEM logo_bmp[] = {};
const int MPU_ADDR = 0x68;                                 // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z;                            // variables for gyro raw data
int16_t temperature;                                       // variables for temperature data
int inputpin = 20;
bool asd = false;
int val = 0;
int i = 0;
int delta = 0;
int bounceDelta = 0;
int bounce = 0;
float x = 0;
class GameState
{
public:
  bool gameOver = false;

  void Restart()
  {
  }
  void GameOver()
  {
    display.clearDisplay();
    printStr("Game Over", 2);
    display.display();
  }
};
GameState gameState;
class InputManager
{
public:
  int buttonPressRaw = 0;
  bool buttonPressed = false;
  bool getKeyToggle = true;
  bool getKeyDown()
  {

    if (getKeyToggle && buttonPressed)
    {
      getKeyToggle = false;
      return true;
    }
    if (!buttonPressed)
    {
      getKeyToggle = true;
      return false;
    }

    return false;
  }
  void _update()
  {
    buttonPressRaw = analogRead(inputpin);
    buttonPressed = buttonPressRaw < 50;
  }
};
InputManager inputManager;

class Player
{
public:
  float playerX = 31;
  const int playerWidth = 8;
  const int playerHeight = 5;
  const int playerYPadding = 10;
  const int playerHeadpos = 9;
  const int leg1Pos = 4;
  const int leg2Pos = 1;
  const int leg3Pos = -2;
  const int playerBoundingBoxMinX = 0;
  const int playerBoundingBoxMinY = 0;
  const int playerBoundingBoxMaxX = 0;
  const int playerBoundingBoxMaxY = 0;
  const int legLength = 4;
  const int footLength = 1;
  const int snoutPosY = 8;
  const int snoutPosX = 2;
  const int snoutLength = 3;
  const int snoutRadius = 1;
  const int snoutPadding = 14;
  bool isAttacking = false;

  void renderPlayer()
  {
    display.fillRect(playerHeight + playerYPadding, (playerX - (playerWidth / 2)) - 1, playerHeight, playerWidth, SSD1306_INVERSE);
    // Draw Head
    display.drawCircle(playerHeadpos + playerYPadding, (playerX - (playerWidth / 2)) + 2, 4, SSD1306_WHITE);
    display.drawCircle(playerHeadpos + playerYPadding, (playerX - (playerWidth / 2)) + 2, 3, SSD1306_WHITE);
    display.drawCircle(playerHeadpos + playerYPadding, (playerX - (playerWidth / 2)) + 2, 2, SSD1306_WHITE);
    display.drawCircle(playerHeadpos + playerYPadding, (playerX - (playerWidth / 2)) + 2, 1, SSD1306_WHITE);
    // Draw Legs
    display.drawLine(playerYPadding, (playerX - (playerWidth / 2)) + 2 + leg1Pos, playerYPadding + legLength, (playerX - (playerWidth / 2)) + 2 + leg1Pos, SSD1306_WHITE);
    display.drawLine(playerYPadding, (playerX - (playerWidth / 2)) + 2 + leg2Pos, playerYPadding + legLength, (playerX - (playerWidth / 2)) + 2 + leg2Pos, SSD1306_WHITE);
    display.drawLine(playerYPadding, (playerX - (playerWidth / 2)) + 2 + leg3Pos, playerYPadding + legLength, (playerX - (playerWidth / 2)) + 2 + leg3Pos, SSD1306_WHITE);
    // Draw feet
    display.drawLine(playerYPadding, (playerX - (playerWidth / 2)) + 2 + leg1Pos, playerYPadding, (playerX - (playerWidth / 2)) + 2 + leg1Pos - footLength, SSD1306_WHITE);
    display.drawLine(playerYPadding, (playerX - (playerWidth / 2)) + 2 + leg2Pos, playerYPadding, (playerX - (playerWidth / 2)) + 2 + leg2Pos - footLength, SSD1306_WHITE);
    display.drawLine(playerYPadding, (playerX - (playerWidth / 2)) + 2 + leg3Pos, playerYPadding, (playerX - (playerWidth / 2)) + 2 + leg3Pos - footLength, SSD1306_WHITE);
    // Draw Snout
    if (isAttacking)
    {
      display.drawLine(playerYPadding + snoutPadding, (playerX - (playerWidth / 2)) + 2, playerYPadding + snoutPadding + snoutLength, (playerX - (playerWidth / 2)) + 2, SSD1306_WHITE);
      display.drawCircle(playerYPadding + snoutPadding + snoutLength, (playerX - (playerWidth / 2)) + 2, snoutRadius, SSD1306_WHITE);
    }
    else
    {
      display.drawLine(playerYPadding + snoutPosY, (playerX - (playerWidth / 2)) - snoutPosX, playerYPadding + snoutPosY, (playerX - (playerWidth / 2)) - snoutPosX - snoutLength, SSD1306_WHITE);
      display.drawCircle(playerYPadding + snoutPosY, (playerX - (playerWidth / 2)) - snoutPosX - snoutLength, snoutRadius, SSD1306_WHITE);
    }
  }

  void playerMovement()
  {
    playerX += (int16_t)(x);
    if (playerX > display.height() + 1)
    {
      playerX = 0;
    }
    if (playerX < 0)
    {
      playerX = display.height() + 1;
    }
  }
  void handleInput()
  {
    isAttacking = inputManager.getKeyDown();
  }
  void _update()
  {
    handleInput();
    playerMovement();
    renderPlayer();
  }
};
Player player;
class Bullet
{
private:
  bool render = true;

public:
  float bulletSize = 2;
  bool onInit = true;
  float bulletPosX = 0;
  float bulletPosY = 0;
  float initPosX = 0; 
  float initPosY = 128;
  int bullet_id = random();
  int position = 0;
  int speed = 0;
  int translate = 0;
  int cspeed = 6;
  bool fire = false;

  float calculateInitPosY()
  {
    return player.playerYPadding + player.snoutLength + player.snoutRadius + player.legLength + player.playerHeight + player.playerHeadpos - 4;
  }
  float calculateInitPosX()
  {
    return ((player.playerX - (player.playerWidth / 2)) + 2);
  }
  void _start()
  {
    speed = cspeed;
    initPosX = calculateInitPosX();
    initPosY = calculateInitPosY();
    bulletPosX = initPosX;
    bulletPosY = initPosY;
    translate = 0;
    fire = false;
    render = false;
  }
  void reset()
  {
    initPosX = calculateInitPosX();
    initPosY = calculateInitPosY();
    bulletPosY = calculateInitPosY();
    bulletPosX = calculateInitPosX();
    render = false;
    fire = false;
  }
  void hibernate()
  {
    speed = 0;
    bulletPosX = 100;
    bulletPosY = 100;
  }
  void renderBullet()
  {
    display.drawCircle(bulletPosY, bulletPosX, bulletSize, SSD1306_WHITE);
    display.drawPixel(bulletPosY, bulletPosX, SSD1306_WHITE);
  }
  void _update()
  {
    if (!fire)
      bulletPosX = ((player.playerX - (player.playerWidth / 2)) + 2);
    else
    {
      translate += speed;
      bulletPosY = initPosY + translate;
    }
    if (fire)
    {
      renderBullet();
    }
    if (bulletPosY > 128)
    {
      _start();
    }
  }
};
class Enemy
{
public:
  Enemy(){};
  int id = random();
  v2 position{display.height() / 2, 40};
  void _start()
  {
    position.x += id;
  }
  void renderEnemy()
  {
    display.fillCircle(position.y, position.x, 5, SSD1306_WHITE);
    display.fillCircle(position.y, position.x - 1, 2, SSD1306_INVERSE);
    display.drawPixel(position.y, position.x - 1, SSD1306_WHITE);
    for (int i = 0; i < 5; i++)
    {
      display.drawLine(position.y + 3, position.x - 4 + (i * 2), position.y + 7 - (i == 0 || i == 4 ? 1 : 0), position.x - 4 + (i * 2), SSD1306_WHITE);
    }
    display.fillRect(position.y - 6, position.x - 4, 4, 10, SSD1306_WHITE);
    for (int i = 0; i < 2; i++)
    {
      display.drawLine(position.y - 6, position.x - 3 + (i * 7), position.y - 9, position.x - 3 + (i * 7), SSD1306_WHITE);
      display.drawLine(position.y - 9, position.x - 3 + (i * 7), position.y - 9, position.x - 5 + (i * 7), SSD1306_WHITE);
    }
    for (int i = 0; i < 3; i++)
    {
      int yoffset = -4;
      display.drawPixel(position.y + yoffset, position.x - 2 - 1, SSD1306_INVERSE);
      display.drawLine(position.y + yoffset, position.x + (i * 2) - 1, position.y - 1 + yoffset, position.x - 1 + (i * 2) - 1, SSD1306_INVERSE);
    }
  }
  void _update()
  {
    renderEnemy();
  }
};
class EnemySpawner
{
public:
  int currentEnemyIndex = 0;
  Enemy enemyPool[16];
  Enemy currentEnemy;
  Enemy getFromPool()
  {
    if (currentEnemyIndex > 15)
    {
      currentEnemyIndex = 0;
    }
    return enemyPool[currentEnemyIndex++];
  }
  void _start()
  {
    for (int i = 0; i < 16; i++)
    {
      Enemy e{};
      e.id = i;
      enemyPool[i] = e;
      e._start();
    }
  }
  void _update()
  {
    for (int i = 0; i < 16; i++)
    {
      Enemy _e = enemyPool[i];
      _e._update();
      enemyPool[i] = _e;
    }
  }
};
EnemySpawner enemySpawner;
class BulletSpawner
{
public:
  int currentBulletIndex = 0;
  Bullet bulletPool[16];
  Bullet currentBullet;
  Bullet getFromPool()
  {
    if (currentBulletIndex > 15)
    {
      currentBulletIndex = 0;
    }
    return bulletPool[currentBulletIndex++];
  }
  void _start()
  {
    for (int i = 0; i < 16; i++)
    {
      Bullet b;
      b.bullet_id = i;
      b._start();
      bulletPool[i] = b;
    }
    currentBullet = getFromPool();
  }
  void _update()
  {
    if (player.isAttacking)
    {
      currentBullet = getFromPool();
      currentBullet._start();
      currentBullet.fire = true;
      bulletPool[currentBulletIndex - 1] = currentBullet;
    }
    for (int i = 0; i < 16; i++)
    {
      bulletPool[i]._update();
    }
  }
};
BulletSpawner bulletSpawner;
class Plattform
{
public:
  v2 position{10, 10};
  bool isBroken = false;
  bool isMovingPlattform = false;
  bool hasEnemy = false;
  int movingPlattformDir = 1;
  
  int id = random();
  void _start()
  {
  }
  void renderBrokenPlattform()
  {
    for (int i = 0; i < 5; i++)
    {
      display.drawLine(position.y +2, position.x - 4-3-1 + (i * 3), position.y - 3 +2, position.x - 1-1 -3+ (i * 3), SSD1306_WHITE);
    }
  }
  void renderPlattform()
  {
    for (int i = 0; i < 5; i++)
    {
      display.fillCircle(position.y , position.x - 5-1 + (3 * i) - (i==4?1:0), 2, SSD1306_WHITE);
    }
  }
  void _update()
  {
    if(isMovingPlattform){
      position.x+= movingPlattformDir;
      if (position.x > 63){
        movingPlattformDir =  movingPlattformDir *-1;
      }
      if(position.x<0){
         movingPlattformDir =  movingPlattformDir *-1;
      }
    }
    if (!isBroken)
    {
      renderPlattform();
    }
    else
    {
      renderBrokenPlattform();
    }
  }
};

class PlattformSpawner
{
public:
  int currentPlattformIndex = 0;
  Plattform plattformPool[16];
  Plattform currentPlattform;
  int rnd = 0;
  Plattform getFromPool()
  {
    if (currentPlattformIndex > 15)
    {
      currentPlattformIndex = 0;
    }
    return plattformPool[currentPlattformIndex++];
  }
  void _start()
  {
    for (int i = 0; i < 16; i++)
    {
      Plattform p;
      p.id = i;
      p.position.y = p.id*5; 
      plattformPool[i] = p;
    }
    currentPlattform = getFromPool();
  }
  void _update()
  {
    if (player.isAttacking)
    {
      rnd = random(0,99);
      currentPlattform = getFromPool();
#define RND random(0,99)
      currentPlattform.isBroken = RND <30;
      currentPlattform.movingPlattformDir = RND <50 ? -1:1;
      currentPlattform.isMovingPlattform = RND <20;
      currentPlattform.hasEnemy = RND <30;
      currentPlattform.position.x = random(0,63);
      currentPlattform._start();
      plattformPool[currentPlattformIndex - 1] = currentPlattform;
    }
    for (int i = 0; i < 16; i++)
    {
      plattformPool[i]._update();
    }
  }
};
PlattformSpawner plattformSpawner;

void START()
{
  bulletSpawner._start();
  enemySpawner._start();
  plattformSpawner._start();
  for (int i = 0; i < 16; i++)
  {
    Enemy enemies = enemySpawner.enemyPool[i];
    int ids = enemies.id;
    enemies.position.y = ids;
  }
}

void setup()
{
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  pinMode(inputpin, INPUT);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
  delay(2000);
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B);                 // PWR_MGMT_1 register
  Wire.write(0);                    // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  START();
}

#define calcCoef 6.32455f
#define calcRoot 12.64910f

float calcBounce(float x) {
  return -0.1f*(x-calcCoef)*(x-calcCoef)+4;
}

void UPDATE()
{
  inputManager._update();
  player._update();
  bulletSpawner._update();
  enemySpawner._update();
  plattformSpawner._update();
  delta++;

  bounceDelta++;
  bounce = calcBounce(bounceDelta);
  if(bounce> calcRoot){
    bounce = 0;
    bounceDelta = 0;
  }
  char str[20];
  sprintf(str, "          SCORE: %d", 9999);
  printStr(str, 1);
}
void loop()
{
  //gyro setup
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);                                 // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false);                      // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7 * 2, true);          // request a total of 7*2=14 registers
  accelerometer_x = Wire.read() << 8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read() << 8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accelerometer_z = Wire.read() << 8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read() << 8 | Wire.read();     // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro_x = Wire.read() << 8 | Wire.read();          // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read() << 8 | Wire.read();          // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read() << 8 | Wire.read();          // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
  // Get Gyro y
  x = gyro_y;
  x = map(x, -32768, 32768, -50, 51);
  display.clearDisplay();
  if (gameState.gameOver)
  {
    gameState.GameOver();
    return;
  }
  UPDATE();
  display.display();
  delay(25);
}
#define XPOS 0 // Indexes into the 'icons' array in function below
#define YPOS 1
#define DELTAY 2
