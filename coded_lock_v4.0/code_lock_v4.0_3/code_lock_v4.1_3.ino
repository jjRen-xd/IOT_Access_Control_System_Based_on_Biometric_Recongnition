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

SoftwareSerial Serial_key(2,3);   //RX,TX,与密码模块通信的软串口

/*************************处理oled*********************/
void oled_display(char x,int _size = 1,int _color = 1);
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
    IC_UID(){
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
    }

//    byte data[MAXSIZE_IC_UID][4];
    byte **data;
    int size = 0;
};
IC_UID icUid;

void new_uid(){
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
  
    /*// 检查是否MIFARE卡类型
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println("不支持读取此卡类型");
      return;
    }*/
    
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
    
    // 使放置在读卡区的IC卡进入休眠状态，不再重复读卡
    rfid.PICC_HaltA();
  
    // 停止读卡模块编码
    rfid.PCD_StopCrypto1();
    label++;
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
      //匹配成功，并返回卡在icUid.data中的索引
      if (j == 3){
        // 使放置在读卡区的IC卡进入休眠状态，不再重复读卡
        rfid.PICC_HaltA();
        // 停止读卡模块编码
        rfid.PCD_StopCrypto1();
        return i;
      }
    }
  }
  // 使放置在读卡区的IC卡进入休眠状态，不再重复读卡
  rfid.PICC_HaltA();
  // 停止读卡模块编码
  rfid.PCD_StopCrypto1();
  return -1;
}

//删除已保存的卡
void delete_uid() {
  int index;  
  while(1){
    index = match_uid();
    if(index >= 0)
      break;
  }
  for (int i = index + 1; i < icUid.size; i++){
    int j = i - 1;
    icUid.data[j] = icUid.data[i];
  }
  icUid.size--;
}
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
/**********************************处理oled显示*******************************/

void oled_display(char x,int _size = 1,int _color = 1){
  display.setTextSize(_size);//字体大小
  if(_color = 1){
    display.setTextColor(WHITE);//文字颜色
  }
  else{
    display.setTextColor(BLACK, WHITE);
  }
  display.print(x);
  display.display(); 
}
void oled_init_begin(){
  display.setTextSize(1);//字体大小
  display.setTextColor(WHITE);//文字颜色
  display.setCursor(0,0);//设置游标位置 //(列,行)
  display.println("Input your init key:");
  display.display(); 
}
void oled_init_over(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("SET UP!!!");
  display.display(); 
  delay(1000);
  display.setCursor(0,0); 
  display.clearDisplay();
}
void oled_lock_begin(){
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Input your key:");
  display.display(); 
}
void oled_lock_error(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println("ERROR!!!");
  display.display(); 
  delay(1000);
  display.setCursor(0,0); 
  display.clearDisplay();
}
void oled_lock_pass(char name = ' '){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("Welcome!!!");
  display.println(name);
  display.display(); 
  delay(1000);
  display.setCursor(0,0); 
  display.clearDisplay();
}

void oled_ray_or_TTP(unsigned char choice = 0){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.setTextColor(WHITE);
  if(choice == 1)
    display.setTextColor(BLACK,WHITE);
  display.println("1.Select remote");

  display.setTextColor(WHITE);
  if(choice == 2)
    display.setTextColor(BLACK,WHITE);
  display.println("2.Select the keyboard");

  display.display(); 
  if(choice != 0){
    delay(500);
    display.setCursor(0,0); 
    display.clearDisplay();
  }
}

void oled_key_or_biosignature(unsigned char choice = 0){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.setTextColor(WHITE);
  if(choice == 1)
    display.setTextColor(BLACK,WHITE);
  display.println("1.Password unlock");

  display.setTextColor(WHITE);
  if(choice == 2)
    display.setTextColor(BLACK,WHITE);
  display.println("2.Biology unlock");

  display.setTextColor(WHITE);
  if(choice == 3)
    display.setTextColor(BLACK,WHITE);
  display.println("3.Finger ID");

  display.setTextColor(WHITE);
  if(choice == 4)
    display.setTextColor(BLACK,WHITE);
  display.print("4.Face ID");

  display.setTextColor(WHITE);
  display.print("     ");

  if(choice == 5)
    display.setTextColor(BLACK,WHITE);
  display.println("5.RFID");

  display.display();

  if(choice != 0){
    delay(500);
    display.setCursor(0,0); 
    display.clearDisplay();
  }
}

