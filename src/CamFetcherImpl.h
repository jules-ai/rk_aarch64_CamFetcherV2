#ifndef _VIDEO_FETCHER_IMPL_CW_HEADER_
#define _VIDEO_FETCHER_IMPL_CW_HEADER_

#include "RgaUserImpl.h"

enum CamFetcherFormatDDR
{
	CamFetcherFormatDDR_NV12 = 842094158,
	CamFetcherFormatDDR_NV21 = 825382478,
	CamFetcherFormatDDR_GREY = 1497715271,
	CamFetcherFormatDDR_MJPG = 1196444237,
	CamFetcherFormatDDR_YUYV = 1448695129,
};

typedef struct
{
	void* start;
	int   length;
} BUFTYPE;

class CamFetcherImpl
{
public:
	CamFetcherImpl(int height, int width, int format, int device_id, int fps=25);
	~CamFetcherImpl();

	int Init(char * device_path = nullptr);
	int Start();
	int Get(char * buffer);
	int Stop();

private:
	int m_video_fd;
	int m_device_id;
	int m_height;
	int m_width;
	int m_format;
	int m_initialized;
	int m_buffer_count;
	int m_fps;

	BUFTYPE* m_user_buffer;
};


#endif