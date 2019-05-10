#include<python2.7/Python.h>
#include<opencv2/face/facerec.hpp>
#include<opencv2/core.hpp>
#include<opencv2/face.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

#include<math.h>

#include<iostream>
#include<fstream>
#include<sstream>

using namespace cv;
using namespace std;
using namespace cv::face;

//使用CSV文件读取图片和标签
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

int main(){
    cout<<"training..."<<endl;
    string fn_csv =  "../../orl_faces/csv.txt";//读取CSV文件
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
    //Ptr<BasicFaceRecognizer> model_Eigen = EigenFaceRecognizer::create();
    //model_Eigen->train(images,lab   els);
    //model_Eigen->save("../train_result/MyFaceEigenModel.xml");

    Ptr<BasicFaceRecognizer> model_Fisher = FisherFaceRecognizer::create();
    model_Fisher->train(images,labels);
    model_Fisher->save("../train_result/MyFaceFisherModel.xml");

    //Ptr<LBPHFaceRecognizer> model_LBPH = LBPHFaceRecognizer::create();
    //model_LBPH->train(images,labels);
    //model_LBPH->save("../train_result/MyFaceLBPHModel.xml");

    //调用python脚本,更新csv文件
    
	cout<<"Complete the training"<<endl;
	return 0;

}
