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
// 注意.v 的node可能有 n329 N329 .... 的可能 //
// 如果 wire 不是全部在同一行 或 input output wire沒有在最前面 會出錯   要改可以改成先全部讀完再計算// 
struct nnode {
    int num=0;
    char name;
    int max_id=0;
};
struct matrix_id {
    int row=0;
    int column=0;
};
struct gate_delay {
    double delay=0;
    double transition_time=0;
    bool rise ;
};


vector<string> input ,output  ;
vector<nnode> node ;
vector<vector<double>> transition_time,acc_maxdelay , capacitance , delay ,acc_mindelay     ;  // capacitance[i] is delay of node i  // acc_delay為該節點的最前端
vector<vector<bool>> valid_node ,node_rise; // true: the node has been caculated 
vector<vector<string>> ans_delay  , ans_maxdelay ,in_max , in_min ;
vector<string> gate_ans  ;
vector<vector<string>> gate ;
vector<vector<double>>rise_power_INVX1       (7, vector<double>(7));
vector<vector<double>>fall_power_INVX1       (7, vector<double>(7));
vector<vector<double>>cell_rise_INVX1        (7, vector<double>(7));
vector<vector<double>>cell_fall_INVX1        (7, vector<double>(7));
vector<vector<double>>rise_transition_INVX1  (7, vector<double>(7));
vector<vector<double>>fall_transition_INVX1  (7, vector<double>(7));

vector<vector<double>>cell_rise_NANDX1      (7, vector<double>(7));
vector<vector<double>>cell_fall_NANDX1       (7, vector<double>(7));
vector<vector<double>>rise_transition_NANDX1   (7, vector<double>(7));
vector<vector<double>>fall_transition_NANDX1   (7, vector<double>(7));

vector<vector<double>>cell_rise_NOR2X1      (7, vector<double>(7));
vector<vector<double>>cell_fall_NOR2X1       (7, vector<double>(7));
vector<vector<double>>rise_transition_NOR2X1   (7, vector<double>(7));
vector<vector<double>>fall_transition_NOR2X1   (7, vector<double>(7));
vector <double>index_output_net_capacitance,index_input_transition_time;
bool inCommentBlock = false;

 // chat gpt //
std::string removeComments(const std::string& line) {
    std::string result;
    inCommentBlock = false;//bool inCommentBlock = false;

    for (size_t i = 0; i < line.length(); ++i) {
        if (!inCommentBlock && line[i] == '/' && i + 1 < line.length() && line[i + 1] == '*') {
            inCommentBlock = true;
            ++i;
        } else if (inCommentBlock && line[i] == '*' && i + 1 < line.length() && line[i + 1] == '/') {
            inCommentBlock = false;
            ++i;
        } else if (!inCommentBlock && line[i] == '/' && i + 1 < line.length() && line[i + 1] == '/') {
            // 遇到單行註解，忽略本行後的內容
            break;
        } else if (!inCommentBlock) {
            result += line[i];
        }
    }

    return result;
}

int table_id(const char name , vector<nnode> node ){
    int m ;
    for(int i=0; i<node.size(); i++){
        if(node[i].name == name)
            m = i;
    }
    return m;
}
matrix_id string2matrix_index (const string str){
    matrix_id a;
    char ch;
    int num ;
    int id ;
    stringstream in;
    in << str;
    in >> ch >> num ;
    id = table_id(ch ,node);
    a.row = id ;
    a.column = num;
    return a;
}

vector<double> interp_point (vector<double> index , double in){
    vector<double> vector_double ;
    if(in < index[0]){
        vector_double.push_back(0);
        vector_double.push_back(1);
    }
    if(in > index[index.size()-1]){
        vector_double.push_back(index.size()-2); // vector 倒數第2個
        vector_double.push_back(index.size()-1); // vector 最後一個
    }

    for(int i=0 ; i< (index.size()-1) ;i++){  //夾在中間
        // bool a =false;
        // if(i==3){
        //     a = ( in<index[i+1]); // index[i]<in<index[i+1]
        //     cout << "i: " << i<< " " << a <<endl ;
        //     cout << " in " << in <<" index[i]"<< index[i] <<" index[i+1]"<< index[i+1] <<endl;
        // }
        if(( index[i]<in)&&(in<index[i+1])) { //index[i]<in<index[i+1] 會錯
            vector_double.push_back(i); 
            vector_double.push_back(i+1); 
           // cout << i << endl;
            break ;
        }

        if( index[i]==in) { //index[i]<in<index[i+1] 會錯
            vector_double.push_back(i); 
            vector_double.push_back(i); 
            break ;
        }
        else if( index[i+1]==in) { //index[i]<in<index[i+1] 會錯
            vector_double.push_back(i+1); 
            vector_double.push_back(i+1); 
            break ;
        }

    }
    return vector_double;
}
double interp1(double x0, double y0, double x1, double y1, double x) {
    double y =0  ;
    // 確保 x0 不等於 x1，避免除以 0 的情況
    // cout << x0 <<" "<<x1 << endl;
    if (x0 == x1) {
        y = y0 ;
        return y ;
    }
    else {
    // 使用線性外插計算在 x 點的估計值
    double y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
    // cout << "what: "<< y << endl;
     return y;
    }
}
double interp2(double x0, double y0 ,double x1, double y1,double z0 ,double z1 , double z2 ,  double z3, double x ,double y ) { // double z0 z1 左至右上而下
    double a0 ,a1;
    double z ;
    a0 = interp1(x0 , z0 , x1 , z2 , x) ;
    //cout <<  a0 <<endl ;
    a1 = interp1(x0 , z1 , x1 , z3 , x) ;
    //cout <<  a1 <<endl ;
    z  = interp1(y0 , a0 , y1 , a1 , y) ;
    return z; 
}


