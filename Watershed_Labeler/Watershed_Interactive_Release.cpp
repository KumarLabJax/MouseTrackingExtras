#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>
#include <math.h>

#ifdef __APPLE__
	#include <sys/uio.h>
#else
	#ifdef __linux
		#include <unistd.h>
	#else // Windows...
		#include <io.h> 
		#define access    _access_s
	#endif
#endif

using namespace std;
using namespace cv;

int pos=0, neg=0, squareSize=3;
int bgMark=255, fgMark=127;
Mat img, imgPlot, markers;
bool lbutton=false, rbutton=false;

const char* keys = 
{
	"{fn|movie.avi|file_name}"
	"{sf|0|start_frame}"
	"{nf|50000|number_frames}"
	"{rm|false|random_mode}"
	"{ps|3|pointSize}"
};

/*
// Alternate windows keys
	#define UPKEY 65362
	#define DOWNKEY 65364
	#define LEFTKEY 65361
	#define RIGHTKEY 65363
*/
#ifdef __APPLE__
	#define RIGHTKEY 63235
	#define LEFTKEY 63234
	#define UPKEY 63232
	#define DOWNKEY 63233
#else
	#ifdef __linux
		#define UPKEY 56
		#define DOWNKEY 50
		#define LEFTKEY 52
		#define RIGHTKEY 54
	#else // Windows...
		#define UPKEY 2490368
		#define DOWNKEY 2621440
		#define LEFTKEY 2424832
		#define RIGHTKEY 2555904
	#endif
#endif

void GammaCorrection(Mat& src, Mat& dst, float fGamma)
{
	unsigned char lut[256];
	for (int i = 0; i < 256; i++)
	{
		lut[i] = saturate_cast<uchar>(pow((float)(i / 255.0), fGamma) * 255.0f);
	}
	dst = src.clone();
	const int channels = dst.channels();
	switch (channels)
	{
		case 1:
		{
			MatIterator_<uchar> it, end;
			for (it = dst.begin<uchar>(), end = dst.end<uchar>(); it != end; it++)
				*it = lut[(*it)];
			break;
		}
		case 3:
		{
			MatIterator_<Vec3b> it, end;
			for (it = dst.begin<Vec3b>(), end = dst.end<Vec3b>(); it != end; it++)
			{
				(*it)[0] = lut[((*it)[0])];
				(*it)[1] = lut[((*it)[1])];
				(*it)[2] = lut[((*it)[2])];
			}
			break;
		}
	}
}

void paintWatershed(int event, int x, int y, int flags, void* userdata)
{
	if ((event==EVENT_LBUTTONDOWN || (event==EVENT_MOUSEMOVE && lbutton)) && x<img.size().width && x>=squareSize && y<img.size().height && y>=squareSize)
	{
		lbutton=true;
		circle(imgPlot,Point(x,y),squareSize,Scalar(0,255,0),-1);
		circle(markers,Point(x,y),squareSize,Scalar(fgMark),-1);
		imshow("Frame",imgPlot);
	}
	else if ((event==EVENT_RBUTTONDOWN || (event==EVENT_MOUSEMOVE && rbutton)) && x<img.size().width && x>=squareSize && y<img.size().height && y>=squareSize)
	{
		rbutton=true;
		circle(imgPlot,Point(x,y),squareSize,Scalar(0,0,255),-1);
		circle(markers,Point(x,y),squareSize,Scalar(bgMark),-1);
		imshow("Frame",imgPlot);
	}
	
	else if (event==EVENT_LBUTTONUP)
	{
		lbutton=false;
	}
	else if (event==EVENT_RBUTTONUP)
	{
		rbutton=false;
	}
}

