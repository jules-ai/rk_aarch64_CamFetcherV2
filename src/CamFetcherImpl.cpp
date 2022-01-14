#include "CamFetcherImpl.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

CamFetcherImpl::CamFetcherImpl(int height, int width, int format, int device_id, int fps)
{
	m_device_id = (device_id<10 && device_id>=0)?device_id:3;
	m_video_fd = -1;
	m_height = height;
	m_width = width;
	m_format = format;
	m_fps = fps;
	m_initialized = 0;
	m_buffer_count = 0;
	m_user_buffer = nullptr;
}

CamFetcherImpl::~CamFetcherImpl()
{
	int i;

	for (i = 0; i < m_buffer_count; i++)
	{
		if (-1 == munmap(m_user_buffer[i].start, m_user_buffer[i].length))
		{
			perror("munmap error!");
			exit(EXIT_FAILURE);
		}
	}

	free(m_user_buffer);

	if (-1 == close(m_video_fd))
	{
		perror("Fail to close fd");
		exit(EXIT_FAILURE);
	}
}

int CamFetcherImpl::Init(char * device_path /*= nullptr */)
{
	if (m_initialized==1)
	{
		printf("Init error: Already initialized once!");
		return -100;
	}

	//// open device
	char device_name[64] = {0};
	snprintf(device_name, sizeof(device_name), "/dev/video%d", m_device_id);
	m_video_fd = open(device_name, O_RDWR);
	if (m_video_fd < 0 && device_path!=nullptr && strlen(device_path)<=64)	m_video_fd = open(device_path, O_RDWR);

	if (m_video_fd < 0)
	{
		perror("open device error!");
		return -1;
	}

	//// get device capabilities
	struct v4l2_capability cap;
	int                    ret;
	ret = ioctl(m_video_fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0)
	{
		perror("FAIL to ioctl VIDIOC_QUERYCAP");
		return (-2);
	}
	if (!(cap.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE))
	{
		printf("The Current device is not a video capture device\n");
		return (-3);
	}
	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		printf("The Current device does not support streaming i/o\n");
		return (-4);
	}


	//// get formats supported
	struct v4l2_fmtdesc    fmt;
	std::vector<struct v4l2_fmtdesc> vec_allowed_fmts;
	bool format_allowed = false;

	/*video fromat suppoted by device*/
	memset(&fmt, 0, sizeof(fmt));
	fmt.index = 0;
	fmt.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while ((ret = ioctl(m_video_fd, VIDIOC_ENUM_FMT, &fmt)) == 0)
	{
		fmt.index++;
		if (fmt.pixelformat == m_format) format_allowed=true;
		vec_allowed_fmts.push_back(fmt);
	}
	if (!format_allowed)
	{
		printf("Format not support! Please refer to the list of formats supported:\n");
		for (auto allowed_fmt:vec_allowed_fmts)
		{
			printf("Format:\"%c%c%c%c\", description: '%s'\n", allowed_fmt.pixelformat & 0xff, (allowed_fmt.pixelformat >> 8) & 0xff,
				(allowed_fmt.pixelformat >> 16) & 0xff, (allowed_fmt.pixelformat >> 24) & 0xff, allowed_fmt.description);
		}
		return (-5);
	}


	//// set FPS
	struct v4l2_format     stream_fmt;
	stream_fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	stream_fmt.fmt.pix.width       = m_width;
	stream_fmt.fmt.pix.height      = m_height;
	stream_fmt.fmt.pix.pixelformat = m_format;
	stream_fmt.fmt.pix.field       = V4L2_FIELD_ANY;

	if (-1 == ioctl(m_video_fd, VIDIOC_S_FMT, &stream_fmt))
	{
		perror("VIDIOC_S_FMT Fail to ioctl");
		return (-6);
	}

	struct v4l2_streamparm param;
	memset(&param, 0, sizeof(param));
	param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	param.parm.capture.timeperframe.denominator = m_fps;
	ret = ioctl(m_video_fd, VIDIOC_S_PARM, &param);
	if (ret < 0)
	{
		perror("Warning: VIDIOC_S_PARM Fail to ioctl");
		return (-7);
	}

	//// alloc memory
	struct v4l2_requestbuffers reqbuf;

	bzero(&reqbuf, sizeof(reqbuf));
	reqbuf.count  = 3; //缓存区个数
	reqbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE; //视频捕获模式
	reqbuf.memory = V4L2_MEMORY_MMAP; //内存区的使用方式

	ret = ioctl(m_video_fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0)
	{
		perror("Warning: VIDIOC_REQBUFS Fail to ioctl");
		return (-8);
	}
	m_buffer_count = reqbuf.count;
	printf("Got buffer count = %d\n", m_buffer_count);

	m_user_buffer = (BUFTYPE*)calloc(reqbuf.count, sizeof(BUFTYPE));
	if (m_user_buffer == NULL)
	{
		fprintf(stderr, "Out of memory\n");
		return -9;
	}
	printf("user_buf = %p\n", m_user_buffer);

	for (unsigned int i = 0; i < reqbuf.count; i++)
	{
		struct v4l2_buffer buf;

		bzero(&buf, sizeof(buf));
		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index  = i;
		ret = ioctl(m_video_fd, VIDIOC_QUERYBUF, &buf);
		if (ret<0)
		{
			perror("Fail to ioctl : VIDIOC_QUERYBUF");
			return -10;
		}
		m_user_buffer[i].length = buf.length;
		m_user_buffer[i].start  = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, m_video_fd, buf.m.offset);
		if (MAP_FAILED == m_user_buffer[i].start)
		{
			perror("Fail to mmap");
			return -11;
		}

		ret = ioctl(m_video_fd, VIDIOC_QBUF, &buf);
		if (ret<0)
		{
			perror("Fail to ioctl 'VIDIOC_QBUF'");
			return -12;
		}
	}

	printf("Initialization finished successfully.\n");
	m_initialized = 1;
	return 0;
}

int CamFetcherImpl::Start()
{
	enum v4l2_buf_type  type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1==ioctl(m_video_fd, VIDIOC_STREAMON, &type))
	{
		perror("Fail to ioctl 'VIDIOC_STREAMON'");
		return -13;
	}
	return 0;
}

int CamFetcherImpl::Get(char * buffer)
{
	if (m_initialized==0)
	{
		printf("Get error: Not been initialized yet!");
		return -100;
	}

	struct v4l2_buffer buf;

	bzero(&buf, sizeof(buf));
	buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (-1 == ioctl(m_video_fd, VIDIOC_DQBUF, &buf))
	{
		perror("Fail to ioctl 'VIDIOC_DQBUF'");
		return -1;
	}
	assert(buf.index < m_buffer_count);

	memcpy(buffer, m_user_buffer[buf.index].start, m_user_buffer[buf.index].length);

	if (-1 == ioctl(m_video_fd, VIDIOC_QBUF, &buf))
	{
		perror("Fail to ioctl 'VIDIOC_QBUF'");
		return -2;
	}

	return 0;
}

int CamFetcherImpl::Stop()
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(m_video_fd, VIDIOC_STREAMOFF, &type))
	{
		perror("Fail to ioctl 'VIDIOC_STREAMOFF'");
		return -1;
	}

	return 0;
}