gate_delay caculate_delay (const double input_transition , const double cap ,const string gate_name ){
    gate_delay ans;
    vector<double> i;
    vector<double> j;
    double delay , output_transition ,output_transition2;
    double x0 , x1 , y0 , y1 , z0 , z1 , z2 , z3 , x , y;
    double cell_fall , cell_rise ; 
    i =  interp_point (index_input_transition_time , input_transition);
    cout <<"output 電容大小"<< cap << endl;
    //double cc =1.00140001; //test cap 
    j = interp_point (index_output_net_capacitance , cap);
    cout << gate_name << " i: " << i[0]<<" " << i[1]<< " j: " << j[0]<<" " << j[1]  << endl;
    x0 =  index_input_transition_time[i[0]];
    x1 =  index_input_transition_time[i[1]];
    y0 = index_output_net_capacitance[j[0]];
    y1 = index_output_net_capacitance[j[1]];
    x = input_transition;
    y= cap;
    if(gate_name == "INVX1"){
        z0 = cell_fall_INVX1[i[0]][j[0]];
        z1 = cell_fall_INVX1[i[0]][j[1]];
        z2 = cell_fall_INVX1[i[1]][j[0]];
        z3 = cell_fall_INVX1[i[1]][j[1]];
        //cell_fall = interp2( 0.0208,  0.012500 , 0.0336,  0.025100, 0.028037,0.031885 , 0.032287,0.03668,  0 , 0.0188961 );
        cell_fall = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
        z0 = cell_rise_INVX1[i[0]][j[0]];
        z1 = cell_rise_INVX1[i[0]][j[1]];
        z2 = cell_rise_INVX1[i[1]][j[0]];
        z3 = cell_rise_INVX1[i[1]][j[1]];
        cell_rise = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
        cout <<"delay fall: " << cell_fall <<endl ;
        cout << "delay rise: " << cell_rise <<endl ;

        if(cell_fall >cell_rise){
            delay = cell_fall;
            z0 = fall_transition_INVX1[i[0]][j[0]];
            z1 = fall_transition_INVX1[i[0]][j[1]];
            z2 = fall_transition_INVX1[i[1]][j[0]];
            z3 = fall_transition_INVX1[i[1]][j[1]];
            //cell_fall = interp2( 0.0208,  0.012500 , 0.0336,  0.025100, 0.028037,0.031885 , 0.032287,0.03668,  0 , 0.0188961 );
            output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
            ans.rise = false ;
            }
        else if (cell_fall == cell_rise){
            delay = cell_fall;
            z0 = fall_transition_INVX1[i[0]][j[0]];
            z1 = fall_transition_INVX1[i[0]][j[1]];
            z2 = fall_transition_INVX1[i[1]][j[0]];
            z3 = fall_transition_INVX1[i[1]][j[1]];
            output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);

            z0 = rise_transition_INVX1[i[0]][j[0]];
            z1 = rise_transition_INVX1[i[0]][j[1]];
            z2 = rise_transition_INVX1[i[1]][j[0]];
            z3 = rise_transition_INVX1[i[1]][j[1]];
            output_transition2 = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
            if(output_transition2 >output_transition)
                output_transition = output_transition2;
            ans.rise = true ;
        }
        else {
            delay = cell_rise;
            z0 = rise_transition_INVX1[i[0]][j[0]];
            z1 = rise_transition_INVX1[i[0]][j[1]];
            z2 = rise_transition_INVX1[i[1]][j[0]];
            z3 = rise_transition_INVX1[i[1]][j[1]];
           
            output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
            ans.rise = true ;
        }
        ans.delay = delay;
        ans.transition_time = output_transition;

        return ans;
    }

    if(gate_name == "NOR2X1"){
        z0 = cell_fall_NOR2X1[i[0]][j[0]];
        z1 = cell_fall_NOR2X1[i[0]][j[1]];
        z2 = cell_fall_NOR2X1[i[1]][j[0]];
        z3 = cell_fall_NOR2X1[i[1]][j[1]];
        //cell_fall = interp2( 0.0208,  0.012500 , 0.0336,  0.025100, 0.028037,0.031885 , 0.032287,0.03668,  0 , 0.0188961 );
        cell_fall = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
        z0 = cell_rise_NOR2X1[i[0]][j[0]];
        z1 = cell_rise_NOR2X1[i[0]][j[1]];
        z2 = cell_rise_NOR2X1[i[1]][j[0]];
        z3 = cell_rise_NOR2X1[i[1]][j[1]];
        cell_rise = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
        cout <<"delay fall: " << cell_fall <<endl ;
        cout << "delay rise: " << cell_rise <<endl ;

        if(cell_fall >cell_rise){
            delay = cell_fall;
            z0 = fall_transition_NOR2X1[i[0]][j[0]];
            z1 = fall_transition_NOR2X1[i[0]][j[1]];
            z2 = fall_transition_NOR2X1[i[1]][j[0]];
            z3 = fall_transition_NOR2X1[i[1]][j[1]];
            //cell_fall = interp2( 0.0208,  0.012500 , 0.0336,  0.025100, 0.028037,0.031885 , 0.032287,0.03668,  0 , 0.0188961 );
            output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
            ans.rise = false ;
            }
        else if (cell_fall == cell_rise){
            delay = cell_fall;
            z0 = fall_transition_NOR2X1[i[0]][j[0]];
            z1 = fall_transition_NOR2X1[i[0]][j[1]];
            z2 = fall_transition_NOR2X1[i[1]][j[0]];
            z3 = fall_transition_NOR2X1[i[1]][j[1]];
            output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);

            z0 = rise_transition_NOR2X1[i[0]][j[0]];
            z1 = rise_transition_NOR2X1[i[0]][j[1]];
            z2 = rise_transition_NOR2X1[i[1]][j[0]];
            z3 = rise_transition_NOR2X1[i[1]][j[1]];
            output_transition2 = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
            if(output_transition2 >output_transition)
                output_transition = output_transition2;
            ans.rise = true ;
        }
        else {
            delay = cell_rise;
            z0 = rise_transition_NOR2X1[i[0]][j[0]];
            z1 = rise_transition_NOR2X1[i[0]][j[1]];
            z2 = rise_transition_NOR2X1[i[1]][j[0]];
            z3 = rise_transition_NOR2X1[i[1]][j[1]];
           
            output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
            ans.rise = true ;
        }
        ans.delay = delay;
        ans.transition_time = output_transition;

        return ans;
    }

    if(gate_name == "NANDX1"){
        z0 = cell_fall_NANDX1[i[0]][j[0]];
        z1 = cell_fall_NANDX1[i[0]][j[1]];
        z2 = cell_fall_NANDX1[i[1]][j[0]];
        z3 = cell_fall_NANDX1[i[1]][j[1]];
        //cell_fall = interp2( 0.0208,  0.012500 , 0.0336,  0.025100, 0.028037,0.031885 , 0.032287,0.03668,  0 , 0.0188961 );
        cell_fall = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
        z0 = cell_rise_NANDX1[i[0]][j[0]];
        z1 = cell_rise_NANDX1[i[0]][j[1]];
        z2 = cell_rise_NANDX1[i[1]][j[0]];
        z3 = cell_rise_NANDX1[i[1]][j[1]];
        cell_rise = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
        cout <<"delay fall: " << cell_fall <<endl ;
        cout << "delay rise: " << cell_rise <<endl ;

        if(cell_fall >cell_rise){
            delay = cell_fall;
            z0 = fall_transition_NANDX1[i[0]][j[0]];
            z1 = fall_transition_NANDX1[i[0]][j[1]];
            z2 = fall_transition_NANDX1[i[1]][j[0]];
            z3 = fall_transition_NANDX1[i[1]][j[1]];
            //cell_fall = interp2( 0.0208,  0.012500 , 0.0336,  0.025100, 0.028037,0.031885 , 0.032287,0.03668,  0 , 0.0188961 );
            output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
            ans.rise = false ;
            }
        else if (cell_fall == cell_rise){
            delay = cell_fall;
            z0 = fall_transition_NANDX1[i[0]][j[0]];
            z1 = fall_transition_NANDX1[i[0]][j[1]];
            z2 = fall_transition_NANDX1[i[1]][j[0]];
            z3 = fall_transition_NANDX1[i[1]][j[1]];
            output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);

            z0 = rise_transition_NANDX1[i[0]][j[0]];
            z1 = rise_transition_NANDX1[i[0]][j[1]];
            z2 = rise_transition_NANDX1[i[1]][j[0]];
            z3 = rise_transition_NANDX1[i[1]][j[1]];
            output_transition2 = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
            if(output_transition2 >output_transition)
                output_transition = output_transition2;
            ans.rise = true ;
        }
        else {
            delay = cell_rise;
            z0 = rise_transition_NANDX1[i[0]][j[0]];
            z1 = rise_transition_NANDX1[i[0]][j[1]];
            z2 = rise_transition_NANDX1[i[1]][j[0]];
            z3 = rise_transition_NANDX1[i[1]][j[1]];
           
            output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
            ans.rise = true ;
        }
        ans.delay = delay;
        ans.transition_time = output_transition;

        return ans;
    }
}

