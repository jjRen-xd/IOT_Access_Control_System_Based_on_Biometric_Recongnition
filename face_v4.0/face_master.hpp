#include <vector>
#include <stdlib.h>
#include <iostream>
#include<fstream>

#include<opencv2/opencv.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/face/facerec.hpp>
#include<opencv2/core.hpp>
#include<opencv2/face.hpp>

using namespace std;
using namespace cv;
#define maxname_size 50

class Face{
    public:
        Face(){
            face_data.push_back(0);
            name_data.push_back("Stranger");//位置0不可用，开辟后空出存放stranger
            
            //读取上次储存的人脸信息
            std::ifstream last_Faces_Data("../face_data/SAVED_FACES.txt",ifstream::in);
            if(!last_Faces_Data){
                string error_message = "No valid input file was given,please check the given filename SAVED_FACES.txt.";
                CV_Error(CV_StsBadArg,error_message);
            }
            string line,quantity,name;
            while(getline(last_Faces_Data,line)){
                stringstream liness(line);
                getline(liness,quantity,';');
                getline(liness,name);
                stringstream quantity_ss(quantity);//stringstream可以吞下任何类型，根据实际需要吐出不同的类型。
                static int quantity_int;
                if(!quantity.empty() && !name.empty()){
                    quantity_ss>>quantity_int;
                    face_data.push_back(quantity_int);
                    name_data.push_back(name);
                }
            }
        }
        // ~Face(){};
        int record_times(int face_num){
            return face_data[face_num];
        }
        int size(){
            return face_data.size()-1;
        }
        bool New_face();
        bool Delete_face();
        int Recongnise_face(bool times);

        void show_detail_data(){
            for(int i = 0;i<face_data.size();i++){
                cout<<cv::format("face_data[%d]",i)<<face_data[i]<<endl;
            }
            for(int i = 0;i<name_data.size();i++){
                cout<<cv::format("name_data[%d]",i)<<name_data[i]<<endl;
            }
        }
        int recongnise_result_label;
        bool faceReconFlag = false;
    private:
        std::vector<int> face_data;//存储人脸录入信息
        std::vector<std::string> name_data;//存储人脸对应姓名

        // bool load_flag;
        void prepare_next_face(){
            face_data.push_back(0);
            name_data.push_back(cv::format("%d",size()));
        }
        std::vector<cv::Rect> choice_faces_from_image(cv::Mat img);
        void mark_faces(cv::Mat &img,std::vector<cv::Rect> facePosition);
        void mark_faces(cv::Mat &img,std::vector<cv::Rect> facePosition,std::vector<cv::Point3f> angle);
        void mark_faces(cv::Mat &img,std::vector<cv::Rect> facePosition,std::vector<int> label,std::vector<cv::Point3f> angle);
        bool save_face_img(cv::Mat img,std::vector<cv::Rect> facePosition);

        void train_face_data();

        std::vector<int> predict_faces(std::vector<cv::Mat> faces_src,Ptr<cv::face::FaceRecognizer> &model);
        int judge_recon_result(std::vector<int> labels,std::vector<int> &names_weight);
        void record_recon_data(string str,cv::Mat face_record);
        cv::Point3f get_Ang(int center_x,int center_y,int width);

};
