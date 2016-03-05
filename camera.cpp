#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <pthread.h>
#include <termios.h>
#include <time.h> 

#include "opencv2/opencv.hpp"

#define byte uint8_t
#define H 240
#define W 320

using namespace std;
using namespace cv;

VideoCapture cap(0);

vector<Point2f> f_corners;
Mat transmtx;

Point2f Project(Point2f vp)
{
	vector<Point2f> vecc;
	vector<Point2f> veco (1);
	vecc.push_back(vp);
	perspectiveTransform(vecc, veco, transmtx);
	return veco[0];
}

bool calib()
{
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, H*2);
	cap.set(CV_CAP_PROP_FRAME_WIDTH,  W*2);
	
	Mat frame, dst;
	cap >> frame;
	
	Mat frame_g(frame.size(), CV_8UC1);
	cvtColor(frame, frame_g, CV_BGR2GRAY);
	
	double vmin, vmax;
	minMaxLoc(frame_g, &vmin, &vmax);
	
	addWeighted(frame_g, (255.0/vmax)*2, frame_g, 0.0, -16, frame_g);
	
	
	vector<Point2f> corners;
	goodFeaturesToTrack(frame_g, corners, 100, 0.05, 0.018 * W, Mat(), 5);
	
	Size winSize = Size( 5, 5 );
	Size zeroZone = Size( -1, -1 );
	TermCriteria criteria = TermCriteria( CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 40, 0.001 );
	if (corners.size() > 19)
		cornerSubPix( frame_g, corners, winSize, zeroZone, criteria );
	
	vector<int32_t> markers;
	
	for(uint32_t i = 0; i < corners.size(); i++)
	{
		vector<int32_t> neighbors;
		for(uint32_t ii = 0; ii < corners.size(); ii++)
		{
			if (i != ii)
			{
				double dist = norm(corners[i] - corners[ii]);
				if (dist > 0.02 * W && dist < 0.2*W)
				{
					neighbors.insert(neighbors.end(), ii);
				}
			}
		}
		if (neighbors.size() == 4)
		{
			bool coincided = false;
			for (uint32_t ii = 0; ii < markers.size(); ii++)
			{
			if (norm(corners[markers[ii]] - corners[i]) < 20)
					coincided = true;
			}
			
			if (!coincided)
			{
				Point2f vec = Point2f(0,0);
				for(uint32_t ii = 0; ii < 4; ii++)
				{
					vec += corners[neighbors[ii]] - corners[i];
				}
				
				double dist = norm(vec);
				
				if (dist < 2)
				{
					Point2f vc = (corners[i] - corners[neighbors[3]]);
					double lc = norm(vc);
					vc.x /= lc; vc.y /= lc;
					
					vector<int32_t> d0;
					d0.insert(d0.end(), neighbors[3]);
					vector<int32_t> d1;
					
					double minc = 0;
					int32_t ci = 0;
					for (uint32_t ii = 0; ii < 3; ii++)
					{
						Point2f v = (corners[neighbors[ii]] - corners[neighbors[3]]);
						double l = norm(v);
						v.x /= l; v.y /= l;
						
						double adot = abs(v.dot(vc));
						if (adot > minc)
						{
							minc = adot;
							ci = neighbors[ii];
						}
					}
					
					for (uint32_t ii = 0; ii < 3; ii++)
					{
						if (neighbors[ii] != ci)
							d1.insert(d1.end(), neighbors[ii]);
						else
							d0.insert(d0.end(), neighbors[ii]);
					}
					
					//d0, d1 — диаглнали
					Point2f v0 = (corners[d0[0]] - corners[d0[1]]);
					Point2f v1 = (corners[d1[0]] - corners[d1[1]]);
					double l0 = norm(v0);
					double l1 = norm(v1);
					v0.x /= l0; v0.y /= l0;
					v1.x /= l1; v1.y /= l1;
					
					double cdot = abs(v0.dot(v1));
					
					if (cdot < 0.7)//<45grad
						markers.insert(markers.end(), i);
				}
			}
		}
	}
	
	if (markers.size() == 4)
	{
		//center
		Point2f center(0,0);
		for (uint32_t ii = 0; ii < 4; ii++)
			center += corners[markers[ii]];
		center *= 0.25;
		
		//sort
		vector<int32_t> top, bot;

		for (uint32_t ii = 0; ii < 4; ii++)
		{
			if (corners[markers[ii]].y < center.y)
				top.push_back(markers[ii]);
			else
				bot.push_back(markers[ii]);
		}

		int32_t tl = corners[top[0]].x > corners[top[1]].x ? top[1] : top[0];
		int32_t tr = corners[top[0]].x > corners[top[1]].x ? top[0] : top[1];
		int32_t bl = corners[bot[0]].x > corners[bot[1]].x ? bot[1] : bot[0];
		int32_t br = corners[bot[0]].x > corners[bot[1]].x ? bot[0] : bot[1];
		
		Point2f corr(2,3);//640 -> 320 shift
		f_corners.clear();
		f_corners.push_back((corners[tl] + corr)*0.5);
		f_corners.push_back((corners[tr] + corr)*0.5);
		f_corners.push_back((corners[br] + corr)*0.5);
		f_corners.push_back((corners[bl] + corr)*0.5);
		
		int32_t Size = 200;
		vector<Point2f> quad_pts;
		quad_pts.push_back(Point2f(0, 0));
		quad_pts.push_back(Point2f(Size, 0));
		quad_pts.push_back(Point2f(Size, Size));
		quad_pts.push_back(Point2f(0, Size));
		
		// Get transformation matrix
		transmtx = getPerspectiveTransform(f_corners, quad_pts);
	}
	
	for(uint32_t i = 0; i < markers.size(); i++)
	{
		circle(frame_g, Point2f(corners[markers[i]].x, corners[markers[i]].y), 5, Scalar(0,0,0), 3, CV_AA);
		circle(frame_g, Point2f(corners[markers[i]].x, corners[markers[i]].y), 5, Scalar(255,255,255), 1, CV_AA);
	}

	for(uint32_t i = 0; i < corners.size(); i++)
	{
		circle(frame, Point2f(corners[i].x, corners[i].y), 5, Scalar(0,255,0), 1, CV_AA);
	}
	
	imwrite("out.jpg", frame);
	imwrite("out_o.jpg", frame_g);
	
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, H);
	cap.set(CV_CAP_PROP_FRAME_WIDTH,  W);
	cap.set(CV_CAP_PROP_FPS, 90);
	
	return markers.size() == 4;
}

bool detect(double *x, double *y, double *d)
{
	Mat frame, dst;
	cap >> frame;
	
	Mat frame_g(frame.size(), CV_8UC1);
	
	cvtColor(frame, frame_g, CV_BGR2GRAY);
	
	boxFilter(frame_g, dst, -1, Size(7,7));
	
	addWeighted(frame_g, 1, dst, -0.5, 0, frame_g);

	threshold(frame_g, dst, 135, 255, THRESH_BINARY);

	double vmin, vmax;
	minMaxLoc(dst, &vmin, &vmax);
	if (vmax > 127)
	{
		vector<vector<Point> > contours;
		findContours(dst, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);		
		if (contours.size() > 0)
		{
			Point2f pos;
			float r = 0;
			minEnclosingCircle(contours[0], pos, r);
			
			Point2f position = Project(pos);
			*x = position.x;
			*y = position.y;
			*d = norm(position - Point2f(100,100));
			return true;
		}
	}
	return false;
}

void camerainit(void)
{
	if(!cap.isOpened())
	{
        cout << "Camera Error" << endl;
		return;
	}
	
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, H);
	cap.set(CV_CAP_PROP_FRAME_WIDTH,  W);
	cap.set(CV_CAP_PROP_FPS, 90);
}
