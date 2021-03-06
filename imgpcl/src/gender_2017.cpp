//Includes all the headers necessary to use the most common public pieces of the ROS system.    
#include <ros/ros.h>    
//Use image_transport for publishing and subscribing to images in ROS    
#include <image_transport/image_transport.h>    
//Use cv_bridge to convert between ROS and OpenCV Image formats    
#include <cv_bridge/cv_bridge.h>    

#include <sensor_msgs/image_encodings.h>    
//Include headers for OpenCV Image processing    
#include <opencv2/imgproc/imgproc.hpp>    
//Include headers for OpenCV GUI handling    
#include <opencv2/highgui/highgui.hpp>    
#include <string>        
#include <sstream>    
#include <stdio.h>
#include "imgpcl/auto_tchar.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <math.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>
#include <vector>

using namespace cv;    
using namespace std;    

//Store all constants for image encodings in the enc namespace to be used later.      
namespace enc = sensor_msgs::image_encodings; 
struct PathPoint
{
    int state;
    float vel;
    float time;
};     
vector<PathPoint> vppt;             //储存反馈调节时的调节参数
bool ifcapimg = 0;
Mat capimg;       
const int Width = 92;
const int Height = 112;
//int gender_width;
//int gender_height;
int im_width;
int im_height;
int g_howManyPhotoForTraining = 260;
//
int g_photoNumberOfOnePerson = 279;
//ORL
void image_process(Mat inImg); 
void gender_predict( Mat frame ); 
void spCallback(const std_msgs::String::ConstPtr& msg); 
/// 全局变量
Mat src, dst, tmp;
const char* window_name = "gender recognise";
String face_cascade_name = "~/catkin_ws/src/imgpcl/haarcascade_frontalface_alt.xml";  
//String eyes_cascade_name = "/home/isi/2017_ws/src/imgpcl/haarcascade_eye_tree_eyeglasses.xml";  
CascadeClassifier face_cascade;
//CascadeClassifier eyes_cascade;
//string window_name = "Capture - Face detection";
RNG rng(12345);
static int image_num = 1;
ros::Publisher move_pub;            //向navigation发消息，微调机器人
ros::Publisher female_numPub;   //检测到物体的数量
ros::Publisher male_numPub;   //检测到物体的数量
ros::Publisher human_numPub;   //检测到物体的数量
ros::Publisher taking_photo_Pub;   //给语音发消息，将要拍照片

std_msgs::String photo_signal;
std_msgs::String female_numP;
std_msgs::String male_numP;
std_msgs::String human_numP;
int female_num = 0;
int male_num = 0;
int human_num = 0;

