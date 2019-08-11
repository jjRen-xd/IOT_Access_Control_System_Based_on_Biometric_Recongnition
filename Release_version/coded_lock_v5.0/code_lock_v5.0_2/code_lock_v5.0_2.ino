#include <Adafruit_Fingerprint.h>

// #define SHOW_IN_SERIAL
SoftwareSerial mySerial(14,15);      //RX,TX 与指纹传感器通信
SoftwareSerial Serial_terminal(8,9);//RX,TX 与终端通信
int reset_Pin = 5;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;
void setup(){
  Serial_terminal.begin(9600);
  Serial.begin(9600);
  pinMode(reset_Pin,INPUT);
 
  while(!Serial_terminal)
    #ifdef SHOW_IN_SERIAL
      Serial.println("Can't open serial_terminal");
    #endif
  while (!Serial);                                                                  
  delay(100);
  finger.begin(57600);//设置传感器串口波特率
  finger_init();
}

void loop(){
  Serial_terminal.listen();
  char data_in;
  if(Serial_terminal.available()>0){
    data_in = Serial_terminal.read();//从终端接收消息，执行功能函数 
    mySerial.listen();
    switch (data_in){
      //指纹指令
      case 'Y':
        finger_unlock();
        break;
      case 'N':
        finger_new();
        break;
      case 'M':
        finger_delete();
        break;
      case 'Z':
        finger.getTemplateCount();    
        Serial_terminal.print(finger.templateCount);
        break;
      //人脸指令，只传输
      case 'X': //关闭人脸识别
      case 'F': //开启人脸识别
      case 'Q': //新建人脸
      case 'T': //查看人脸数量
        Serial.print(data_in);
        break;
      //记录指令，传输2位
      case 'C':
        Serial.print('C');  //密码开锁记录
        break;
      case 'A':
        Serial.print('A');  //刷卡开锁记录
        break;
      case 'U':
        Serial.print('U');  //报警
        break;
      default:
        delay(500);
        Serial.print(data_in);
        break;
    }
  }
  Serial_terminal.listen();
  if(Serial.available()>0){
    Serial_terminal.print(data_in = Serial.read());
    // Serial.print(data_in);
  }
}

void finger_unlock(){
  while(1){//RESET终止
    mySerial.listen();
    finger_matching();//默认一直匹配指纹
    
    if(digitalRead(reset_Pin)<=0){//按下任意键就RESET
      return;
    }
  }
}

//指纹模块初始化
void finger_init(){ 
  #ifdef SHOW_IN_SERIAL
    Serial.println("Fingerprint init over");
  #endif
  if (finger.verifyPassword()) {
    #ifdef SHOW_IN_SERIAL
      Serial.println("Found fingerprint sensor!");
    #endif
  } 
  else {
    #ifdef SHOW_IN_SERIAL 
      Serial.println("Did not find fingerprint sensor :(");
    #endif
    while(1){
      delay(1);
    }
  }
  
  finger.getTemplateCount();
  #ifdef SHOW_IN_SERIAL 
    Serial.print("The number of fingerprints is "); 
    Serial.print(finger.templateCount); 
    Serial.println("Waiting for valid finger...");
  #endif
}

//指纹匹配
void finger_matching(){
  int finger_label = getFingerprintIDez();
  if(finger_label != -1 && finger_label != 0){
    Serial_terminal.print('O');//给终端发数据开锁
    //给linux发送数据记录开锁记录
    Serial.print('B');
    delay(500);
    Serial.print(finger_label); 

    #ifdef SHOW_IN_SERIAL
      Serial.println('');
      Serial.println('O');
    #endif
  }
    
  delay(50);//最高速率
}

//指纹录入
void finger_new(){
  #ifdef SHOW_IN_SERIAL
    Serial.println("Ready to enroll a fingerprint!");
    Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  #endif
  finger.getTemplateCount();
  #ifdef SHOW_IN_SERIAL
    Serial.println(finger.templateCount+1);
  #endif
  id = finger.templateCount + 1;
  // if (id == 0) {// ID #0 not allowed, try again!
  //    return;
  // }
  #ifdef SHOW_IN_SERIAL
    Serial.print("Enrolling ID #");
    Serial.println(id);
  #endif
  Serial_terminal.print((char)92);  //提示用户按下手指
  while (!getFingerprintEnroll() );
}

//指纹删除
void finger_delete(){
  #ifdef SHOW_IN_SERIAL
    Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
  #endif
  Serial_terminal.print((char)92);  //提示用户按下手指
  int finger_label = -1;
  while (finger_label == -1){ //等待按下有效指纹
    finger_label = getFingerprintIDez();
    if(finger_label == 0){
      Serial_terminal.print((char)124);  //按下的指纹没有被录入，删除失败
    }
  }
  uint8_t id = (uint8_t)finger_label;
  // if (id == 0) {// ID #0 not allowed, try again!
  //    return;
  // }

  #ifdef SHOW_IN_SERIAL
    Serial.print("Deleting ID #");
    Serial.println(id);
  #endif
  
  deleteFingerprint(id);
}

//指纹匹配核心
//返回0即为没有匹配到指纹，-1为没有指纹，其他为指纹标签
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return 0;
  
  //匹配到指纹
  #ifdef SHOW_IN_SERIAL
    Serial.print("Found ID #"); Serial.print(finger.fingerID); 
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
  #endif
  return finger.fingerID; 
}


