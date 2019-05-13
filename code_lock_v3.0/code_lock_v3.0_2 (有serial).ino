#include <Adafruit_Fingerprint.h>

SoftwareSerial mySerial(2, 3);      //RX,TX 与指纹传感器通信
SoftwareSerial Serial_terminal(8,9);//RX,TX 与终端通信

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;
void setup(){
  Serial_terminal.begin(9600);
  Serial.begin(115200);
  while(!Serial_terminal)
    Serial.println("Can't open serial_terminal");
  while (!Serial);                                                                  
  delay(100);
  finger.begin(57600);//设置传感器串口波特率
  finger_init();
}

void loop(){
  
  Serial_terminal.listen();
  char data_in;
  while(Serial_terminal.available()>0){
    data_in = Serial_terminal.read();//从终端接收消息，执行功能函数
    mySerial.listen(); 
    switch (data_in){
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
      case 'Q':
        
      case 'Y':
        while(1){//RESET终止
          mySerial.listen();
          finger_matching();//默认一直匹配指纹

          if(Serial.available()>0){//接收人脸识别结果
            char data;
            Serial_terminal.print(data = Serial.read());
            Serial.print(data);
          }
        }
      default:
        break;
    }
  }


}

//指纹模块初始化
void finger_init(){ 
  Serial.println("Fingerprint init over");
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } 
  else {
    Serial.println("Did not find fingerprint sensor :(");
    while(1){
      delay(1);
    }
  }
  
  finger.getTemplateCount();
  Serial.print("The number of fingerprints is "); 
  Serial.print(finger.templateCount); 
  Serial.println("Waiting for valid finger...");
}

//指纹匹配
void finger_matching(){
  int finger_label = getFingerprintIDez();
  if(finger_label != -1){
    Serial_terminal.print('O');
    Serial.println('O');
  }
    
  delay(50);//最高速率
}

//指纹录入
void finger_new(){
  Serial.println("Ready to enroll a fingerprint!");
  Serial_terminal.print('[');//发送显示指令
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);
  
  while (!getFingerprintEnroll() );
}

//指纹删除
void finger_delete(){
  Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
  Serial_terminal.print((char)91);//发送显示指令
  uint8_t id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }

  Serial.print("Deleting ID #");
  Serial.println(id);
  
  deleteFingerprint(id);
}

//从终端读取标号数据
uint8_t readnumber(void) {
  Serial_terminal.listen();//转换到监听终端数据
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial_terminal.available());
    num = Serial_terminal.parseInt();
  }
  mySerial.listen();
  return num;
}


//指纹匹配核心
//返回-1即为没有匹配到有效指纹，其他为指纹标签
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  //匹配到指纹
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}


uint8_t getFingerprintEnroll() {//指纹录入核心
  int p = -1;
  Serial_terminal.print((char)92);//发送显示指令
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
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
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial_terminal.print((char)93);//发送显示指令
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial_terminal.print((char)94);//发送显示指令
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
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
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial_terminal.print((char)95);//发送显示指令
    Serial.println("Stored!");
    return 1;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial_terminal.print((char)96);//发送显示指令
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial_terminal.print((char)96);//发送显示指令
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial_terminal.print((char)96);//发送显示指令
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    Serial_terminal.print((char)96);//发送显示指令
    return p;
  }   
}
//指纹删除核心
uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;
  
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    Serial_terminal.print((char)123);//发送显示指令
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    Serial_terminal.print((char)124);//发送显示指令
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    Serial_terminal.print((char)124);//发送显示指令
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    Serial_terminal.print((char)124);//发送显示指令
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    Serial_terminal.print((char)124);//发送显示指令
    return p;
  }   
}
