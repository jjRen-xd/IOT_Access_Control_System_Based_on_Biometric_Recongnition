#include "face_master.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/face/facerec.hpp>

#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

Face _face;

int main(){
   _face.New_face();
   _face.show_detail_data();
   _face.New_face();
   _face.show_detail_data();  
}
