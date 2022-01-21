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

const int height = 640;
const int width = 480;

void irthr(int ang, int mir, int loops)
{
	rk_aarch64_driver::CamFetcher cam;
	cam.Init(1,25,height,width,ang,mir);
	cam.Start();

	char buffer[height*width*3];
	char fname[512];

	for (int i = 0;i<loops;i++)
	{
		cam.Get(buffer);
		cv::Mat frame(height,width,CV_8UC3,buffer);

		sprintf(fname,"./debugCamV2/%lld_nir.png",timestamp_ms());
		cv::imwrite(fname,frame);
	}

	cam.Stop();
}

int main(int argc, char **argv)
{
	if(argc==0)
	{
		printf("============== USAGE ==============\n");
		printf("%s [angle=0] [mirror=0] [loops]\n",argv[0]);
		printf("angle : 0-None, 1-90, 2-180, 3-270\n");
		printf("mirror: 0-None, 1-Hori, 2-Vert\n");
		printf("output to ./debugCamV2/*.png\n");
		printf("============= Example =============\n");
		printf("%s\n",argv[0]);
		printf("%s 1\n",argv[0]);
		printf("%s 1 1\n",argv[0]);
		printf("%s 1 1 100\n",argv[0]);
		printf("===================================\n");
	}

	int ang = 0;
	int mir = 0;
	int loops = 1;
	if(argc>1) ang = ::atoi(argv[1]);
	if(argc>2) mir = ::atoi(argv[2]);
	if(argc>3) loops = ::atoi(argv[3]);

	char buffer[height*width*3];
	char fname[512];

	std::thread t1(irthr, ang, mir, loops);

	rk_aarch64_driver::CamFetcher cam;
	cam.Init(0,25,height,width,ang,mir);
	cam.Start();


	for (int i = 0;i<loops;i++)
	{
		cam.Get(buffer);
		cv::Mat frame(height,width,CV_8UC3,buffer);

		sprintf(fname,"./debugCamV2/%lld_vis.png",timestamp_ms());
		cv::imwrite(fname,frame);
	}

	cam.Stop();
	t1.join();

	return 0;
}