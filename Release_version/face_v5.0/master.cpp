#include "face_master.hpp"
#include"LinuxSerial.hpp"
#include <iostream>
#include <thread>
#include <string>
 
using namespace std;

Face _face;
CLinuxSerial _arduino(0,9600);

void thread_FaceRecon(){
   _face.Recongnise_face(0);
}

//从保存的文件中读取arduino的发过来的指令
char receive_Arduino_Data(){  //由于fileChange是static的缘故，两个receive不能合并
   std::ifstream arduino_Receive("../receive_arduino_data/ARDUINO_COMMAND.txt");
   if(!arduino_Receive){
      string error_message = "No valid input file was given,please check the given filename.";
      CV_Error(CV_StsBadArg,error_message);
   }

   char command;
   static int fileChange = 0,fileChange_last = 0;
   arduino_Receive >> command;
   arduino_Receive >> fileChange;
   if(fileChange != fileChange_last){
      fileChange_last = fileChange;
      arduino_Receive.close();
      return command;
   }
   arduino_Receive.close();
   return '#';
}
//从保存的文件中读取blinker的发过来的指令
char receive_Blinker_Data(){
   std::ifstream blinker_Receive("../../BLINKER/db/BLINKER_COMMAND.txt");
   
   if(!blinker_Receive){
      string error_message = "No valid input file was given,please check the given filename.";
      CV_Error(CV_StsBadArg,error_message);
   }

   char command;
   static int fileChange = 0,fileChange_last = 0;
   blinker_Receive >> command;
   blinker_Receive >> fileChange;
   if(fileChange != fileChange_last){
      fileChange_last = fileChange;
      blinker_Receive.close();
      return command;
   }
   blinker_Receive.close();
   return '#';
}

void record_unlock_message(string unlock_name,string unlock_way){
   ofstream NEWEST_DATA("../../BLINKER/db/NEWEST_DATA.txt");
   ofstream ALL_RECORD_DATA("../RECORD_DATA/ALL_RECORD_DATA.txt",ios_base::app);
   string unlock_time = _face.getTime();
   
   NEWEST_DATA<<"["<<unlock_time<<"]-[Name of Unlocker]:"<<unlock_name<<"-[Unlocking mode]:"<<unlock_way<<endl; 
   ALL_RECORD_DATA<<"["<<unlock_time<<"]-[Name of Unlocker]:"<<unlock_name<<"-[Unlocking mode]:"<<unlock_way<<endl; 
   
   NEWEST_DATA.close();
   ALL_RECORD_DATA.close();
}
/* //test
int main(){
   _face.show_detail_data();
   _face.New_face();
   _face.show_detail_data();
   _face.faceReconFlag = true;
   _face.Delete_face();

   // _face.faceReconFlag = true;
   // _face.Recongnise_face(0);

} */

//通信版本
int main(){
   system("gnome-terminal -x bash -c 'cd ../../BLINKER/bin && sudo python3 start.py;exec bash;'");
   system("gnome-terminal -x bash -c 'cd ../receive_arduino_data && sudo ./save_Arduino_Data;exec bash;'");
   char data_from_arduino;
   char data_from_blinker;
   string unlock_name,unlock_way;
   _face.show_detail_data();
   while(1){
      data_from_arduino = receive_Arduino_Data();
      if(data_from_arduino == 'F'){ //开启人脸识别
         _face.faceReconFlag = true;
         std::thread thread_face(thread_FaceRecon);//thread不能放入switch中，必须单独列出
         thread_face.detach();
      }
      if(data_from_arduino == 'U'){  //逆序输入密码，报警发送邮件
         cout<<"send---------->urgency";
         VideoCapture cap(0);
         if(!cap.isOpened()){
            cout<<"cap can't opened!"<<endl;
         }
         Mat frame;
         for(int i = 0;i<10;i++)
            cap>>frame;
         imwrite("../RECORD_DATA/door_state.jpg",frame);
         cap.release();

         string time = _face.getTime();
         string command = cv::format("echo '[%s]-有人使用了逆序密码解锁，触发报警信息，请立即处理！！！（使用该功能务必关闭人脸识别）'| mutt -s 紧急事件！！！ 1411102509@qq.com -a ../RECORD_DATA/door_state.jpg",time.c_str());
         system(command.c_str());
      }
      switch (data_from_arduino){
         case 'X':
            _face.faceReconFlag = false;
            break;
         case 'Q':
            _face.New_face();
            break;
         case 'T':   //查看人脸数量
            _arduino._send_data(_face.size()+48);
            break;
         case 'D':   //查看人脸数量
            _face.faceReconFlag = true;
            _face.Delete_face();
            _face.faceReconFlag = false;
            break;
         case 'A':
            unlock_way = "刷卡解锁";
            break;
         case 'B':   //指纹开锁记录
            unlock_way = "指纹解锁";
            break;
         case 'C':   //密码开锁记录
            unlock_way = "密码解锁";
            record_unlock_message("密码",unlock_way);
            break;
         case 'U':   //报警信息发送给blinker
            unlock_way = "逆序密码";
            record_unlock_message("报警",unlock_way);
            break;
         case '#':
            break;
         default:
            if(unlock_way == "刷卡解锁" || unlock_way == "指纹解锁"){
               unlock_name = cv::format("%c",data_from_arduino);
               record_unlock_message(unlock_name,unlock_way);
            }
            break;
      }

      data_from_blinker = receive_Blinker_Data();
      if(data_from_blinker == 'e'){
         cout<<"send---------->door_state";
         VideoCapture cap(0);
         if(!cap.isOpened()){
            cout<<"cap can't opened!"<<endl;
         }
         Mat frame;
         for(int i = 0;i<10;i++)
            cap>>frame;
         imwrite("../RECORD_DATA/door_state.jpg",frame);
         cap.release();

         string time = _face.getTime();
         string command = cv::format("echo '%s :此时的门前状态如下图，另附上所有开锁记录（使用该功能务必关闭人脸识别）'| mutt -s 门锁状态+开锁记录 1411102509@qq.com -a ../RECORD_DATA/door_state.jpg -a ../RECORD_DATA/ALL_RECORD_DATA.txt",time.c_str());
         system(command.c_str());

      }
      switch (data_from_blinker){
         case '#':
            break;
         case 'O':
            record_unlock_message("管理员","APP解锁");
         default:
            _arduino._send_data(data_from_blinker);
            break;
      }

   }
   return 0;
}