void calcWatershed(Mat markers, Mat img, Mat &result, int fg, int bg)
{
	vector<vector<Point> > contours;
	Mat markers_copy;
	markers.copyTo(markers_copy);
	//findContours(markers,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);
	findContours(markers_copy,contours,CV_RETR_LIST,CV_CHAIN_APPROX_NONE);
	Mat markers2=Mat::zeros(img.size(),CV_32SC1);
	for (size_t i=0;i<contours.size();i++)
		drawContours(markers2,contours,static_cast<int>(i),Scalar::all(static_cast<int>(i)+1), 1, 8);

	// Copy image to not modify original
	Mat segmented;
	segmented.create(img.size(),CV_8UC1);
	img.convertTo(segmented,CV_8UC1);

	watershed(segmented,markers2);

	// Draw resulting image
	// Make list of colors
	vector<uchar> colors;
	for (size_t i = 0; i < contours.size(); i++)
	{
		colors.push_back(uchar((i+1)*1));
	}
	// Create the result image
	Mat dst = Mat::zeros(markers2.size(), CV_8UC1);
	// Fill labeled objects with colors
	for (int i = 0; i < markers_copy.rows; i++)
	{
		for (int j = 0; j < markers_copy.cols; j++)
		{
			int index = markers2.at<int>(i,j);
			if (index > 0 && index <= static_cast<int>(contours.size()))
			    dst.at<uchar>(i,j) = colors[index-1];
			else
			    dst.at<uchar>(i,j) = 0;
		}
	}

	// Mask image to find color, then only pick the segment with that color
	dst.convertTo(dst,CV_8UC1);
	Mat final1, final2, final3;
	final1.create(img.size(),CV_8UC1);
	final1=Scalar::all(0);
	final2.create(img.size(),CV_8UC3);
	final2=Scalar::all(0);
	// Output image needs to be C3 format to save correctly.
	final3.create(img.size(),CV_8UC3);
	final3=Scalar::all(195);

	// Incase the watershed segmented it into multiple sections, find them all (minimum 1 section found)
	if(contours.size()>1)
	{
		markers.copyTo(final1,markers==fg);
		vector<vector<Point> > contours_2;
		findContours(final1,contours_2,CV_RETR_LIST,CV_CHAIN_APPROX_NONE);
		final1=Scalar::all(195);
		for (size_t i=0;i<contours_2.size();i++)
		{
			Scalar colorVal;
			vector<Point> pts = contours_2[i];
			Mat mask(img.size(),CV_8UC1,Scalar::all(0));
			// Grab color at the first point location of the contour
			colorVal=dst.at<uchar>(pts[0].y,pts[0].x);
			mask=abs(dst-colorVal.val[0])<1;
			final3.copyTo(final2,mask);
		}
		// Remove the watershed boundary edges
		dilate(final2,final2,Mat::ones(3,3,CV_8UC1),Point(-1,-1),1);
		// Copy the foreground to a final image
		img.copyTo(final3,final2);
	}

	cvtColor(final2,final2,CV_BGR2GRAY);
	final2.copyTo(result);
}

RotatedRect getEllipseFit(Mat src)
{
	int thresh = 100;
	Mat src_gray;
	Mat threshold_output;

	/// Convert image to gray and remove tail
	cvtColor(src, src_gray, CV_BGR2GRAY);
	morphologyEx(src_gray, src_gray, MORPH_OPEN, Mat::ones(3,3,CV_8UC1), Point(-1,-1), 3);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using Threshold
	threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY);

	/// Find contours
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0));

	/// Find the ellipses for each contour
	vector<RotatedRect> minEllipse(contours.size());
	for( int i = 0; i < contours.size(); i++ )
	{
		if(contours[i].size() > 5)
		{
			minEllipse[i] = fitEllipse(Mat(contours[i]));
			return minEllipse[i];
		}
	}

	// If no ellipse is found, return a default
	return RotatedRect(Point2f(0,0), Size2f(1,1), 30);
}

