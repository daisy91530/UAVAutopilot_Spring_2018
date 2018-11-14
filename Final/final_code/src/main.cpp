/*#include <opencv2/highgui.hpp>  // final project
#include <opencv2/aruco.hpp>
#include <iostream>
#include "ardrone/ardrone.h"
#include "pid.hpp"
#include <cstring>
#include <algorithm>
#include <vector>

//#include <opencv2/core.hpp>
//#include <opencv2/videoio.hpp>
//#include <opencv2/objdetect.hpp>
//#include <opencv2/imgproc.hpp>

#include "opencv2/objdetect/objdetect.hpp"
//#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;
// --------------------------------------------------------------------------
// main(Number of arguments, Argument values)
// Description  : This is the entry point of the program.
// Return value : SUCCESS:0  ERROR:-1
// --------------------------------------------------------------------------

static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs) {
    FileStorage fs(filename, FileStorage::READ);
    if(!fs.isOpened())
        return false;
    fs["intrinsic"] >> camMatrix;
    fs["distortion"] >> distCoeffs;
    return true;
}


int main(int argc, char *argv[])
{
    //AR.Drone class
    PIDManager PID("pid.yaml");
    
    CascadeClassifier face_cascade;
    //face_cascade.load( "haarcascade_frontalface_alt.xml" );
    if( !face_cascade.load( "haarcascade_frontalface_alt.xml") ){ printf("--(!)Error loading\n"); return -1; };
    ARDrone ardrone;
    
    // Initialize
    if (!ardrone.open()) {
        std::cout << "Failed to initialize." << std::endl;
        return -1;
    }
    
    
    // Battery
    std::cout << "Battery = " << ardrone.getBatteryPercentage() << "[%]" << std::endl;
    
    // Instructions
    std::cout << "***************************************" << std::endl;
    std::cout << "*       CV Drone sample program       *" << std::endl;
    std::cout << "*           - How to play -           *" << std::endl;
    std::cout << "***************************************" << std::endl;
    std::cout << "*                                     *" << std::endl;
    std::cout << "* - Controls -                        *" << std::endl;
    std::cout << "*    'Space' -- Takeoff/Landing       *" << std::endl;
    std::cout << "*    'Up'    -- Move forward          *" << std::endl;
    std::cout << "*    'Down'  -- Move backward         *" << std::endl;
    std::cout << "*    'Left'  -- Turn left             *" << std::endl;
    std::cout << "*    'Right' -- Turn right            *" << std::endl;
    std::cout << "*    'Q'     -- Move upward           *" << std::endl;
    std::cout << "*    'A'     -- Move downward         *" << std::endl;
    std::cout << "*                                     *" << std::endl;
    std::cout << "* - Others -                          *" << std::endl;
    std::cout << "*    'C'     -- Change camera         *" << std::endl;
    std::cout << "*    'Esc'   -- Exit                  *" << std::endl;
    std::cout << "*                                     *" << std::endl;
    std::cout << "***************************************" << std::endl;
    int done_id1=0;
    int detect_id1=0;
    int done_id2=0;
    int detect_id3=0;
    int done_id3=0;
    int done_id4=0;
    int detect_id4=0;
    float markerLength = 9.4;
    Mat cameraMatrix, distCoeffs;
    readCameraParameters("camera.xml", cameraMatrix, distCoeffs);
    cout << cameraMatrix << endl;
    
    Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::DICT_6X6_250);
    int go_right=0;
    
    while (1) {
        
        // Key input
        int key = cv::waitKey(100);
        if (key == 0x1b) break;
        
        // Get an image
        cv::Mat image = ardrone.getImage();
        
        // Take off / Landing
        if (key == ' ') {
            if (ardrone.onGround()) ardrone.takeoff();
            else                    ardrone.landing();
        }
        
        // Move
        double vx = 0.0, vy = 0.0, vz = 0.0, vr = 0.0;
        if (key == 'i' || key == CV_VK_UP)    vx =  1.0;
        if (key == 'k' || key == CV_VK_DOWN)  vx = -1.0;
        if (key == 'u' || key == CV_VK_LEFT)  vr =  1.0;
        if (key == 'o' || key == CV_VK_RIGHT) vr = -1.0;
        if (key == 'j') vy =  1.0;
        if (key == 'l') vy = -1.0;
        if (key == 'q') vz =  1.0;
        if (key == 'a') vz = -1.0;
        
        // Change camera
        static int mode = 0;
        if (key == 'c') ardrone.setCamera(++mode % 4);
        Mat imageCopy;
        if(image.empty()){
            continue;
        }
        image.copyTo(imageCopy);
        vector<int> ids; vector<vector<Point2f> > corners;
        aruco::detectMarkers(image, dictionary, corners, ids);
        vector< Vec3d > rvecs, tvecs;
        
        /*** this is new *********
        
        // Container of faces
        vector<Rect> faces;
        // Detect faces
        face_cascade.detectMultiScale( image, faces, 1.1, 5, 0|CV_HAAR_SCALE_IMAGE, Size(10, 10) );
        //if no marker detected, turn to find marker
        if(faces.size()>0){
            // To draw rectangles around detected faces
            for (unsigned i = 0; i<faces.size(); i++){
                rectangle(image,faces[i], Scalar(255, 255, 0), 2, 1);
                cout<<faces[i]<<endl;
                if(go_right==1){ //開始避障，我們讓人臉保持在畫面的左方
                    if(faces[i].x>30){          //大於30代表 人臉不在畫面左方             (x, y)=(0, 0)-->/***********
                        vx=0;vy=-0.3;vz=0;vr=0; //就讓他往右邊飛                                        /*
                    }else if(faces[i].x<30){    //如果在畫面左方，就往前飛                                /*
                        vx=0.5;vy=0;vz=0;vr=0;                                                       /***********<--(x, y)=(600, 300)
                    }
                }else if(faces[i].width<80){ //越靠近人臉 faces[i].width 就越大，實測結果如果小於80 表示我們距離不夠近，往前飛
                    vx=0.5;vy=0;vz=0;vr=0;
                }else {                      //如果大於80 就讓他開始避障
                    go_right=1;
                    cout<<"go_right"<<endl;
                }
            }
        }else if (ids.size()<=0) {
            if(done_id4==1){
                vx=0;vy=0;vz=0.5;vr=0;
            }
            else {
                vx=0;vy=0;vz=0;vr=-0.15;
            }//#######1.check if the speed of rotating is ok.
            
        }else{
            // if at least one marker detected
            //cout<<"inelse"<<endl;
            //cout<<ids[0]<<endl;
            //cout<<done_id2<<endl;
            aruco::drawDetectedMarkers(imageCopy, corners, ids);
            aruco::estimatePoseSingleMarkers(corners, markerLength, cameraMatrix, distCoeffs, rvecs, tvecs);
            for(int i=0; i<ids.size(); i++){
                cv::aruco::drawAxis(imageCopy, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.1);
            }
            for(int i=0;i<=ids.size();i++){
                if (ids[i]==1 && key == -1 && tvecs.size()&& done_id1==0) {//detect marker id1
                    detect_id1=1; cout<<"detect_id1"<<endl;
                    vx=0;vz=0;
                    Mat _error = Mat(4, 1, CV_64F);
                    _error.at<double>(0, 0)=tvecs[0][2]-80;
                    _error.at<double>(1, 0)=tvecs[0][0]-0;
                    _error.at<double>(2, 0)=tvecs[0][1]-0;
                    if(rvecs[0][0]<0)
                        _error.at<double>(3, 0)= -(rvecs[0][2]-(-0.0));
                    else
                        _error.at<double>(3, 0)= (rvecs[0][2]-(-0.0));
                    //cout<<_error.at<double>(3,0)<<endl;
                    Mat _output;
                    PID.getCommand(_error, _output);
                    //cout<<_output.at<double>(3,0)<<endl;                     //vr =  parallized to marker id1
                    if(_error.at<double>(0, 0)<20 && _error.at<double>(0, 0)>(-20) && _output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                        done_id1=1;cout<<"done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1"<<endl;
                    }
                    if(_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){ //叫正差不多了 fly straight
                        vr=0; vx=0.7;
                        cout<<"movingtoid1"<<endl;
                    }else if(_output.at<double>(3, 0)<=0.2 && _output.at<double>(3,0)>(-0.2)){
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.7;
                        else
                            vr=_output.at<double>(3, 0)/4.5;
                    }else if(_output.at<double>(3, 0)<=0.4 && _output.at<double>(3,0)>(-0.4)){
                        cout<<"turning rf"<<endl;
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.4;
                        else
                            vr=_output.at<double>(3, 0)/3.5;
                    }else if(_output.at<double>(3, 0)<=0.8 && _output.at<double>(3,0)>(-0.8)){
                        cout<<"turning rf"<<endl;
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.1;
                        else
                            vr=_output.at<double>(3, 0)/3;
                    }else{
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/0.8;
                        else
                            vr=_output.at<double>(3, 0)/2.3;
                    }
                }else if(ids[i]==2 && key == -1 && tvecs.size() && done_id2!=1 ){//detect id2
                    Mat _error = Mat(4, 1, CV_64F);
                    vz=0;
                    cout<<"inid2"<<endl;
                    _error.at<double>(0, 0)=tvecs[0][2]-100;
                    _error.at<double>(1, 0)=tvecs[0][0]-0;
                    _error.at<double>(2, 0)=tvecs[0][1]-0;
                    if(rvecs[0][0]<0)
                        _error.at<double>(3, 0)= -(rvecs[0][2]-(-0.0));
                    else
                        _error.at<double>(3, 0)= (rvecs[0][2]-(-0.0));
                    Mat _output;
                    PID.getCommand(_error, _output);
                    // cout<<_error.at<double>(0,0)<<endl;
                    // cout<<_error.at<double>(0, 0)<<endl;
                    //vx
                    if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)&&_error.at<double>(3, 0)<=0.3 && _error.at<double>(3,0)>(-0.3)){
                        done_id2=1;cout<<"done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2"<<endl;
                            go_right=0; //init for next face
                    }
                    if(_error.at<double>(0, 0)<=10 && _error.at<double>(0,0)>(-10)){
                        vx=0;
                    }else if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)){
                        vx=_error.at<double>(0, 0)/170;
                    }else if(_error.at<double>(0, 0)<=70 && _error.at<double>(0,0)>(-70)){
                        vx=_error.at<double>(0, 0)/200;
                    }else if(_error.at<double>(0, 0)<=160 && _error.at<double>(0,0)>(-160)){
                        vx=_error.at<double>(0, 0)/250;
                    }else{
                        vx=0.75;
                    }
                    //vr
                    if(_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                        vr=0;
                    }else if(_output.at<double>(3, 0)<=0.2 && _output.at<double>(3,0)>(-0.2)){
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.7;
                        else
                            vr=_output.at<double>(3, 0)/4.5;
                    }else if(_output.at<double>(3, 0)<=0.4 && _output.at<double>(3,0)>(-0.4)){
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.4;
                        else
                            vr=_output.at<double>(3, 0)/3.5;
                    }else if(_output.at<double>(3, 0)<=0.8 && _output.at<double>(3,0)>(-0.8)){
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.1;
                        else
                            vr=_output.at<double>(3, 0)/3;
                    }else{
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/0.8;
                        else
                            vr=_output.at<double>(3, 0)/2.3;
                    }
                }else if(ids[0]!=3 && ids[1]!=3 && key == -1 && tvecs.size() && done_id2==1 && detect_id3==0){//not yet detect id3
                    vr=-0.15;vx=0;vz=0;
                    cout<<" id2done"<<endl;
                }else if(ids[i]==3 && key == -1 && tvecs.size()&& done_id3==0 &&done_id2==1){//detect id3
                    detect_id3=1;cout<<"detect_id3"<<endl;
                    vz=0;
                    Mat _error = Mat(4, 1, CV_64F);
                    _error.at<double>(0, 0)=tvecs[0][2]-80;
                    _error.at<double>(1, 0)=tvecs[0][0]-0;
                    _error.at<double>(2, 0)=tvecs[0][1]-0;
                    if(rvecs[0][0]<0)
                        _error.at<double>(3, 0)= -(rvecs[0][2]-(-0.0));
                    else
                        _error.at<double>(3, 0)= (rvecs[0][2]-(-0.0));
                    Mat _output;
                    PID.getCommand(_error, _output);
                    //vx
                    if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)&&_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                        done_id3=1;cout<<"done_id3"<<endl;
                    }
                    if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)){
                        vx=0;
                    }else if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)){
                        vx=_error.at<double>(0, 0)/170;
                    }else if(_error.at<double>(0, 0)<=70 && _error.at<double>(0,0)>(-70)){
                        vx=_error.at<double>(0, 0)/200;
                    }else if(_error.at<double>(0, 0)<=160 && _error.at<double>(0,0)>(-160)){
                        vx=_error.at<double>(0, 0)/250;
                    }else{
                        vx=0.75;
                    }
                    //vr
                    if(_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                        vr=0;
                    }else if(_output.at<double>(3, 0)<=0.2 && _output.at<double>(3,0)>(-0.2)){
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.7;
                        else
                            vr=_output.at<double>(3, 0)/4.5;
                    }else if(_output.at<double>(3, 0)<=0.4 && _output.at<double>(3,0)>(-0.4)){
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.4;
                        else
                            vr=_output.at<double>(3, 0)/3.5;
                    }else if(_output.at<double>(3, 0)<=0.8 && _output.at<double>(3,0)>(-0.8)){
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.1;
                        else
                            vr=_output.at<double>(3, 0)/3;
                    }else{
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/0.8;
                        else
                            vr=_output.at<double>(3, 0)/2.3;
                    }
                }else if(ids[0]!=4 && ids[1]!=4 && key == -1 && tvecs.size() && done_id3==1 && detect_id4==0){//not yet detect id4, done id3
                    vr=-0.15;vx=0;vz=0;
                }else if(ids[i]==4 && key == -1 && tvecs.size() && done_id4==0 && done_id3==1){//detect id4
                    vz=0;
                    detect_id4=1;cout<<"detect_id4"<<endl;
                    Mat _error = Mat(4, 1, CV_64F);
                    _error.at<double>(0, 0)=tvecs[0][2]-80;
                    _error.at<double>(1, 0)=tvecs[0][0]-0;
                    _error.at<double>(2, 0)=tvecs[0][1]-0;
                    if(rvecs[0][0]<0)
                        _error.at<double>(3, 0)= -(rvecs[0][2]-(-0.0));
                    else
                        _error.at<double>(3, 0)= (rvecs[0][2]-(-0.0));
                    Mat _output;
                    PID.getCommand(_error, _output);
                    //vx
                    if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)&&_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                        done_id4=1;cout<<"done_id4"<<endl;
                        ardrone.setCamera(++mode % 4);
                    }
                    if(_error.at<double>(0, 0)<=10 && _error.at<double>(0,0)>(-10)){
                        vx=0;
                        //change camera
                    }else if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)){
                        vx=_error.at<double>(0, 0)/170;
                    }else if(_error.at<double>(0, 0)<=70 && _error.at<double>(0,0)>(-70)){
                        vx=_error.at<double>(0, 0)/200;
                    }else if(_error.at<double>(0, 0)<=160 && _error.at<double>(0,0)>(-160)){
                        vx=_error.at<double>(0, 0)/250;
                    }else{
                        vx=0.75;
                    }
                    //vr
                    if(_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                        vr=0;
                    }else if(_output.at<double>(3, 0)<=0.2 && _output.at<double>(3,0)>(-0.2)){
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.7;
                        else
                            vr=_output.at<double>(3, 0)/4.5;
                    }else if(_output.at<double>(3, 0)<=0.4 && _output.at<double>(3,0)>(-0.4)){
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.4;
                        else
                            vr=_output.at<double>(3, 0)/3.5;
                    }else if(_output.at<double>(3, 0)<=0.8 && _output.at<double>(3,0)>(-0.8)){
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/1.1;
                        else
                            vr=_output.at<double>(3, 0)/3;
                    }else{
                        if(_output.at<double>(3,0)>0)
                            vr=_output.at<double>(3,0)/0.8;
                        else
                            vr=_output.at<double>(3, 0)/2.3;
                    }
                }else if(ids[0]==5 && key == -1 && tvecs.size() && done_id4==1){//done id4, detect id5
                    cout<<"detect_id5, I'm landing."<<endl;
                    if (!ardrone.onGround()) ardrone.landing();
                }else{
                    vx=0;vy=0;vz=0;vr=-0.3; //因為所有地方都會貼marker，所以如果 see any other markers than id 1-5 就繼續轉
                }
            }
        }
        //cout<<"****"<<endl;
        //cout<<vr<<endl;
        ardrone.move3D(vx, vy, vz, vr);
        
        // Display the image
        cv::imshow("camera", image);
    }//while
    // See you
    ardrone.close();
    
    return 0;
}//main*/