void oled_biology_unlocking(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("Press any key to exit biometrics......");
  display.display(); 
}
void oled_finger_choice(unsigned char choice = 0){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.setTextColor(WHITE);
  if(choice == 1)
    display.setTextColor(BLACK,WHITE);
  display.println("1.New fingerprint");

  display.setTextColor(WHITE);
  if(choice == 2)
    display.setTextColor(BLACK,WHITE);
  display.println("2.Delete fingerprint");

  display.setTextColor(WHITE);
  if(choice == 3)
    display.setTextColor(BLACK,WHITE);
  display.println("3.Check the number");

  display.setTextColor(WHITE);
  if(choice == 4)
    display.setTextColor(BLACK,WHITE);
  display.println("4.<--Back");

  display.display();

  if(choice != 0){
    delay(500);
    display.setCursor(0,0); 
    display.clearDisplay();
  }
}

void oled_face_choice(unsigned char choice = 0){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.setTextColor(WHITE);
  if(choice == 1)
    display.setTextColor(BLACK,WHITE);
  display.println("1.New face");
  
  display.setTextColor(WHITE);
  if(choice == 2)
    display.setTextColor(BLACK,WHITE);
  display.println("2.Delete face");

  display.setTextColor(WHITE);
  if(choice == 3)
    display.setTextColor(BLACK,WHITE);
  display.println("3.Check the number");

  display.setTextColor(WHITE);
  if(choice == 4)
    display.setTextColor(BLACK,WHITE);
  display.println("4.<--Back");

  display.display();

  if(choice != 0){
    delay(500);
    display.setCursor(0,0); 
    display.clearDisplay();
  }
}

void oled_rfid_choice(unsigned char choice = 0){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.setTextColor(WHITE);
  if(choice == 1)
    display.setTextColor(BLACK,WHITE);
  display.println("1.New IC-card");

  display.setTextColor(WHITE);
  if(choice == 2)
    display.setTextColor(BLACK,WHITE);
  display.println("2.Delete IC-card");

  display.setTextColor(WHITE);
  if(choice == 3)
    display.setTextColor(BLACK,WHITE);
  display.println("3.Check the number");

  display.setTextColor(WHITE);
  if(choice == 4)
    display.setTextColor(BLACK,WHITE);
  display.println("4.<--Back");

  display.display();

  if(choice != 0){
    delay(500);
    display.setCursor(0,0); 
    display.clearDisplay();
  }
}

void oled_face(unsigned int num){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  switch(num){
    case 1:
      display.println("Please close your face to the camera for a few seconds.");
      break;
    case 2:
      display.println("New Face Entry Success. :)");
      break;
    default:
      break;
  }
  display.display(); 

  if(num == 2){
    delay(500);
    display.setCursor(0,0); 
    display.clearDisplay();
  }
}

void oled_input_times(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("How many times has this been entered?");
  display.display(); 
}

void oled_input_label(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("What is the label of this entry?");
  display.display(); 
}
void oled_finger_press(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("Please press finger...");
  display.display(); 
}
void oled_finger_move(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("Move finger away!");
  display.display(); 
}
void oled_finger_press_again(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("Please press finger again...");
  display.display(); 
}
void oled_finger_success(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("Successfully created!");
  display.display(); 
  delay(1000);
  display.setCursor(0,0); 
  display.clearDisplay();
}
void oled_finger_deleted(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("Successfully deleted!");
  display.display(); 
  delay(1000);
  display.setCursor(0,0); 
  display.clearDisplay();
}
void oled_finger_fail(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("Failure, please try again!");
  display.display(); 
  delay(1000);
  display.setCursor(0,0); 
  display.clearDisplay();
}
/***********************************指令判断函数********************************/
void judge_Ordered(char data_in){ //根据接收到的键值执行命令
  switch(data_in){
    case 'O': //,某个模块识别成功,开门
      voice_pass();  
      oled_lock_pass();
      servo_open();

      break;
    case 'L': //某个模块识别失败,锁定
      voice_error();
      oled_lock_error();
      servo_init();

      break;
    case 'P'://已经在password_init中处理
      
      break;
    case 'R'://只接收，不反馈
      
      break;
    case 'K':
      
      break;
    default:
      voice_input();
      oled_display(data_in,1,1);
  }
}

