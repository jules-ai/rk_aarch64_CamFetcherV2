#include <rockchip_rga.h>
#include <linux/videodev2.h>
#include <cstring>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <thread>
#include <chrono>
#include <unistd.h>


#define RBS_ALIGN(x, a) (((x)+(a)-1)&~((a)-1))

#define BUFFER_WIDTH_SRC 1280
#define BUFFER_HEIGHT_SRC 720
#define BUFFER_SIZE_SRC BUFFER_WIDTH_SRC*BUFFER_HEIGHT_SRC*2
#define BUFFER_WIDTH_DEST 480
#define BUFFER_HEIGHT_DEST 640
#define BUFFER_SIZE_DEST BUFFER_WIDTH_DEST*BUFFER_HEIGHT_DEST*3

unsigned char *srcBuffer = NULL;
unsigned char *dstBuffer = NULL;
unsigned char *interBuffer = NULL;
unsigned char *dstBufferIR = NULL;

#define NANOTIME_PER_MSECOND 1000000L

int64_t timestamp_ms_fdg()
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
		std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());  

	auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());  

	return tmp.count();  
}


//static inline void Soft_BGRMH(unsigned char *src, unsigned char *dst, int width, int height, int chn)
//{
//	int i, j, c;
//	for(j = 0; j < height; j++) 
//	{
//		for(i = 0; i < width; i++) 
//		{
//			for(c = 0; c < chn; c++) 
//			{
//				*(dst + chn * (j*width+i) + c)=*(src + chn * ((j+1) * width - i) + c);
//			}
//		}
//	}
//}


void rga_copy() {
	RockchipRga *mRga = RgaCreate();
	if (!mRga) {
		printf("create rga failed !\n");
		abort();
	}
	mRga->ops->initCtx(mRga);
	long long b1,e1,b2,e2,b3,e3,b4,e4;


	b1 = timestamp_ms_fdg();
	mRga->ops->setSrcFormat(mRga, V4L2_PIX_FMT_YUYV, BUFFER_WIDTH_SRC, BUFFER_HEIGHT_SRC);
	mRga->ops->setDstFormat(mRga, V4L2_PIX_FMT_BGR24, BUFFER_WIDTH_DEST, BUFFER_HEIGHT_DEST);
	mRga->ops->setRotate(mRga, RGA_ROTATE_270);
	mRga->ops->setSrcBufferPtr(mRga, srcBuffer);
	mRga->ops->setDstBufferPtr(mRga, interBuffer);
	mRga->ops->go(mRga);
	e1= timestamp_ms_fdg();

	b2= timestamp_ms_fdg();
	//Soft_BGRMH(interBuffer,dstBuffer,BUFFER_WIDTH_DEST,BUFFER_HEIGHT_DEST,3);
	mRga->ops->setSrcFormat(mRga, V4L2_PIX_FMT_BGR24, BUFFER_WIDTH_DEST, BUFFER_HEIGHT_DEST);
	mRga->ops->setDstFormat(mRga, V4L2_PIX_FMT_BGR24, BUFFER_WIDTH_DEST, BUFFER_HEIGHT_DEST);
	mRga->ops->setRotate(mRga, RGA_ROTATE_HFLIP);
	mRga->ops->setSrcBufferPtr(mRga, interBuffer);
	mRga->ops->setDstBufferPtr(mRga, dstBuffer);
	mRga->ops->go(mRga);
	e2= timestamp_ms_fdg();


	//b3= timestamp_ms_fdg();
	//mRga->ops->setSrcFormat(mRga, V4L2_PIX_FMT_NV12, BUFFER_WIDTH_SRC, BUFFER_HEIGHT_SRC);
	//mRga->ops->setDstFormat(mRga, V4L2_PIX_FMT_NV12, BUFFER_WIDTH_DEST, BUFFER_HEIGHT_DEST);
	//mRga->ops->setRotate(mRga, RGA_ROTATE_270);
	//mRga->ops->setSrcBufferPtr(mRga, srcBuffer);
	//mRga->ops->setDstBufferPtr(mRga, interBuffer);
	//mRga->ops->go(mRga);
	//e3= timestamp_ms_fdg();


	//b4= timestamp_ms_fdg();
	////Soft_BGRMH(interBuffer,dstBufferIR,BUFFER_WIDTH_DEST,BUFFER_HEIGHT_DEST,1);
	//mRga->ops->setSrcFormat(mRga, V4L2_PIX_FMT_NV12, BUFFER_WIDTH_DEST, BUFFER_HEIGHT_DEST);
	//mRga->ops->setDstFormat(mRga, V4L2_PIX_FMT_NV12, BUFFER_WIDTH_DEST, BUFFER_HEIGHT_DEST);
	//mRga->ops->setRotate(mRga, RGA_ROTATE_HFLIP);
	//mRga->ops->setSrcBufferPtr(mRga, interBuffer);
	//mRga->ops->setDstBufferPtr(mRga, dstBufferIR);
	//mRga->ops->go(mRga);
	//e4= timestamp_ms_fdg();

	printf("%lld, %lld, %lld, %lld\n",(e1-b1),(e2-b2),(e3-b3),(e4-b4));

	RgaDestroy(mRga);

}

int main(int argc, char* argv[])
{
	int loops = 1000;
	if(argc>1) loops = atoi(argv[1]);
	srcBuffer = new unsigned char[BUFFER_SIZE_SRC];
	interBuffer = new unsigned char[BUFFER_SIZE_DEST];
	dstBuffer = new unsigned char[BUFFER_SIZE_DEST];
	dstBufferIR = new unsigned char[BUFFER_SIZE_DEST/3];

	std::ifstream yuv_reader("tmp_test.yuv", std::ios::binary);
	if (!yuv_reader.good())
	{
		printf("yuv_reader init failed!\n");
		return -1;
	}
	yuv_reader.read((char*)srcBuffer, BUFFER_SIZE_SRC);
	yuv_reader.close();

	long long begintime = timestamp_ms_fdg();

	for (int i = 0; i < loops; i++)
	{
		usleep(35000);
		rga_copy();
	}
	long long endtime = timestamp_ms_fdg();
	printf("RGA copy time : %lld ms\n", endtime-begintime);

	cv::Mat img(BUFFER_HEIGHT_DEST,BUFFER_WIDTH_DEST,CV_8UC3,dstBuffer);
	cv::imwrite("tmp_test_out.png",img);

	delete srcBuffer;
	delete dstBuffer;
	delete interBuffer;
	delete dstBufferIR;
	return 0;
}