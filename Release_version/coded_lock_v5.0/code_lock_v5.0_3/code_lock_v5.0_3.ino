#include <Servo.h>  //官方舵机控制库
#include <Adafruit_SSD1306.h> //第三方oled库
#include <SoftwareSerial.h> //软串口库
#include <SPI.h>
#include <MFRC522.h>
#include <HardwareSerial.h>

#define MAXSIZE_IC_UID 8
#define SS_PIN 10
#define RST_PIN 9
// #define SHOW_RFID_IN_SERIAL

/********************各功能状态*************/
bool state_face=0,state_finger=1,state_password=1,state_rfid=1,state_app = 1; 
bool state_input_way = 1; //1：默认红外，2：触摸键盘
SoftwareSerial Serial_key(2,3);   //RX,TX,与密码模块通信的软串口
/*************************处,理蜂鸣器***************************/
int buzzerPin = 7;  //蜂鸣器IO
/**************************处理舵机***************************/
Servo servo_door; //定义舵机变量名
int servoPin = 8; //舵机IO
/**********************处理oled************************/
#define OLED_RESET 4  //UNO:SDA=a4 SCL=a5 ;pro micro:SDA=2 SCL=3
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


/**********************************处理RFID函数*************************/
MFRC522 rfid(SS_PIN, RST_PIN); //实例化类
// 初始化数组用于存储读取到的NUID 
void printUID(byte *buffer, byte bufferSize = 4, bool form = 0) {
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : "");
    Serial.print(buffer[i], form == 0? HEX : DEC);
    Serial.print(' ');
  }
}

class IC_UID{
  public:
  /*IC_UID(){
    data = (byte**)malloc(sizeof(byte*) * MAXSIZE_IC_UID);
    for(int i = 0; i < MAXSIZE_IC_UID; i++) {
      data[i] = (byte*)malloc(sizeof(byte) * 4);
    }
  }
    
    void print_All_Uid(){
      Serial.println("all data:");
      for(int i = 0;i < size;i++){
        Serial.print(i);
        Serial.print(": ");
        printUID(data[i]);
        Serial.println();
      }
    }*/
    // byte **data;
    byte data[MAXSIZE_IC_UID][4];
    int size = 0;
};
IC_UID icUid;

/********************************处理舵机函数******************************/
void servo_open(){  
  for(int jiao=50;jiao<170;jiao++){
     servo_door.write(jiao); 
     delay(10);   
   }
   delay(5000);
   for(int jiao=170;jiao>50;jiao--){
     servo_door.write(jiao); 
     delay(10);   
   }
}

void servo_init(){
  servo_door.write(50);
  delay(1000);
}
/********************************处理声音函数*****************************/
void voice_error(){
  int i;
  for (i = 0; i < 100; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(1);
    digitalWrite(buzzerPin, LOW);
    delay(1);
  }
  for (i = 0; i < 100; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(2);
    digitalWrite(buzzerPin, LOW);
    delay(2);
  }
}

void voice_pass(){
  int i;
  for (i = 0; i < 100; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(1);
    digitalWrite(buzzerPin, LOW);
    delay(1);
  }
  for (i = 0; i < 50; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(2);
    digitalWrite(buzzerPin, LOW);
    delay(2);
  } 
  for (i = 0; i < 100; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(1);
    digitalWrite(buzzerPin, LOW);
    delay(1);
  }
}

void voice_input(){
  for (int i = 0; i < 60; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(1);
    digitalWrite(buzzerPin, LOW);
    delay(1);
  }
}
/******************************   NEW_OLED_DISPLAY  *********************************/
//显示输入的字符,不清屏
void oled_display_char(char x,int _size = 1,unsigned int _color = 1){
  display.setTextSize(_size);//字体大小
  if(_color){
    display.setTextColor(WHITE);//文字颜色
  }
  else{
    display.setTextColor(BLACK, WHITE);
  }
  display.print(x);
  display.display(); 
}

//清屏显示字符串
void oled_display_string(char *str,int _delayTime = 1000,int _size = 1,bool _color = 1){
  display.clearDisplay();
  display.setCursor(0,0);
  if(_color){
    display.setTextColor(WHITE);//文字颜色
  }
  else{
    display.setTextColor(BLACK, WHITE);
  }

  display.setTextSize(_size);
  display.println(str);
  display.display(); 
  if(_delayTime){
    delay(_delayTime);
    display.setCursor(0,0);
    display.clearDisplay();
  }
}

