#!/usr/bin/env python
#coding:UTF-8
#上句表示程序中可以有中文，包括注释
"""
    talkback.py - Say back what is heard by the pocketsphinx recognizer.
"""

import roslib; roslib.load_manifest('speech')
import rospy
from std_msgs.msg import String
from std_msgs.msg import Char
#from std_msgs.msg import Int8

from sound_play.libsoundplay import SoundClient

class gpsr:
      
    def __init__(self):

        rospy.on_shutdown(self.cleanup)
	self.voice = rospy.get_param("~voice", "voice_cmu_us_rms_cg_clunits") 
        self.wavepath = rospy.get_param("~wavepath", "")
	
        self.keywords_to_temp_temp = {'object': ['key','pepsi','nongfu-spring','green-tea','shuttlecock','cup']}
	
	self.speakout1 = ''

        self.soundhandle = SoundClient() 
        rospy.sleep(1)
        self.soundhandle.stopAll()
        rospy.sleep(1) 
        self.soundhandle.say("Ready", self.voice) 
        rospy.Subscriber('/recognizer/output', String, self.recognition)


    def recognition(self, msg) :
	msg.data = msg.data.lower()
	print msg.data
	self.soundhandle.say(msg.data,self.voice)
	rospy.sleep(3)


	
    def cleanup(self):
        rospy.loginfo("Shutting down gpsr node...")

if __name__=="__main__":
    rospy.init_node('gpsr')
    try:
        gpsr()
        rospy.spin()
    except:
        pass

