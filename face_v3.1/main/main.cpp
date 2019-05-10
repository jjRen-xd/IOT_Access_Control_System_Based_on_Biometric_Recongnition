#include<iostream>
#include<stdlib.h>
#include"LinuxSerial.hpp"

using namespace std;
CLinuxSerial serial(0,9600);

unsigned char _get_data(){//从arduino2接收数据
    unsigned char data[] = {0xDA};
    serial.ReadData(data,1);
    std::cout<< "get_data <- "<<data[0]<<std::endl;
    return data[0];        
}

void _send_data(char data){//向arduino2发送数据
    unsigned char send_bytes[] = { 0xFF };
    send_bytes[0] = data;
    serial.WriteData(send_bytes,1);
    std::cout << "send_data -> "<<send_bytes[0]<< std::endl;
}

int main(){
    //bool entry_flag,recongnise_flag;
    int user_laber;
    char data_in;
    while(1){
        data_in = _get_data();
        if(data_in == 'Q'){//接收到命令即开始收集人脸
            cout<<"开始收集"<<endl;
            system("cd ../../core/collection/build && ./collection");
            cout<<"开始创建CSV"<<endl;
            _send_data('U'); 
            system("cd ../../core/orl_faces && python creat_csv.py");
            cout<<"开始训练"<<endl;
            _send_data('V'); 
            system("cd ../../core/train/build && ./train");   
            cout<<endl<<"录入完成,"; 
            _send_data('W'); 
        }
        if(data_in == 'Y'){
            system("cd ../../core/recongnition/build && ./recongnition");
        }
    }
    return 0;
}