#!/bin/sh

echo "turtlebot connected"
gnome-terminal -x bash -c "roslaunch turtlebot_bringup minimal.launch"

sleep 1 

echo "kinect connected"
gnome-terminal -x bash -c "roslaunch freenect_launch doublekinect_test.launch"

sleep 1

echo "gender recognizition start"
gnome-terminal -x bash -c "rosrun imgpcl gender_2017"

sleep 1

echo "riddle.launch start"
gnome-terminal -x bash -c "roslaunch speech SPR.launch;"

sleep 1

echo "dictionary start"
gnome-terminal -x bash -c " pocketsphinx_continuous -inmic yes -dict ~/catkin_ws/src/speech/voice_library/riddle/2568.dic -lm ~/catkin_ws/src/speech/voice_library/riddle/2568.lm"

exit 0
