#include <iostream>
#include <fstream>
#include <sstream>
#include "LinuxSerial.hpp"

using namespace std;
CLinuxSerial _arduino(0,9600);

int main(){
    char data_from_arduino;
    int fileChange = 0;
    while(1){
        data_from_arduino = _arduino._get_data();
        if((data_from_arduino>='A' && data_from_arduino<='Z') || (data_from_arduino>='0' && data_from_arduino<='9')){
            fileChange = !fileChange;
            std::ofstream arduino_Command("ARDUINO_COMMAND.txt");
            if(!arduino_Command){
                cout<<"No valid input file was given,please check the given filename."<<endl;
                return -1;
            }
            arduino_Command<<data_from_arduino<<endl;
            arduino_Command<<fileChange;
            arduino_Command.close();
        }
    }
    return 0;
}