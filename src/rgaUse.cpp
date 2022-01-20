#include "rgaUse.h"
#include <rockchip_rga.h>
#include <linux/videodev2.h>
#include <opencv2/imgproc/imgproc.hpp>
namespace jules
{
	namespace rga
	{
		namespace cv
		{
			void Nv12ToCvBgr(unsigned char* data, int width, int height, ::cv::Mat& bgr_img)
			{
				RockchipRga *mRga = RgaCreate();
				if (!mRga)
				{
					printf("create rga handle failed !\n");
					abort();
				}
				mRga->ops->initCtx(mRga);

				if(!bgr_img.empty() || !bgr_img.isContinuous() || bgr_img.type()!=CV_8UC3 || bgr_img.cols!=width || bgr_img.rows!=height)
					bgr_img.create(height, width, CV_8UC3);

				mRga->ops->setSrcFormat(mRga, V4L2_PIX_FMT_NV12, width, height);
				mRga->ops->setDstFormat(mRga, V4L2_PIX_FMT_BGR24, width, height);
				mRga->ops->setSrcBufferPtr(mRga, data);
				mRga->ops->setDstBufferPtr(mRga, bgr_img.data);
				mRga->ops->go(mRga);

				RgaDestroy(mRga);
			}

		}


		void LetterBoxFrom640x480BGRto320x256RGB(const ::cv::Mat& src, ::cv::Mat& dst)
		{
			const int src_image_h = 480;
			const int src_image_w = 640;
			const int dest_image_h = 256;
			const int dest_image_w = 320;
			const int dest_image_h0 = src_image_h*dest_image_w/src_image_w;
			const int margin_image = dest_image_h-dest_image_h0;

			if(!src.isContinuous())
			{
				printf("cannot use rga: src Mat is not continuous !\n");
				abort();
			}

			if(CV_8UC3!=src.type())
			{
				printf("not realized: src Mat is not bgr. contact author to support !\n");
				abort();
			}

			if (src.cols!=src_image_w||src.rows!=src_image_h)
			{
				printf("Error input image size is not 640x480 !\n");
				abort();
			}

			RockchipRga *mRga = RgaCreate();
			if (!mRga)
			{
				printf("create rga handle failed !\n");
				abort();
			}
			mRga->ops->initCtx(mRga);

			dst.create(dest_image_h, dest_image_w, CV_8UC3);

			mRga->ops->setSrcFormat(mRga, V4L2_PIX_FMT_RGB24, src_image_w, src_image_h);
			mRga->ops->setDstFormat(mRga, V4L2_PIX_FMT_BGR24, dest_image_w, dest_image_h0);
			mRga->ops->setSrcBufferPtr(mRga, src.data);
			mRga->ops->setDstBufferPtr(mRga, dst.data);
			mRga->ops->go(mRga);

			RgaDestroy(mRga);

			memset(dst.data+(dest_image_h0*dest_image_w*3), -1, margin_image*dest_image_w*3);
		}

		void LetterBoxFrom1280x720BGRto640x384RGB(const ::cv::Mat& src, ::cv::Mat& dst)
		{
			const int src_image_h = 720;
			const int src_image_w = 1280;
			const int dest_image_h = 384;
			const int dest_image_w = 640;
			const int dest_image_h0 = src_image_h*dest_image_w/src_image_w;
			const int margin_image = dest_image_h-dest_image_h0;
			
			if(!src.isContinuous())
			{
				printf("cannot use rga: src Mat is not continuous !\n");
				abort();
			}

			if(CV_8UC3!=src.type())
			{
				printf("not realized: src Mat is not bgr. contact author to support !\n");
				abort();
			}

			if (src.cols!=src_image_w||src.rows!=src_image_h)
			{
				printf("Error input image size is not 640x480 !\n");
				abort();
			}

			RockchipRga *mRga = RgaCreate();
			if (!mRga)
			{
				printf("create rga handle failed !\n");
				abort();
			}
			mRga->ops->initCtx(mRga);

			dst.create(dest_image_h, dest_image_w, CV_8UC3);

			mRga->ops->setSrcFormat(mRga, V4L2_PIX_FMT_RGB24, src_image_w, src_image_h);
			mRga->ops->setDstFormat(mRga, V4L2_PIX_FMT_BGR24, dest_image_w, dest_image_h0);
			mRga->ops->setSrcBufferPtr(mRga, src.data);
			mRga->ops->setDstBufferPtr(mRga, dst.data);
			mRga->ops->go(mRga);

			RgaDestroy(mRga);

			memset(dst.data+(dest_image_h0*dest_image_w*3), -1, margin_image*dest_image_w*3);
		}

	}
}