int main(int argc, const char** argv)
{
	CommandLineParser parser(argc, argv, keys);
	string file = parser.get<string>("fn");
	int framenum = parser.get<int>("sf");
	bool randomizedEnabled = parser.get<bool>("rm");
	int numberFrames = parser.get<int>("nf");
	squareSize = parser.get<int>("ps");
	int keyPressed;
	bool isTraining=true;
	VideoCapture cap;
	ofstream ellipseFile;
	srand(time(NULL));

	cap.open(file.c_str());
	if( !cap.isOpened() )
	{
		printf("Can not open video file\n");
		return -1;
	}
	cap.set(CV_CAP_PROP_POS_FRAMES,framenum);
	cap >> img;
	if (img.empty())
		return -1;
	framenum++;
	img.copyTo(imgPlot);
	#ifdef __linux
		cvNamedWindow("Frame", CV_GUI_NORMAL | CV_WINDOW_KEEPRATIO);
		
	#else
		cvNamedWindow("Frame", CV_WINDOW_OPENGL | CV_WINDOW_AUTOSIZE);
	#endif
	imshow("Frame",imgPlot);
	setMouseCallback("Frame",paintWatershed,NULL);
	Mat segmented=Mat::zeros(img.size(),CV_8UC1), segmentedPreview, output;
	markers = Mat::zeros(img.size(),CV_8UC1);
	segmentedPreview.setTo(0);

	// Make the output folder
	string outputFolder = file.substr(0,file.find_last_of("."));
	string segFolder = file.substr(0,file.find_last_of("."))+"/Seg";
	string refFolder = file.substr(0,file.find_last_of("."))+"/Ref";
	string ellFolder = file.substr(0,file.find_last_of("."))+"/Ell";
	system( ("mkdir \"" + outputFolder + "\"").c_str());
	system( ("mkdir \"" + segFolder + "\"").c_str());
	system( ("mkdir \"" + refFolder + "\"").c_str());
	system( ("mkdir \"" + ellFolder + "\"").c_str());

	string outFName;
	string ellFName;

	keyPressed=waitKey();
	
	Mat channel[3];
	Mat gmcrt;
	int minFrameNum=0;
	int maxFrameNum=cap.get(CV_CAP_PROP_FRAME_COUNT);
	RotatedRect minEllipse;
	int currentFrame = framenum;
	framenum=1;
	for(;;)
	{
		// Useful for finding the value for the keystroke
		//cout << "Key: " << keyPressed << " " << char(keyPressed) << endl;
		switch(keyPressed)
		{
			case 'n': // "n" New frame without saving
				cout << "Frame Skipped: " << currentFrame << "\n";
				// Seek to random frame within the video
				if (randomizedEnabled)
					currentFrame = rand()%(maxFrameNum-minFrameNum + 1) + minFrameNum;
				else
					currentFrame++;
				cap.set(CV_CAP_PROP_POS_FRAMES,currentFrame);
				cap >> img;
				if (img.empty())
					return -1;
				img.copyTo(imgPlot);
				imshow("Frame",imgPlot);

				markers.setTo(0);
				keyPressed=waitKey();
				break;
			case '+': // Increase Gamma
				GammaCorrection(imgPlot, gmcrt, 0.9);
				gmcrt.copyTo(imgPlot);
				imshow("Frame",imgPlot);
				keyPressed=waitKey();
				break;
			case UPKEY: // Up arrow key
				calcWatershed(markers,img,segmented,fgMark,bgMark);
				// Output all data
				outFName = file.substr(0,file.find_last_of("."))+"/Ref/"+file.substr(0,file.find_last_of("."))+format("_%d.png",currentFrame);
				imwrite(outFName,img);
				output=Mat::zeros(img.size(),CV_8UC3);
				output.setTo(fgMark,segmented);
				minEllipse = getEllipseFit(output);
				minEllipse.angle = minEllipse.angle>90 ? 360-minEllipse.angle : 180-minEllipse.angle;
				outFName = file.substr(0,file.find_last_of("."))+"/Seg/"+file.substr(0,file.find_last_of("."))+format("_%d.png",currentFrame);
				imwrite(outFName,output);
				ellFName = file.substr(0,file.find_last_of("."))+"/Ell/"+file.substr(0,file.find_last_of("."))+format("_%d.txt",currentFrame);
				ellipseFile.open(ellFName.c_str());
				ellipseFile << minEllipse.center.x << "\t" << minEllipse.center.y << "\t" << minEllipse.size.width << "\t" << minEllipse.size.height << "\t" << minEllipse.angle << endl;
				ellipseFile.close();
				cout << "Frame Num: " << framenum << " , Frame Saved: " << currentFrame << " Angle: " << minEllipse.angle << "\n";
				// Seek to random frame within the video
				if (randomizedEnabled)
					currentFrame = rand()%(maxFrameNum-minFrameNum + 1) + minFrameNum;
				else
					currentFrame++;
				cap.set(CV_CAP_PROP_POS_FRAMES,currentFrame);
				cap >> img;
				if (img.empty())
					return -1;
				framenum++;
				img.copyTo(imgPlot);
				imshow("Frame",imgPlot);

				markers.setTo(0);
				keyPressed=waitKey();
				break;
			case DOWNKEY: // Down arrow key
				calcWatershed(markers,img,segmented,fgMark,bgMark);
				// Output all data
				outFName = file.substr(0,file.find_last_of("."))+"/Ref/"+file.substr(0,file.find_last_of("."))+format("_%d.png",currentFrame);
				imwrite(outFName,img);
				output=Mat::zeros(img.size(),CV_8UC3);
				output.setTo(fgMark,segmented);
				minEllipse = getEllipseFit(output);
				minEllipse.angle = minEllipse.angle>90 ? 180-minEllipse.angle : 360-minEllipse.angle;
				outFName = file.substr(0,file.find_last_of("."))+"/Seg/"+file.substr(0,file.find_last_of("."))+format("_%d.png",currentFrame);
				imwrite(outFName,output);
				ellFName = file.substr(0,file.find_last_of("."))+"/Ell/"+file.substr(0,file.find_last_of("."))+format("_%d.txt",currentFrame);
				ellipseFile.open(ellFName.c_str());
				ellipseFile << minEllipse.center.x << "\t" << minEllipse.center.y << "\t" << minEllipse.size.width << "\t" << minEllipse.size.height << "\t" << minEllipse.angle << endl;
				ellipseFile.close();
				cout << "Frame Num: " << framenum << " , Frame Saved: " << currentFrame << " Angle: " << minEllipse.angle << "\n";
				// Seek to random frame within the video
				if (randomizedEnabled)
					currentFrame = rand()%(maxFrameNum-minFrameNum + 1) + minFrameNum;
				else
					currentFrame++;
				cap.set(CV_CAP_PROP_POS_FRAMES,currentFrame);
				cap >> img;
				if (img.empty())
					return -1;
				framenum++;
				img.copyTo(imgPlot);
				imshow("Frame",imgPlot);

				markers.setTo(0);
				keyPressed=waitKey();
				break;
			case RIGHTKEY: // Right arrow key
				calcWatershed(markers,img,segmented,fgMark,bgMark);
				// Output all data
				outFName = file.substr(0,file.find_last_of("."))+"/Ref/"+file.substr(0,file.find_last_of("."))+format("_%d.png",currentFrame);
				imwrite(outFName,img);
				output=Mat::zeros(img.size(),CV_8UC3);
				output.setTo(fgMark,segmented);
				minEllipse = getEllipseFit(output);
				minEllipse.angle = 180-minEllipse.angle;
				outFName = file.substr(0,file.find_last_of("."))+"/Seg/"+file.substr(0,file.find_last_of("."))+format("_%d.png",currentFrame);
				imwrite(outFName,output);
				ellFName = file.substr(0,file.find_last_of("."))+"/Ell/"+file.substr(0,file.find_last_of("."))+format("_%d.txt",currentFrame);
				ellipseFile.open(ellFName.c_str());
				ellipseFile << minEllipse.center.x << "\t" << minEllipse.center.y << "\t" << minEllipse.size.width << "\t" << minEllipse.size.height << "\t" << minEllipse.angle << endl;
				ellipseFile.close();
				cout << "Frame Num: " << framenum << " , Frame Saved: " << currentFrame << " Angle: " << minEllipse.angle << "\n";
				// Seek to random frame within the video
				if (randomizedEnabled)
					currentFrame = rand()%(maxFrameNum-minFrameNum + 1) + minFrameNum;
				else
					currentFrame++;
				cap.set(CV_CAP_PROP_POS_FRAMES,currentFrame);
				cap >> img;
				if (img.empty())
					return -1;
				framenum++;
				img.copyTo(imgPlot);
				imshow("Frame",imgPlot);

				markers.setTo(0);
				keyPressed=waitKey();
				break;
			case LEFTKEY: // Left arrow key
				calcWatershed(markers,img,segmented,fgMark,bgMark);
				// Output all data
				outFName = file.substr(0,file.find_last_of("."))+"/Ref/"+file.substr(0,file.find_last_of("."))+format("_%d.png",currentFrame);
				imwrite(outFName,img);
				output=Mat::zeros(img.size(),CV_8UC3);
				output.setTo(fgMark,segmented);
				minEllipse = getEllipseFit(output);
				minEllipse.angle = 360-minEllipse.angle;
				outFName = file.substr(0,file.find_last_of("."))+"/Seg/"+file.substr(0,file.find_last_of("."))+format("_%d.png",currentFrame);
				imwrite(outFName,output);
				ellFName = file.substr(0,file.find_last_of("."))+"/Ell/"+file.substr(0,file.find_last_of("."))+format("_%d.txt",currentFrame);
				ellipseFile.open(ellFName.c_str());
				ellipseFile << minEllipse.center.x << "\t" << minEllipse.center.y << "\t" << minEllipse.size.width << "\t" << minEllipse.size.height << "\t" << minEllipse.angle << endl;
				ellipseFile.close();
				cout << "Frame Num: " << framenum << " , Frame Saved: " << currentFrame << " Angle: " << minEllipse.angle << "\n";
				// Seek to random frame within the video
				if (randomizedEnabled)
					currentFrame = rand()%(maxFrameNum-minFrameNum + 1) + minFrameNum;
				else
					currentFrame++;
				cap.set(CV_CAP_PROP_POS_FRAMES,currentFrame);
				cap >> img;
				if (img.empty())
					return -1;
				img.copyTo(imgPlot);
				imshow("Frame",imgPlot);
				framenum++;

				markers.setTo(0);
				keyPressed=waitKey();
				break;
			case 'r':
				img.copyTo(imgPlot);
				markers.setTo(0);
				imshow("Frame",imgPlot);
				keyPressed=waitKey();
				break;
			case 'w':
				// Run the watershed
				img.copyTo(imgPlot);
				split(imgPlot, channel); // Split out the channel for watershed masking
				calcWatershed(markers,img,segmented,fgMark,bgMark);
				segmentedPreview.setTo(0);
				img.copyTo(segmentedPreview,segmented);
				output=Mat::zeros(img.size(),CV_8UC3);
				output.setTo(fgMark,segmented);
				channel[1] = channel[1] - segmented;
				minEllipse = getEllipseFit(output);
				ellipse(channel[2], minEllipse, Scalar(0), 1, 8);
				channel[0] = channel[0] - markers*2;
				merge(channel, 3, imgPlot);
				//std::cout << "Angle Value: " << minEllipse.angle << " Degrees." << endl;
				imshow("Frame",imgPlot);
				keyPressed=waitKey();
				break;
			case 'q':
				return 0;
				break;
			default:
				keyPressed=waitKey();
				break;
		}
	}

	return 0;
}