void lib_setting ( ){//vector<vector<double>>& delay_table_INVX1 
    index_output_net_capacitance = {0.001400,0.003000,0.006200,0.012500,0.025100,0.050400,0.101000};
    index_input_transition_time = {0.0208,0.0336,0.06,0.1112,0.2136,0.4192,0.8304};
    rise_power_INVX1  = {
            {0.006596,0.008956,0.009062,0.009373,0.011152,0.014745,0.022057},
            {0.002327,0.001160,0.008388,0.009796,0.010724,0.014648,0.022397},
            {0.005876,0.008937,0.005470,0.009384,0.011156,0.013834,0.021505},
            {0.009800,0.013258,0.006654,0.007355,0.011751,0.013941,0.019906},
            {0.009265,0.003829,0.001602,0.005923,0.009966,0.013130,0.018865},
            {0.007550,0.006479,0.006212,0.005920,0.001487,0.012963,0.016774},
            {0.007470,0.005900,0.004800,0.001880,0.000000,0.000000,0.019160} 
    };
    fall_power_INVX1 = {
            {0.000000,0.000000,0.000000,0.000000,0.001344,0.004706,0.011590},
            {0.000000,0.000000,0.000000,0.000000,0.001375,0.004474,0.011340},
            {0.000147,0.000000,0.000810,0.000000,0.001046,0.004332,0.010929},
            {0.000000,0.000000,0.000000,0.000000,0.001021,0.003817,0.010102},
            {0.000000,0.000000,0.000000,0.000000,0.000787,0.003143,0.009316},
            {0.000000,0.000000,0.000000,0.000000,0.000254,0.002648,0.007777},
            {0.000000,0.000000,0.000000,0.000000,0.000124,0.001639,0.006515}
    };
    cell_rise_INVX1 = {
            {0.013379,0.016044,0.019907,0.022481,0.031726,0.040185,0.050385},
            {0.01525 ,0.018376,0.022972,0.026053,0.03705 ,0.047418,0.059875},
            {0.018871,0.022403,0.028108,0.03261 ,0.045789,0.059925,0.076608},
            {0.02597 ,0.029306,0.033033,0.042608,0.054479,0.080008,0.10394},
            {0.040047,0.040538,0.045817,0.058067,0.076355,0.09933 ,0.14683},
            {0.066295,0.068558,0.073341,0.083717,0.10821 ,0.14397 ,0.18904},
            {0.12223 ,0.12441 ,0.1292  ,0.13863 ,0.15947 ,0.20864 ,0.2792  }

    } ;
    cell_fall_INVX1 = {
            {0.015975,0.018648,0.022904,0.028037,0.031885,0.041402,0.053623},
            {0.018156,0.021149,0.026094,0.032287,0.03668,0.048075,0.062383},
            {0.022594,0.02559,0.031664,0.039739,0.044932,0.059061,0.077439},
            {0.031149,0.034095,0.040822,0.051786,0.060032,0.076833,0.10231},
            {0.047947,0.047918,0.057692,0.063035,0.082424,0.10735,0.14034},
            {0.079511,0.081269,0.085301,0.093628,0.11641,0.1545,0.202   },
            {0.14604,0.14772,0.15158,0.15967,0.17681,0.22372,0.29848     }
    };
    rise_transition_INVX1= {
            {0.017522,0.021682,0.028803,0.041672,0.052872,0.081109,0.12837},
            {0.02198,0.024661,0.033109,0.046939,0.060235,0.088586,0.13712},
            {0.029796,0.032708,0.038207,0.056464,0.07806,0.099037,0.15561},
            {0.044871,0.046301,0.05161,0.068661,0.10222,0.12468,0.18211},
            {0.076836,0.076772,0.076054,0.089262,0.12837,0.18676,0.23216},
            {0.1417,0.14121,0.14198,0.14212,0.17126,0.2441,0.35429},
            {0.27081,0.26681,0.26943,0.27422,0.27429,0.33228,0.47341}
    };
    fall_transition_INVX1= {
            {0.017349,0.017893,0.024178,0.035104,0.051821,0.075654,0.12316},
            {0.018614,0.022157,0.027649,0.040185,0.059238,0.085087,0.13344},
            {0.027419,0.029372,0.034899,0.049446,0.072893,0.10429,0.15246},
            {0.043657,0.044863,0.049056,0.060723,0.091025,0.13231,0.18596},
            {0.07653,0.073661,0.079082,0.087928,0.12085,0.17316,0.24881},
            {0.13749,0.13518,0.13793,0.14913,0.16585,0.22913,0.33942},
            {0.26533,0.26248,0.26553,0.26592,0.28688,0.33103,0.45258}
    };
    cell_rise_NANDX1= {
            {0.020844,0.02431,0.030696,0.039694,0.048205,0.072168,0.10188},
            {0.024677,0.027942,0.035042,0.045467,0.054973,0.082349,0.11539},
            {0.032068,0.035394,0.042758,0.055361,0.065991,0.090936,0.13847},
            {0.046811,0.049968,0.057164,0.064754,0.086481,0.11676,0.15744},
            {0.073919,0.078805,0.080873,0.091007,0.11655,0.1579,0.21448},
            {0.13162,0.13363,0.1383,0.14793,0.1685,0.22032,0.30054},
            {0.24661,0.24835,0.25294,0.26221,0.282,0.32417,0.42783}
    };      
    cell_fall_NANDX1 = {
                {0.020382,0.023257,0.027894,0.033548,0.03574,0.04179,0.044857},
                {0.024029,0.026956,0.032263,0.039221,0.042085,0.050402,0.056668},
                {0.031269,0.034119,0.039869,0.049176,0.053737,0.065333,0.07578},
                {0.045259,0.047829,0.053987,0.057975,0.072805,0.089369,0.10958},
                {0.071393,0.072867,0.076097,0.082901,0.10269,0.13035,0.16091},
                {0.12714,0.12851,0.13175,0.13825,0.15292,0.19222,0.2452},
                {0.23846,0.23967,0.2429,0.24923,0.26252,0.29317,0.37116}
        };          
    rise_transition_NANDX1= {
                {0.030236,0.03225,0.038219,0.052208,0.080479,0.10719,0.15147},
                {0.039514,0.039353,0.046985,0.061163,0.087011,0.11697,0.17515},
                {0.051087,0.053959,0.058155,0.070827,0.10934,0.15223,0.20641},
                {0.085969,0.08422,0.092301,0.096621,0.12925,0.19941,0.26597},
                {0.15372,0.14705,0.15469,0.1603,0.17955,0.24852,0.35789},
                {0.28669,0.28022,0.28121,0.28888,0.27863,0.3316,0.48256},
                {0.5525,0.53592,0.53441,0.53503,0.55683,0.54271,0.65957}
        };
    fall_transition_NANDX1= {
                {0.027802,0.028745,0.035327,0.047243,0.067704,0.097889,0.15165},
                {0.03416,0.03663,0.041513,0.054197,0.079795,0.11013,0.16545},
                {0.048764,0.050279,0.053838,0.06597,0.098021,0.13303,0.19592},
                {0.076954,0.077267,0.077694,0.08918,0.11528,0.17286,0.23832},
                {0.13307,0.13316,0.13508,0.14106,0.16249,0.2137,0.3243},
                {0.24709,0.24136,0.24516,0.25064,0.26398,0.31964,0.44603},
                {0.47444,0.46022,0.46284,0.46654,0.48133,0.48929,0.60317}
        };
    cell_rise_NOR2X1  = {
                {0.023104,0.026347,0.032439,0.037211,0.050187,0.075629,0.10893},
                {0.027062,0.029858,0.036397,0.042338,0.055823,0.083858,0.11942},
                {0.034333,0.037002,0.043649,0.050318,0.064815,0.089926,0.13827},
                {0.048838,0.051443,0.053267,0.064115,0.083493,0.11205,0.17069},
                {0.076172,0.077668,0.081485,0.08978,0.11263,0.14904,0.20359},
                {0.13351,0.13486,0.13847,0.14635,0.16278,0.20974,0.27949},
                {0.24777,0.24888,0.25234,0.25971,0.27596,0.30968,0.40393}
        };   
    cell_fall_NOR2X1 = {
                {0.017693,0.020473,0.024855,0.029528,0.03475,0.038561,0.030505},
                {0.019856,0.022665,0.027792,0.033415,0.03982,0.045554,0.039165},
                {0.024076,0.026936,0.032932,0.040339,0.048928,0.057779,0.054661},
                {0.03228,0.035141,0.041568,0.051796,0.064313,0.069003,0.080192},
                {0.048376,0.051227,0.057825,0.070186,0.089026,0.09852,0.11886},
                {0.078389,0.08013,0.090011,0.091689,0.11283,0.15092,0.17976},
                {0.14202,0.14372,0.14735,0.15483,0.17074,0.21341,0.2901}
            };        
    rise_transition_NOR2X1= {
                {0.047372,0.047388,0.05291,0.069558,0.079757,0.11295,0.16298},
                {0.055454,0.057237,0.061359,0.076048,0.089491,0.12218,0.1786},
                {0.070803,0.069305,0.07765,0.08839,0.10706,0.15805,0.20462},
                {0.097384,0.095745,0.10667,0.11394,0.1372,0.17641,0.23705},
                {0.16606,0.16368,0.16716,0.17,0.18593,0.23581,0.33823},
                {0.2963,0.29,0.29077,0.29997,0.29596,0.34204,0.4312},
                {0.55605,0.54072,0.53969,0.54185,0.56377,0.57026,0.65233}
            };   
    fall_transition_NOR2X1= {
                {0.0178,0.020672,0.025912,0.038106,0.054758,0.087109,0.14053},
                {0.02211,0.024126,0.029235,0.043016,0.061952,0.094784,0.15217},
                {0.02905,0.032076,0.035248,0.05071,0.073218,0.11045,0.17034},
                {0.045602,0.046546,0.049838,0.065112,0.089226,0.13818,0.22076},
                {0.07724,0.077419,0.079545,0.087836,0.11532,0.18049,0.25971},
                {0.13593,0.13398,0.1414,0.14681,0.16852,0.22528,0.3437},
                {0.25981,0.25461,0.25367,0.26263,0.27702,0.33235,0.44369}
            };   

}

