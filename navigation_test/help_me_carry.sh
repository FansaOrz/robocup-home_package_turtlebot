#!/bin/sh

echo "gender recognizition start"
gnome-terminal -x bash -c "roslaunch navigation_test navi_help.launch"

sleep 10

echo "riddle.launch start"
gnome-terminal -x bash -c "roslaunch speech help_me_cary.launch;"


sleep 1


echo "dictionary start"
gnome-terminal -x bash -c " pocketsphinx_continuous -inmic yes -dict ~/catkin_ws/src/speech/voice_library/help_me_carry/help_me_carry.dic -lm ~/catkin_ws/src/speech/voice_library/help_me_carry/help_me_carry.lm"



exit 0