//主界面
void oled_Main_interface(unsigned char choice,bool setting_flag = 0){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.setTextColor(WHITE);
  if(setting_flag)
    display.println("    SYSTEM-SETTING");
  else
    display.println("    ACCESS-CONTROL");

  if(choice == 1){
    display.setTextColor(BLACK,WHITE);
  }
  display.print("1.Password");

  display.setTextColor(WHITE);
  display.print("  ");
  if(choice == 2){
    display.setTextColor(BLACK,WHITE);
  }
  display.println("2.Finger");

  display.setTextColor(WHITE);
  if(choice == 3){
    display.setTextColor(BLACK,WHITE);
  }
  display.print("3.Face");

  display.setTextColor(WHITE);
  display.print("      ");
  if(choice == 4){
    display.setTextColor(BLACK,WHITE);
  }
  display.println("4.Rfid");

  display.setTextColor(WHITE);
  if(choice == 5){
    display.setTextColor(BLACK,WHITE);
  }
  display.print("5.APP");

  display.setTextColor(WHITE);
  display.print("       ");
  if(choice == 6){
    display.setTextColor(BLACK,WHITE);
  }
  if(setting_flag)
    display.println("6.<--Exit");
  else
    display.println("6.Setting");
  display.display();
  if(choice != 0){
    delay(500);
    display.setCursor(0,0);
    display.clearDisplay();
  }
}

void oled_func_setting(char *func,unsigned char choice = 0){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.setTextColor(WHITE);
  display.print("    ");
  display.print(func);
  display.println("-SETTING");
  if(choice == 1)
    display.setTextColor(BLACK,WHITE);
  display.print("1.New");

  display.setTextColor(WHITE);
  display.print("      ");
  if(choice == 2)
    display.setTextColor(BLACK,WHITE);
  display.println("2.Delete");

  display.setTextColor(WHITE);
  if(choice == 3)
    display.setTextColor(BLACK,WHITE);
  display.print("3.Number");

  display.setTextColor(WHITE);
  display.print("   ");
  if(choice == 4)
    display.setTextColor(BLACK,WHITE);
  display.println("4.<-Back");

  display.setTextColor(WHITE);
  display.print("5.Change state:  ");
  switch (func[1]){ //显示该功能开关状态
    case 'A':
      if(state_face)
        display.print("ON");
      else
        display.print("OFF");
      break;
    case 'I':
      if(state_finger)
        display.print("ON");
      else
        display.print("OFF");
      break;
    case 'F':
      if(state_rfid)
        display.print("ON");
      else
        display.print("OFF");
      break;
    default:
      break;
  }
  display.display();

  if(choice > 0 && choice <5){
    delay(500);
    display.setCursor(0,0); 
    display.clearDisplay();
  }
}

void oled_passward_setting(unsigned char choice = 0){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.setTextColor(WHITE);
  display.println("  PASSWORD-SETTING");
  if(choice == 1)
    display.setTextColor(BLACK,WHITE);
  display.println("1.Change password");
  
  display.setTextColor(WHITE);
  display.print("2.Change state:  ");
  if(state_password)
    display.println("ON");
  else
    display.println("OFF");

  display.setTextColor(WHITE);
  if(choice == 3)
    display.setTextColor(BLACK,WHITE);
  display.print("3.<-Back");

  display.setTextColor(WHITE);
  display.print("   4.");
  if(state_input_way)
    display.println("Remote");
  else
    display.println("Keyboard");

  display.display();
  if(choice == 3 || choice == 4){
    delay(500);
    display.setCursor(0,0); 
    display.clearDisplay();
  }
}
/**********************************密码**********************************/
void password_unlock(){
  Serial_key.print('P');//向主控板1发送用密码解锁指令
  char data_in;
  oled_display_string("Enter your password:",0);
  while(1){
    if(Serial_key.available()>0){  //Arduino1-密码
      data_in = Serial_key.read();
      //Serial.println(data_in);
      switch (data_in){
        case 'O': //密码匹配成功,开门
          Serial.print('C');
          voice_pass(); 
          oled_display_string("Welcome.    :)",1000,2);
          servo_open();
          return;
        case 'L': //密码匹配失败,锁定
          voice_error();
          oled_display_string("Error! :(",1000,2);
          servo_init();
          return;
        case 'U': //密码匹逆序输入，报警
          Serial.print('U');
          voice_pass(); 
          oled_display_string("Welcome.    :)",1000,2);
          servo_open();
          return;
        default:  //显示输入的密码
          voice_input();
          oled_display_char(data_in,1,1);
          break;
      }
    }
  }
}

