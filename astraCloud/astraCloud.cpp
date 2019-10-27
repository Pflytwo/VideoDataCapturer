
/*************************
OpenNI2 Point-cloud collect program
auth:rlanffy
*************************/

#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <direct.h>
#include "OpenNI.h"
//#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include <iostream>
#include <omp.h>

//��ȡ��������
#include <conio.h>
#include <Windows.h>

#pragma comment(lib, "opencv_imgproc241d.lib")
#pragma comment(lib, "opencv_highgui241d.lib")
#pragma comment(lib, "opencv_core241d.lib")


using namespace std;
using namespace cv;
using namespace openni;

#define SAMPLE_READ_WAIT_TIMEOUT 5000 //2000ms


/*ˮƽ����*/
void hMirrorTrans(const Mat& src, Mat& dst)
{
	dst.create(src.rows, src.cols, src.type());

	int rows = src.rows;
	int cols = src.cols;

	switch (src.channels())
	{
	case 1:   //1ͨ���������ͼ��
		const uchar * origal;
		uchar* p;
		for (int i = 0; i < rows; i++) {
			origal = src.ptr<uchar>(i);
			p = dst.ptr<uchar>(i);
			for (int j = 0; j < cols; j++) {
				p[j] = origal[cols - 1 - j];
			}
		}
		break;
	case 3:   //3ͨ�������ɫͼ��
		const Vec3b * origal3;
		Vec3b* p3;
		for (int i = 0; i < rows; i++) {
			origal3 = src.ptr<Vec3b>(i);
			p3 = dst.ptr<Vec3b>(i);
			for (int j = 0; j < cols; j++) {
				p3[j] = origal3[cols - 1 - j];
			}
		}
		break;
	default:
		break;
	}

}

int main(int argc, char** argv)
{

	Status result = STATUS_OK;
	int high = 640;
	int weight = 480;

	//OpenNI2 Frame
	VideoFrameRef oniDepthImg;
	VideoFrameRef oniColorImg;

	//OpenCV
	//IplImage* imgDepth16u = cvCreateImage(cvSize(640, 480), IPL_DEPTH_16U, 1);
	//IplImage* imgRGB8u = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
	//IplImage* depthShow = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
	//IplImage* imageShow = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);


	char key = 0;

	result = OpenNI::initialize();
	if (result != STATUS_OK)
	{
		printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
		return 1;
	}

	openni::Array<openni::DeviceInfo> deviceList;
	openni::OpenNI::enumerateDevices(&deviceList);//��ȡ���豸�б�
	int cc = deviceList.getSize();
	printf("device count : %d ok\n", cc);
	if (cc <= 0)
	{
		return -1;
	}
	const char* deviceUri;//�豸��ʶ
	deviceUri = deviceList[0].getUri();
	printf("device uri : %s ok\n", deviceUri);

	Device device;
	result = device.open(deviceUri);

	// set depth video mode
	VideoMode modeDepth;
	// �ֱ���
	modeDepth.setResolution(high, weight);
	// ÿ��30֡
	modeDepth.setFps(30);
	// ���ظ�ʽ
	modeDepth.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);

	VideoStream oniDepthStream;  // create depth stream
	oniDepthStream.setVideoMode(modeDepth);
	oniDepthStream.create(device, openni::SENSOR_DEPTH);
	result = oniDepthStream.start();  // start depth stream
	printf("start depth and video :%d\n", result);

	// set color video mode
	VideoMode modeColor;
	// �ֱ���
	modeColor.setResolution(320,240);
	// ÿ��30֡
	modeColor.setFps(30);
	// ���ظ�ʽ
	modeColor.setPixelFormat(PIXEL_FORMAT_RGB888);
	VideoStream oniColorStream;  // create COLOR stream
	oniColorStream.setVideoMode(modeColor);
	oniColorStream.create(device, openni::SENSOR_COLOR);
	result = oniColorStream.start();  // start depth stream
	printf("start color :%d\n", result);

	// ͼ��ģʽע��,��ɫͼ�����ͼ����
	if (device.isImageRegistrationModeSupported(IMAGE_REGISTRATION_DEPTH_TO_COLOR))
	{
		device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	}

	string depthWinName = "depth";
	string imageWinName = "image";
	cv::namedWindow(depthWinName, 1);
	//cv::namedWindow(imageWinName, 1);

	while ((key != 27)) // ESC
	{
		int changedStreamDummy;
		VideoStream* pStream = &oniDepthStream;
		OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);
		oniDepthStream.readFrame(&oniDepthImg);

		// ���������ת����OpenCV��ʽ
		const Mat mImageDepth(oniDepthImg.getHeight(), oniDepthImg.getWidth(), CV_16UC1, (void*)oniDepthImg.getData());
		// Ϊ�������ͼ����ʾ�ĸ�������һЩ����CV_16UC1 ==> CV_8U��ʽ
		Mat mScaledDepth, hScaledDepth;
		mImageDepth.convertTo(mScaledDepth, CV_8U, 255.0 / oniDepthStream.getMaxPixelValue());
		// ��ʾ�����ͼ��
		hMirrorTrans(mScaledDepth, hScaledDepth);
		resize(hScaledDepth, hScaledDepth, Size(640, 480));
		cv::imshow(depthWinName, hScaledDepth);

		/*
		int changedColorStream;
		VideoStream* colorStream = &oniColorStream;
		OpenNI::waitForAnyStream(&colorStream, 1, &changedColorStream, SAMPLE_READ_WAIT_TIMEOUT);
		oniColorStream.readFrame(&oniColorImg);
		// ͬ���Ľ���ɫͼ������ת����OpenCV��ʽ
		const Mat mImageRGB(oniColorImg.getHeight(), oniColorImg.getWidth(), CV_8UC3, (void*)oniColorImg.getData());
		// ���Ƚ�RGB��ʽת��ΪBGR��ʽ
		Mat cImageBGR, bImageBGR, hImageBGR;
		//cvtColor(mImageRGB, cImageBGR, CV_RGB2BGR);
		cv::cvtColor(mImageRGB, cImageBGR, CV_RGB2BGR);
		//ˮƽ�������ͼ
		hMirrorTrans(cImageBGR, hImageBGR);
		resize(hImageBGR, hImageBGR, Size(640, 480));
		// Ȼ����ʾ��ɫͼ��
		cv::imshow(imageWinName, hImageBGR);
		*/
		
		//key = cvWaitKey(20);
		key = cv::waitKey(20);

		if (key == 13) // Enter
		{
			DepthPixel* pDepth = (DepthPixel*)oniDepthImg.getData();
			ofstream outfile;
			string tmpfilename = "CloudData.xyz";
			outfile.open(tmpfilename);
			float x = 0.0, y = 0.0, z = 0.0, depthv = 0.0;
			float i, j, k;
			for (k = 0; k < 1; k++)
			{
				for (i = 0; i < oniDepthImg.getHeight(); i++)
				{
					for (j = 0; j < oniDepthImg.getWidth(); j++)
					{
						int k = i;
						int m = j;
						depthv = pDepth[k * oniDepthImg.getWidth() + m];
						CoordinateConverter::convertDepthToWorld(oniDepthStream, j, i, depthv, &x, &y, &z);
						outfile << x << " " << y << " " << z << std::endl;
					}
				}
			}
			outfile.close();
			printf("write file done.\n");
		}
	}

	//destroy
	oniDepthStream.destroy();  //OpenNI2 destroy
	device.close();
	OpenNI::shutdown();
	return 0;
}
