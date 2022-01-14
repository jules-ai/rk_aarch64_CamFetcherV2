#include "CamFetcherV2.h"
#include "CamFetcherImpl.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

namespace rk_aarch64_driver
{
	CamFetcher::CamFetcher()
	{
		inner_handle = nullptr;
		outter_handle = nullptr;
		mid_handle = nullptr;
	}

	CamFetcher::~CamFetcher()
	{
		if (inner_handle)
		{
			delete (CamFetcherImpl*)inner_handle;
		}
		if (outter_handle)
		{
			delete (RgaUserImpl*)outter_handle;
		}
		if (mid_handle)
		{
			delete [](char*)mid_handle;
		}
	}


	int CamFetcher::Init(int device_id /*= 0*/, int fps /*= 25*/, int height /*= 640*/, int width /*= 480*/, int rotate /*= CAM_ROTATE_NONE */, int mirror /*= CAM_FLIP_NONE*/)
	{
		if (nullptr!=inner_handle)
		{
			printf("Init Error: VideoFetcher has already ben initialized once!\n");
			return -1;
		}
		int format = CamFetcherFormatDDR_YUYV;

		if(rotate==CAM_ROTATE_90||rotate==CAM_ROTATE_270)
			inner_handle = new CamFetcherImpl(width,height,format,device_id,fps*2);
		else
			inner_handle = new CamFetcherImpl(height,width,format,device_id,fps);

		int ret = ((CamFetcherImpl*)inner_handle)->Init();

		if (ret)
		{
			printf("Init Error: CamFetcherImpl init failed!\n");
			return ret;
		}

		outter_handle = new RgaUserImpl(height,width,rotate,mirror);

		mid_handle = new char [height*width*2];

		return 0;
	}

	int CamFetcher::Start()
	{
		if (nullptr==inner_handle)
		{
			printf("Start Error: VideoFetcher has not been initialized yet!\n");
			return -2;
		}

		return ((CamFetcherImpl*)inner_handle)->Start();
	}

	int CamFetcher::Get(char * buffer)
	{
		if (nullptr==inner_handle)
		{
			printf("Start Error: VideoFetcher has not been initialized yet!\n");
			return -2;
		}
		int ret = ((CamFetcherImpl*)inner_handle)->Get((char *)mid_handle);
		if(ret)
		{
			printf("Get Error: CamFetcherImpl Get failed[%d]!\n",ret);
			return ret;
		}


		ret = ((RgaUserImpl*)outter_handle)->Get((char *)mid_handle,buffer);
		if(ret)
		{
			printf("Get Error: RgaUserImpl Get failed[%d]!\n",ret);
			return ret;
		}

		return 0;
	}

	int CamFetcher::Stop()
	{
		if (nullptr==inner_handle)
		{
			printf("Start Error: VideoFetcher has not been initialized yet!\n");
			return -2;
		}

		return ((CamFetcherImpl*)inner_handle)->Stop();
	}
}