bool password_setting(){
  char data_in;
  oled_passward_setting(0);
  Serial_key.print('@');
  while(1){ //接收到用户输入就返回,1,2,3有效
    if(Serial_key.available()>0){
      switch (data_in = Serial_key.read()){
        case '1': //更换密码
          voice_input();
          oled_passward_setting(1);
          password_change();
          break;
        case '2': //功能开关
          voice_input();
          state_password = !state_password;
          break;
        case '3': //返回
          voice_input();
          oled_passward_setting(3);
          return 1;
        case '4': //输入方式开关
          voice_input();
          state_input_way = !state_input_way;
          if(state_input_way) //向主控板1发送默认输入方式
            Serial_key.print('R');
          else
            Serial_key.print('K');
          break;
        default:
          voice_error();
          oled_display_string("Invalid!",1000,2);
          break;
      }
      return 0;
    }
  }
}

void password_change(){
  Serial_key.print('C');
  oled_display_string("Input your key:",0);
  char data_in;
  while(1){ //接收到密码设定成功就返回,P有效
    if(Serial_key.available()>0){
      data_in = Serial_key.read();
      if(data_in == 'P'){ //接收到p即密码设定完成
        voice_pass();
        oled_display_string("OK! :)",1000,2);
        servo_init();
        return;
      }
      else{ //显示输入的密码
        voice_input();
        oled_display_char(data_in,1,1);
      }
    }
  }
}
/*************************************指纹***********************************/
bool finger_unlock(bool verify = 0){
  digitalWrite(4,HIGH);
  Serial.print('Y');  //向arduino2发送指令,开始识别
  Serial_key.print('@');  //向arduino1发送指令,准备接收任意键返回
  oled_display_string("Verify finger,any key to exit...");
  char data_in;
  while(1){
    if(Serial_key.available()>0){//按任意键返回,并RESET指纹Arduino
      Serial_key.read();
      voice_input();
      //通过D4口RESET Arduino2,解决双软串口无法同时监视的问题
      digitalWrite(4,LOW);
      return 0;
    }
    if(Serial.available()>0){      //Arduino2-指纹
      data_in = Serial.read();
      if(data_in == 'O'){
        digitalWrite(4,LOW);
        voice_pass();  
        if(!verify){
          oled_display_string("Welcome.    :)",1000,2);
          servo_open();
        }
        return 1;
      }
      oled_display_string("Verify finger,any key to exit...");
      while(Serial.read()>=0){}//清空指纹和人脸识别结果的缓存区
    }
  }
}

bool finger_setting(){
  char data_in;
  oled_func_setting("FINGER",0);
  Serial_key.print('@');
  while(1){ //接收到用户输入就返回,1,2,3有效
    if(Serial_key.available()>0){
      switch (data_in = Serial_key.read()){
        case '1':
          voice_input();
          oled_func_setting("FINGER",1);
          Serial.print('N');//向主控板2发送新建指纹指令
          finger_judge();
          break;
        case '2':
          voice_input();
          oled_func_setting("FINGER",2);
          Serial.print('M');//向主控板2发送删除指纹指令
          finger_judge();
          break;
        case '3':
          voice_input();
          oled_func_setting("FINGER",3);
          Serial.print('Z');//向主控板2发送查看指纹数量指令
          while(1){
            if(Serial.available()>0){
              oled_display_char(Serial.read(),4);
              delay(1000);
              break;
            }
          }
          break;
        case '4':
          voice_input();
          oled_func_setting("FINGER",4);
          return 1;
        case '5':
          voice_input();
          state_finger = !state_finger;
          break;
        default:
          voice_error();
          oled_display_string("Invalid!",1000,2);
          break;
      }
      return 0;
    }
  }
}

