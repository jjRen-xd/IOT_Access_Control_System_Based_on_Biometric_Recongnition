#include "face_master.hpp"
#include"LinuxSerial.hpp"
#include <iostream>
#include <thread>

using namespace std;

Face _face;
CLinuxSerial _arduino(0,9600);

void thread_FaceRecon(){
   _face.Recongnise_face(0);
}

int main(){
   _face.show_detail_data();
 /*   _face.New_face();
   _face.show_detail_data();
   _face.New_face();
   _face.show_detail_data(); */

   // _face.Delete_face();
   while(1){
      char data_in = _arduino._get_data();

      if(data_in == 'Y'){ //开启人脸识别
         _face.faceReconFlag = true;
         std::thread thread(thread_FaceRecon);
         thread.detach();
      }
      else if(data_in == 'X'){ //关闭人脸识别
         _face.faceReconFlag = false;
      }
      else if(data_in == 'Q'){ //新建人脸
         _face.New_face();
      }
/*       else if(_arduino._get_data() == 'T'){ //查看人脸数量
         _arduino._send_data('1');
         cout<<"send--------->"<<1<<endl;
      } */
   }
   return 0;
}