void gender_predict( Mat frame )  
{  
    photo_signal.data="over";
    taking_photo_Pub.publish(photo_signal);//向语音发布消息，提醒人们要拍照了
    sleep(1);
    std::vector<Rect> faces;  
    Mat frame_gray;  
    cvtColor( frame, frame_gray, CV_BGR2GRAY );  
    equalizeHist( frame_gray, frame_gray );  
    //-- Detect faces  
    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );  
    //   参数1：image--待检测图片，一般为灰度图像加快检测速度；
    // 参数2：objects--被检测物体的矩形框向量组；
    // 参数3：scaleFactor--表示在前后两次相继的扫描中，搜索窗口的比例系数。默认为1.1即每次搜索窗口依次扩大10%;
    // 参数4：minNeighbors--表示构成检测目标的相邻矩形的最小个数(默认为3个)。
    //         如果组成检测目标的小矩形的个数和小于 min_neighbors - 1 都会被排除。
    //         如果min_neighbors 为 0, 则函数不做任何操作就返回所有的被检候选矩形框，
    //         这种设定值一般用在用户自定义对检测结果的组合程序上；
    // 参数5：flags--要么使用默认值，要么使用CV_HAAR_DO_CANNY_PRUNING，如果设置为
    //         CV_HAAR_DO_CANNY_PRUNING，那么函数将会使用Canny边缘检测来排除边缘过多或过少的区域，
    //         因此这些区域通常不会是人脸所在区域；
    // 参数6、7：minSize和maxSize用来限制得到的目标区域的范围。
    human_num = faces.size();
    cout<<"human_num:"<<human_num<<endl;
    for( size_t i = 0; i < faces.size(); i++ )  
    {   
    
       // Rect face_id = faces[i];
        //Mat dectImg;
        Mat face = frame_gray( faces[i] ); 
        Mat face_resized;
        im_width = Width;
        im_height = Height;
        cv::resize(face, face_resized, Size(im_width, im_height), 1.0, 1.0, INTER_CUBIC);

        rectangle(frame,faces[i],Scalar(0,255,0),1);
        Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
        model->load("~/catkin_ws/src/imgpcl/eigenfacepca.yml");
        string box_text;
        box_text = format( "" );
        if(model->predict(face_resized) == 0)
        {
            cout<<"woman"<<endl;
            box_text.append( "Female" );
            female_num ++;
        }
        else
        {
            cout<<"man"<<endl;
            box_text.append( "Male" );
            male_num ++;
        }
        int pos_x = faces[i].x + faces[i].width * 0.1;
        int pos_y = faces[i].y + faces[i].height + 10;

        // And now put it into the image:
         putText(frame, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 1.0);

    }
    cout<<"female_num"<<female_num<<endl;
    imshow( window_name, frame );
    imwrite("~/catkin_ws/src/imgpcl/gender_detect.jpg",frame);//保存图片
        if(female_num == 0)
            female_numP.data = "zero";
        if(female_num == 1)
            female_numP.data = "one";
        if(female_num == 2)
            female_numP.data = "two";
        if(female_num == 3)
            female_numP.data = "three";
        if(female_num == 4)
            female_numP.data = "four";
        if(female_num == 5)
            female_numP.data = "five";
        if(female_num == 6)
            female_numP.data = "six";
        if(female_num == 7)
            female_numP.data = "seven";
        if(female_num == 8)
            female_numP.data = "eight";
        if(female_num == 9)
            female_numP.data = "nine";
        if(female_num == 10)
            female_numP.data = "ten";

        if(male_num == 0)
            male_numP.data = "zero";
        if(male_num == 1)
            male_numP.data = "one";
        if(male_num == 2)
            male_numP.data = "two";
        if(male_num == 3)
            male_numP.data = "three";
        if(male_num == 4)
            male_numP.data = "four";
        if(male_num == 5)
           male_numP.data = "five";
        if(male_num == 6)
            male_numP.data = "six";
        if(male_num == 7)
            male_numP.data = "seven";
        if(male_num == 8)
            male_numP.data = "eight";
        if(male_num == 9)
            male_numP.data = "nine";
        if(male_num == 10)
            male_numP.data = "ten";

        if(human_num == 0)
             human_numP.data = "zero";
        if(human_num == 1)
             human_numP.data = "one";
        if(human_num == 2)
             human_numP.data = "two";
        if(human_num == 3)
             human_numP.data = "three";
        if(human_num == 4)
             human_numP.data = "four";
        if(human_num == 5)
             human_numP.data = "five";
        if(human_num == 6)
             human_numP.data = "six";
        if(human_num == 7)
             human_numP.data = "seven";
        if(human_num == 8)
             human_numP.data = "eight";
        if(human_num == 9)
             human_numP.data = "nine";
        if(human_num == 10)
             human_numP.data = "ten";
        human_numPub.publish(human_numP);//发送此时检测到的物体数量
        sleep(3);
        female_numPub.publish(female_numP);//发送此时检测到的物体数量
        sleep(3);
        male_numPub.publish(male_numP);//发送此时检测到的物体数量 
}  
//This function is called everytime a new image is published    
void imageCallback(const sensor_msgs::ImageConstPtr& original_image)    
{    
    //Convert from the ROS image message to a CvImage suitable for working with OpenCV for processing    
    cv_bridge::CvImagePtr cv_ptr;      
    try      
    {      
        //Always copy, returning a mutable CvImage      
        //OpenCV expects color images to use BGR channel order.      
        cv_ptr = cv_bridge::toCvCopy(original_image, enc::BGR8);      
    }      
    catch (cv_bridge::Exception& e)      
    {
        //if there is an error during conversion, display it      
        ROS_ERROR("tutorialROSOpenCV::main.cpp::cv_bridge exception: %s", e.what());      
        return;      
    }      
    image_process(cv_ptr->image);    
}    
    