bool finger_judge(){
  int ID;
  char data_in;
  while(1){
    if(Serial.available()>0){
      data_in = Serial.read();
      switch (data_in)
      {
        case 92:
          oled_display_string("Press finger...");
          break;
        case 93:
          oled_display_string("Move finger away.");
          break;
        case 94:
          oled_display_string("Press finger again...");
          break;
        case 95:
          oled_display_string("Succeed :)",1000,2);
          return 1;
          break;
        case 96:
          oled_display_string("Failed,try again! :(");
          return 0;
          break;
        case 123:
          oled_display_string("Succeed :)",1000,2);
          break;
        case 124:
          oled_display_string("Failed,try again! :(");
          break;
        case 'S':
          oled_display_string("Put your face close...");
          break;
        case 'W':
          oled_display_string("Succeed :)",1000,2);
          return 1;
        default:
          break;
      }
      if(data_in >=95 && data_in <= 123)
        break;
    }
  }
}
/***************************************人脸*****************************/
bool face_setting(){
  char data_in;
  oled_func_setting("FACE",0);
  Serial_key.print('@');
  while(1){ //接收到用户输入就返回,1,2,3有效
    if(Serial_key.available()>0){
      switch (data_in = Serial_key.read()){
        case '1':
          voice_input();
          oled_func_setting("FACE",1);
          Serial.print('Q');//向主控板2发送新建人脸指令
          finger_judge();
          break;
        case '2':
          voice_input();
          Serial.print('D');//向主控板2发送新建人脸指令
          finger_judge();
          break;
        case '3':
          voice_input();
          oled_func_setting("FACE",3);
          Serial.print('T');//向主控板2发送查看人脸数量指令
          while(1){
            if(Serial.available()>0){
              oled_display_char(Serial.read(),4);
              delay(1000);
              break;
            }
          }
          break;
        case '4':
          voice_input();
          oled_func_setting("FACE",4);
          return 1;
        case '5':
          voice_input();
          state_face = !state_face;
          if(state_face)
            Serial.print('F');  //开启人脸识别
          else
            Serial.print('X');  //关闭人脸识别
          break;
        default:
          voice_error();
          oled_display_string("Invalid!",1000,2);
          break;
      }
      return 0;
    }
  }
}


/***********************************刷卡*********************************/
bool rfid_setting(){
  char data_in;
  oled_func_setting("RFID",0);
  Serial_key.print('@');
  while(1){ //接收到用户输入就返回,1-5有效
    if(Serial_key.available()>0){
      switch (data_in = Serial_key.read()){
        case '1':
          voice_input();
          oled_func_setting("RFID",1);
          new_uid();
          break;
        case '2':
          voice_input();
          oled_func_setting("RFID",2);
          delete_uid();
          break;
        case '3':
          voice_input();
          oled_func_setting("RFID",3);
          oled_display_char(icUid.size+48,4);
          delay(1000);
          break;
        case '4':
          voice_input();
          oled_func_setting("RFID",4);
          return 1;
        case '5':
          voice_input();
          state_rfid = !state_rfid;
          break;
        default:
          voice_error();
          oled_display_string("Invalid!",1000,2);
          break;
      }
      return 0;
    }
  }
}

void new_uid(){
  oled_display_string("Place your card...",0);
  static int label = 0; 
  /*Serial.print("Input your label:");
  while(Serial.available()<=0){}*/
  //while(Serial.read()<0){};
  #ifdef SHOW_RFID_IN_SERIAL
    Serial.println(label);
    Serial.println("Put your card... ...");
  #endif
  while(1){
    //找卡
    if ( ! rfid.PICC_IsNewCardPresent())
      continue;
  
    // 验证NUID是否可读
    if ( ! rfid.PICC_ReadCardSerial())
      continue;
  
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    
    // 将NUID保存到数组
    for (byte i = 0; i < 4; i++) {
      icUid.data[label][i] = rfid.uid.uidByte[i];
    }   
    icUid.size++;
    #ifdef SHOW_RFID_IN_SERIAL
      Serial.print("十六进制UID：");
      printUID(icUid.data[label]);
      Serial.println();
    #endif
    /*Serial.print("十进制UID：");
    printUID(rfid.uid.uidByte, rfid.uid.size, 1);
    Serial.println();*/
    
    // 使放置在读卡区的IC卡进入休眠状态,不再重复读卡
    rfid.PICC_HaltA();
  
    // 停止读卡模块编码
    rfid.PCD_StopCrypto1();
    label++;
    voice_pass();
    oled_display_string("Successfully created! :)");
    return;
  }
}

