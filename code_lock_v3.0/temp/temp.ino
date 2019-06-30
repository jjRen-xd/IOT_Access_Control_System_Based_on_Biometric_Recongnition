#include <SPI.h>
#include <MFRC522.h>
#include <HardwareSerial.h>
#define MAXSIZE_IC_UID 128
 
#define SS_PIN 10
#define RST_PIN 9
 
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
    void print_All_Uid(){
      Serial.println("all data:");
      for(int i = 0;i < size;i++){
        Serial.print(i);
        Serial.print(": ");
        printUID(data[i]);
        Serial.println();
      }
    }

    byte data[MAXSIZE_IC_UID][4];
    int size = 0;
};
IC_UID icUid;


void new_uid(){
  static int label = 0; 
  /*Serial.print("Input your label:");
  while(Serial.available()<=0){}*/
  //while(Serial.read()<0){};
  Serial.println(label);
  Serial.println("Put your card... ...");

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
    Serial.print("十六进制UID：");
    printUID(icUid.data[label]);
    Serial.println();
    
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
 
void setup() { 
  Serial.begin(9600);
  SPI.begin(); // 初始化SPI总线
  rfid.PCD_Init(); // 初始化 MFRC522 
}
 
void loop() {
  while(Serial.read()>0);
  new_uid();
  //new_uid();
  icUid.print_All_Uid();
}
 
