#ifndef _JULES_USE_HEADER_
#define _JULES_USE_HEADER_

#include <opencv2/core/core.hpp>
#define JulesApi __attribute__((visibility ("default")))

namespace jules
{
	namespace rga
	{
		namespace cv
		{
			JulesApi void Nv12ToCvBgr(unsigned char* data, int width, int height, ::cv::Mat& bgr_img);
		}
		JulesApi void LetterBoxFrom640x480BGRto320x256RGB(const ::cv::Mat& src, ::cv::Mat& dst);

		JulesApi void LetterBoxFrom1280x720BGRto640x384RGB(const ::cv::Mat& src, ::cv::Mat& dst);
	}
}
#endif
