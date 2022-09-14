#include <cstring>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <thread>
#include <chrono>
#include <unistd.h>
#include "rgaUse.h"

#define BUFFER_WIDTH_SRC 1280
#define BUFFER_HEIGHT_SRC 720

int main(int argc, char* argv[])
{
	if (argc!=2) return -11;
	std::string fp = argv[1];
	std::ifstream rd(fp, std::ios::binary);
	if(!rd.good()) return -2;

	char buffer[BUFFER_WIDTH_SRC*BUFFER_HEIGHT_SRC*3/2];
	rd.read(buffer,BUFFER_WIDTH_SRC*BUFFER_HEIGHT_SRC*3/2);

	cv::Mat img;
	jules::rga::cv::Nv12ToCvBgr((unsigned char*)buffer, BUFFER_WIDTH_SRC, BUFFER_HEIGHT_SRC, img);
	cv::imwrite("out.png",img);

	return 0;
}