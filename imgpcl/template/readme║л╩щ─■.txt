1. 图片名称为物体名称
2. 将图片名称写在list.txt文件中
3. 在程序中修改 模板的个数 temp_num
4. 标定完成后，修改坐标系转换参数
5. 订阅的摄像头话题（如果修改openni的设置），保持一致
6. 采集图片，运行 rosrun object cv_bgcapture
7. 摄像机标定，运行 rosrun obejct camera_calibration
8. 识别，运行 rosrun object detect

PS:如果模板图片上有锁，会报错，因为权限不够，此时要把模板图片另存一下。
