#include <Keypad.h>    //矩阵键盘库头文件
#include <MsTimer2.h>    //定时器库头文件
#include "LedControl.h"  //max7219库头文件

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {3, 8, 7, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 2, 6}; //connect to the column pinouts of the keypad
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
//初始化矩阵键盘 
char key = customKeypad.getKey();

int type = 0;  //输入密码计数器清零
int correctkey = 0;  //正确密码计数清零
int passwordAccount = 7;  //定义密码位数
char pass[7] = {'7','3','5','5','6','0','8'};  //定义密码
char newpass[7];  // 定义输入密码存入数组的位数
int tick = 0; //拆弹计数器清零
int SW = 15;  //拓展开关引脚
int beep = 9; //蜂鸣器引脚
int LED = 18;  //多彩LED引脚
int defuseTime = 700;  //中断时间
int val;
LedControl lc=LedControl(14,16,10,1); //max7219引脚
/*
 pin 14 is connected to the DIn 
 pin 16 is connected to the CLK 
 pin 10 is connected to CS 
 */
void welcome(void){  //初始化界面显示8位'8’
  for (int i=0;i<8;i++){
    lc.setChar(0,i,'8',false);
  }
  delay(1000);
  digitalWrite(SW, LOW);
  lc.clearDisplay(0);
}

void inputpassword(void){  //等待输入界面显示8位'-'
  for (int i=0;i<7;i++){
    lc.setChar(0,i,'-',false);
  }  
}

void correct(void){
  for (int i=0;i<7;i++){
    lc.setChar(0,i,'_',false);
  }
}

//触发中断检测按键
void defuse(){
  int val = digitalRead(SW);
  Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
  char key = customKeypad.getKey();
  if (key != NO_KEY && key == '#'){   //如果#号键被按下
    if (tick == 0){
      digitalWrite(beep, HIGH);  //开始拆弹提示音
      delay(50);
      digitalWrite(beep, LOW);  
      delay(50);
      digitalWrite(beep, HIGH);
      delay(50);
    }
    if(val==1){  //没有拆弹钳
      int ticks = tick/2;
      int p = passwordAccount - 1 - ticks ;
      lc.setChar(0,p,pass[ticks],false);  //依次显示破解密码
      Serial.println(ticks); 
      Serial.println(tick); 
      Serial.println(val);//串口调试
      delay(1000);
      tick++;
      if(ticks > 7){  //如果计数器累计按下8秒，拆弹成功
        digitalWrite(beep, LOW);
        digitalWrite(LED, LOW);
        tick = 0;
        type = 0;
        correctkey = 0;      
        //lc.clearDisplay(0);
        //inputpassword();
        delay(200);
        while(1){
        }
      }
    }
    else{ //有拆弹钳
      int p = passwordAccount - 1 - tick ;
      lc.setChar(0,p,pass[tick],false);  //依次显示破解密码
      Serial.println(tick); 
      Serial.println(val);//串口调试
      delay(1000);
      tick++;
      if(tick > 7){  //如果计数器累计按下8秒，拆弹成功
        digitalWrite(beep, LOW);
        digitalWrite(LED, LOW);
        tick = 0;
        type = 0;
        correctkey = 0;      
        //lc.clearDisplay(0);
        //inputpassword();
        delay(200);
        while(1){
        }
      }
    }
  }
  else if (key == NO_KEY && tick != 0){  //中间松手检测，拆弹计数清零
    tick = 0;  //拆弹计数器清零
    correct();  //显示复位
  }
  else{
    tick = 0;  //拆弹计数器清零
  }
}

void boom(void) {  //爆炸倒计时
  int delaytime = 1000;
  for (int i = 0;i < 75; i ++){
    delaytime = 1000 - 13*i;
    digitalWrite(beep, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);                       // wait for a second
    digitalWrite(beep, LOW);    // turn the LED off by making the voltage LOW
    delay(delaytime);   
    } 
  for (int i=0;i<7;i++){
    lc.setChar(0,i,'8',false);
  }
  digitalWrite(LED, HIGH);
  digitalWrite(beep, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(SW, HIGH);
  delay(2000);
}

void setup(){
  pinMode(SW,INPUT_PULLUP);  //拓展开关拉高
  pinMode(beep, OUTPUT);
  Serial.begin(9600);  //串口调试波特率
  MsTimer2::set(defuseTime, defuse); //设置中断，每1000ms检测是否拆弹 defuse()
  MsTimer2::start(); //开始计时_开启定时器中断
  /* start display */
  digitalWrite(beep, HIGH);
  delay(70);
  digitalWrite(beep, LOW);
  delay(100);
  digitalWrite(beep, HIGH);
  delay(70);
  digitalWrite(beep, LOW);
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
  /* 初始化 */
  welcome();
  inputpassword();
}
  
void loop(){
  char key = customKeypad.getKey();
  if (key != NO_KEY && type < passwordAccount){  //检测到输入密码且密码位数不足
    newpass[type] = key;
    int pos = passwordAccount - type - 1;
    lc.setChar(0,pos,key,false);
    delay(200);
    if(newpass[type] == pass[type]){
      correctkey++;
    }
    type++;  //输入密码计数
  }
  if(correctkey == passwordAccount && type == passwordAccount){  //有7位输入且全部正确
    type = 0;
    correctkey = 0;
    lc.clearDisplay(0);  //数码管清屏
    correct();  //显示7位'_'
    boom();  //倒计时子程序
    digitalWrite(beep, LOW);
    digitalWrite(LED, LOW);
    //digitalWrite(SW, LOW);
    inputpassword();
  }
  if(correctkey < passwordAccount && type == passwordAccount){  //有7位输入但不正确
    type = 0;
    correctkey = 0;
    lc.clearDisplay(0);
    inputpassword();
  }
}
