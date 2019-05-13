#include<IRremote.h> //第三方红外解码库
#include <TTP229.h> //第三方触摸键盘库
#include <SoftwareSerial.h> //软串口库

SoftwareSerial Serial_terminal(8,9);//RX,TX,设置模块的软串口

#define SHOW_IN_SERIAL

bool ray_or_TTP = 0;
/*************************处理触摸键盘**************************/
const int TTP229_sclPin = 2;  // 触摸时钟IO
const int TTP229_sdoPin = 3;  // 触摸数据IO
TTP229 ttp229(TTP229_sclPin,TTP229_sdoPin);

/************************处理红外信号***************************/
int recvPin = 11; //红外IO

IRrecv irrecv(recvPin);     //定义该io为红外接收口
decode_results results;    //使用decode_results类，用于存储解码结果

const byte ROWS = 7;        //解码含义表行列数
const byte COLS = 3;

long remote_Code[ROWS][COLS] = {  //遥控红外信号码值
  {0x00FFA25D,0x00FF629D,0x00FFE21D},
  {0x00FF22DD,0x00FF02FD,0x00FFC23D},
  {0x00FFE01F,0x00FFA857,0x00FF906F},
  {0x00FF6897,0x00FF9867,0x00FFB04F},
  {0x00FF30CF,0x00FF18E7,0x00FF7A85},
  {0x00FF10EF,0x00FF38C7,0x00FF5AA5},
  {0x00FF42BD,0x00FF4AB5,0x00FF52AD}
};
char remote_Mean[ROWS][COLS] = { //遥控码值对应意义
  {'A','B','C'},
  {'D','E','F'},
  {'G','H','#'},
  {'0','#','*'},
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'}
};
/*********************处理密码问题*************************/
#define pass_admin_len_MAX 64     //密码最大长度

char password_custom[pass_admin_len_MAX] = {NULL};   //用户输入密码
char password_admin[pass_admin_len_MAX] = {NULL};    //初始标准密码
int pass_admin_len = 0; //用户所输入初始密码的长度


/**************************自定义函数**********************/
char get_ray_key(){ //通过红外线接收密码
  irrecv.enableIRIn();  //红外端口使能，准备接受红外信号
  while(1){
    if(irrecv.decode(&results)){  //接收到信号并解码成功，把数据放入results变量中
      for(int i = 0;i<ROWS;i++){
        for(int j = 0;j<COLS;j++){
          if((results.value) == remote_Code[i][j]){ //该信号有效，返回信号含义
            Serial_terminal.print(remote_Mean[i][j]);
            #ifdef SHOW_IN_SERIAL
              Serial.print(remote_Mean[i][j]);
            #endif

            return (remote_Mean[i][j]); 
          }
        }
      }
      irrecv.resume();  //继续等待接收下一组信号 
    }
  }
}

char get_TTP229_key(){  //通过键盘接收密码
  int key = ttp229.ReadKey16(); // Blocking
  char key_c;
  if (key){
    switch(key){
      case 10:
        key_c = '0';
        break;
      case 11:
        key_c = 'A';
        break;
      case 12:
        key_c = 'B';
        break;
      case 13:
        key_c = 'C';
        break;
      case 14:
        key_c = 'D';
        break;
      case 15:
        key_c = 'E';
        break;
      case 16:
        key_c = '#';
        break;
      default:
        key_c = key+48;
    }
    ttp229.ReadKey16(); //清空缓存区,解决choice被计入密码的bug
    Serial_terminal.print(key_c);
    #ifdef SHOW_IN_SERIAL
      Serial.print(key_c);
    #endif
    
    return key_c;
  }
}