void password_init(){
  choice_ray_or_TTP();
  oled_init_begin();
  char data_in;
  while(1){ //接收到密码设定成功就返回，P有效
    if(Serial_key.available()>0){
      data_in = Serial_key.read();
      switch(data_in){
        case 'P'://接收到p即初始密码设定完成
          voice_pass();
          oled_init_over();
          servo_init();

          return;
        default://其他即交给judge处理显示
          judge_Ordered(data_in);
      }
    }
  }
}

void choice_ray_or_TTP(){ //选择红外输入或者键盘输入
  oled_ray_or_TTP();
  while(1){ //接收到用户输入就返回，均有效
    if(Serial_key.available()>0){
      voice_input();
      Serial_key.read();//数据示例：1R，该语句过滤数字1
      switch (Serial_key.read()){
        case 'R':
          oled_ray_or_TTP(1);
          break;
        case 'K':
          oled_ray_or_TTP(2);
          break;
      }
      break;
    }
  }
}

void password_unlock(){
  char data_in;
  choice_ray_or_TTP();
  oled_lock_begin();
  while(1){
    if(Serial_key.available()>0){  //Arduino1-密码
      data_in = Serial_key.read();
      //Serial.println(data_in);
      
      judge_Ordered(data_in);
      if(data_in == 'L' || data_in == 'O')
        return;
    }
  }
}

void biology_unlock(){
  digitalWrite(4,HIGH);
  Serial.print('Y');  //向arduino2发送指令，开始识别
  Serial_key.print('@');  //向arduino1发送指令，准备接收任意键返回
  oled_biology_unlocking();
  char data_in;
  while(1){
    if(Serial_key.available()>0){//按任意键返回，并RESET指纹Arduino
      Serial_key.read();
      voice_input();
      //通过D4口RESET Arduino2，解决双软串口无法同时监视的问题
      digitalWrite(4,LOW);
      break;
    }
    if(Serial.available()>0){      //Arduino2-指纹
      data_in = Serial.read();
      judge_Ordered(data_in); 
      oled_biology_unlocking();
      while(Serial.read()>=0){}//清空指纹和人脸识别结果的缓存区
    }
  }
}

void finger_choice(){
  char data_in;
  oled_finger_choice();
  Serial_key.print('@');
  while(1){ //接收到用户输入就返回，1,2,3有效
    if(Serial_key.available()>0){
      switch (data_in = Serial_key.read()){
        case '1':
          voice_input();
          oled_finger_choice(1);
          Serial.print('N');//向主控板2发送新建指纹指令
          finger_judge();
          break;
        case '2':
          voice_input();
          oled_finger_choice(2);
          Serial.print('M');//向主控板2发送删除指纹指令
          finger_judge();
          break;
        case '3':
          voice_input();
          oled_finger_choice(3);
          Serial.print('Z');//向主控板2发送查看指纹数量指令
          while(1){
            if(Serial.available()>0){
              oled_display(Serial.read(),2);
              delay(500);
              break;
            }
          }
          break;
        case '4':
          voice_input();
          oled_finger_choice(4);
          break;
        default:
          voice_error();
          oled_lock_error();
          break;
      }
      break;
    }
  }
}

void face_choice(){
  char data_in;
  oled_face_choice();
  Serial_key.print('@');
  while(1){ //接收到用户输入就返回，1有效
    if(Serial_key.available()>0){
      switch (data_in = Serial_key.read()){
        case '1':
          voice_input();
          oled_face_choice(1);
          new_face();
          break;
        case '2':
          voice_input();
          oled_face_choice(2);
          break;
        case '3':
          voice_input();
          oled_face_choice(3);
          check_face_number();
          break;
        case '4':
          voice_input();
          oled_face_choice(4);
          break;
        default:
          voice_error();
          oled_lock_error();
          break;
      }
      break;
    }
  }
}

