#include "CamFetcher.h"
#include <thread>
#include <chrono>
#include <fstream>

int main(int argc, char **argv)
{
	CamFetcherDDR camera;
	camera.Init(DEFAULT_CAM_HEIGHT,DEFAULT_CAM_WIDTH,CamFetcherFormatDDR_YUYV,0);

	char * NV12_buffer = new char[DEFAULT_CAM_HEIGHT*DEFAULT_CAM_WIDTH*2];


	camera.Start();
	while (0==camera.Get(NV12_buffer))
	{
		break;

		std::this_thread::sleep_for(std::chrono::microseconds(40*1000));
	}

	camera.Stop();

	char namebuf[512];
	sprintf(namebuf, "./cap-w%d-h%d-yuyv422.bin", DEFAULT_CAM_WIDTH, DEFAULT_CAM_HEIGHT);
	std::ofstream wr(namebuf, std::ios::binary);
	wr.write(NV12_buffer,DEFAULT_CAM_HEIGHT*DEFAULT_CAM_WIDTH*2);
	wr.close();

	delete NV12_buffer;
	return 0;
}