#include<opencv2/opencv.hpp>
#include<opencv2/face.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/face/facerec.hpp>

#include"LinuxSerial.hpp"

#include<iostream>
#include<fstream>
#include<sstream>
#include<math.h>
#include<time.h>
#include<vector>

using namespace std;
using namespace cv;
using namespace cv::face;
CLinuxSerial serial (0,9600);

#define maxname_size 50
vector<string> names(maxname_size);
vector<unsigned int> names_weight = {0};
int stranger_weight = 0;
//RNG g_rng(12345);
Ptr<FaceRecognizer> model;
ofstream allData("../DATA/allData.txt");
bool file_change = 0;
string face_temp = "NULL";

unsigned char _get_data(){//从arduino2接收数据
    unsigned char data[] = {0xDA};
    serial.ReadData(data,1);
    //std::cout<< "get_data <- "<<data[0]<<std::endl;
    return data[0];        
}

void _send_data(char data){//向arduino2发送数据
    uchar send_bytes[] = { 0xFF };
    send_bytes[0] = data;
    serial.WriteData(send_bytes,1);
    std::cout << "send_data -> "<<send_bytes[0]<< std::endl;
}
int Predict(Mat src_image){//识别图片
    Mat face_test;
    int predit = 0;
    //截取ROI
    if(src_image.rows >= 120){
        resize(src_image,face_test,Size(92,112));
    }
    //判断是否正确检测ROI
    if(!face_test.empty()){
        predit = model->predict(face_test);
    }
    cout<<predit<<endl;
    return predit;
}

string getTime(){
    time_t timep;
    time (&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep) );
    return tmp;
}

void record(string str,cv::Mat face_record){//记录人脸信息,并向串口发数据
    if(str != face_temp){
        file_change = !file_change; //文件变化标志位,用于邮件发送
        ofstream tempData("../DATA/tempDATA.txt");
        string time_now = getTime();
        if(str == "Stranger"){
            tempData<<file_change<<" 1 "<<str<<" "<<time_now;  
            allData<<file_change<<" 1 "<<str<<" "<<time_now<<endl;
            _send_data('L');
        }
        else{
            tempData<<file_change<<" 0 "<<str<<" "<<time_now;  
            allData<<file_change<<" 0 "<<str<<" "<<time_now<<endl;
            _send_data('O');//向arduino发送开门指令
        }
        cout<<str<<endl;
        string filename = cv::format("../DATA/FACES_RECORD/%s %s.jpg",time_now.c_str(),str.c_str());//储存路径
        imwrite(filename,face_record);//保存人脸照片
        string command = cv::format("echo '门前有人，请注意,识别到人脸：%s，记录时间：%s' | mail -s '门禁提醒消息' 1411102509@qq.com",str.c_str(),time_now.c_str());
        system(command.c_str());
        face_temp = str;
    }
}

int main(){
    for(unsigned char i = 0;i<names.size();i++){
        names[i] = cv::format("%d",i);
    }
    names[3] = "RenJunJie";
    names[4] = "Stranger";
    VideoCapture cap(0);
    if(!cap.isOpened()){
        cout<<"cap can't opened!"<<endl;
        return -1;
    }
    Mat frame;
    Mat gray;
    CascadeClassifier cascade;//人脸检测分类器
  
    cascade.load("../haarcascade_frontalface_alt.xml");//CV官方的数据集

    model = FisherFaceRecognizer::create();
    model->read("../../train/train_result/MyFaceFisherModel.xml");//加载训练好的分类器

    while(1){
        cap>>frame;
        vector<Rect> faces(0);  //存放人脸
        cvtColor(frame,gray,CV_RGB2GRAY);   //灰度
        equalizeHist(gray,gray);    //直方图均值化
        cascade.detectMultiScale(gray,faces,1.1,4,0|CV_HAAR_DO_ROUGH_SEARCH,Size(30,30),Size(500,500));//检测人脸
        Mat* pImage_roi = new cv::Mat[faces.size()];
        Mat face;
        Point text;
        //框出人脸
        string str;
        for(int i = 0;i < faces.size();i++){
            pImage_roi[i] = gray(faces[i]);//将所有的脸部保存起来
			text = Point(faces[i].x, faces[i].y);
			if (pImage_roi[i].empty())
				continue;
                
			int result = Predict(pImage_roi[i]);//识别人脸
            if(result>maxname_size||result == 0){//不在标签范围内的为stranger
                str = "Stranger";
                if(stranger_weight++ > 10){//stranger判断权重,可更改调节灵敏度,识别10次即可确定陌生人身份
                    record(str,frame(faces[i]));
                    //初始化
                    stranger_weight = 0;
                    names_weight[result] = 0;
                }
			}  
            else{
                str = names[result];
                if(names_weight[result]++ > 5){//识别到5次即可确认身份
                    record(str,frame(faces[i]));
                    //初始化
                    stranger_weight = 0;
                    names_weight[result] = 0;
                }
            }
			//Scalar color = Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255));
			rectangle(frame, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(255,0,0), 2, 8);//框出人脸
			putText(frame, str, text, FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255));//显示标签
		}
		delete[]pImage_roi;
		imshow("face", frame);
       /* if(_get_data() == 'X')
            break;*/
        int c = cv::waitKey(30);
        if((char)c == 27){ //按esc则退出
            break;
        }

    }

    return 0;
}
