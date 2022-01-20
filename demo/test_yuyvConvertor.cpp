#include "CamFetcherV2.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

static inline int64_t timestamp_ms()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

const int height = 1280;
const int width = 720;

int main(int argc, char **argv)
{
	if(argc==0)
	{
		printf("============== USAGE ==============\n");
		printf("%s file_name [angle=0] [mirror=0] [loops]\n",argv[0]);
		printf("angle : 0-None, 1-90, 2-180, 3-270\n");
		printf("mirror: 0-None, 1-Hori, 2-Vert\n");
		printf("output to ./debugCamV2/*.png\n");
		printf("============= Example =============\n");
		printf("%s in.yuyv \n",argv[0]);
		printf("%s in.yuyv 1\n",argv[0]);
		printf("%s in.yuyv 1 1\n",argv[0]);
		printf("%s in.yuyv 1 1 100\n",argv[0]);
		printf("===================================\n");
	}

	int ang = 0;
	int mir = 0;
	int loops = 1;
	char* file_name = argv[1];
	if(argc>2) ang = ::atoi(argv[2]);
	if(argc>3) mir = ::atoi(argv[3]);
	if(argc>4) loops = ::atoi(argv[4]);

	char yuyvbuffer[height*width*2];
	std::ifstream fr(file_name, std::ios::binary|std::ios::ate);
	if (!fr.good())
	{
		printf("file open error!\n");
		return -5;
	}
	if (fr.tellg()!=height*width*2)
	{
		fr.close();
		printf("file size error!\n");
		return -6;
	}
	fr.seekg(0,std::ios::beg);
	fr.read(yuyvbuffer,height*width*2);
	fr.close();

	char buffer[height*width*3];
	char fname[512];

	rk_aarch64_driver::yuyvConvertor cvt;
	cvt.Init(height,width,ang,mir);

	int64_t t0,t1;
	auto ts = timestamp_ms();
	for (int i = 0;i<loops;i++)
		cvt.Cvt(yuyvbuffer,buffer);
	auto te = timestamp_ms();
	printf("time consume %ld ms for %d loops\n",te-ts,loops);
	
	cv::Mat frame(height,width,CV_8UC3,buffer);

	sprintf(fname,"./debugCamV2/%ld.png",te);
	cv::imwrite(fname,frame);

	return 0;
}