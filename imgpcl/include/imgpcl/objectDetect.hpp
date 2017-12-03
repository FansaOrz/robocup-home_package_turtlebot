#include <ros/ros.h>
#include <imgpcl/pos.h>
#include <std_msgs/Int32.h> 
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>

#include <opencv2/ml/ml.hpp>
#include <opencv/cv.h>
#include <opencv2/imgproc/imgproc.hpp>    
    #include <opencv2/core/core.hpp>         
    #include <opencv2/opencv.hpp>        
    #include <opencv2/highgui/highgui.hpp>    
#include "opencv2/video/tracking.hpp"  
    #include <iostream>        
    #include<string>        
    #include <sstream>      

//write txt
#include <fstream>  
#include <vector>
#include <float.h>  //isnan 
//pcl
#include <sensor_msgs/PointCloud2.h>  
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_ros/point_cloud.h>
#include <boost/foreach.hpp>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>
using namespace std;
using namespace cv; 
using namespace message_filters;
using namespace sensor_msgs;

typedef pcl::PointCloud<pcl::PointXYZRGBA> PointCloud;
typedef boost::shared_ptr<PointCloud> PointCloudPtr;
typedef boost::shared_ptr<const PointCloud> PointCloudConstPtr;

#define PI 3.14159
struct objectRecT   //store results of recognition, include the object flag and its region
{
    char name[100];
    int flag;
    float similarity;
    CvRect rc;
};

bool ComputeClusterCenter(const pcl::PointCloud<pcl::PointXYZ> Points, CvRect rc, imgpcl::pos& p);
bool extrHueFeature(IplImage* src, vector<float>& feature);
void sWindowHueDetect(IplImage *inputImg, IplImage* tempImg, vector<CvRect>& object, float similar);
bool sWindowHueDetectP(IplImage *inputImg, IplImage* tempImg, CvRect& object, float similar);
void getHFromHSV(IplImage* imgHsv, IplImage* imgH);
float compareHist(vector<float> win_hist, vector<float> temp_hist, int method);
void localBinaryPattern(IplImage* src, IplImage* dst, int thresh);
bool extrLbpHist(IplImage* src, vector<float>& feature);

//从hsv图像中分离出hue图像
void getHFromHSV(IplImage* imgHsv, IplImage* imgH)
{
    for(int row=0;row<imgHsv->height;row++)
    {
        uchar* ptr = (uchar*)imgHsv->imageData + row*imgHsv->widthStep;
        uchar* h_ptr = (uchar*)imgH->imageData + row*imgH->widthStep;
        for(int col=0;col<imgHsv->width;col++)
        {
            int h = *(ptr++);
            int s = *(ptr++);
            int v = *(ptr++);
            if( s > 80 )
            {
                *(h_ptr++) = (int)h;
            }
            else
            {
                *(h_ptr++) = 0;
            }
        }
    }
}
//计算hue直方图，src-3通道rgb图像，feature-返回的180维直方图
bool extrHueFeature(IplImage* src, vector<float>& feature)
{
	//计算颜色直方图
	if(src->nChannels != 3)
		return false;
	
	IplImage* hsvImg = cvCreateImage(cvGetSize(src), 8, 3);
	cvCvtColor(src, hsvImg, CV_BGR2HSV);
	
	//统计色调分量h
	float hue[180];
	float effective_num = 0;
	for(int i=0;i<180;i++)
	{
		hue[i] = 0;
	}
	for(int row=0;row<hsvImg->height;row++)
	{
	    uchar* ptr = (uchar*)hsvImg->imageData + row*hsvImg->widthStep;
		for(int col=0;col<hsvImg->width;col++)
		{
			int h = (int)*(ptr);
			int s = (int)*(ptr+1);
			int v = (int)*(ptr+2);
			if( s > 50 )
			{
			    hue[h]++;	
			    effective_num ++;
			}		
			ptr += 3;
		}
	}
	//归一化
	for(int i=0;i<180;i++)
	{
	    hue[i] /= effective_num;
	    feature.push_back(hue[i]);
	}
	
	cvReleaseImage(&hsvImg);
	return true;
}

