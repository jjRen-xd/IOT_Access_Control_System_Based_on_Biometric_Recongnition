#include "face_master.hpp"

#include<python2.7/Python.h>
#include<math.h>
#include<iostream>
#include<fstream>
#include<sstream>

#include"LinuxSerial.hpp"

using namespace cv;
using namespace std;
using namespace cv::face;
// #define SHOW_COLLECTED_IMAGE    //显示收集人脸的过程，大大降低效率
#define SHOW_IMAGE
#define SHOW_DEBUG
#define SEND_MESSAGE_TO_ARDUINO

#define COLLEDTION_TIMES 5//人脸收集的次数
#define STRANGER_WEIGHT 10
#define NORMAL_WEIGHT 5   //判断权重，值越大越可靠，识别速度越慢
#define RECONGNISE_DISTANCE 0.5  //识别距离

ofstream ALL_RECORD_DATA("../RECORD_DATA/ALL_RECORD_DATA.txt");
#ifdef SEND_MESSAGE_TO_ARDUINO
    CLinuxSerial arduino(0,9600);   
#endif
static void read_csv(const string& filename,vector<Mat>& images,vector<int>& labels,char separator = ';'){
    std::ifstream file(filename.c_str(),ifstream::in);
    if(!file){
        string error_message = "No valid input file was given,please check the given filename.";
        CV_Error(CV_StsBadArg,error_message);
    }
    string line,path,classlabel;
    while(getline(file,line)){//从文本文件读取一行字符,未指定字符默认限定字符为"\n"
        stringstream liness(line);//采用stringstream用于字符串分割
        getline(liness,path,separator);//读入图片文件路径,以;作为限定符
        getline(liness,classlabel);//读入图片标签,默认限定符
        classlabel.erase(classlabel.begin());
        //cout<<classlabel<<endl;
        if(!path.empty() && !classlabel.empty()){//如果读取成功,则将图片和对应标签压入对应容器中
            images.push_back(imread(path,0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}

string getTime(){
    time_t timep;
    time (&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d_%H:%M:%S",localtime(&timep));
    return tmp;
}
//检测人脸位置
std::vector<cv::Rect> Face::choice_faces_from_image(cv::Mat img){
    vector<Rect> faces(0);                              //存放人脸

    CascadeClassifier cascade;                          //人脸检测分类器
    cascade.load("../haarcascade_frontalface_alt.xml"); //CV官方的数据集

    cascade.detectMultiScale(img,faces,1.1,4,0|CV_HAAR_DO_ROUGH_SEARCH,Size(30,30),Size(500,500));//检测人脸
    return faces;
}

void Face::mark_faces(cv::Mat &img,std::vector<cv::Rect> facePosition){
    for(int i = 0;i<facePosition.size();i++){
        rectangle(img,facePosition[i],Scalar(0,0,0),2,8);
    }
}
void Face::mark_faces(cv::Mat &img,std::vector<cv::Rect> facePosition,std::vector<cv::Point3f> angle){
    for(int i = 0;i<facePosition.size();i++){
        string text = cv::format("(%0.2f,%0.2f,%0.2f)",angle[i].x,angle[i].y,angle[i].z);
        putText(img,text,Point(facePosition[i].x+facePosition[i].width,facePosition[i].y),FONT_HERSHEY_PLAIN,1,Scalar(0,0,255));//显示解算角度

        rectangle(img,facePosition[i],Scalar(0,0,0),2,8);
    }
}
void Face::mark_faces(cv::Mat &img,std::vector<cv::Rect> facePosition,std::vector<int> label,std::vector<cv::Point3f> angle){
    cv::Scalar color = Scalar(0,0,255);;
    if(facePosition.size() != label.size()){
        cout<<"The size of facePositon and label is mismatch!!!"<<endl;
        return;
    }
    for(int i = 0;i<facePosition.size();i++){
        string text = cv::format("(%0.2f,%0.2f,%0.2f)",angle[i].x,angle[i].y,angle[i].z);
        putText(img,text,Point(facePosition[i].x+facePosition[i].width,facePosition[i].y),FONT_HERSHEY_PLAIN,1,Scalar(0,0,255));//显示解算角度

        if(label[i] == 0){
            color = Scalar(0,0,255);
            rectangle(img,facePosition[i],color,2,8);
            putText(img,"Stranger",Point(facePosition[i].x,facePosition[i].y),FONT_HERSHEY_COMPLEX,1,color);//显示标签
        }
        else{
            color = Scalar(0,255,0);
            rectangle(img,facePosition[i],color,2,8);
            putText(img,to_string(label[i]),Point(facePosition[i].x,facePosition[i].y),FONT_HERSHEY_COMPLEX,1,color);//显示标签
        }
    }
}

bool Face::save_face_img(cv::Mat img,std::vector<Rect> facePosition){
    if(facePosition.size() == 1){
        cv::Mat faceROI = img(facePosition[0]); 
        cv::resize(faceROI,faceROI,cv::Size(92,112));   //分割出人脸

        string command = cv::format("mkdir -p ../face_data/s%d",Face::size());
        std::system(command.c_str());

        string filename = cv::format("%d.jpg",face_data[Face::size()]++);//储存名
        string path = cv::format("../face_data/s%d/%s",Face::size(),filename.c_str());//储存路径
        cv::imwrite(path,faceROI);//储存
        #ifdef SHOW_COLLECTED_IMAGE
            cv::imshow(filename,faceROI);
            cv::waitKey(20);
            cv::destroyWindow(filename);
        #endif
        
        #ifdef SHOW_DEBUG
            cout<<"Have finished "<<face_data[Face::size()]<<"s image."<<endl;
        #endif
        return 1;
    }
}



void Face::train_face_data(){
    cout<<"training..."<<endl;
    string fn_csv =  "../face_data/csv.txt";//读取CSV文件
    //两个容器存放图像数据和对应标签
    vector<Mat> images;
    vector<int> labels;

    //读取数据,如果文件不合法就退出
    try{
        read_csv(fn_csv,images,labels);//从CSV文件中批量读取训练数据
    }
    catch(cv::Exception& e){
        cerr<<"Error opening file \""<<fn_csv<<"\".Reason:"<<e.msg<<endl;
        exit(1);
    }
    //如果没有读取到足够多的图片,也退出
    if(images.size() < 2){
        string error_message = "This demo needs at least 2 images to work.Please add more images to your data set!";
        CV_Error(CV_StsError,error_message);
    }

    for(size_t i = 0;i<images.size();i++){
        if(images[i].size() != Size(92,112)){
            cout<<i<<endl;
            cout<<images[i].size()<<endl;
        }
    }

    // 建立特征脸模型用于人脸识别  
	// 通过CSV文件读取的图像和标签训练它
    // Ptr<BasicFaceRecognizer> model_Eigen = EigenFaceRecognizer::create();
    // model_Eigen->train(images,labels);
    // model_Eigen->save("../face_data/MyFaceEigenModel.xml");

    Ptr<BasicFaceRecognizer> model_Fisher = FisherFaceRecognizer::create();
    model_Fisher->train(images,labels);
    model_Fisher->save("../face_data/MyFaceFisherModel.xml");

    // Ptr<LBPHFaceRecognizer> model_LBPH = LBPHFaceRecognizer::create();
    // model_LBPH->train(images,labels);
    // model_LBPH->save("../face_data/MyFaceLBPHModel.xml");
    
	cout<<"Complete the training"<<endl;
	return ;
}

std::vector<int> Face::predict_faces(std::vector<cv::Mat> faces_src,Ptr<cv::face::FaceRecognizer> &model){
    std::vector<int> predict_result;
    cv::Mat temp_face_resized;
    for(size_t i = 0;i<faces_src.size();i++){
        //截取ROI

        resize(faces_src[i],faces_src[i],Size(92,112));
        //判断是否正确检测ROI
        if(!faces_src[i].empty()){
            predict_result.push_back(model->predict(faces_src[i]));
        }
    }
    return predict_result;
}

int Face::judge_recon_result(std::vector<int> labels,std::vector<int> &names_weight){
    static int judge_times = 0;
    if(judge_times++ > 15){
        for(size_t i = 0;i<names_weight.size();i++)
            names_weight[i] = 0;
        judge_times = 0;
    }
    for(size_t i = 0;i<labels.size();i++){
        names_weight[labels[i]]++;
    }

    if(names_weight[0] >= STRANGER_WEIGHT){
        for(size_t i = 0;i<names_weight.size();i++)
            names_weight[i] = 0;
        return 0;
    }
    for(size_t i = 0;i<names_weight.size();i++){
        if(names_weight[i] >= NORMAL_WEIGHT){
            for(size_t i = 0;i<names_weight.size();i++)
                names_weight[i] = 0;         
            return i;
        }
    }
    return -1;
}

void Face::record_recon_data(string str,cv::Mat face_record){
    string time_now = getTime();
    string filename = cv::format("../RECORD_DATA/FACES/%s_%s_face.jpg",time_now.c_str(),str.c_str());//储存路径

    //保存人脸照片，并生成日志文件
    static int unlock_times = 0;
    ofstream NEWEST_DATA("../RECORD_DATA/NEWEST_DATA.txt");
    imwrite(filename,face_record);
    ALL_RECORD_DATA<<"["<<++unlock_times<<"]-["<<time_now<<"]-[Name of Unlocker]:"<<str<<"-[Unlocking mode]:face unlock"<<endl; 
    NEWEST_DATA<<"["<<time_now<<"]-[Name of Unlocker]:"<<str<<"-[Unlocking mode]:face unlock"<<endl; 
    //发邮件提醒
    #ifdef SEND_EMAIL
        string command = cv::format("echo 'Someone（%s） is in the unlock range, please be vigilant!!!'| mutt -s new 1411102509@qq.com -a %s",str.c_str(),filename.c_str());
        system(command.c_str());
    #endif
    //向arduino发命令开门
    #ifdef SEND_MESSAGE_TO_ARDUINO
        if(str != "stranger"){
            arduino._send_data('O');    //向arduino发送开门指令
        }
    #endif
    #ifdef SHOW_DEBUG
        cout<<"send------------------->'O'*********************[Name of Unlocker]:"<<str.c_str()<<endl;
    #endif
}
// 解算人脸位置
cv::Point3f Face::get_Ang(int center_x,int center_y,int width){
	double calibration[9] = {
		945.2654, 0.000000, 701.7872,
		0.000000, 945.6360, 319.5665,
		0.000000, 0.000000, 1.000000
	};
	double dist_coeffs[5] = { -0.0112, 0.0878, 0.0000, 0.0000, -0.0992 };
	Mat cameraMatrix = Mat(3, 3, CV_64F, calibration);	//内参矩阵
	Mat distCoeffs = Mat(5, 1, CV_64F, dist_coeffs);	//畸变系数
	
	vector<Point3f> object_point;
	object_point.push_back(Point3f(-80, -80, 0.0));	//魔方宽46，以中心为原点
	object_point.push_back(Point3f(80,  -80, 0.0));
	object_point.push_back(Point3f(80,  80 , 0.0));
	object_point.push_back(Point3f(-80, 80 , 0.0));

	vector<Point2f> image_point;
	image_point.push_back(Point2f(center_x - width, center_y - width));
	image_point.push_back(Point2f(center_x + width, center_y - width));
	image_point.push_back(Point2f(center_x + width, center_y + width));
	image_point.push_back(Point2f(center_x - width, center_y + width));

	Mat rvec = Mat::ones(3, 1, CV_64F);	//旋转矩阵
	Mat tvec = Mat::ones(3, 1, CV_64F);	//平移矩阵

	solvePnP(object_point, image_point, cameraMatrix, distCoeffs, rvec, tvec);
	
	double pos_x, pos_y, pos_z;
	const double *_xyz = (const double *)tvec.data;
	pos_z = tvec.at<double>(2) / 1000.0;
	pos_x = atan2(_xyz[0], _xyz[2]);
	pos_y = atan2(_xyz[1], _xyz[2]);
	pos_x *= 180 / 3.1415926;
	pos_y *= 180 / 3.1415926;

	Point3f pos = Point3f(pos_x,pos_y,pos_z);
	return pos;
}

bool Face::New_face(){
    prepare_next_face();    //开辟新空间，准备存储这一次的人脸收集信息
    VideoCapture cap(0);
    if(!cap.isOpened()){
        cout<<"cap can't opened!"<<endl;
        return false;
    }
    #ifdef SEND_MESSAGE_TO_ARDUINO
        arduino._send_data('S');    //向arduino发送放置人脸的指令
    #endif

    Mat frame,gray;
    //收集人脸
    while(1){
        cap>>frame;

        cvtColor(frame,gray,CV_RGB2GRAY);      //灰度
        equalizeHist(gray,gray);               //直方图均值化

        std::vector<Rect> facePosition = choice_faces_from_image(gray); //获取一帧中所有的人脸位置
        mark_faces(frame,facePosition);         //框出人脸
        Face::save_face_img(gray,facePosition); //保存收集到的人脸
        #ifdef SHOW_COLLECTED_IMAGE
            cv::imshow("face",frame);
            //收集途中按ESC退出
            int c = cv::waitKey(30);    
            if((char)c == 27){  //按ESC键退出
                cap.release();
                break;
            }
        #endif
        //一次收集COLLECTION_TIMES张人脸就退出
        if(Face::record_times(Face::size())>=COLLEDTION_TIMES){   
            cout<<"face_"<<Face::size()<<":collection complete"<<endl;
            cap.release();          //关闭摄像头，否则会与下一次使用cap冲突
            break;
        }
    }
    //调用python脚本,更新csv文件
    std::system("cd ../face_data && python creat_csv.py");

    //训练人脸数据
    Face::train_face_data();

    //更新人脸数据文件
    ofstream SAVED_FACES("../face_data/SAVED_FACES.txt");
    for(int i = 1;i<=Face::size();i++){
        SAVED_FACES<<Face::face_data[i]<<";"<<Face::name_data[i]<<endl;
    }
    #ifdef SEND_MESSAGE_TO_ARDUINO
        arduino._send_data('W');    //向arduino发送录入成功的指令
    #endif
    return true;
}

int Face::Recongnise_face(bool times){   //识别人脸，times=1:只成功识别一次（识别多少帧由权重判断）就返回，times=0：无穷
    VideoCapture cap(0);
    if(!cap.isOpened()){
        cout<<"cap can't opened!"<<endl;
        return -1;
    }
    Mat frame,gray;
    //加载训练好的分类器
    // Ptr<BasicFaceRecognizer> model_Eigen = EigenFaceRecognizer::create();
    // model->read("../face_data/MyFaceEigenModel.xml");

    Ptr<FaceRecognizer> model = FisherFaceRecognizer::create();
    model->read("../face_data/MyFaceFisherModel.xml");

    // Ptr<LBPHFaceRecognizer> model_LBPH = LBPHFaceRecognizer::create();
    // model->read("../face_data/MyFaceLBPHModel.xml");

    while(1){
        cap>>frame;
        cvtColor(frame,gray,CV_RGB2GRAY);   //灰度
        equalizeHist(gray,gray);            //直方图均值化
        
        std::vector<cv::Mat> faceImgs;      //所有的人脸像素信息
        std::vector<cv::Rect> facePositions;//人脸在图像中的位置
        std::vector<cv::Point3f> faceAngle; //人脸在世界坐标的位置
        std::vector<int> faceLabels;        //识别成功后的人脸标签

        facePositions = Face::choice_faces_from_image(gray);    //找出图像中的所有人脸位置
        Face::mark_faces(frame,facePositions);

        float minDis = 1024;                //在摄像头前的人脸最小距离
        for(int i = 0;i<facePositions.size();i++){
            faceImgs.push_back(gray(facePositions[i]));         //保存所有脸部信息
            if(faceImgs[i].empty())
                continue;

            //计算出每个人脸中心坐标，并进行PNP解算,找出每张脸在世界坐标的位置
            int center_x,center_y,width;
            center_x = (2 * facePositions[i].x + facePositions[i].width) / 2;
            center_y = (2 * facePositions[i].y + facePositions[i].height) / 2;
            width = facePositions[i].width;
            faceAngle.push_back(get_Ang(center_x,center_y,width));
            if(faceAngle[i].z < minDis)
                minDis = faceAngle[i].z;
        }
        if(minDis < RECONGNISE_DISTANCE){   //距离到达一定距离就开始检测记录
            cout<<"begin detect"<<endl;
            faceLabels = Face::predict_faces(faceImgs,model);//开始预测人脸
            Face::mark_faces(frame,facePositions,faceLabels,faceAngle);//标记
            static std::vector<int> names_weight(Face::size()); //将每一帧识别结果格式化存储起来
            int result = Face::judge_recon_result(faceLabels,names_weight);

            if(result != -1){//即已经成功识别出结果
                Face::recongnise_result_label = result;
                if(times == 1){//只识别一次，用于人脸删除
                    #ifdef SHOW_DEBUG
                        cout<<"Prepare to delete face "<<recongnise_result_label<<"!!!"<<endl;
                    #endif
                    return Face::recongnise_result_label;
                }
                
                for(size_t i = 0;i<faceLabels.size();i++){//找出结果脸的在faceImgs中的位置
                    if(faceLabels[i] == result)
                        result = i;
                }
                Face::record_recon_data(Face::name_data[Face::recongnise_result_label],frame(facePositions[result]));
            }
        }
        else
            Face::mark_faces(frame,facePositions,faceAngle);

        #ifdef SHOW_DEBUG
            cout<<"faceImg.size():"<<faceImgs.size();
            cout<<" faceLabels.size():"<<faceLabels.size();
            cout<<" facePositions.size():"<<facePositions.size();
            cout<<" faceAngle.size():"<<faceAngle.size()<<endl;
            for(int i = 0;i<faceImgs.size();i++){
                cout<<"FACE_Num"<<i<<":";
                cout<<" position:("<<facePositions[i].x<<","<<facePositions[i].y<<")";
                cout<<" angle:("<<faceAngle[i].x<<","<<faceAngle[i].y<<","<<faceAngle[i].z<<")"<<endl<<endl;
            }
            for(int i = 0;i<faceLabels.size();i++)
                cout<<"label:"<<faceLabels[i]<<endl;
        #endif
        #ifdef SHOW_IMAGE
            imshow("face",frame);
            int c = cv::waitKey(30);//1000/30 = 33帧
            if((char)c == 27){ //按esc则退出
                break;
            }
        #endif
        
        if(!Face::faceReconFlag){//接收到命令就退出识别
            cv::destroyWindow("face");
            break;
        }
    }
    return 0;
}

bool Face::Delete_face(){   //删除人脸，先识别出要删除的对象，然后删除
    int delete_Target = Face::Recongnise_face(1);
    Face::face_data.erase(Face::face_data.begin() + delete_Target);
    Face::name_data.erase(Face::name_data.begin() + delete_Target);
    
    //更新人脸数据文件
    ofstream SAVED_FACES("../face_data/SAVED_FACES.txt");
    for(int i = 1;i<=Face::size();i++){
        SAVED_FACES<<Face::face_data[i]<<";"<<Face::name_data[i]<<endl; 
    }
    //删除存放人脸照片的文件夹
    string command = cv::format("cd ../face_data && rm -rf s%s",delete_Target);
    std::system(command.c_str());

    //调用python脚本,更新csv文件
    std::system("cd ../face_data && python creat_csv.py");

    //重新训练人脸数据
    Face::train_face_data();

    return true;
}