void rfid_choice(){
  char data_in;
  oled_rfid_choice();
  Serial_key.print('@');
  while(1){ //接收到用户输入就返回，1有效
    if(Serial_key.available()>0){
      switch (data_in = Serial_key.read()){
        case '1':
          voice_input();
          oled_rfid_choice(1);
          new_uid();
          break;
        case '2':
          voice_input();
          oled_rfid_choice(2);
          delete_uid();
          break;
        case '3':
          voice_input();
          oled_rfid_choice(3);
          oled_display(icUid.size + 48,2);
          delay(1000);
          break;
        case '4':
          voice_input();
          oled_rfid_choice(4);
          break;
        default:
          voice_error();
          oled_lock_error();
          break;
      }
      break;
    }
  }
}

void new_face(){
  char data_in;
  Serial.print('Q');//发送新建人脸指令
  while(1){
    if(Serial.available()>0){ //接收linux的反馈指令
      while(1){
        switch (data_in = Serial.read()){
          case 'S': //提示放置人脸
            oled_face(1);
            break;
          case 'W': //提示录入成功
            oled_face(2);
            break;
          default:
            break;
        }
        if(data_in == 'W')  //录入成功就退出
          break;
      }
      if(data_in == 'W')  //录入成功就退出
        break;
    }
  }
}

void check_face_number(){
  Serial.print('T');
  while(1){
    if(Serial.available()>0){
      oled_display(Serial.read()+48,2);
      delay(1000);
      break;
    }
  }
}
void finger_judge(){
  int ID;
  char data_in;
  while(1){
    if(Serial.available()>0){
      data_in = Serial.read();
      switch (data_in)
      {
        case 91:
          oled_input_label();
          ID = get_choice();
          oled_display(ID);
          Serial.print(ID-48);
          break;
        case 92:
          oled_finger_press();
          break;
        case 93:
          oled_finger_move();
          break;
        case 94:
          oled_finger_press_again();
          break;
        case 95:
          oled_finger_success();
          break;
        case 96:
          oled_finger_fail();
          break;
        case 123:
          oled_finger_deleted();
          break;
        case 124:
          oled_finger_fail();
          break;
        default:
          break;
      }
      if(data_in >=95 && data_in <= 123)
        break;
    }
  }
}

char get_choice(){
    Serial_key.print('@');
    while(1){
      if(Serial_key.available()>0){
        voice_input();
        return (Serial_key.read());
      }
    }
}
/**************************************主函数***************************************/

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
  
    password_init();//密码初始化
}

void loop(){
  while(Serial.read()>=0){}//清空指纹和人脸识别结果的缓存区
  int match_result;
  char data_in;
  Serial_key.print('@');//向主控板1发送用接收选择信息指令
  oled_key_or_biosignature();

  while(1){ //接收到用户输入就返回，1,2,3,4有效，其余交接指令判断函数处理
    if(Serial_key.available()>0){
      switch (data_in = Serial_key.read()){
        case '1':
          voice_input();
          oled_key_or_biosignature(1);
          Serial_key.print('R');//向主控板1发送用密码解锁指令
          password_unlock();
          break;
        case '2':
          voice_input();
          oled_key_or_biosignature(2);
          biology_unlock();
          break;
        case '3':
          voice_input();
          oled_key_or_biosignature(3);
          finger_choice();
          break;
        case '4':
          voice_input();
          oled_key_or_biosignature(4);
          face_choice();
          break;
        case '5':
          voice_input();
          oled_key_or_biosignature(5);
          rfid_choice();
          break;
        default:
          voice_error();
          oled_lock_error();
          break;
      }
      break;
    }
    match_result = match_uid();
    if(match_result >= 0){
      voice_pass();  
      oled_lock_pass(match_result+48);
      servo_open();
      break;
    }
  }
}   