//匹配卡
int match_uid(){
  //找卡
  if (!rfid.PICC_IsNewCardPresent())
    return -1;

  // 验证NUID是否可读
  if (!rfid.PICC_ReadCardSerial())
    return -1;

  //读卡,保存卡id
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  byte temp_uid[4];
  for (int i = 0; i < 4; i++){
    temp_uid[i] = rfid.uid.uidByte[i];
  }

  //匹配
  for (int i = 0; i < icUid.size; i++){
    for (int j = 0; j < 4; j++){
      if (temp_uid[j] != icUid.data[i][j]){
        continue;
      }
      //匹配成功,并返回卡在icUid.data中的索引
      if (j == 3){
        // 使放置在读卡区的IC卡进入休眠状态,不再重复读卡
        rfid.PICC_HaltA();
        // 停止读卡模块编码
        rfid.PCD_StopCrypto1();
        return i;
      }
    }
  }
  // 使放置在读卡区的IC卡进入休眠状态,不再重复读卡
  rfid.PICC_HaltA();
  // 停止读卡模块编码
  rfid.PCD_StopCrypto1();
  return -1;
}

//删除已保存的卡
void delete_uid() {
  oled_display_string("Place your card...",0);
  int index;  
  while(1){
    index = match_uid();
    if(index >= 0)
      break;
  }
  for (int i = index + 1; i < icUid.size; i++){
    int j = i - 1;
    for(unsigned char k = 0;k<4;k++)
      icUid.data[j][k] = icUid.data[i][k];
  }
  icUid.size--;
  voice_pass();
  oled_display_string("Successfully deleted! :)");
}
/**************************************主函数***************************************/

bool system_setting(){
  char data_in;
  Serial_key.print('@');//向主控板1发送用接收选择信息指令
  oled_Main_interface(0,1);

  while(1){ //接收到用户输入就返回,1-6有效
    if(Serial_key.available()>0){
      switch (data_in = Serial_key.read()){
        case '1': //密码设置
          voice_input();
          oled_Main_interface(1,1);
          while(!password_setting());
          break;
        case '2': //指纹设置
          voice_input();
          oled_Main_interface(2,1);
          while(!finger_setting());
          break;
        case '3': //人脸设置
          voice_input();
          oled_Main_interface(3,1);
          while(!face_setting());
          break;
        case '4': //刷卡设置
          voice_input();
          oled_Main_interface(4,1);
          while(!rfid_setting());
          break;
        case '5': //APP设置
          voice_input();
          oled_Main_interface(5,1);
          if(state_app)
            oled_display_string("ON -->",1000,2);
          else{
            oled_display_string("OFF -->",1000,2);
          }
          state_app = !state_app;
          if(state_app)
            oled_display_string("ON",1000,2);
          else{
            oled_display_string("OFF",1000,2);
          }
          break; 
        case '6': //退出管理模式
          voice_input();
          oled_Main_interface(6,1);
          system_init();
          return 1;
        default:
          voice_error();
          oled_display_string("Invalid!",1000,2);
          break;
      }
      return 0;
    }
  }
}

bool system_init(){
  Serial.print('Z');  //向主控板2发送查看指纹数量指令
  if(state_input_way) //向主控板1发送默认输入方式
    Serial_key.print('R');
  else
    Serial_key.print('K');
  unsigned char finger_num;
  while(1){
    if(Serial.available()>0){
      finger_num = Serial.read();
      oled_display_string("Fingerprints:",0);
      oled_display_char(finger_num);
      delay(1000);
      if(finger_num >= '1' && finger_num <= '9'){
        oled_display_string("OK.",500,4);
        return 1;
      }
      if(finger_num == '0'){
        Serial.print('N');
        return (finger_judge());
      }
    }
  }
}

void setup(){
    Serial.begin(9600);//各串口初始化
    while(!Serial){}
    Serial_key.begin(9600);
    pinMode(4,OUTPUT);
    digitalWrite(4,LOW);

    SPI.begin(); // 初始化SPI总线
    rfid.PCD_Init(); // 初始化 MFRC522 
    
    pinMode(buzzerPin,OUTPUT);//蜂鸣器初始化
    servo_door.attach(servoPin);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //oled初始化
    display.display();
    delay(1000);
    display.clearDisplay();
  
    while(!system_init());  //系统初始化,设置初始指纹
}