void image_process(Mat inImg)    
{    
    
    if( inImg.empty() )    
    {    
        ROS_INFO("Camera image empty");    
        return;//break;    
    }     
    imshow("image_process", inImg);//显示图片    
    stringstream sss;        
    string strs;    
    static int image_num = 1;    
    waitKey(1);    
    if( ifcapimg )    
    {   
        //resize(inImg,capimg,Size(imgWidth/6,imgHeight/6),0,0,CV_INTER_LINEAR);      
        capimg=inImg(Rect(0,0,inImg.cols,inImg.rows));    
        imwrite("~/catkin_ws/src/imgpcl/capimg.jpg",capimg);//保存图片  
        cout<<image_num<<"images have saved!"<<endl;  
        if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); };  
        //if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading\n"); };  
        ///// 指示说明
        printf( "\n Zoom In-Out demo  \n " );
        printf( "------------------ \n" );
        printf( " * [u] -> Zoom in  \n" );
        printf( " * [d] -> Zoom out \n" );
        printf( " * [ESC] -> Close program \n \n" );

        /// 测试图像 - 尺寸必须能被 2^{n} 整除
        src = imread( "~/catkin_ws/src/imgpcl/capimg.jpg" );
        if( !src.data )
        {
            printf(" No data! -- Exiting the program \n");
        }
        gender_predict( src ); 
        ifcapimg = 0;
    }
} 
void spCallback(const std_msgs::String::ConstPtr& msg)
{
    ROS_INFO("spCallback received: %s\n", msg->data.c_str());
    if(msg->data == "turn_robot")
    {
        cout<<"received!!!!!\n";
        sleep(10);
        geometry_msgs::Twist vel;
        int count = 0;
        float time = 4.5;
        ros::Rate loop_rate(10);
        int num = time*10;
        PathPoint pt;
        //转180度
        float theta = 4;
		//cout<<"theta:"<<theta<<endl;
		float theta2 = theta/time;
		vel.angular.z = theta2;    

        count = 0;
        num = time*10;
        while(count < num)
        {
            count++;
            move_pub.publish(vel);
            loop_rate.sleep();
        }
        //pt.state = 0;
        //pt.time = time;
        //pt.vel = vel.angular.z;
        //vppt.push_back(pt);
        //vel.angular.z = 0.0;
        //move_pub.publish(vel);
        cout<<"转弯\n";
        sleep(2);
        ifcapimg = 1;
	photo_signal.data="start";
	taking_photo_Pub.publish(photo_signal);//向语音发布消息，提醒人们要拍照了
	sleep(5);
    }
}
/** 
* This is ROS node to track the destination image 
*/    
int main(int argc, char **argv)    
{    
    ros::init(argc, argv, "image_process");    
    ROS_INFO("-----------------");    
        
    ros::NodeHandle nh;
    move_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel_mux/input/navi",1);  //移动
    female_numPub = nh.advertise<std_msgs::String>("female_num", 1);//检测到的物体数量
    male_numPub = nh.advertise<std_msgs::String>("male_num", 1);//检测到的物体数量
    human_numPub = nh.advertise<std_msgs::String>("human_num", 1);//检测到的物体数量
    taking_photo_Pub = nh.advertise<std_msgs::String>("taking_photo_signal", 1);//即将拍照片的信号
    image_transport::ImageTransport it(nh);    
    
    image_transport::Subscriber sub = it.subscribe("camera/rgb/image_raw", 1, imageCallback);    
   // ros::Subscriber camInfo         = nh.subscribe("camera/rgb/camera_info", 1, camInfoCallback);    
    ros::Subscriber sp_sub = nh.subscribe("turn_signal", 1, spCallback);  //订阅voice的消息
    ROS_INFO("===============");
    ros::spin();    
    
    //ROS_INFO is the replacement for printf/cout.    
    ROS_INFO("tutorialROSOpenCV::main.cpp::No error.");    
}