uint8_t getFingerprintEnroll() {//指纹录入核心
  int p = -1;
  Serial_terminal.print((char)92);//发送显示指令
  #ifdef SHOW_IN_SERIAL
    Serial.print("Waiting for valid finger to enroll as #"); 
    Serial.println(id);
  #endif 
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    #ifdef SHOW_IN_SERIAL
      switch (p) {
        case FINGERPRINT_OK:
          Serial.println("Image taken");
          break;
        case FINGERPRINT_NOFINGER:
          Serial.println(".");
          break;
        case FINGERPRINT_PACKETRECIEVEERR:
          Serial.println("Communication error");
          break;
        case FINGERPRINT_IMAGEFAIL:
          Serial.println("Imaging error");
          break;
        default:
          Serial.println("Unknown error");
          break;
      }
    #endif 
  }
  // OK success!

  p = finger.image2Tz(1);
  #ifdef SHOW_IN_SERIAL
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
      case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        Serial_terminal.print((char)96);//发送显示指令
        return 1;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        Serial_terminal.print((char)96);//发送显示指令
        return 1;
      case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        Serial_terminal.print((char)96);//发送显示指令
        return 1;
      case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        Serial_terminal.print((char)96);//发送显示指令
        return 1;
      default:
        Serial.println("Unknown error");
        Serial_terminal.print((char)96);//发送显示指令
        return 1;
    }
  #endif

  Serial_terminal.print((char)93);//发送显示指令
  #ifdef SHOW_IN_SERIAL
    Serial.println("Remove finger");
  #endif
  delay(500);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  #ifdef SHOW_IN_SERIAL
    Serial.print("ID "); Serial.println(id);
  #endif
  p = -1;
  Serial_terminal.print((char)94);//发送显示指令
  #ifdef SHOW_IN_SERIAL
    Serial.println("Place same finger again");
  #endif
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    #ifdef SHOW_IN_SERIAL
      switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
      }
    #endif
  }

  // OK success!

  p = finger.image2Tz(2);
  #ifdef SHOW_IN_SERIAL
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
      case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        Serial_terminal.print((char)96);//发送显示指令
        return 1;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        Serial_terminal.print((char)96);//发送显示指令
        return 1;
      case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        Serial_terminal.print((char)96);//发送显示指令
        return 1;
      case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        Serial_terminal.print((char)96);//发送显示指令
        return 1;
      default:
        Serial.println("Unknown error");
        Serial_terminal.print((char)96);//发送显示指令
        return 1;
    }
  #endif
  
  // OK converted!
  #ifdef SHOW_IN_SERIAL
    Serial.print("Creating model for #");  Serial.println(id);
  #endif
  p = finger.createModel();
  
  if (p == FINGERPRINT_OK) {
    #ifdef SHOW_IN_SERIAL
      Serial.println("Prints matched!");
    #endif
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    #ifdef SHOW_IN_SERIAL
      Serial.println("Communication error");
    #endif
    Serial_terminal.print((char)96);//发送显示指令
    return 1;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    #ifdef SHOW_IN_SERIAL
      Serial.println("Fingerprints did not match");
    #endif
    Serial_terminal.print((char)96);//发送显示指令
    return 1;
  } else {
    #ifdef SHOW_IN_SERIAL
      Serial.println("Unknown error");
    #endif
    Serial_terminal.print((char)96);//发送显示指令
    return 1;
  }   
  
  #ifdef SHOW_IN_SERIAL
    Serial.print("ID "); Serial.println(id);
  #endif
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial_terminal.print((char)95);//发送显示指令
    #ifdef SHOW_IN_SERIAL
      Serial.println("Stored!");
    #endif
    return 1;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial_terminal.print((char)96);//发送显示指令
    #ifdef SHOW_IN_SERIAL
      Serial.println("Communication error");
    #endif
    return 1;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial_terminal.print((char)96);//发送显示指令
    #ifdef SHOW_IN_SERIAL
      Serial.println("Could not store in that location");
    #endif
    return 1;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial_terminal.print((char)96);//发送显示指令
    #ifdef SHOW_IN_SERIAL
      Serial.println("Error writing to flash");
    #endif
    return 1;
  } else {
    #ifdef SHOW_IN_SERIAL
      Serial.println("Unknown error");
    #endif
    Serial_terminal.print((char)96);//发送显示指令
    return 1;
  }   
}
//指纹删除核心
uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;
  
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    #ifdef SHOW_IN_SERIAL
      Serial.println("Deleted!");
    #endif
    Serial_terminal.print((char)123);//发送显示指令
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    #ifdef SHOW_IN_SERIAL
      Serial.println("Communication error");
    #endif
    Serial_terminal.print((char)124);//发送显示指令
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    #ifdef SHOW_IN_SERIAL
      Serial.println("Could not delete in that location");
    #endif
    Serial_terminal.print((char)124);//发送显示指令
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    #ifdef SHOW_IN_SERIAL
      Serial.println("Error writing to flash");
    #endif
    Serial_terminal.print((char)124);//发送显示指令
    return p;
  } else {
    #ifdef SHOW_IN_SERIAL
      Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    #endif
    Serial_terminal.print((char)124);//发送显示指令
    return p;
  }   
}          
