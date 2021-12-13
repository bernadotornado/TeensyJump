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
    float x;
    float y;
    v2(float _x, float _y)
    {
      x = _x;
      y = _y;
    }
    float magnitude()
    {
      return sqrt(x*x + y*y);
    }
    static v2 subtract(v2 a, v2 b)
    {
      return v2(a.x - b.x, a.y - b.y);
    }
    static v2 add(v2 a, v2 b)
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
  float bounceDelta = 0;
  int bounceInvert = 1;
  int bounce = 0;
  float x = 0;
  class GameState
  {
  public:
    bool gameOver = false;
    int score = 0;
    int highscore = 0;
    int gameOverCounter;
    void Restart()
    {
    }
    void GameOver()
    {
      gameOverCounter++;
      if (gameOverCounter>50){
        gameOver = false;
        gameOverCounter = 0;
        
         score = 0;
         return;
      }
      display.clearDisplay();
      printStr("Game Over", 2);
      display.setTextSize(1.5f);
      display.print("Score: ");
      display.print(convert_int16_to_str(score));
      display.println("");
      display.print("High-Score: ");
      display.println(convert_int16_to_str(highscore));
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

    v2 position {31,20};
    const int playerWidth = 8;
    const int playerHeight = 5;
    const int playerHeadpos = 9;
    const int leg1Pos = 4;
    const int leg2Pos = 1;
    const int leg3Pos = -2;
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
      display.fillRect(playerHeight + position.y, (position.x - (playerWidth / 2)) - 1, playerHeight, playerWidth, SSD1306_INVERSE);
      // Draw Head
      display.drawCircle(playerHeadpos + position.y, (position.x - (playerWidth / 2)) + 2, 4, SSD1306_WHITE);
      display.drawCircle(playerHeadpos + position.y, (position.x - (playerWidth / 2)) + 2, 3, SSD1306_WHITE);
      display.drawCircle(playerHeadpos + position.y, (position.x - (playerWidth / 2)) + 2, 2, SSD1306_WHITE);
      display.drawCircle(playerHeadpos + position.y, (position.x - (playerWidth / 2)) + 2, 1, SSD1306_WHITE);
      // Draw Legs
      display.drawLine(position.y, (position.x - (playerWidth / 2)) + 2 + leg1Pos, position.y + legLength, (position.x - (playerWidth / 2)) + 2 + leg1Pos, SSD1306_WHITE);
      display.drawLine(position.y, (position.x - (playerWidth / 2)) + 2 + leg2Pos, position.y + legLength, (position.x - (playerWidth / 2)) + 2 + leg2Pos, SSD1306_WHITE);
      display.drawLine(position.y, (position.x - (playerWidth / 2)) + 2 + leg3Pos, position.y + legLength, (position.x - (playerWidth / 2)) + 2 + leg3Pos, SSD1306_WHITE);
      // Draw feet
      display.drawLine(position.y, (position.x - (playerWidth / 2)) + 2 + leg1Pos, position.y, (position.x - (playerWidth / 2)) + 2 + leg1Pos - footLength, SSD1306_WHITE);
      display.drawLine(position.y, (position.x - (playerWidth / 2)) + 2 + leg2Pos, position.y, (position.x - (playerWidth / 2)) + 2 + leg2Pos - footLength, SSD1306_WHITE);
      display.drawLine(position.y, (position.x - (playerWidth / 2)) + 2 + leg3Pos, position.y, (position.x - (playerWidth / 2)) + 2 + leg3Pos - footLength, SSD1306_WHITE);
      // Draw Snout
      if (isAttacking)
      {
        display.drawLine(position.y + snoutPadding, (position.x - (playerWidth / 2)) + 2, position.y + snoutPadding + snoutLength, (position.x - (playerWidth / 2)) + 2, SSD1306_WHITE);
        display.drawCircle(position.y + snoutPadding + snoutLength, (position.x - (playerWidth / 2)) + 2, snoutRadius, SSD1306_WHITE);
      }
      else
      {
        display.drawLine(position.y + snoutPosY, (position.x - (playerWidth / 2)) - snoutPosX, position.y + snoutPosY, (position.x - (playerWidth / 2)) - snoutPosX - snoutLength, SSD1306_WHITE);
        display.drawCircle(position.y + snoutPosY, (position.x - (playerWidth / 2)) - snoutPosX - snoutLength, snoutRadius, SSD1306_WHITE);
      }
    }

    void playerMovement()
    {
      position.x += (int16_t)(x);
      if (position.x > display.height() + 1)
      {
        position.x = 0;
      }
      if (position.x < 0)
      {
        position.x = display.height() + 1;
      }

      //player.position.y = bounce;
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
  class Platform
  {
  public:
    v2 position{10, 10};
    bool isBroken = false;
    bool isMovingPlatform = false;
    bool hasEnemy = false;
    bool playerAbove = false;
    bool isUsed = false;
    bool shouldReset = false;
    int movingPlatformDir = 1;
    bool playerDistBigEnoughToRestart = false;
    
    int id = random();
    void _start()
    {

    }
    bool CheckIfPlayerAbove() {
      if((position.x-11) < player.position.x  &&   player.position.x<(position.x+14 )) {
        if(player.position.y > position.y+3 && player.position.y < position.y+6) {
          return true;
        }
      }
      return false;
    }
    bool CheckIfPlayerDistBigEnoughToRestart(){
      if(player.position.y < position.y - 100){
        return true;
      }
      return false;
    }

    void Randomize() {
      
  #define RND random(0,99)
        isBroken = RND <30;
        movingPlatformDir = RND <50 ? -1:1;
        isMovingPlatform = RND <20;
        hasEnemy = RND <30;
        position.x = random(0,63);
    }
    void renderBrokenPlatform()
    {
      if(playerAbove){
        display.drawPixel(position.y, position.x,SSD1306_WHITE );
        return;
      }
      for (int i = 0; i < 5; i++)
      {
        display.drawLine(position.y +2, position.x - 4-3-1 + (i * 3), position.y - 3 +2, position.x - 1-1 -3+ (i * 3), SSD1306_WHITE);
      }
    }
    void renderPlatform()
    {

      for (int i = 0; i < 5; i++)
      {
        display.fillCircle(position.y , position.x - 5-1 + (3 * i) - (i==4?1:0), 2, SSD1306_WHITE);
      }
    }
    void _update()
    {
      
      playerAbove = CheckIfPlayerAbove();
      if (position.y < -50)
      {
        gameState.score += 10;
        position.y = 128;
        Randomize();
      }
      
      if(isUsed && !playerAbove){
      // shouldReset = true;
      }




      if(isMovingPlatform){
        position.x+= movingPlatformDir;
        if (position.x > 63){
          movingPlatformDir =  movingPlatformDir *-1;
        }
        if(position.x<0){
          movingPlatformDir =  movingPlatformDir *-1;
        }
      }
    // position.y = bounce;
      if (!isBroken)
      {
        renderPlatform();
      }
      else
      {
        renderBrokenPlatform();
      }
    }
  };
  class PlatformSpawner
  {
  public:
    float bounceDelta = 0;
    int currentPlatformIndex = 0;
    Platform platformPool[16];
    Platform currentPlatform;
    int rnd = 0;
    Platform getFromPool()
    {
      if (currentPlatformIndex > 15)
      {
        currentPlatformIndex = 0;
      }
      return platformPool[currentPlatformIndex++];
    }
    
    void _start()
    {
      for (int i = 0; i < 16; i++)
      {
        Platform p;
        p.id = i;
        p.position.y = p.id*10 -50 ; 
        p.Randomize();
        platformPool[i] = p;
      }
      currentPlatform = getFromPool();
    }
    float calculateBounce(float _delta){
      //return 4*abs(sin(_delta*3));
      float pi =  3.14159265f;
      float _delay= 0;//0.05f;
      float __x = _delta -_delay;
      if(_delta>= _delay) {
        return (((6*__x)/pi)-(9*__x*__x))/(pi*pi);
      }
      else {
        return 0;
      }
    }

  bool shouldBounce = true;

    void _update()
    {
  #define BOUNCE_RESET 3.14159265f / 3

    bool restart = true;
    for (int i = 0; i < 16; i++)
    {
        restart = restart && platformPool[i].CheckIfPlayerDistBigEnoughToRestart();
    }
    

    if(restart){
      gameState.gameOver = true;
    }

      bounceDelta+= 0.01f;
      bool isAboveAnyPlatform = false;
      for (int i = 0; i < 16; i++) {
        Platform _p = platformPool[i];
        isAboveAnyPlatform = isAboveAnyPlatform || _p.playerAbove;
        platformPool[i] = _p;
      }

      float res = calculateBounce(bounceDelta);

      if(res < 0) {
        if(isAboveAnyPlatform){
          bounceDelta = 0;
          res = calculateBounce(bounceDelta);
          shouldBounce = true;
        }
        else {
          shouldBounce = false;
        }
      }

      for (int i = 0; i < 16; i++) {
        Platform _p = platformPool[i];
        if(shouldBounce){
          if(_p.isBroken && _p.playerAbove){ 
            _p.position.y = -512;
          }
          _p.position.y -=400*res;
        }
        else if((_p.position.y < 255+_p.id*10))
          _p.position.y += shouldBounce ? 0:3;
        _p._update();
        platformPool[i] = _p;
      }
    }
  };
  PlatformSpawner platformSpawner;
  class Bullet
  {
  private:
    bool render = true;

  public:
    float bulletSize = 2;
    bool onInit = true;
    v2 position {0,0};
    v2 initPos{0, 128};
    float initPosX = 0; 
    float initPosY = 128;
    int bullet_id = random();
    int speed = 0;
    int translate = 0;
    int cspeed = 6;
    bool fire = false;

    float calculateInitPosY()
    {
      return player.position.y + player.snoutLength + player.snoutRadius + player.legLength + player.playerHeight + player.playerHeadpos - 4;
    }
    float calculateInitPosX()
    {
      return ((player.position.x - (player.playerWidth / 2)) + 2);
    }
    void _start()
    {
      speed = cspeed;
      initPos.x = calculateInitPosX();
      initPos.y = calculateInitPosY();
      position.x = initPos.x;
      position.y = initPos.y;
      translate = 0;
      fire = false;
      render = false;
    }
    void reset()
    {
      initPosX = calculateInitPosX();
      initPosY = calculateInitPosY();
      position.y = calculateInitPosY();
      position.x = calculateInitPosX();
      render = false;
      fire = false;
    }
    void hibernate()
    {
      speed = 0;
      position.x = 100;
      position.y = 100;
    }
    void renderBullet()
    {
      display.drawCircle(position.y, position.x, bulletSize, SSD1306_WHITE);
      display.drawPixel(position.y, position.x, SSD1306_WHITE);
    }
    void _update()
    {
      if (!fire){
        position.x = ((player.position.x - (player.playerWidth / 2)) + 2);
        position.y= -255;}
      else
      {
        translate += speed;
        position.y = initPos.y + translate;
      }
      if (fire)
      {
        renderBullet();
      }
      if (position.y > 128)
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
    int platformIndex = 0;
    void _start()
    {
        platformIndex = random(0,15);
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
      
       position.x = platformSpawner.platformPool[platformIndex].position.x;
       position.y = platformSpawner.platformPool[platformIndex].position.y+15;
       if(position.y< -49){
         _start();
       }
      renderEnemy();
    }
  };
  class EnemySpawner
  {
  public:
    int currentEnemyIndex = 0;
    Enemy enemyPool[1];
    Enemy currentEnemy;
    Enemy getFromPool()
    {
      if (currentEnemyIndex > 0)
      {
        currentEnemyIndex = 0;
      }
      return enemyPool[currentEnemyIndex++];
    }
    void _start()
    {
      for (int i = 0; i < 1; i++)
      {
        Enemy e{};
        e.id = i;
        e._start();
        enemyPool[i] = e;
        
      }
    }
    void _update()
    {
      for (int i = 0; i < 1; i++)
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
        
        for (int j = 0; j < 1; j++)
        { 
            // if(!player.isAttacking){
            //   break;
            // }
              
            v2 dist = v2::subtract(enemySpawner.enemyPool[j].position, bulletPool[i].position);
            float mag = dist.magnitude();
            //Serial.print("Magnitude: ");Serial.print(convert_int16_to_str( mag)); Serial.println("");
            // Serial.print("Dist X: ");Serial.print(convert_int16_to_str( dist.x*1000)); Serial.println("");
            // Serial.print("Dist Y: ");Serial.print(convert_int16_to_str( dist.y*1000)); Serial.println("");
            if(mag<11)
            {
              gameState.score += 50;
              Enemy _e = enemySpawner.enemyPool[i];
              _e.platformIndex = random(0,15);   
              enemySpawner.enemyPool[i] = _e;
              Bullet _b = bulletPool[i];
              _b._start();
              bulletPool[i] =_b;              
            }
            
        }
        

        bulletPool[i]._update();
      }
    }
  };
  BulletSpawner bulletSpawner;


  void START()
  {
    bulletSpawner._start();
    enemySpawner._start();
    platformSpawner._start();
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
    platformSpawner._update();
    delta++;
    char str[20];
    sprintf(str, "          SCORE: %d", gameState.score);
    printStr(str, 1);
  }



  void loop()
  {
    // Serial.print(" Score: "); Serial.print(convert_int16_to_str(gameState.score));Serial.println("");
    // Serial.print(" HighScore: "); Serial.print(convert_int16_to_str(gameState.highscore));Serial.println("");
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
    if(gameState.score > gameState.highscore){
        gameState.highscore = gameState.score;
    }
    if (gameState.gameOver)
    {
      platformSpawner._start();
      bulletSpawner._start();
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
