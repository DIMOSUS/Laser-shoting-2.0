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

#include "opencv2/opencv.hpp"
#include "gui.h"

#define TARGET_H 500
#define TARGET_W 500
#define TARGET_X 50
#define TARGET_Y 50

using namespace std;
using namespace cv;

Mat Target;
Mat Win;

void putTextCorr(Mat& img, const string& text, Point org, int32_t fontFace, double fontScale, Scalar color, int32_t thickness=1, int32_t lineType=8, Point2f corr = Point2f(0, 0))
{
	int32_t baseline = 0;
	Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
	Point2f c = Point2f(textSize.width, -textSize.height/2);
	c = Point2f(c.x*corr.x, c.y*corr.y);
	putText(img, text, Point2f(org.x , org.y) - c, fontFace, fontScale, color, thickness, lineType);
}

void initgui()
{
	Target = imread("target.png", CV_LOAD_IMAGE_UNCHANGED);
	Win = Target.clone();
	cout << "init" << endl;
	namedWindow("Game", CV_WINDOW_AUTOSIZE);
	imshow("Game", Win);
	waitKey(100);
	cout << "init ok" << endl;
}

void clear()
{
	Win = Target.clone();
	imshow("Game", Win);
	waitKey(100);
}

void addhole(double x, double y)
{
	Point2f pos(x,y);
	pos = Point2f(TARGET_X, TARGET_Y) +  Point2f(TARGET_W*x*0.005, TARGET_H*y*0.005);
	
	circle(Win, pos, 5, Scalar(127,127,127), 10, CV_AA);
    circle(Win, pos, 9, Scalar(255,255,255), 1, CV_AA);
	circle(Win, pos, 10, Scalar(0,0,0), 1, CV_AA);
	imshow("Game", Win);
	waitKey(100);
}

void addShotScoore(int32_t scoore, int32_t slot, bool final)
{
	Point2f pos = Point2f(TARGET_X+TARGET_W + 40, TARGET_Y + slot * 50);
	
	ostringstream ost;
	if (final)
	{
		ost << "Score: ";
		pos += Point2f(-103, 0);
	}
	else ost << slot << ": ";
	ost << scoore;
	
	putTextCorr(Win, ost.str(), pos, FONT_HERSHEY_DUPLEX, 1.5, Scalar(0,0,0), 5, CV_AA, Point2f(0.0, 1.0));
	imshow("Game", Win);
	waitKey(100);
}