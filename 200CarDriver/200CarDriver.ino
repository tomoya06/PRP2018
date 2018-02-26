// 本代碼為執行端使用

#include <Servo.h> 
#include <Wire.h>
#include <JY901.h>

#define CODE_FORWARD      111
#define CODE_LEFT         101
#define CODE_RIGHT        110
#define CODE_BACKWARD     100
#define CODE_BACK         011
#define CODE_ZERO         000

// 馬達引脚
int pinLF = 14;      
int pinLB = 15;      
int pinRF = 16;      
int pinRB = 17;    

// 接收信號引脚
int pinCode1 = 9;
int pinCode2 = 10;
int pinCode3 = 11;
int pinSTATE = 12;

// 計數算子
int curCNT = 0;

// 左右轮计数
int leftWheelCNT = 0;
int rightWheelCNT = 0;

// 各方向位移计数
char DIR[1000];
int  CNT[1000];
int  node = 0;

void setup() {
    Serial.begin(9600);            
    pinMode(pinLB,  OUTPUT);        
    pinMode(pinLF,  OUTPUT);       
    pinMode(pinRB,  OUTPUT);       
    pinMode(pinRF,  OUTPUT);       

    pinMode(pinCode1, INPUT);
    pinMode(pinCode2, INPUT);
    pinMode(pinCode3, INPUT);
    pinMode(pinSTATE, OUTPUT);

    attachInterrupt(0,  leftWheel_cnt, RISING);  
    attachInterrupt(1, rightWheel_cnt, RISING);

    /*
     * 初始化：旋轉至角度為0的位置
     */
    dirInit();
}

void loop() {

    int codes = decoder();
    if (codes > CODE_ZERO) {
        setStateRunning();

        // 解码并执行指令
        switch (codes) {
            case CODE_FORWARD:
                
                break;
            case CODE_LEFT:
    
                break;
            case CODE_RIGHT:
    
                break;
            case CODE_BACKWARD:
    
                break;
            case CODE_BACK:
    
                break;
            default: 
                break;
        }
      
        setStateStopped();
    }
    
}

int decoder() {
    int c1 = digitalRead(pinCode1);
    int c2 = digitalRead(pinCode2);
    int c3 = digitalRead(pinCode3);

    return c1*100 + c2*10 + c3;
} 

void setStateRunning() {
    digitalWrite(pinSTATE, 1);
}

void setStateStopped() {
    digitalWrite(pinSTATE, 0);
}


/*
 * 行進策略：
 * 以碼盤爲準，行進一個單位 = 碼盤經過500格；
 * 向前/向後：直接前後一個單位
 * 向左/向右：原地轉之後前進一個單位
 * 
 * 計數策略：
 * 新建兩個超大數組，DIR為方向，有前F/后B/左L/右R四個值
 * CNT為對應該方向行進距離（碼盤數值）
 * 向前距離做纍計
 */

/*
 * 角度策略：
 * 開始前先校準，以陀螺儀0度為前向標準
 * 之後旋轉90度則以陀螺儀標準角度為標準
 * 
 * 前右后左依次為0-1-2-3
 */

void movement_forward() {
    setStateRunning();
    clearWheelCnt();

    advance();
    delay(2000);
    stopp();
    
    setStateStopped();

    
}

void movement_backward() {
    setStateRunning();
    clearWheelCnt();

    back();
    delay(2000);
    stopp();
    
    setStateStopped();
}

void movement_turnLeft() {
    setStateRunning();

    turnL90();
    delay(500);
    
    clearWheelCnt();
    advance();
    delay(2000);
    stopp;
    
    setStateStopped();
}

void movement_turnRight() {
    setStateRunning();

    turnR90();
    delay(500);
    
    clearWheelCnt();
    advance();
    delay(2000);
    stopp;
    
    setStateStopped();
}

boolean theyAreClose(float cur, int dir) {
    switch (dir) {
        case 0: if (cur > -5 && cur < 5) return true;
        case 1: if (cur > 85 && cur < 95) return true;
        case 2: if (cur >175 || cur <-175) return true;
        default: if (cur <-85 && cur >-95) return true;
    }
    return false;
}
 
float getAngle() {
    while (Serial.available()) { JY901.CopeSerialData(Serial.read()); }//Call JY901 data cope function 
    
    float a = (float)JY901.stcAngle.Angle[2]/32768*180;
    return a;
}

int getDir() {
    float cur = getAngle();
    int curDir;
    if (cur > -5 && cur < 5)        curDir = 0;
    else if (cur > 85 && cur < 95)  curDir = 1;
    else if (cur >175 || cur <-175) curDir = 2;
    else if (cur <-85 && cur >-95)  curDir = 3;

    return curDir;
}

float getTargetDir(float cur, boolean left) {
    int curDir;
    if (cur > -5 && cur < 5)        curDir = 0;
    else if (cur > 85 && cur < 95)  curDir = 1;
    else if (cur >175 || cur <-175) curDir = 2;
    else if (cur <-85 && cur >-95)  curDir = 3;

    if (left) curDir = (curDir+1)%4;
        else  curDir = (curDir+3)%4;

    return curDir;
    
}

void dirInit() {
    while (theyAreClose(getAngle(), 0)) {
        turnL();
    }
    stopp();
}

void turnL90() {
    int target = getTargetDir(getAngle(), true);
    while (theyAreClose(getAngle(), target)) {
        turnL();
    }
    stopp();
}

void turnR90() {
    int target = getTargetDir(getAngle(), false);
    while (theyAreClose(getAngle(), target)) {
        turnL();
    }
    stopp();
}

void advance() {
    analogWrite(pinRF, 250);
    analogWrite(pinRB,  0);
    analogWrite(pinLF, 250);
    analogWrite(pinLB, 0);  
}

void turnR() {
    analogWrite(pinRF, 250);
    analogWrite(pinRB,  0);
    analogWrite(pinLF, 0);
    analogWrite(pinLB, 150);
}

void turnL() {  
    analogWrite(pinRF, 0);
    analogWrite(pinRB, 150);
    analogWrite(pinLF, 150);
    analogWrite(pinLB, 0);
}    

void stopp() {
    digitalWrite(pinRB, LOW);
    digitalWrite(pinRF, LOW);
    digitalWrite(pinLB, LOW);
    digitalWrite(pinLF, LOW);
}

void back() {
    analogWrite(pinRF, 0);
    analogWrite(pinRB, 150);
    analogWrite(pinLF, 0);
    analogWrite(pinLB, 150);
}

void clearWheelCnt() {
    leftWheelCNT = 0;
    rightWheelCNT= 0;
}

void leftWheel_cnt() {
    leftWheelCNT++;
}

void rightWheel_cnt() {
    rightWheelCNT++;
}

int getCnt() {
    return (leftWheelCNT+rightWheelCNT)/2;
}


