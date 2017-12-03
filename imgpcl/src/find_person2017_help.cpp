//找到一个人并走到人跟前。通过人脸的位置在图像中的高处，认为到了人跟前。   
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
#include <stdlib.h>
int system(const char *string); 
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
void image_process(Mat inImg); 
void spCallback(const std_msgs::String::ConstPtr& msg); 
void forward_robot(float x);
void turn_robot(float theta);
/// 全局变量

const char* window_name = "gender recognise";
String face_cascade_name = "~/catkin_ws/src/imgpcl/haarcascade_frontalface_alt.xml";  
//String eyes_cascade_name = "/home/isi/2017_ws/src/imgpcl/haarcascade_eye_tree_eyeglasses.xml";  
CascadeClassifier face_cascade;

ros::Publisher move_pub; //向navigation发消息，微调机器人
ros::Publisher iffind_person_Pub;   //检测到物体的数量
ros::Publisher image2voice_Pub;

std_msgs::String img2voiceP;
//ros::Publisher iffind_person;
std_msgs::String iffind_personP;
int state = -1;
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
    if(state == -1 || state == 2)
        return;
    if( inImg.empty() )    
    {    
        ROS_INFO("Camera image empty");    
        return;//break;    
    } 
    if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); };  
    //if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading\n"); };
    std::vector<Rect> faces;  
    Mat inImg_gray;  
    cvtColor( inImg, inImg_gray, CV_BGR2GRAY );  
    equalizeHist( inImg_gray, inImg_gray );  
    //-- Detect faces  
    face_cascade.detectMultiScale( inImg_gray, faces, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );  
    int human_num = faces.size();
    cout<<"human_num:"<<human_num<<endl;
    if(human_num == 0)
    {
        //img2voiceP.data = "I find no people, I am finding";
        //image2voice_Pub.publish(img2voiceP);//发送此时检测到的物体数量
        turn_robot(1.5);
    }
    if(human_num >= 1)
    {   
        img2voiceP.data = "I am going to the people";
        image2voice_Pub.publish(img2voiceP);
        Mat face = inImg_gray( faces[0] ); 
        Mat face_resized;
        rectangle(inImg,faces[0],Scalar(0,255,0),1);
        //imwrite("/home/isi/2017_ws/src/imgpcl/person.jpg",inImg);
        //int pos_x = 0;
        //int pos_y = 0;
        //pos_x = faces[0].x + faces[0].width ;
        //pos_y = faces[0].y + faces[0].height;
        //cout<<"pos_x"<<pos_x<<endl;
        //cout<<"pos_y"<<pos_y<<endl;
        //if(pos_x - 320 > 80)
         //   turn_robot(-0.8);
        //if(pos_x - 320 < -80)
        //    turn_robot(0.8);
        //if(pos_y > 140)
        //    forward_robot(0.2);
        //if(abs(pos_x - 320) < 80 && pos_y < 140)
	    //{
	cout<<"OK"<<endl;
        img2voiceP.data = "I have reached the person!";
        image2voice_Pub.publish(img2voiceP); 
        iffind_personP.data = "found_person";
        iffind_person_Pub.publish(iffind_personP);                       
        state = 2;		
	    //}
    }
	imwrite("~/catkin_ws/src/imgpcl/person.jpg",inImg);//保存图片    
    imshow( window_name, inImg );
    waitKey(5); 
} 
void turn_robot(float theta)
{
    geometry_msgs::Twist vel;
    int count = 0;
    float time = 2;
    ros::Rate loop_rate(10);
    int num = time*10;
    PathPoint pt;
    //转90度
    //float theta = 5;
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
    pt.state = 0;
    pt.time = time;
    pt.vel = vel.angular.z;
    vppt.push_back(pt);
    vel.angular.z = 0.0;
    move_pub.publish(vel);
    cout<<"转弯\n";
    sleep(1);
}
void forward_robot(float x)
{
    geometry_msgs::Twist vel;
    int count = 0;
    float time = 4;
    vel.linear.x = x/time; 
    ros::Rate loop_rate(10);
    int num = time*10;
    //cout<<"armCallback1()\n";
    while(count < num)
    {
        count++;
        move_pub.publish(vel);
        loop_rate.sleep();
    }
    PathPoint pt;
    pt.state = 0;
    pt.time = time;
    pt.vel = vel.linear.x;
    vppt.push_back(pt);
    vel.linear.x = 0.0;
    move_pub.publish(vel);
    cout<<"直行\n";
    sleep(1);
}
void findPersonCallback(const std_msgs::String::ConstPtr& msg)
{
    ROS_INFO("findPersonCallback received: %s\n", msg->data.c_str());
    if(msg->data == "release")   //接受到navigation的消息，开始检测
    {
        sleep(12);
        state = 0;
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
    iffind_person_Pub = nh.advertise<std_msgs::String>("found_person", 1);
    image2voice_Pub = nh.advertise<std_msgs::String>("img2voice", 1);
    image_transport::ImageTransport it(nh); 
    image_transport::Subscriber sub = it.subscribe("camera1/rgb/image_raw", 1, imageCallback);    
   // ros::Subscriber camInfo         = nh.subscribe("camera/rgb/camera_info", 1, camInfoCallback);    
    ros::Subscriber arm_sub = nh.subscribe("/nav2image", 10, findPersonCallback);        //订阅navigation的消息
    ros::spin();
    //ROS_INFO is the replacement for printf/cout.    
    ROS_INFO("main_No error.");  
}
