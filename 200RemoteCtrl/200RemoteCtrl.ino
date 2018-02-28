// 本代碼為信號端使用

#include <SPI.h>
#include <Ethernet.h>
#include <LeweiTcpClient.h>
#include <EEPROM.h>

#define LW_USERKEY  "61ae3f8608624a17867efdb95252dc3e"
#define LW_GATEWAY  "01"

#define MSG_OK      " COMMAND ACCEPTED"
#define MSG_NO      " DENIED. RUNNING"

#define CODE_FORWARD      111
#define CODE_LEFT         101
#define CODE_RIGHT        110
#define CODE_BACKWARD     100
#define CODE_BACK         011
#define CODE_ZERO         000

#define LED_PIN           3

int pinCode1 = 7;
int pinCode2 = 6;
int pinCode3 = 5;
// pinSTATE = A0;

long lastMillis = 0;
boolean isCodesCleared = 1;

int i = 1;
String state = MSG_OK;

LeweiTcpClient *client;

void setup()
{
    Serial.begin(115200);
    pinMode(pinCode1, OUTPUT);
    pinMode(pinCode2, OUTPUT);
    pinMode(pinCode3, OUTPUT);

    pinMode(LED_PIN, OUTPUT);

    Serial.println("Loading...");
    client = new LeweiTcpClient(LW_USERKEY, LW_GATEWAY);
  
    //UserFunction uf1(functionIWrote,"functionNameDefinedOnWeb");
    //client->addUserFunction(uf1);
    //UserFunction uf2(ledOn,"turnLedOn");
    //client->addUserFunction(uf2);
    //UserFunction uf3(ledOff,"turnLedOff");
    //client->addUserFunction(uf3);
    UserFunction uf4(goForward,"forward");
    client->addUserFunction(uf4);
    UserFunction uf5(goBackward,"backward");
    client->addUserFunction(uf5);
    UserFunction uf6(goLeft,"left");
    client->addUserFunction(uf6);
    UserFunction uf7(goRight,"right");
    client->addUserFunction(uf7);
    UserFunction uf8(goBack,"goback");
    client->addUserFunction(uf8);

    digitalWrite(LED_PIN, 1);
}

void loop()
{
    client->keepOnline();

    if (abs(millis() - lastMillis) > 500) {
        clearCodes();
        lastMillis = millis();
    }
}

/*
 * 信息传递策略:
 * 3个指令信号口，1个状态信号口。
 * 信号端：收到服务器下达指令后，检查状态口信号；
 *        若执行端空闲，则向服务器反馈接受信号，并传送指令，更新时间戳；
 *        若执行段忙碌，则向服务器反馈拒绝信号，不传送指令，不更新时间戳；
 *        时间戳后2000ms，清空指令。
 * 执行端：空闲时，循环检查信号端是否有指令。
 *        若收到指令，更新状态口信号为忙碌，并执行指令，结束后更新状态口信号为空闲。
 */

// 往服务器发送反馈信息。
void sendMSG(char* str) {
    // 检查状态口信号
    updateState();
    
    if (state.equals(MSG_NO)) {
        client->setRevCtrlMsg("true", "Received but denied.");
    } else {
        client->setRevCtrlMsg("true", str);
    }
}

// 發送指令信號。在向服务器发完信息后使用。
void writeCodes(int codes) {
    if (state.equals(MSG_NO)) return ;
    
    int c1 = codes/100;
    int c2 = (codes-c1*100)/10;
    int c3 = codes%10;
    
    digitalWrite(pinCode1, c1); delay(2);
    digitalWrite(pinCode2, c2); delay(2);
    digitalWrite(pinCode3, c3);

    isCodesCleared = 0;
    
    lastMillis = millis();
}

void clearCodes() {
    if (isCodesCleared) return;
    
    digitalWrite(pinCode1, 0); delay(2);
    digitalWrite(pinCode2, 0); delay(2);
    digitalWrite(pinCode3, 0);

    isCodesCleared = 1;
}

void functionIWrote(char * p1, char * p2)
{
    client->setRevCtrlMsg("true","message to server");
    Serial.println(p1);
    Serial.println(p2);
}

void ledOn()
{
    sendMSG("on");
    digitalWrite(LED_PIN, 1);
}
void ledOff()
{
    sendMSG("off");
    digitalWrite(LED_PIN, 0);
}

void goForward() {
    sendMSG("forward");
    writeCodes(CODE_FORWARD);
}

void goLeft() {
    sendMSG("left");
    writeCodes(CODE_LEFT);
}

void goRight() {
    sendMSG("right");
    writeCodes(CODE_RIGHT);
}

void goBackward() {
    sendMSG("backward");
    writeCodes(CODE_BACKWARD);
}

void goBack() {
    sendMSG("back to base");
    writeCodes(CODE_BACK);
}

void updateState() {
    int val = analogRead(A0);
    if (val < 100) 
        state = MSG_OK;
    else
        state = MSG_NO;

    Serial.println(val);
}