void pass_Init(){
  ray_or_TTP = choice_ray_or_TTP();
  pass_admin_len = 0;
  #ifdef SHOW_IN_SERIAL
    Serial.println("Input your init_key:");
  #endif
  char customKey;
  //接受用户输入的初始密码，以"EQ"作为结束符

  if(ray_or_TTP)
    customKey = get_ray_key();
  else
    customKey = get_TTP229_key();

  while(customKey != '#'){
    password_admin[pass_admin_len++] = customKey;
    if(ray_or_TTP)
      customKey = get_ray_key();
    else
      customKey = get_TTP229_key();
  }
  #ifdef SHOW_IN_SERIAL
    Serial.println("The initial password is set.");
  #endif
  Serial_terminal.print('P');
  //Serial.println(pass_admin_len);
}

void pass_lock(){
  ray_or_TTP = choice_ray_or_TTP();
  int pass_custom_len = 0;  //用户输入密码长度
  #ifdef SHOW_IN_SERIAL
    Serial.print("Input your passward:");
  #endif
  char customKey;
  //先将用户输入密码保存，以"#"作为结束符

  if(ray_or_TTP)
    customKey = get_ray_key();
  else
    customKey = get_TTP229_key();

  while(customKey != '#'){
    password_custom[pass_custom_len++] = customKey;
    if(ray_or_TTP)
      customKey = get_ray_key();
    else
      customKey = get_TTP229_key();
  }
  //Serial.println(pass_custom_len);

//判断密码是否正确，flag为标志位
  bool flag = 0;  //密码是否正确标志位，1为正确
  /*if(pass_custom_len > pass_admin_len){    //越位
    flag = 0;    
  }
  for(int k = 0;k<pass_admin_len;k++){      //密码不匹配
    if(password_custom[k] != password_admin[k]){
      flag = 0;  
    }
  }*/
  int custom_i = 0,admin_j = 0;
  bool last_same = 0;
  while((custom_i<pass_custom_len) && (admin_j<pass_admin_len)){
    if((password_custom[custom_i] != password_admin[admin_j]) && (last_same == 0)){
      custom_i++;
      last_same = 0;
    }
    else if((password_custom[custom_i] != password_admin[admin_j]) && (last_same == 1)){
      admin_j = 0;
      last_same = 0;
    }
    else if(password_custom[custom_i] == password_admin[admin_j]){
      custom_i++;
      admin_j++;
      last_same = 1;
    }
    if(admin_j == pass_admin_len)
      flag = 1;
  }
  
//判断完成执行的操作
  if(flag == 1){
    #ifdef SHOW_IN_SERIAL
      Serial.println("  PASS!");
    #endif
    Serial_terminal.print('O');
  }
  else{
    #ifdef SHOW_IN_SERIAL
      Serial.println("  ERROR!");
    #endif
    Serial_terminal.print('L');
  }
//执行完毕后，破坏原数组
  for(int i = 0;i<pass_admin_len;i++){
     password_custom[i] = {"x"}; 
  }
 
}

bool choice_ray_or_TTP(){//1红外,其他:键盘
  #ifdef SHOW_IN_SERIAL
    Serial.println("Input you choice:");
    Serial.println("  1:Unlock with remote control");
    Serial.println("  other:Unlock with keyboard");
  #endif
  delay(100);
  char user_choice;
  user_choice = get_ray_key();
  switch (user_choice)
  {
    case '1':
      #ifdef SHOW_IN_SERIAL
        Serial.print("Remote");
      #endif

      Serial_terminal.print('R');
      return 1;
      break;
    default:
      #ifdef SHOW_IN_SERIAL
        Serial.print("key");
      #endif

      Serial_terminal.print('K');
      return 0;
  }
} 


/*************************主函数******************************/
void setup(){
  Serial_terminal.begin(9600);
  Serial.begin(115200);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);

  irrecv.enableIRIn();
  pass_Init();  //设置初始密码
}
void loop(){
  static char data_in;
  if(Serial_terminal.available()>0){//接收终端指令
    switch(data_in = Serial_terminal.read()){
      case '@':
        get_ray_key();//向终端发送选择信息
        break;
      case 'K':
      case 'R'://执行密码解锁
        pass_lock();
        break;
      //default:
    }
  }
}