//比较两个直方图的距离，method-1为直方图相交距离，2-巴氏距离
float compareHist(vector<float> win_hist, vector<float> temp_hist, int method)
{
    if((win_hist.size() != temp_hist.size()) || win_hist.size() == 0) 
        return -1;
    
    float similarity = 0;	
	if(method == 1)
	{
	    //采用直方图相交运算计算相似度
	    float sumMin = 0;
	    float sumWin = 0;
	    for(int i=0;i<win_hist.size();i++)
	    {
	        sumWin += win_hist.at(i);
	        float min = (win_hist.at(i) < temp_hist.at(i))?win_hist.at(i):temp_hist.at(i);
	        sumMin += min;
	    }
	    similarity = sumMin/sumWin;
	}
	else if(method == 2)
	{
	    //巴氏距离
	    float avgWin = 0;
	    float avgTemp = 0;
	    float winMultiTemp = 0;
	    int N = win_hist.size();
	    for(int i=0; i<N; i++)
	    {
	        avgWin += win_hist.at(i);
	        avgTemp += temp_hist.at(i);
	        winMultiTemp +=  sqrt(win_hist.at(i)*temp_hist.at(i));
	    }
	    avgWin /= N;
	    avgTemp /= N;
	    similarity = sqrt( 1 - 1.0/sqrt(avgWin*avgTemp*N*N) * winMultiTemp );
	}
	return similarity;
}
//滑动窗口的模板颜色直方图匹配方法，inputImg-输入rgb图像，tempImg-输入rgb模板，objetc-返回的最可能1个的结果，similar-输入相似系数阈值 
bool sWindowHueDetect(IplImage *inputImg, IplImage* tempImg, objectRecT& object, float similar)
{
    if(inputImg->nChannels != 3)
        return false;
   
    int width = inputImg->width;
    int height = inputImg->height;
    int search_width = 50;
    int search_height = 100;
    int step = 20;
    //提取模板的颜色直方图
    vector<float> temp_hist;
    extrHueFeature(tempImg, temp_hist);
    
    //滑动窗口提取颜色直方图
    IplImage* colorImg = cvCreateImage(cvGetSize(inputImg), 8, 3);
    
    int mark_width = int( (width-search_width)/step );
    int mark_height = int( (height-search_height)/step );
    IplImage* markImg = cvCreateImage(cvSize(mark_width, mark_height), 8, 1);
    cvZero(markImg);
    
    cvCopy(inputImg, colorImg);
    for(int row=0; row<height-search_height; row+=step)
    {
        for(int col=0; col<width-search_width; col+=step)
        {
            CvRect rc = cvRect(col, row, search_width, search_height);
            if(rc.x + rc.width > width-1 || rc.y+rc.height > height-1)
                break;
            IplImage* winImg = cvCreateImage(cvSize(search_width, search_height), 8, 3);
            cvSetImageROI(inputImg, rc);
            cvCopy(inputImg, winImg);
            cvResetImageROI(inputImg);
            
            vector<float> win_hist;
            extrHueFeature(winImg, win_hist);

            //比较temp_hist, win_hist的差别
            float similarity = compareHist(win_hist, temp_hist, 1);
           
            int newrow = row/step;
            int newcol = col/step;
            CV_IMAGE_ELEM(markImg, uchar, newrow, newcol) = int(similarity*255);
              
            cvReleaseImage(&winImg);
            win_hist.clear();
        }
    }
    //取相似度最大的区域
    int max = -1;
    CvRect rect;
    for(int row=0;row<markImg->height;row++)
    {
        for(int col=0;col<markImg->width;col++)
        {
            int v = CV_IMAGE_ELEM(markImg, uchar, row, col);
            if(max < v && v > similar*255)
            {
                max = v;
                rect.x = col*step;
                rect.y = row*step;
                rect.width = search_width;
                rect.height = search_height;
            }
        }
    }
    if(max != -1)
    {
        object.rc = rect;
        object.similarity = max/255.0;
    }
    
    //cvNamedWindow("compare", 0);
    //cvShowImage("compare", markImg);
    //cvWaitKey(100);
    
    cvReleaseImage(&markImg);
    cvReleaseImage(&colorImg);
    temp_hist.clear(); 
    
    if(max == -1)
        return false;
    else
        return true;
}
//多尺度的滑动窗口的模板颜色直方图匹配方法，inputImg,tempImg-输入rgb图像，object-返回的最可能的结果，similar-相似度阈值
//建立5层金字塔
bool sWindowHueDetectP(IplImage *inputImg, IplImage* tempImg, objectRecT& object, float similar)
{
    if(inputImg->nChannels != 3)
        return false;
   
    //cout<<"windowDetect\n";
    //提取模板的颜色直方图
    vector<float> temp_hist;
    extrHueFeature(tempImg, temp_hist);
  
    int origin_width = inputImg->width;
    int origin_height = inputImg->height;
    int search_width = 20;
    int search_height = 20;
    int step = 5;
    
    float max = 0;
    //图像进行采样
    for(int times =0;times<5;times++)
    {
        //cout<<"times = "<<times<<"\n";
        int scale = pow(2, times);
        IplImage* curImg = cvCreateImage(cvSize(origin_width/scale, origin_height/scale), 8, 3);
        int width = curImg->width;
        int height = curImg->height;
          
        if(width < search_width || height < search_height)
        {
            cvReleaseImage(&curImg);
            break;    
        } 
        //滑动窗口提取颜色直方图
        cvResize(inputImg, curImg);
        
        IplImage* colorImg = cvCreateImage(cvGetSize(curImg), 8, 3);
        cvCopy(curImg, colorImg);
        
        //cout<<"go to search in the curImg\n";
        for(int row=0; row<height-search_height; row+=step)
        {
            for(int col=0; col<width-search_width; col+=step)
            {
                CvRect rc = cvRect(col, row, search_width, search_height);
                if(rc.x + rc.width > width-1 || rc.y+rc.height > height-1)
                    break;
                IplImage* winImg = cvCreateImage(cvSize(search_width, search_height), 8, 3);
                cvSetImageROI(inputImg, rc);
                cvCopy(inputImg, winImg);
                cvResetImageROI(inputImg);
                
                vector<float> win_hist;
                extrHueFeature(winImg, win_hist);
       
                //比较temp_hist, win_hist的差别
                float similarity = compareHist(win_hist, temp_hist, 1);
                if(similarity >= similar && similarity > max)
                {
                    max = similarity;
                    //全部扩大为原来的scale倍
                    object.rc = cvRect(rc.x*scale, rc.y*scale, rc.width*scale, rc.height*scale);
                    object.similarity = max;   
                }
            
                cvReleaseImage(&winImg);
                win_hist.clear();                
            }
        }
   
        cvReleaseImage(&curImg);
        cvReleaseImage(&colorImg);  
    } 
    temp_hist.clear();
    if(max == 0)
        return false;
    else
        return true;
}
//由二维图像坐标到三维点云坐标的转换
bool ComputeClusterCenter(const pcl::PointCloud<pcl::PointXYZ> Points, CvRect rc, imgpcl::pos& p)
{
	cout<<"CCCCCCCCCCCC"<<endl;
	//找距离最近的n个点
	int nearNum = 5;
	vector<imgpcl::pos> vpt;     //储存所有可能的点
	for(int i=rc.x; i<rc.x+rc.width; i++)
	{
	    for(int j=rc.y; j<rc.y+rc.height; j++)
	    {
	        float x = Points(j,i).x;
            float y = Points(j,i).y;
            float z = Points(j,i).z;
            if( !(isnan(x) || isnan(y) || isnan(z)) )
            {
                /*if(z < p.z)
                {
                    p.x = x;
                    p.y = y;
                    p.z = z;
                }*/
                imgpcl::pos pt;
                pt.x = x;
                pt.y = y;
                pt.z = z;
                vpt.push_back(pt);
            }
	    }
	}
	cout<<"FFFFFFFFF"<<endl;
	//对所有的点按照z从大到小排序
	if(vpt.size() > 1)
	{
		for(int i=0; i<vpt.size(); i++)
		{
		    for(int j=vpt.size()-1; j>i; j--)
		    {
			if(vpt.at(j).z < vpt.at(j-1).z)
			{
			    imgpcl::pos temp;
			    temp.x = vpt.at(j).x;
			    temp.y = vpt.at(j).y;
			    temp.z = vpt.at(j).z;
			    
			    vpt.at(j).x = vpt.at(j-1).x;
			    vpt.at(j).y = vpt.at(j-1).y;
			    vpt.at(j).z = vpt.at(j-1).z;
			    
			    vpt.at(j-1).x = temp.x;
			    vpt.at(j-1).y = temp.y;
			    vpt.at(j-1).z = temp.z;
			}
		    }
		}
		cout<<"最远的点"<<vpt.at(vpt.size()-1).z<<endl;
        cout<<"最近的点"<<vpt.at(0).z<<endl;	
	}
	
	//取前n个点
	p.x = 0;
	p.y = 0;
	p.z = 0;
	if(nearNum > vpt.size())
	{
	    if(vpt.size() < 1)
	    {
			cout<<"no point in cloud"<<endl;
	        return false;
	    }
	    nearNum = vpt.size();
	}
	while(vpt.at(nearNum-1).z > (vpt.at(vpt.size()-1).z+vpt.at(0).z)/2)
	{
	     if(nearNum > 1)
	       nearNum--;
         else
               break;
	}
	
	for(int i=0; i<nearNum; i++)
	{
	    p.x += vpt.at(i).x;
	    p.y += vpt.at(i).y;
	    p.z += vpt.at(i).z;
	}
	p.x /= nearNum;
	p.y /= nearNum;
	p.z /= nearNum;
	cout<<"finish"<<endl;
    return true;
}
void localBinaryPattern(IplImage* src, IplImage* dst, int thresh)
{
	//src--gray, dst--gray
	int height = src->height;
	int width = src->width;

	for(int row=0;row<height;row++)
	{
		for(int col=0;col<width;col++)
		{
			int g0 = CV_IMAGE_ELEM(src, uchar, row, col);
			int lbp = 0;
			
			CvPoint neibor[8];
			neibor[0] = cvPoint(-1, -1);
			neibor[1] = cvPoint(0, -1);
			neibor[2] = cvPoint(1, -1);
			neibor[3] = cvPoint(-1, 0);
			neibor[4] = cvPoint(1, 0);
			neibor[5] = cvPoint(-1, 1);
			neibor[6] = cvPoint(0, 1);
			neibor[7] = cvPoint(1, 1);
			for(int i=0;i<8;i++)
			{
				int newrow = row+neibor[i].y;
				int newcol = col+neibor[i].x;
				if(newrow < 0 || newcol < 0 || newrow >= height || newcol >= width)
					continue;
				int gi = (int)CV_IMAGE_ELEM(src, uchar, newrow, newcol);
				if(gi - g0 >= thresh)  //修改了阈值
					lbp += pow(2.0, i);
			}
			CV_IMAGE_ELEM(dst, uchar, row, col) = lbp;
		}
	}
}
bool extrLbpHist(IplImage* src, vector<float>& feature)
{
    if(src->nChannels != 1)
        return false;
    feature.clear();
       
    float lbp[256];
    for(int i=0;i<256;i++)
    {
        lbp[i] = 0;
    }
    for(int row=0;row<src->height;row++)
    {
        uchar* ptr = (uchar*)(src->imageData + row*src->widthStep);
        for(int col=0;col<src->width;col++)
        {
            int v = *(ptr++);
            lbp[v]++;
        }
    }
    //normalize
    for(int i=0;i<256;i++)
    {
        lbp[i] = lbp[i]/(src->width*src->height)*256;
        feature.push_back(lbp[i]);
    }
    return true;
}
