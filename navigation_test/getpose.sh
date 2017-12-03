#!/bin/sh

 
echo "get waypose"
gnome-terminal -x bash -c "roslaunch navigation_test get_waypoints.launch;"

sleep 1

echo "key-tele"
gnome-terminal -x bash -c "roslaunch turtlebot_teleop keyboard_teleop.launch;"



exit 0