int main(int argc, char* argv[])
{
     // read .v //
	ifstream inFile;
        inFile.open(argv[1]); 
        if (!inFile) {
        return 1;
        } 
    ofstream txt_delay ,txt_capacitance,txt_max_delay , txt_gate ,tx_path; 
    string line;
    string str ; 
    char ch;
    int num  ; 
    int node_num = 0 ;
    int id ; 
    bool output_read, input_read,wire_read =false ;
    while (getline(inFile, line)) {

        // 如果是註解，移除註解部分 chat gpt //
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        line = removeComments(line); // 移除 /* */

        stringstream in;
        in << line;
        in >> str ;    //讀每一行第一個單字
       if( str == "input"){
            while (in >> ch){
                in >> num;
                str = ch + to_string(num);
                input.push_back(str);
                bool used = false;
                for(int i=0 ; i< node.size(); i++){
                    if(ch == node[i].name){  // 辨認節點前面的字母 n、N
                        node[i].num ++;
                        used = true;
                        if(num > node[i].max_id) 
                            node[i].max_id = num;
                        break;
                    }
                }
                if(used == false){
                    nnode tmp;
                    tmp.name = ch;
                    tmp.num = 0; 
                    node.push_back (tmp);
                }
                in >> ch ;  // 拿掉分號 逗號
            }
            input_read = true ;
       }
       if( str == "output"){
            while (in >> ch){
                in >> num;
                str = ch + to_string(num);
                output.push_back(str);
                bool used = false;
                for(int i=0 ; i< node.size(); i++){
                    if(ch == node[i].name){
                        node[i].num ++;
                        used = true;
                        if(num > node[i].max_id) 
                            node[i].max_id = num;
                        break;
                    }
                }
                if(used == false){
                    nnode tmp;
                    tmp.name = ch;
                    tmp.num = 0; 
                    node.push_back (tmp);
                }
                in >> ch ; 
                //if(num > node_num) node_num = num;

            }
            output_read = true ;
       }
       if( str == "wire"){
            while (in >> ch){
                in >> num;  // 讀出n*
                bool used = false;
                for(int i=0 ; i< node.size(); i++){
                    if(ch == node[i].name){
                        node[i].num ++;
                        used = true;
                        if(num > node[i].max_id) 
                            node[i].max_id = num;
                        break;
                    }
                }
                if(used == false){
                    nnode tmp;
                    tmp.name = ch;
                    tmp.num = 0; 
                    node.push_back (tmp);
                }
                //if(num > node_num) node_num = num;//node_num ++;
                in >> ch ; 
            }
            wire_read= true;
       }
       if(wire_read&&output_read&&input_read) break;
    
    } 
       ///////////////////////////////////////////////////////////////////
      //           step 1 read the gate and update capacitance     // 
      ///////////////////////////////////////////////////////////////////
        vector <string> gate_tmp;
        vector <double> double_vector;
        // 建 cap 表 //
        for (int i=0 ; i<node.size(); i++){
            double_vector.resize (node[i].max_id+1);
            capacitance.push_back (double_vector);
        }
        for (int i=0 ; i<capacitance.size(); i++){
            cout << "cap" << i <<": " <<capacitance[i].size() << endl;
        }
    while (getline(inFile, line)) {
        stringstream in;
        in << line;
        in >> str ;
       if((str == "INVX1")||(str == "NANDX1")||(str == "NOR2X1")){
            bool second_input = false;
            gate_tmp.push_back(str); //寫回gate種類
            in >>ch>> num;
            str = ch + to_string(num);
            gate_tmp.push_back(str); // g* 寫回
            gate_tmp.push_back(str);    // 準備好output 的位置，怕output不是下一個寫進來的
            in >> ch>>ch ;  //把第一個 '(' 拿掉
            while(in >> ch){
                if( ch == 'Z'){
                    in >> ch >> ch; //去掉 'N('
                    in >>ch>> num;
                    str = ch + to_string(num);
                    //gate_tmp.push_back(str); //寫回output output不一定在input前面 會有bug 
                    gate_tmp[2] =str;
                }
                else if(ch == '('){  //判斷為input port
                    in >>ch>> num;
                    str = ch + to_string(num);
                    gate_tmp.push_back(str); //寫回input
                    // 加電容 
                    id = table_id(ch ,node);
                    //cout << "id: "<< id << endl;
                        if(gate_tmp[0] == "INVX1"){
                            capacitance[id][num] +=  0.0109115; // INVX1 input capacitance
                        }
                        else if (gate_tmp[0] == "NANDX1"){
                           if(second_input)
                               capacitance[id][num] +=  0.00798456;
                           else 
                               capacitance[id][num] += 0.00683597;
                        }
                        else if (gate_tmp[0] == "NOR2X1"){
                            if(second_input)
                              capacitance[id][num] +=   0.0108106; 
                            else 
                               capacitance[id][num] +=  0.0105008;
                        }
                     second_input = true;
                }
            }
            gate.push_back(gate_tmp);
            gate_tmp.clear();
       }
/*
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
       */
       in.clear();
    }   
     inFile.close(); 
        stringstream in;                                        /// ----output capacitance 要加回去 ---//
        for (int i = 0; i < output.size(); i++) {
            in << output[i];
	    	in >>ch>> num; 
            // cout << num << endl;
            // cout<< output[i]<<" " <<num << endl;
            id = table_id(ch ,node);
            capacitance[id][num] += 0.03;
            in.clear();
	    }

   ///////////////////////////////////////////////////////////////////
   //            capacitance  output                            // 
   ///////////////////////////////////////////////////////////////////
    vector<vector<string>> ans_capacitance ;
    for (int i = 0; i < gate.size(); i++) {
        vector<string> str_vector;
        stringstream in , cap_str; 
        double cap ;
        in << gate[i][2];
        in >>ch>> num;   // read ans_capacitance
        id = table_id(ch ,node);
        cap = round(capacitance[id][num]*1e6) / 1e6; // 小數點第6位 4捨5入
        str_vector.push_back( gate[i][1]); 
        str_vector.push_back(to_string(cap));
        ans_capacitance.push_back(str_vector);
        //  sort // 
        for(int i = ans_capacitance.size()-1; i > 0; i--){
            if((ans_capacitance[i][1] == ans_capacitance[i-1][1])&&(ans_capacitance[i][0] < ans_capacitance[i-1][0])
                    &&(ans_capacitance[i][0].length() < ans_capacitance[i-1][0].length())){
                swap (ans_capacitance[i],ans_capacitance[i-1]);
            }
            else if ((ans_capacitance[i][1] > ans_capacitance[i-1][1]))
                swap (ans_capacitance[i],ans_capacitance[i-1]);
        }
	}
    


      //for (int i = 0; i < capacitance.size(); i++) {
	    	//cout<< capacitance [i] << endl;
	   // }
        // output capacitance 要加回去 //

    //    for (int i = 0; i <gate.size() ; i++) {
    //         for (int j = 0 ; j < gate[i].size(); j++)
	//     	cout<< gate [i][j] << endl;
	//     }
       // cout<<"node number: "<< node_num << endl;

    

    //cout<< delay_table_INVX1[0].size() << endl;
///////////////////////////////////////////////////////////////////
//            step 2 caculate delay                             // 
//////////////////////////////////////////////////////////////////
    lib_setting( );

    // 建表 //
    vector<bool> bool_vector;
    vector<string> str_vector;

        for (int i=0 ; i<node.size(); i++){
            double_vector.resize (node[i].max_id+1,0);
            bool_vector.resize (node[i].max_id+1,false);
            str_vector.resize (node[i].max_id+1);
            transition_time.push_back (double_vector);
            acc_maxdelay.push_back (double_vector);
            acc_mindelay.push_back (double_vector);
            in_min.push_back (str_vector);
            in_max.push_back (str_vector);
            delay.push_back (double_vector);
            valid_node.push_back(bool_vector);
            node_rise.push_back(bool_vector);
        }

    
    //set input transition 0ns //
    for(int i=0 ; i<input.size(); i++){
        stringstream in;
        in << input[i];
        in >> ch >>num;
        id = table_id(ch ,node);
        //transition_time[id][num] = 0;
        valid_node[id][num] =true;
        in.clear();
    }

    
    //  for (int i = 0; i < valid_node[0].size(); i++) {
	//  	cout<< valid_node[0][i] << endl;
	//  }

    // 開始算delay output transition time // 
    vector<bool>gate_delay_used ;
    gate_delay_used.resize(gate.size(),false);
    matrix_id in_a_id ,in_b_id ,out_id ;
    double cap ,db ;
    string gate_name;
    cout << endl;
    double acc_delay_1 ,acc_delay_2 ; //acc_min_delay_1 ,acc_min_delay_2;
    double input_transition;
    string in_a,in_b ; 
    gate_delay gate_inf;
    vector<string> vector_str;
    bool done =false;
    int tt=0 ; 
    while(done==false){
        done =true ;
        tt++ ;
        for(int i=0 ; i<gate_delay_used.size(); i++){
            
            if(gate_delay_used[i]) continue;
            cout << endl;
            done =false ;
            gate_name =gate[i][0];
            str = gate[i][1];
            cout <<str << endl;
            str = gate[i][2];  // read output capacitance
            out_id = string2matrix_index(str);
            cap = capacitance[out_id.row][out_id.column];

            in_a = gate[i][3]; //read input 1 delay
            in_a_id = string2matrix_index(in_a);
            if (valid_node[in_a_id.row][in_a_id.column]==0) continue; // input1 不valid 就跳掉
            acc_delay_1 = acc_maxdelay[in_a_id.row][in_a_id.column]+0.005;
            //acc_min_delay_1 = acc_mindelay[in_a_id.row][in_a_id.column]+0.005;
            input_transition = transition_time[in_a_id.row][in_a_id.column];


            acc_delay_2 = 0;
            if((gate_name =="NANDX1" )||(gate_name == "NOR2X1")){ //這兩種gate要讀input 2
                in_b=gate[i][4]; 
                in_b_id = string2matrix_index(in_b);
                if (valid_node[in_b_id.row][in_b_id.column]==0) continue; // input2 不valid 就跳掉
                acc_delay_2 = acc_maxdelay[in_b_id.row][in_b_id.column]+0.005;
               // acc_min_delay_2 = acc_mindelay[in_b_id.row][in_b_id.column]+0.005;
            }

            for(int i=0; i<input.size(); i++){ //node 是input的話 把wire delay加0.005扣掉
                if(in_a == input[i] ){
                    acc_delay_1 = acc_delay_1-0.005;
                   // acc_min_delay_1 = acc_min_delay_1-0.005;
                }
                if(in_b == input[i]){
                     acc_delay_2 = acc_delay_2-0.005;
                    // acc_min_delay_2 = acc_min_delay_2-0.005;
                }
            }
            if(acc_delay_2 > acc_delay_1){
                input_transition =transition_time[in_b_id.row][in_b_id.column];
                acc_delay_1 =acc_delay_2; // acc_delay_1 這邊更新為最大delay 到gate input的delay
                swap(in_a,in_b); //in_a 為max_input 的index  
            }
            else if (acc_delay_2 == acc_delay_1){
                if(transition_time[in_b_id.row][in_b_id.column] > input_transition){
                    input_transition = transition_time[in_b_id.row][in_b_id.column] ;
                }
            }

            // min delay //
            // min_id = in_a;
            // if(acc_min_delay_2 < acc_min_delay_1){
            //     acc_min_delay_1 =acc_min_delay_2; // acc_delay_1 這邊更新為最大delay 到gate input的delay
            //     min_id = in_b; //in_a 為max_input 的index  
            // }

            
            gate_inf = caculate_delay (input_transition,cap ,gate_name );

            acc_maxdelay[out_id.row][out_id.column] = acc_delay_1 +gate_inf.delay ;
           //acc_mindelay[out_id.row][out_id.column] = acc_min_delay_1 +gate_inf.delay ;
            in_max[out_id.row][out_id.column]=in_a;
            in_min[out_id.row][out_id.column]=in_b;

            transition_time[out_id.row][out_id.column] =gate_inf.transition_time;
            delay[out_id.row][out_id.column]= gate_inf.delay;
            node_rise[out_id.row][out_id.column]= gate_inf.rise;
            valid_node[out_id.row][out_id.column] =true ;
            cout<<"valid_node" <<out_id.row <<out_id.column << endl;
	        cout << "rise: "<<gate_inf.rise << " delay: "<<gate_inf.delay<< "  transition_time: " << gate_inf.transition_time << endl;

            gate_delay_used[i] = true ;

            str = gate[i][1];
            vector_str.push_back(str);
            str =(gate_inf.rise)? "1":"0";
            vector_str.push_back(str);
            db = round(gate_inf.delay*1e6) / 1e6;
            vector_str.push_back(to_string(db));
            db = round(gate_inf.transition_time*1e6) / 1e6;
            vector_str.push_back(to_string(db));
            
            ans_delay.push_back (vector_str);
            vector_str.clear();
          

            // debug max_delay // /////debug
            str = gate[i][1];
            vector_str.push_back(str);
            vector_str.push_back( to_string(acc_maxdelay[out_id.row][out_id.column]));
            ans_maxdelay.push_back(vector_str);
            vector_str.clear();
            gate_ans.push_back(gate[i][1]);
        /////////////////////////////////////////////////////////////////////////
            for(int i = ans_delay.size()-1; i > 0; i--){
                if((ans_delay[i][2] == ans_delay[i-1][2])&&(ans_delay[i][0] < ans_delay[i-1][0])
                        &&(ans_delay[i][0].length() < ans_delay[i-1][0].length())){
                    swap (ans_delay[i],ans_delay[i-1]);
                }
                else if ((ans_delay[i][2] > ans_delay[i-1][2]))
                    swap (ans_delay[i],ans_delay[i-1]);
            }
            // for(int i = ans_maxdelay.size()-1; i > 0; i--){
            //     if((ans_maxdelay[i][0] < ans_maxdelay[i-1][0])||(ans_maxdelay[i][0].length() > ans_maxdelay[i-1][0].length())){
            //         swap (ans_maxdelay[i],ans_maxdelay[i-1]);
            //     }
            // }
                        //  for (int i = 0; i < valid_node[0].size(); i++) {
	            // 	cout<< valid_node[0][i] << endl;
	            // }

        vector_str.clear();
        }
    }
    cout << "迴圈次數" << tt << endl;

   // for (int i = 0; i < ans_delay.size(); i++) {
            // for (int j = 0 ; j < vector_str.size(); j++){
	    	// cout<< vector_str[j] << " ";}
            // cout<< endl;
	   // }

//////////////////////////////////////////////////////////////////
//                  step 3     path                               //
///////////////////////////////////////////////////////////////
    double mmax_delay;
    double mmin_delay;
    string max_id , min_id ;
    vector<string> max_path,min_path;

    for(int i=0; i<output.size(); i++){ 
        double acc_delay;
        str = output[i];  // read output capacitance
        out_id = string2matrix_index(str);
        acc_delay =acc_maxdelay[out_id.row][out_id.column];
        cout <<"min delay : "<< i <<" :"<< to_string(acc_delay)<< endl;
        if(i==0){
            mmin_delay = acc_delay;
            min_id = output[i]; 
        }
        if(acc_delay < mmin_delay ){
            mmin_delay = acc_delay;
            min_id = output[i]; 
            // cout<<"min delay : "<< i <<" :"<< mmin_delay<< endl;
            //  cout << to_string(acc_mindelay[out_id.row][out_id.column])<< endl;
            //  cout <<output[i]<< endl;
        }
        if(acc_delay>mmax_delay){
            mmax_delay = acc_delay;
            max_id = output[i]; 
            //  cout<<"max delay : "<< i <<" :"<< mmax_delay<< endl;
            //  cout << to_string(acc_maxdelay[out_id.row][out_id.column])<< endl;
            //  cout <<output[i]<< endl;
        }
    }
    cout<<"max delay : "<< to_string(mmax_delay) << "   max id: "<< max_id<< endl;
    cout<<"min delay : "<<to_string(mmin_delay) <<"    min id: "<< min_id<< endl;

    



    // max path //
    str = max_id;
    done = false; 
    max_path.push_back(max_id);
    while(done ==0  ){
        out_id = string2matrix_index(str);
        str = in_max[out_id.row][out_id.column];
        //cout <<str<< endl;
        max_path.push_back(str);
        for(int i=0; i<input.size(); i++){ 
            if(str == input[i] ){ 
                done = true ;
                break;
            }
        }
    }
    reverse(max_path.begin(), max_path.end());

    // min path //
    min_path.push_back(min_id);
    cout <<endl<< endl;
    str = min_id;
    done = false; 
    while(done ==0  ){
        out_id = string2matrix_index(str);
        str = in_max[out_id.row][out_id.column];
       // cout <<str<< endl;
        min_path.push_back(str);
        for(int i=0; i<input.size(); i++){ //node 是input的話 把wire delay加0.005扣掉
            if(str == input[i] ){ 
                done = true ;
                break;
            }
        }
    }
    reverse(min_path.begin(), min_path.end());

    // ans 四捨五入小數第6位 //
    mmax_delay = (round(mmax_delay*1e6)) / 1e6;
    mmin_delay = round(mmin_delay*1e6) / 1e6;
  

    cout <<"max path "<< endl;
    for (int i = 0; i < max_path.size(); i++) {
	   	cout<< max_path[i] << endl;
	}
    cout << endl;
    cout <<"min path "<< endl;
    for (int i = 0; i < min_path.size(); i++) {
	    cout<< min_path[i] << endl;
	}
   
    
    



//}


    //  for (int i = 0; i < gate_delay_used.size(); i++) {
	//     	cout<< gate_delay_used[i] << endl;
	//     }

    // for (int i = 0; i < valid_node[0].size(); i++) {
    // 	cout<< valid_node[0][i] << endl;
    // }



    // cout<< rise_transition_NOR2X1[0].size() << endl;
    //  for (int i = 0; i < rise_transition_NOR2X1[0].size(); i++) {
	//     	cout<< rise_transition_NOR2X1[0][i] << endl;
	//     }


    // cout<< rise_power_INVX1[0].size() << endl;
    //  for (int i = 0; i < rise_power_INVX1[0].size(); i++) {
	//     	cout<< rise_power_INVX1[0][i] << endl;
	//     }
	    // for (int i = 0; i < input.size(); i++) {
	    // 	cout<< input [i] << endl;
	    // }

        cout<<"node : "<< node.size()<< endl;
        for (int i = 0; i < node.size(); i++) {
	    	cout<<"node : "<< node[i].name<<" "<<node[i].num<<" "<<node[i].max_id << endl;
	    }

        // double cell_fall; 
        //  cell_fall = interp2( 0.0208,  0.012500 , 0.0336,  0.025100, 0.028037,0.031885 , 0.032287,0.03668,  0 , 0.017337 );
        // cout<<cell_fall<< endl;
				
    //////////////////////////////////////////////////////////////////
    //                  write output to txt                       //
    ///////////////////////////////////////////////////////////////


    string base_name = argv[1];
    base_name.pop_back();
    base_name.pop_back();
    txt_capacitance.open("311510207_"+base_name+"_load.txt");
    for (int i = 0; i < ans_capacitance.size(); i++) {
            for (int j = 0 ; j < ans_capacitance[i].size(); j++){
	    	txt_capacitance<< ans_capacitance [i][j] << " ";}
            txt_capacitance<< endl;
	}
    txt_capacitance.close();
     

    txt_delay .open("311510207_"+base_name+"_delay.txt");
        for (int i = 0; i < ans_delay.size(); i++) {
            for (int j = 0 ; j < ans_delay[i].size(); j++){
	    	txt_delay<< ans_delay [i][j] << " ";}
            txt_delay<< endl;
	    }
    txt_delay.close();

    txt_max_delay .open("311510207_"+base_name+"_maxdelay.txt");
        for (int i = 0; i < ans_maxdelay.size(); i++) {
            for (int j = 0 ; j < ans_maxdelay[i].size(); j++){
	    	txt_max_delay<< ans_maxdelay [i][j] << " ";
            }
            txt_max_delay<< endl;
	    }
    txt_max_delay.close();

    txt_gate.open(base_name+"_gate.txt");
        for (int j = 0 ; j < gate_ans.size(); j++){
	        txt_gate<< gate_ans[j] << endl;
        }
    txt_gate.close();

    tx_path.open("311510207_"+base_name+"_path.txt");
        tx_path << "Longest delay = "<< to_string(mmax_delay)<<", the path is: " ;
        for (int j = 0 ; j < max_path.size(); j++){
	        tx_path<< max_path[j] ;
            if(j==max_path.size()-1 ) break;
            tx_path << " -> ";
        }

        tx_path<< endl;

        tx_path << "Shortest delay = "<< to_string(mmin_delay)<<", the path is: " ;
        for (int j = 0 ; j < min_path.size(); j++){
	        tx_path<< min_path[j];
            if(j==min_path.size()-1 ) break;
            tx_path << " -> ";
        }
    tx_path.close();


    return 0;
}