void loop(){
  while(Serial.read()>=0){}//清空指纹和人脸识别结果的缓存区
  int match_result;
  char data_in;
  Serial_key.print('@');//向主控板1发送用接收选择信息指令
  oled_Main_interface(0);

  while(1){ //接收到用户输入就返回,1,2,3,4有效,其余交接指令判断函数处理
    if(Serial_key.available()>0){
      switch (data_in = Serial_key.read()){
        case '1': //密码解锁,需要进入
          voice_input();
          oled_Main_interface(1);
          if(state_password)
            password_unlock();
          else
            oled_display_string("Disabled. :(",1000,2);
          break;
        case '2': //指纹解锁,需要进入
          voice_input();
          oled_Main_interface(2);
          if(state_finger)
            finger_unlock();
          else
            oled_display_string("Disabled. :(",1000,2);
          break;
        case '3': //人脸解锁,默认开启
          voice_input();
          oled_Main_interface(3);
          if(state_face)
            oled_display_string("Enabled.   :)",1000,2);
          else
            oled_display_string("Disabled. :(",1000,2);
          break;
        case '4': //刷卡解锁,默认开启
          voice_input();
          oled_Main_interface(4);
          if(state_rfid)
            oled_display_string("Enabled.   :)",1000,2);
          else
            oled_display_string("Disabled. :(",1000,2);
          break;
        case '5': //APP解锁,默认开启
          voice_input();
          oled_Main_interface(5);
          if(state_app)
            oled_display_string("Enabled.   :)",1000,2);
          else
            oled_display_string("Disabled. :(",1000,2);
          break;
        case '6': //管理模式,进入系统设置
          voice_input();
          oled_Main_interface(6);
          if(finger_unlock(1))
            while(!system_setting());
          break;
        default:
          voice_error();
          oled_display_string("Invalid!",1000,2);
          break;
      }
      break;
    }
    if(state_rfid){
      match_result = match_uid();
      if(match_result >= 0){
        Serial.print('A');
        Serial.print(match_result);
        voice_pass();  
        oled_display_string("Welcome.    :)",0,2);
        servo_open();
        break;
      }
    }
    if(Serial.available()>0){
      voice_input();
      switch (data_in = Serial.read()){
        case 'O':
          voice_pass();  
          oled_display_string("Welcome.    :)",0,2);
          servo_open();
          break;
        case 'F':
          state_face = 1;
          Serial.print('F');  //开启人脸识别
          break;
        case 'f':
          state_face = 0;
          Serial.print('X');  //关闭人脸识别
          break;
        case 'q':
          Serial.print('Q');  //向主控板2发送新建人脸指令
          finger_judge();
          break;
        case 'd':
          Serial.print('D');//向主控板2发送删除人脸指令
          finger_judge();
          break;
        case 't':
          Serial.print('T');//向主控板2发送查看人脸数量指令
          while(1){
            if(Serial.available()>0){
              display.clearDisplay();
              display.setCursor(0,0);
              oled_display_char(Serial.read(),4);
              delay(1000);
              break;
            }
          }
          break;
        case 'P':
          state_password = 1;
          break;
        case 'p':
          state_password = 0;
          break;
        case 'b':
          password_change();
          break;
        case 'r':
          state_input_way = !state_input_way;
          if(state_input_way) //向主控板1发送默认输入方式
            Serial_key.print('R');
          else
            Serial_key.print('K');
          break;
        case 'Z':
          state_finger = 1;
          break;
        case 'z':
          state_finger = 0;
          break;
        case 'n':
          Serial.print('N');//向主控板2发送新建指纹指令
          finger_judge();
          break;
        case 'm':
          Serial.print('M');//向主控板2发送删除指纹指令
          finger_judge();
          break;
        case 'l':
          Serial.print('Z');//向主控板2发送查看指纹数量指令
          while(1){
            if(Serial.available()>0){
              display.clearDisplay();
              display.setCursor(0,0);
              oled_display_char(Serial.read(),4);
              delay(1000);
              break;
            }
          }
          break;
        case 'C':
          state_rfid = 1;
          break;
        case 'c':
          state_rfid = 0;
          break;
        case 'i':
          new_uid();
          break;
        case 'j':
          delete_uid();
          break;
        case 'k':
          display.clearDisplay();
          display.setCursor(0,0);
          oled_display_char(icUid.size+48,4);
          delay(1000);
          break;
        default:
          break;
      }
      break;
    }
  }
}  