#include <opencv2/highgui.hpp>  // final project
#include <opencv2/aruco.hpp>
#include <iostream>
#include "ardrone/ardrone.h"
#include "pid.hpp"
#include <cstring>
#include <algorithm>
#include <vector>

//#include <opencv2/core.hpp>
//#include <opencv2/videoio.hpp>
//#include <opencv2/objdetect.hpp>
//#include <opencv2/imgproc.hpp>

#include "opencv2/objdetect/objdetect.hpp"
//#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;
// --------------------------------------------------------------------------
// main(Number of arguments, Argument values)
// Description  : This is the entry point of the program.
// Return value : SUCCESS:0  ERROR:-1
// --------------------------------------------------------------------------

static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs) {
    FileStorage fs(filename, FileStorage::READ);
    if(!fs.isOpened())
        return false;
    fs["intrinsic"] >> camMatrix;
    fs["distortion"] >> distCoeffs;
    return true;
}


int main(int argc, char *argv[])
{
    //AR.Drone class
    PIDManager PID("pid.yaml");
    
    CascadeClassifier face_cascade;
    //face_cascade.load( "haarcascade_frontalface_alt.xml" );
    if( !face_cascade.load( "haarcascade_frontalface_alt.xml") ){ printf("--(!)Error loading\n"); return -1; };
    ARDrone ardrone;
    
    // Initialize
    if (!ardrone.open())
    {
        std::cout << "Failed to initialize." << std::endl;
        return -1;
    }
    
    
    // Battery
    std::cout << "Battery = " << ardrone.getBatteryPercentage() << "[%]" << std::endl;
    
    // Instructions
    std::cout << "***************************************" << std::endl;
    std::cout << "*       CV Drone sample program       *" << std::endl;
    std::cout << "*           - How to play -           *" << std::endl;
    std::cout << "***************************************" << std::endl;
    std::cout << "*                                     *" << std::endl;
    std::cout << "* - Controls -                        *" << std::endl;
    std::cout << "*    'Space' -- Takeoff/Landing       *" << std::endl;
    std::cout << "*    'Up'    -- Move forward          *" << std::endl;
    std::cout << "*    'Down'  -- Move backward         *" << std::endl;
    std::cout << "*    'Left'  -- Turn left             *" << std::endl;
    std::cout << "*    'Right' -- Turn right            *" << std::endl;
    std::cout << "*    'Q'     -- Move upward           *" << std::endl;
    std::cout << "*    'A'     -- Move downward         *" << std::endl;
    std::cout << "*                                     *" << std::endl;
    std::cout << "* - Others -                          *" << std::endl;
    std::cout << "*    'C'     -- Change camera         *" << std::endl;
    std::cout << "*    'Esc'   -- Exit                  *" << std::endl;
    std::cout << "*                                     *" << std::endl;
    std::cout << "***************************************" << std::endl;
    int done_id1 = 0;
    int done_id2 = 0;
    int done_id3 = 0;
    int done_id4 = 0;
    int done_id5 = 0;
    int done_id11=0;
    int done_id12=0;
    int done_id13=0;
    int cam = 0;//記錄鏡頭切換 0代表前鏡頭 1代表下鏡頭
    int detec = 0;
    int yahe=0;
    int detect_id1=0;
    int detect_id3=0;
    int detect_id4=0;
    int detect_id11=0;
    int detect_id12=0;
    int detect_id13=0;
    float markerLength = 9.4;
    Mat cameraMatrix, distCoeffs;
    readCameraParameters("camera.xml", cameraMatrix, distCoeffs);
    cout << cameraMatrix << endl;
    
    Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::DICT_6X6_250);
    int go_right = 0;
    
    while (1)
    {
        // Key input
        int key = cv::waitKey(100);
        if (key == 0x1b) break;
        
        // Get an image
        cv::Mat image = ardrone.getImage();
        //cv::Mat image;
        //cv::resize(image1, image, cv::Size(image1.cols*0.75, image1.rows*0.75));
        // Take off / Landing
        if (key == ' ')
        {
            if (ardrone.onGround()) ardrone.takeoff();
            else                    ardrone.landing();
        }
        
        // Move
        double vx = 0.0, vy = 0.0, vz = 0.0, vr = 0.0;
        if (key == 'i' || key == CV_VK_UP)    vx =  0.7;
        if (key == 'k' || key == CV_VK_DOWN)  vx = -0.7;
        if (key == 'u' || key == CV_VK_LEFT)  vr =  0.7;
        if (key == 'o' || key == CV_VK_RIGHT) vr = -0.7;
        if (key == 'j') vy =  0.7;
        if (key == 'l') vy = -0.7;
        if (key == 'q') vz =  0.7;
        if (key == 'a') vz = -0.7;
        
        // Change camera
        static int mode = 0;
        if (key == 'c') ardrone.setCamera(++mode % 4);
        Mat imageCopy;
        if(image.empty())
            continue;
        if(key==-1){
        image.copyTo(imageCopy);
        vector<int> ids; vector<vector<Point2f> > corners;
        aruco::detectMarkers(image, dictionary, corners, ids);
        vector<Vec3d> rvecs, tvecs;
        
        /*** this is new **********/
        
        // Container of faces
        vector<Rect> faces;
        // Detect faces
        
        face_cascade.detectMultiScale( image, faces, 1.1, 4.7, 0|CV_HAAR_SCALE_IMAGE, Size(10, 10) );
        //if no marker detected, turn to find marker
        
        if(detec == 1)//每十張做一次face detection
            detec = 0;
        else
            detec++;
        if(faces.size() > 0 && yahe==0)
        {
            // To draw rectangles around detected faces
            for (unsigned i = 0; i<faces.size(); i++)
            {
                if(detec !=0 ) break;
                rectangle(image,faces[i], Scalar(255, 255, 0), 2, 1);
                cout << faces[i] << endl;
                if(go_right == 1)
                { //開始避障，我們讓人臉保持在畫面的左方
                    if(faces[i].x > 60)
                    {          //大於30代表 人臉不在畫面左方
                        vx = 0;vy = -0.13;vz = 0;vr = 0; //就讓他往右邊飛
                        cout<<"moving vy"<<endl;
                    }
                    else if(faces[i].x<60)
                    {    //如果在畫面左方，就往前飛
                        yahe=1;
                        go_right=0;
                        cout<<"yahe==1"<<endl;
                        vx = 0.3;vy = 0;vz = 0;vr = 0;
                        //cout<<"go vx"<<endl;
                    }
                }
                else if(faces[i].width<65)
                { //越靠近人臉 faces[i].width 就越大，實測結果如果小於80 表示我們距離不夠近，往前飛
                    vx = 0.2;vy = 0;vz = 0;vr = 0;
                    cout<<"going forward"<<endl;
                }else
                {                      //如果大於80 就讓他開始避障
                    go_right = 1;
                    cout << "go_right" << endl;
                }
            }
            if (detec != 0) {
                vx = 0.1;vy = 0;vz = 0;vr = 0;
            }
            
        }
        else if (ids.size() <= 0)
        {
            if(yahe==1){
                cout<<"yahe == 1 flying straight"<<endl;
                vx=0.08;vy=0;vz=0;vr=0;
            }
            else if(done_id4 == 1 && done_id5 == 0 && cam == 1)//看不到marker切回marker4對準
            {
                ardrone.setCamera(++mode % 4);
                cam = 0;
                done_id4=0;
                cout << "看不到Marker5 切回鏡頭 重新校正id4" << endl;
            }
            else
            {
                vx = 0;vy = 0;vz = 0; vr=-0.01;//vr = -0.08;
                cout << "看不到Marker 自轉中..." << endl;
            }
        }
        else
        {
            aruco::drawDetectedMarkers(imageCopy, corners, ids);
            aruco::estimatePoseSingleMarkers(corners, markerLength, cameraMatrix, distCoeffs, rvecs, tvecs);
            for(int i = 0; i<ids.size(); i++)
            {
                cv::aruco::drawAxis(imageCopy, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.1);
            }
            for(int i = 0;i < ids.size();i++)
            {   cout<<"idsize:"<<ids.size()<<endl;
                cout<<"id:"<<ids[0]<<endl;
                if(key == -1 && tvecs.size()){
                    if(ids[i] == 11 && done_id11 == 0)//marker右邊60cm 前面80公分
                    {
                        detect_id11=1; cout<<"detect_id11"<<endl;
                        vx=0;vz=0;vy=0;
                        Mat _error = Mat(4, 1, CV_64F);
                        _error.at<double>(0, 0)=tvecs[0][2]-80;
                        _error.at<double>(1, 0)=tvecs[0][0]-0;
                        _error.at<double>(2, 0)=tvecs[0][1]-0;
                        if(rvecs[0][0]<0)
                            _error.at<double>(3, 0)= -(rvecs[0][2]-(-0.0));
                        else
                            _error.at<double>(3, 0)= (rvecs[0][2]-(-0.0));
                        //cout<<_error.at<double>(3,0)<<endl;
                        Mat _output;
                        PID.getCommand(_error, _output);
                        cout<<_error.at<double>(3, 0)<<"!!!!!"<<endl;
                        //cout<<_output.at<double>(3,0)<<endl;                     //vr =  parallized to marker id1
                        if(_error.at<double>(0, 0)<20 && _error.at<double>(0, 0)>(-20) && _output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                            done_id11=1;cout<<"done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1done_id1"<<endl;
                        }
                        if(_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){ //叫正差不多了 fly straight
                            vr=0; vx=0.8;
                            cout<<"movingtoid11"<<endl;
                        }else if(_output.at<double>(3, 0)<=0.4 && _output.at<double>(3,0)>(-0.4)){
                            cout<<"turning rf"<<endl;
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.5;
                            else
                                vr=_output.at<double>(3, 0)/3.6;
                        }else if(_output.at<double>(3, 0)<=0.8 && _output.at<double>(3,0)>(-0.8)){
                            cout<<"turning rf"<<endl;
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.2;
                            else
                                vr=_output.at<double>(3, 0)/3.1;
                        }else{
                            cout<<"turning rf"<<endl;
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/0.9;
                            else
                                vr=_output.at<double>(3, 0)/2.4;
                        }
                        
                    }else if(ids[i] == 2  && done_id2 == 0 && cam == 0)// Marker前面100公分
                    {
                        Mat _error = Mat(4, 1, CV_64F);
                        vz=0;
                        cout<<"inid2"<<endl;
                        _error.at<double>(0, 0)=tvecs[0][2]-100;
                        _error.at<double>(1, 0)=tvecs[0][0]-0;
                        _error.at<double>(2, 0)=tvecs[0][1]-0;
                        if(rvecs[0][0]<0)
                            _error.at<double>(3, 0)= -(rvecs[0][2]-(-0.0));
                        else
                            _error.at<double>(3, 0)= (rvecs[0][2]-(-0.0));
                        Mat _output;
                        PID.getCommand(_error, _output);
                        //cout<<"error vx: "<<_error.at<double>(0, 0)<<endl<<"error vy: "<<_error.at<double>(3,0)<<endl;
                        if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)&&_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                            done_id2=1;cout<<"done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2done_id2"<<endl;
                        }
                        if(_error.at<double>(0, 0)<=10 && _error.at<double>(0,0)>(-10)){
                            vx=0;
                        }else if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)){
                            vx=_error.at<double>(0, 0)/180;
                        }else if(_error.at<double>(0, 0)<=70 && _error.at<double>(0,0)>(-70)){
                            vx=_error.at<double>(0, 0)/210;
                            yahe =0;
                        }else if(_error.at<double>(0, 0)<=160 && _error.at<double>(0,0)>(-160)){
                            vx=_error.at<double>(0, 0)/260;
                        }else{
                            vx=0.74;
                        }
                        //vr
                        if(_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                            vr=0;
                        }else if(_output.at<double>(3, 0)<=0.2 && _output.at<double>(3,0)>(-0.2)){
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.8;
                            else
                                vr=_output.at<double>(3, 0)/4.6;
                        }else if(_output.at<double>(3, 0)<=0.4 && _output.at<double>(3,0)>(-0.4)){
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.5;
                            else
                                vr=_output.at<double>(3, 0)/3.6;
                        }else if(_output.at<double>(3, 0)<=0.8 && _output.at<double>(3,0)>(-0.8)){
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.2;
                            else
                                vr=_output.at<double>(3, 0)/3.1;
                        }else{
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/0.9;
                            else
                                vr=_output.at<double>(3, 0)/2.4;
                        }
                        vy=0;
                        //vy
                        /*if(_error.at<double>(1, 0)  <=  15 && _error.at<double>(1, 0) > (-15))
                        {
                            vy = 0;
                        }
                        else if(_error.at<double>(1, 0) <= 20 && _error.at<double>(1, 0) > (-20))
                        {
                            vy = _error.at<double>(1, 0)/500;
                        }
                        else if(_error.at<double>(1, 0) <= 70 && _error.at<double>(1, 0) > (-70))
                        {
                            vy = _error.at<double>(1, 0)/800;
                        }
                        else if(_error.at<double>(1, 0) <= 160 && _error.at<double>(1, 0) > (-160))
                        {
                            vy = _error.at<double>(1, 0)/1000;
                        }
                        else{
                            vy = 0.75;
                        }*/
                        
                        
                    }else if(ids[i] == 3 && done_id2 == 1 && done_id3 == 0 && cam == 0)// Marker前面100公分
                        {
                            detect_id3=1;cout<<"detect_id3"<<endl;
                            vz=0;
                            Mat _error = Mat(4, 1, CV_64F);
                            _error.at<double>(0, 0)=tvecs[0][2]-80;
                            _error.at<double>(1, 0)=tvecs[0][0]-0;
                            _error.at<double>(2, 0)=tvecs[0][1]-0;
                            if(rvecs[0][0]<0)
                                _error.at<double>(3, 0)= -(rvecs[0][2]-(-0.0));
                            else
                                _error.at<double>(3, 0)= (rvecs[0][2]-(-0.0));
                            Mat _output;
                            PID.getCommand(_error, _output);
                            //vx
                            if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)&&_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                                done_id3=1;cout<<"done_id3"<<endl;
                            }
                            if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)){
                                vx=0;
                            }else if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)){
                                vx=_error.at<double>(0, 0)/175;
                            }else if(_error.at<double>(0, 0)<=70 && _error.at<double>(0,0)>(-70)){
                                vx=_error.at<double>(0, 0)/210;
                            }else if(_error.at<double>(0, 0)<=160 && _error.at<double>(0,0)>(-160)){
                                vx=_error.at<double>(0, 0)/260;
                            }else{
                                vx=0.7;
                            }
                            //vr
                            if(_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                                vr=0;
                            }else if(_output.at<double>(3, 0)<=0.2 && _output.at<double>(3,0)>(-0.2)){
                                if(_output.at<double>(3,0)>0)
                                    vr=_output.at<double>(3,0)/1.8;
                                else
                                    vr=_output.at<double>(3, 0)/4.6;
                            }else if(_output.at<double>(3, 0)<=0.4 && _output.at<double>(3,0)>(-0.4)){
                                if(_output.at<double>(3,0)>0)
                                    vr=_output.at<double>(3,0)/1.5;
                                else
                                    vr=_output.at<double>(3, 0)/3.6;
                            }else if(_output.at<double>(3, 0)<=0.8 && _output.at<double>(3,0)>(-0.8)){
                                if(_output.at<double>(3,0)>0)
                                    vr=_output.at<double>(3,0)/1.2;
                                else
                                    vr=_output.at<double>(3, 0)/3.1;
                            }else{
                                if(_output.at<double>(3,0)>0)
                                    vr=_output.at<double>(3,0)/0.9;
                                else
                                    vr=_output.at<double>(3, 0)/2.4;
                            }
                        
                        //vy 原本沒有，如果不好，把他刪掉 記得補vy=0;
                        if(_error.at<double>(1, 0)  <=  15 && _error.at<double>(1, 0) > (-15))
                        {
                            vy = 0;
                        }
                        else if(_error.at<double>(1, 0) <= 20 && _error.at<double>(1, 0) > (-20))
                        {
                            vy = _error.at<double>(1, 0)/180;
                        }
                        else if(_error.at<double>(1, 0) <= 70 && _error.at<double>(1, 0) > (-70))
                        {
                            vy = _error.at<double>(1, 0)/210;
                        }
                        else if(_error.at<double>(1, 0) <= 160 && _error.at<double>(1, 0) > (-160))
                        {
                            vy = _error.at<double>(1, 0)/260;
                        }
                        else{
                            vy = 0.7;
                        }
                    }else if(ids[i] == 4 && done_id3 == 1 && done_id4 == 0 && cam == 0)// Marker前面100公分切換鏡頭
                    {
                            vz=0;
                            detect_id4=1;cout<<"detect_id4"<<endl;
                            Mat _error = Mat(4, 1, CV_64F);
                            _error.at<double>(0, 0)=tvecs[0][2]-80;
                            _error.at<double>(1, 0)=tvecs[0][0]-0;
                            _error.at<double>(2, 0)=tvecs[0][1]-0;
                            if(rvecs[0][0]<0)
                                _error.at<double>(3, 0)= -(rvecs[0][2]-(-0.0));
                            else
                                _error.at<double>(3, 0)= (rvecs[0][2]-(-0.0));
                            Mat _output;
                            PID.getCommand(_error, _output);
                            //vx
                            if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)&&_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                                done_id4=1;cout<<"done_id4"<<endl;
                                cam == 1;
                                ardrone.setCamera(++mode % 4);
                            }
                            if(_error.at<double>(0, 0)<=10 && _error.at<double>(0,0)>(-10)){
                                vx=0;
                                //change camera
                            }else if(_error.at<double>(0, 0)<=20 && _error.at<double>(0,0)>(-20)){
                                vx=_error.at<double>(0, 0)/175;
                            }else if(_error.at<double>(0, 0)<=70 && _error.at<double>(0,0)>(-70)){
                                vx=_error.at<double>(0, 0)/205;
                            }else if(_error.at<double>(0, 0)<=160 && _error.at<double>(0,0)>(-160)){
                                vx=_error.at<double>(0, 0)/255;
                            }else{
                                vx=0.7;
                            }
                            //vr
                            if(_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                                vr=0;
                            }else if(_output.at<double>(3, 0)<=0.2 && _output.at<double>(3,0)>(-0.2)){
                                if(_output.at<double>(3,0)>0)
                                    vr=_output.at<double>(3,0)/1.8;
                                else
                                    vr=_output.at<double>(3, 0)/4.6;
                            }else if(_output.at<double>(3, 0)<=0.4 && _output.at<double>(3,0)>(-0.4)){
                                if(_output.at<double>(3,0)>0)
                                    vr=_output.at<double>(3,0)/1.5;
                                else
                                    vr=_output.at<double>(3, 0)/3.6;
                            }else if(_output.at<double>(3, 0)<=0.8 && _output.at<double>(3,0)>(-0.8)){
                                if(_output.at<double>(3,0)>0)
                                    vr=_output.at<double>(3,0)/1.2;
                                else
                                    vr=_output.at<double>(3, 0)/3.1;
                            }else{
                                if(_output.at<double>(3,0)>0)
                                    vr=_output.at<double>(3,0)/0.9;
                                else
                                    vr=_output.at<double>(3, 0)/2.4;
                            }
                    
                        //vy 原本沒有 若不好刪掉 加上 vy=0;
                        if(_error.at<double>(1, 0)  <=  15 && _error.at<double>(1, 0) > (-15))
                        {
                            vy = 0;
                        }
                        else if(_error.at<double>(1, 0) <= 20 && _error.at<double>(1, 0) > (-20))
                        {
                            vy = _error.at<double>(1, 0)/175;
                        }
                        else if(_error.at<double>(1, 0) <= 70 && _error.at<double>(1, 0) > (-70))
                        {
                            vy = _error.at<double>(1, 0)/205;
                        }
                        else if(_error.at<double>(1, 0) <= 160 && _error.at<double>(1, 0) > (-160))
                        {
                            vy = _error.at<double>(1, 0)/255;
                        }
                        else{
                            vy = 0.7;
                        }
                    }
                    else if(ids[i] == 5 && done_id4 == 1 && done_id5 == 0 && cam == 1)//對準marker直接landing
                    {
                        
                         cout<<"detect_id5, I'm landing."<<endl;
                         if (!ardrone.onGround()) ardrone.landing();
                         
                        
                        
                        /*Mat _error = Mat(4, 1, CV_64F);
                        _error.at<double>(0, 0) = tvecs[0][2] - 0;
                        _error.at<double>(1, 0) = tvecs[0][0] - 0;
                        _error.at<double>(2, 0) = tvecs[0][1] - 80;
                        if(rvecs[0][0]<0)
                            _error.at<double>(3, 0) = -(rvecs[0][2] - (-0.0));
                        else
                            _error.at<double>(3, 0) = (rvecs[0][2] - (-0.0));
                        
                        Mat _output;
                        PID.getCommand(_error, _output);
                        
                        if(_error.at<double>(0, 0) < 2 && _error.at<double>(0, 0) > (-2) && _error.at<double>(1, 0) < 2 && _error.at<double>(1, 0) > (-2)
                           && _output.at<double>(3, 0) <= 0.1 && _output.at<double>(3, 0) > (-0.1))//vx vy vr 對準
                        {
                            ardrone.landing();
                            done_id5 = 1;
                        }
                        
                        //vx
                        if(_error.at<double>(0, 0) <= 5 && _error.at<double>(0, 0) > (-5))
                        {
                            vx = 0;
                        }
                        else if(_error.at<double>(0, 0) <= 20 && _error.at<double>(0, 0) > (-20))
                        {
                            vx = _error.at<double>(0, 0)/170;
                        }
                        else if(_error.at<double>(0, 0) <= 70 && _error.at<double>(0, 0) > (-70))
                        {
                            vx = _error.at<double>(0, 0)/200;
                        }
                        else if(_error.at<double>(0, 0) <= 160 && _error.at<double>(0, 0) > (-160))
                        {
                            vx = _error.at<double>(0, 0)/250;
                        }
                        else{
                            vx = 0.75;
                        }
                        
                        //vr
                        if(_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                            vr=0;
                        }else if(_output.at<double>(3, 0)<=0.2 && _output.at<double>(3,0)>(-0.2)){
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.7;
                            else
                                vr=_output.at<double>(3, 0)/4.5;
                        }else if(_output.at<double>(3, 0)<=0.4 && _output.at<double>(3,0)>(-0.4)){
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.4;
                            else
                                vr=_output.at<double>(3, 0)/3.5;
                        }else if(_output.at<double>(3, 0)<=0.8 && _output.at<double>(3,0)>(-0.8)){
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.1;
                            else
                                vr=_output.at<double>(3, 0)/3;
                        }else{
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/0.8;
                            else
                                vr=_output.at<double>(3, 0)/2.3;
                        }
                        
                        //vy
                        if(_error.at<double>(1, 0)  <=  10 && _error.at<double>(1, 0) > (-10))
                        {
                            vy = 0;
                        }
                        else if(_error.at<double>(1, 0) <= 20 && _error.at<double>(1, 0) > (-20))
                        {
                            vy = _error.at<double>(1, 0)/170;
                        }
                        else if(_error.at<double>(1, 0) <= 70 && _error.at<double>(1, 0) > (-70))
                        {
                            vy = _error.at<double>(1, 0)/200;
                        }
                        else if(_error.at<double>(1, 0) <= 160 && _error.at<double>(1, 0) > (-160))
                        {
                            vy = _error.at<double>(1, 0)/250;
                        }
                        else{
                            vy = 0.75;
                        }*/
                    }else{
                        cout << "看到其他的maker " << endl;
                        vx = 0; vy = 0; vz = 0; vr=-0.15;//vr = -0.15;
                        //自轉或是下面的方式 但是怕id2 to 3 看到marker 結果錯了地方
                        /*Mat _error = Mat(4, 1, CV_64F);
                        vz=0;
                        cout<<"inid2"<<endl;
                        _error.at<double>(0, 0)=tvecs[0][2]-100;
                        _error.at<double>(1, 0)=tvecs[0][0]-0;
                        _error.at<double>(2, 0)=tvecs[0][1]-0;
                        if(rvecs[0][0]<0)
                            _error.at<double>(3, 0)= -(rvecs[0][2]-(-0.0)); //如果marker都是貼著牆壁，那基準要設成 90度
                        else
                            _error.at<double>(3, 0)= (rvecs[0][2]-(-0.0));
                        Mat _output;
                        PID.getCommand(_error, _output);
                        vx=0;
                        //vr
                        if(_output.at<double>(3, 0)<=0.3 && _output.at<double>(3,0)>(-0.3)){
                            vr=0;
                        }else if(_output.at<double>(3, 0)<=0.2 && _output.at<double>(3,0)>(-0.2)){
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.7;
                            else
                                vr=_output.at<double>(3, 0)/4.5;
                        }else if(_output.at<double>(3, 0)<=0.4 && _output.at<double>(3,0)>(-0.4)){
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.4;
                            else
                                vr=_output.at<double>(3, 0)/3.5;
                        }else if(_output.at<double>(3, 0)<=0.8 && _output.at<double>(3,0)>(-0.8)){
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/1.1;
                            else
                                vr=_output.at<double>(3, 0)/3;
                        }else{
                            if(_output.at<double>(3,0)>0)
                                vr=_output.at<double>(3,0)/0.8;
                            else
                                vr=_output.at<double>(3, 0)/2.3;
                        }
                        vy=0;*/
                    }
                }
                //當切換鏡頭後看不到marker5 切回去找marker4(寫在轉圈那邊) 不升高找是怕偏離太多 會失控
            } //for
            
        } //有看到任何marker的
        }//if key==-1
        ardrone.move3D(vx, vy, vz, vr);
        
        // Display the image
        cv::imshow("camera", image);
    }//while
    // See you
    ardrone.close();
    
    return 0;
}//main*/

