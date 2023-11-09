#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <iomanip>
#include <time.h>
#include <algorithm>
#include <set>
using namespace std;

///////////////////////////////////////////////////////////////////
//                            main.cpp                           //
///////////////////////////////////////////////////////////////////
vector<string> input ,output  ;
vector<double> delay , capacitance ;  // capacitance[i] is delay of node i 
vector<vector<string>> gate ;


int main(int argc, char* argv[])
{
     // read .v //
	ifstream inFile;
        inFile.open(argv[1]); 
        if (!inFile) {
        return 1;
        } 
    string line;
    string str ; 
    char ch;
    int num  ; 
    int node_num = 0 ;
    while (getline(inFile, line)) {
        stringstream in;
        in << line;
        in >> str ;    //讀每一行第一個單字
       if( str == "input"){
            while (in >> ch){
                in >> num;
                str = ch + to_string(num);
                input.push_back(str);
                in >> ch ;  // 拿掉分號 逗號
                node_num ++;
            }
       }
       if( str == "output"){
            while (in >> ch){
                in >> num;
                str = ch + to_string(num);
                output.push_back(str);
                in >> ch ; 
                 node_num ++;
            }
       }
       if( str == "wire"){
            while (in >> ch){
                in >> num;  // 讀出n*
                node_num ++;
                in >> ch ; 
            }
       }

       ///////////////////////////////////////////////////////////////////
      //           step 1 read the gate and update capacitance     // 
      ///////////////////////////////////////////////////////////////////
        vector <string> gate_tmp;
        capacitance.resize (node_num+1);

       if(str == "INVX1"){
            gate_tmp.push_back(str); //寫回gate種類
            in >>ch>> num;
            str = ch + to_string(num);
            gate_tmp.push_back(str); // g* 寫回
            in >> ch ;  //把第一個 '(' 拿掉

            while(in >> ch){
                if( ch == 'Z'){
                    in >> ch >> ch; //去掉 'N('
                    in >>ch>> num;
                    str = ch + to_string(num);
                    gate_tmp.push_back(str); //寫回output
                }
                if(ch == '('){  //判斷為input port
                    in >>ch>> num;
                    str = ch + to_string(num);
                    gate_tmp.push_back(str); //寫回input
                    capacitance[num] +=  0.0109115; // INVX1 input capacitance
                }
            }
            gate.push_back(gate_tmp);
            //   for (int i = 0; i <gate_tmp.size(); i++) {
	    	//     cout<<gate_tmp[i] << endl;
	        //     }
       }

       if(str == "NANDX1"){
            bool second_input = false;
            gate_tmp.push_back(str); //寫回gate種類
            in >>ch>> num;
            str = ch + to_string(num);
            gate_tmp.push_back(str); // g* 寫回
            in >> ch ;  //把第一個 '(' 拿掉

            while(in >> ch){
                if( ch == 'Z'){
                    in >> ch >> ch; //去掉 'N('
                    in >>ch>> num;
                    str = ch + to_string(num);
                    gate_tmp.push_back(str); //寫回output
                }
                if(ch == '('){  //判斷為input port
                    in >>ch>> num;
                    str = ch + to_string(num);
                    gate_tmp.push_back(str); //寫回input
                    if(second_input)
                        capacitance[num] +=  0.00798456;
                    else 
                        capacitance[num] += 0.00683597;

                    second_input = true;
                }
            }
            gate.push_back(gate_tmp);
       }

        if(str == "NOR2X1"){
            bool second_input = false;
            gate_tmp.push_back(str); //寫回gate種類
            in >>ch>> num;
            str = ch + to_string(num);
            gate_tmp.push_back(str); // g* 寫回
            in >> ch ;  //把第一個 '(' 拿掉

            while(in >> ch){
                if( ch == 'Z'){
                    in >> ch >> ch; //去掉 'N('
                    in >>ch>> num;
                    str = ch + to_string(num);
                    gate_tmp.push_back(str); //寫回output
                }
                if(ch == '('){  //判斷為input port
                    in >>ch>> num;
                    str = ch + to_string(num);
                    gate_tmp.push_back(str); //寫回input
                    if(second_input)
                        capacitance[num] +=   0.0108106; 
                    else 
                        capacitance[num] +=  0.0105008;

                    second_input = true;
                }
            }
            gate.push_back(gate_tmp);
       }
   
    }
                                                                   /// ----output capacitance 要加回去 ---//
      for (int i = 0; i < capacitance.size(); i++) {
	    	cout<< capacitance [i] << endl;
	    }
        /// output capacitance 要加回去 //

    //    for (int i = 0; i < gate.size(); i++) {
    //         for (int j = 0 ; j < gate[i].size(); j++)
	//     	cout<< gate [i][j] << endl;
	//     }
    //     cout<<"node number: "<< node_num << endl;

    //  for (int i = 0; i < gate[2].size(); i++) {
	//     	cout<< gate [2][i] << endl;
	//     }

    inFile.close(); 
	    // for (int i = 0; i < input.size(); i++) {
	    // 	cout<< input [i] << endl;
	    // }
				
    return 0;
}