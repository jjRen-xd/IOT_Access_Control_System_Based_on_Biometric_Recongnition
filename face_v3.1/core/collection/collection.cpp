#include<opencv2/opencv.hpp>
#include<vector>
#include<iostream>
#include<stdio.h>
#include"LinuxSerial.hpp"

using namespace std;
using namespace cv;
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

void get_num(int &num){
    char data_in;
    while(1){
        data_in = _get_data();
        if(data_in >= '0' && data_in <= '9'){
            num = data_in - 48;                                   
            cout<<num;
            break;
        }
    }
}

int main(){
    CascadeClassifier cascade;
    cascade.load("../haarcascade_frontalface_alt.xml");//载入CV脸部数据
    cv::VideoCapture cap(0);

    cv::Mat frame,face_img;
    int record_times,face_num;
    int folder_num = 1;

    cout<<endl<<"准备录入的人脸序号是多少呢?--:";//用户输入录入人脸标签
    _send_data('S');
    get_num(folder_num);

    cout<<endl<<"这是第几次录入呢?--:";
    _send_data('X');
    get_num(record_times);

    face_num = (record_times-1)*15+1;
    _send_data('T'); //显示开始收集
    while(1){
        cap >> frame;
        vector<Rect> faces;
        cv::Mat frame_gray;
        cv::cvtColor(frame,frame_gray,COLOR_BGR2GRAY);//减少运算

        cascade.detectMultiScale(frame_gray,faces,1.1,4,CV_HAAR_DO_CANNY_PRUNING,Size(30,30));//检测人脸

    //框出人脸
        for(size_t i = 0;i < faces.size();i++){
            cv::rectangle(frame,faces[i],Scalar(255,0,0),2,8,0);
        }
    //收集人脸
        if(faces.size() == 1){
            string command = cv::format("mkdir -p ../../orl_faces/s%d",folder_num);
            system(command.c_str());//建文件夹

            cv::Mat faceROI = frame_gray(faces[0]);//剪出ROI
            cv::resize(faceROI,face_img,Size(92,112));

            string face_number = cv::format("%d.jpg",face_num);//储存名
            cv::putText(frame,face_number,faces[0].tl(),3,1.2,Scalar(0,0,225),2,0);//标号

            string filename = cv::format("../../orl_faces/s%d/%d.jpg",folder_num,face_num++);//储存路径
            cv::imwrite(filename,face_img);//储存
            cv::imshow(face_number,face_img);
            cv::waitKey(500);
            cv::destroyWindow(face_number);

            if(face_num > record_times*15){
                cout<<"Collection complete"<<endl;
                cap.release();
                return 0;
            }
        }

        int c = cv::waitKey(100);
        if((char)c == 27){//未识别到人脸可按esc则退出
            break;
        }
        cv::imshow("frame",frame);
    }
    cout<<"Collection fail"<<endl;
    return 0;

}