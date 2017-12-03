#include <ros/ros.h>
#include <std_msgs/String.h>
#include <kobuki_msgs/MotorPower.h>
#include <string.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <kobuki_msgs/DigitalInputEvent.h>
#include <move_base_msgs/MoveBaseGoal.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <cmath>
const double pi = 3.141592653;
using namespace std;
bool estop_in;
ros::Subscriber emergency_sub;
ros::Publisher motor_power_pub;
ros::Publisher move_base_cancel_pub;
void DigitalInputEventCallback(const kobuki_msgs::DigitalInputEvent::ConstPtr& msg)
{
  cout<<"DigitalInputEventCallback"<<endl;
  estop_in = msg->values[2];
  cout<< "estop_in"<<estop_in<<endl;

  //cout<<"msg value:"<<msg->values[0]<<msg->values[1]<<msg->values[2]<<estop_in;
  if(estop_in)
    {
      kobuki_msgs::MotorPower offcmd;
      offcmd.state = kobuki_msgs::MotorPower::OFF;
      motor_power_pub.publish(offcmd);
      actionlib_msgs::GoalID cancelcmd;
      move_base_cancel_pub.publish(cancelcmd);
    }
}

int main (int argc, char** argv)
{
  ros::init (argc, argv, "emergency");
  ros::NodeHandle nh;
  emergency_sub = nh.subscribe("/mobile_base/events/digital_input", 1, DigitalInputEventCallback);
  motor_power_pub = nh.advertise<kobuki_msgs::MotorPower>("/mobile_base/commands/motor_power", 1);
  move_base_cancel_pub = nh.advertise<actionlib_msgs::GoalID>("move_base/cancel",1);
  estop_in = 0;
  ros::spin();
  return 0;
}
