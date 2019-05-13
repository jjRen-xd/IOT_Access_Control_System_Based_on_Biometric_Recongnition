#include <Servo.h>  //官方舵机控制库
#include <Adafruit_SSD1306.h> //第三方oled库
#include <SoftwareSerial.h> //软串口库


SoftwareSerial Serial_key(2,3);   //RX,TX,与密码模块通信的软串口

/*************************处理oled*********************/
void oled_display(char x,int _size = 1,int _color = 1);
/*************************处,理蜂鸣器***************************/
int buzzerPin = 10;  //蜂鸣器IO
/**************************处理舵机***************************/
Servo servo_door; //定义舵机变量名
int servoPin = 9; //舵机IO
/**********************处理oled************************/
#define OLED_RESET 4  //SDA=a4  SCL=a5 
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


/********************************处理舵机函数******************************/
void servo_open(){  
  for(int jiao=50;jiao<170;jiao++)
   {
     servo_door.write(jiao); 
     delay(10);   
   }
   delay(5000);
   for(int jiao=170;jiao>50;jiao--)
   {
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
void oled_lock_pass(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println("PASS!!!");
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
  display.println("4.Face ID");

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

void oled_face(unsigned int num){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  switch(num){
    case 1:
      display.println("Collecting Face information, please wait...");
      break;
    case 2:
      display.println("Generating CSV, please wait...");
      break;
    case 3:
      display.println("Training, please wait...");
      break;
    case 4:
      display.println("New Face Success!!!");
      break;
    default:
      break;
  }
  display.display(); 

  if(num == 4){
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
  Serial.print('Y');  //向arduino2发送指令，开始识别
  Serial_key.print('@');  //向arduino1发送指令，准备接收任意键返回
  oled_biology_unlocking();
  char data_in;
  while(1){
    if(Serial_key.available()>0){
      Serial_key.read();
      voice_input();

      digitalWrite(13,LOW);//RESTE Arduino2
      delay(100);
      digitalWrite(13,HIGH);
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

void new_face(){
  char data_in;
  Serial.print('Q');//发送新建人脸指令
  while(1){
    if(Serial.available()>0){
      while(1){
        switch (data_in = Serial.read()){
          case 'S':
            oled_input_label();
            data_in = get_choice();
            Serial.print(data_in);
            oled_display(data_in,2); 
            break;
          case 'X':
            oled_input_times();
            data_in = get_choice();
            Serial.print(data_in);
            oled_display(data_in,2); 
            break;
          case 'T':
            oled_face(1);
            break;
          case 'U':
            oled_face(2);
            break;
          case 'V':
            oled_face(3);
            break;
          case 'W':
            oled_face(4);
            break;
          default:
            break;
        }
        if(data_in == 'W')
          break;
      }
      if(data_in == 'W')
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
    while(!Serial){
        
    }
    Serial_key.begin(9600);
       
    pinMode(13,OUTPUT); //关指示灯
    digitalWrite(13,LOW);
    delay(100);
    digitalWrite(13,HIGH);

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
  
  char data_in;
  Serial_key.print('@');//向主控板1发送用接收选择信息指令
  oled_key_or_biosignature();

  while(1){ //接收到用户输入就返回，1,2,3有效，其余交接指令判断函数处理
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
        default:
          voice_error();
          oled_lock_error();
          break;
      }
      break;
    }
  }
}   
