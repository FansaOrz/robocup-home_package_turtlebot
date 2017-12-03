/***********************************************************
Author: Hu Ziqi
Date: 11/5/2017
Abstract: Code for speech and recognition task 
************************************************************/
//标准头文件
#include<ros/ros.h>
#include<iostream>
#include<std_msgs/String.h>
#include<string.h>
//navigation中需要使用的位姿信息头文件
#include<geometry_msgs/Pose.h>
#include<geometry_msgs/Point.h>
#include<geometry_msgs/PoseWithCovariance.h>
#include<geometry_msgs/PoseWithCovarianceStamped.h>
#include<geometry_msgs/Twist.h>
#include<geometry_msgs/Quaternion.h>
//move_base头文件
#include<move_base_msgs/MoveBaseGoal.h>
#include<move_base_msgs/MoveBaseAction.h>
//actionlib头文件
#include<actionlib/client/simple_action_client.h>
#include<stdlib.h>
#include<cstdlib>
using namespace std;
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient; //简化类型书写为MoveBaseClient

geometry_msgs::Twist vel; //控制机器人速度
std_msgs::String sound_flag; //语音控制标志
std_msgs::String send_flag; 
geometry_msgs::PoseWithCovariance  car_pose;
geometry_msgs::Pose goal_pose;//目标位置

//定义不同房间位置的坐标点
geometry_msgs::Pose livingroom;
geometry_msgs::Pose kitchen;
geometry_msgs::Pose bedroom;
geometry_msgs::Pose entrance;
geometry_msgs::Pose balcony;
geometry_msgs::Pose start;
geometry_msgs::Pose door;

ros::Subscriber speech_sub;//接受speech和recognition的退出区域的指令

void speechCallback(const std_msgs::String::ConstPtr& msg);
void speechCallback(const std_msgs::String::ConstPtr& msg)
{
	ROS_INFO("RECEIVED!");
	if(msg->data == "area_leave")
	{
ROS_INFO("!!!!!!!!");
		move_base_msgs::MoveBaseGoal naviGoal1; //导航目标点
		naviGoal1.target_pose.header.frame_id = "map"; 
		naviGoal1.target_pose.header.stamp = ros::Time::now();
		naviGoal1.target_pose.pose = geometry_msgs::Pose(livingroom);
	}
}

void initplace()
{
  start.position.x = 1.8315198099;
  start.position.y = -2.70883212347;
  start.position.z = 0;
  start.orientation.x = 0;
  start.orientation.y = 0;
  start.orientation.z =  -0.993983728736;
  start.orientation.w = 0.10952783668;
  
  door.position.x = -4.51509063669;
  door.position.y = -1.63136150946;
  door.position.z = 0;
  door.orientation.x = 0;
  door.orientation.y = 0;
  door.orientation.z = -0.378580264205;
  door.orientation.w = 0.925568465082;
  
  livingroom.position.x = 10.4924;
  livingroom.position.y = 2.84849;
  livingroom.position.z = 0;
  livingroom.orientation.x = 0;
  livingroom.orientation.y = 0;
  livingroom.orientation.z =  0.963997;
  livingroom.orientation.w = 0.265912;

  kitchen.position.x = 1.77283289516;
  kitchen.position.y = -4.05372810971;
  kitchen.position.z = 0;
  kitchen.orientation.x = 0;
  kitchen.orientation.y = 0;
  kitchen.orientation.z =  -0.636697410912;
  kitchen.orientation.w = 0.771113744487;

  bedroom.position.x = -2.21886371669;
  bedroom.position.y = -5.29413074921;
  bedroom.position.z = 0;
  bedroom.orientation.x = 0;
  bedroom.orientation.y = 0;
  bedroom.orientation.z = -0.650948613053;
  bedroom.orientation.w = 0.759121797319;

  entrance.position.x = -3.29376092612;
  entrance.position.y = -2.52392025721;
  entrance.position.z = 0;
  entrance.orientation.x = 0;
  entrance.orientation.y = 0;
  entrance.orientation.z = -0.379128067642;
  entrance.orientation.w = 0.925344210727;

  balcony.position.x = 2.98617397856;
  balcony.position.y = -1.49467780406;
  balcony.position.z = 0;
  balcony.orientation.x = 0;
  balcony.orientation.y = 0;
  balcony.orientation.z = 0.492294159753;
  balcony.orientation.w = 0.870428894438;
}


int main(int argc, char** argv)
{
	ros::init(argc, argv, "navi_demo");
	ros::NodeHandle myNode;
	initplace();
	cout<<"Welcome to Help-me-carry!"<<endl;
	MoveBaseClient  mc_("move_base", true); //建立导航客户端
	ros::Subscriber speech_sub = myNode.subscribe("voice_speech_nav", 1, speechCallback);
	ros::spinOnce();
	return 0;
}

