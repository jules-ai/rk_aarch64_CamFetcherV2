#include "RgaUserImpl.h"
#include "CamFetcherV2.h"
#include <stdio.h>
#include <stdlib.h>


RgaUserImpl::RgaUserImpl(int height, int width, int rotate, int mirror)
{
	int BUFFER_WIDTH_SRC;
	int BUFFER_HEIGHT_SRC;

	if(rotate==rk_aarch64_driver::CAM_ROTATE_90||rotate==rk_aarch64_driver::CAM_ROTATE_270)
	{
		BUFFER_WIDTH_SRC = height;
		BUFFER_HEIGHT_SRC = width;
	}
	else
	{
		BUFFER_WIDTH_SRC = width;
		BUFFER_HEIGHT_SRC = height;
	}

	int BUFFER_SIZE_SRC = BUFFER_WIDTH_SRC*BUFFER_HEIGHT_SRC*2;//YUYV
	int BUFFER_WIDTH_DEST = width;
	int BUFFER_HEIGHT_DEST = height;
	int BUFFER_SIZE_DEST = BUFFER_WIDTH_DEST*BUFFER_HEIGHT_DEST*3;

	if (rotate!=0 && mirror!=0)
		inner_buffer = new char[height*width*3];
	else
		inner_buffer = nullptr;

	if (rotate)
	{
		mRga_ROTA = RgaCreate();
		if (!mRga_ROTA)
		{
			printf("create rga failed !\n");
			abort();
		}
		mRga_ROTA->ops->initCtx(mRga_ROTA);
		mRga_ROTA->ops->setSrcFormat(mRga_ROTA, V4L2_PIX_FMT_YUYV, BUFFER_WIDTH_SRC, BUFFER_HEIGHT_SRC);
		mRga_ROTA->ops->setDstFormat(mRga_ROTA, V4L2_PIX_FMT_BGR24, BUFFER_WIDTH_DEST, BUFFER_HEIGHT_DEST);
		mRga_ROTA->ops->setRotate(mRga_ROTA, RgaRotate(rotate));
		mRga_ROTA->ops->setDstBufferPtr(mRga_ROTA, (unsigned char *)inner_buffer);
	}
	else
		mRga_ROTA = nullptr;

	if (mirror)
	{
		mRga_FLIP = RgaCreate();
		if (!mRga_FLIP)
		{
			printf("create rga failed !\n");
			abort();
		}
		mRga_FLIP->ops->initCtx(mRga_FLIP);
		if(rotate)
			mRga_FLIP->ops->setSrcFormat(mRga_FLIP, V4L2_PIX_FMT_BGR24, BUFFER_WIDTH_DEST, BUFFER_HEIGHT_DEST);
		else
			mRga_FLIP->ops->setSrcFormat(mRga_FLIP, V4L2_PIX_FMT_YUYV, BUFFER_WIDTH_DEST, BUFFER_HEIGHT_DEST);

		mRga_FLIP->ops->setDstFormat(mRga_FLIP, V4L2_PIX_FMT_BGR24, BUFFER_WIDTH_DEST, BUFFER_HEIGHT_DEST);
		mRga_FLIP->ops->setRotate(mRga_FLIP, RgaRotate(mirror+3));
		mRga_FLIP->ops->setSrcBufferPtr(mRga_FLIP, (unsigned char *)inner_buffer);
	}
	else
		mRga_FLIP = nullptr;

}

RgaUserImpl::~RgaUserImpl()
{
	if (mRga_ROTA)
	{
		RgaDestroy(mRga_ROTA);
	}

	if (mRga_FLIP)
	{
		RgaDestroy(mRga_FLIP);
	}

	if (inner_buffer)
	{
		delete []inner_buffer;
	}
}

int RgaUserImpl::Get(char* src, char* dest)
{
	if (!src || !dest)
		return -1;

	if(mRga_ROTA!=nullptr && mRga_FLIP!=nullptr)
	{
		mRga_ROTA->ops->setSrcBufferPtr(mRga_ROTA, (unsigned char *)src);
		mRga_FLIP->ops->setDstBufferPtr(mRga_FLIP, (unsigned char *)dest);

		mRga_ROTA->ops->go(mRga_ROTA);
		mRga_FLIP->ops->go(mRga_FLIP);

		return 0;
	}

	if(mRga_ROTA)
	{
		mRga_ROTA->ops->setSrcBufferPtr(mRga_ROTA, (unsigned char *)src);
		mRga_ROTA->ops->setDstBufferPtr(mRga_ROTA, (unsigned char *)dest);
		mRga_ROTA->ops->go(mRga_ROTA);
		return 0;
	}

	if(mRga_FLIP)
	{
		mRga_FLIP->ops->setSrcBufferPtr(mRga_FLIP, (unsigned char *)src);
		mRga_FLIP->ops->setDstBufferPtr(mRga_FLIP, (unsigned char *)dest);
		mRga_FLIP->ops->go(mRga_FLIP);
		return 0;
	}

	return -10;
}

