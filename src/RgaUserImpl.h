#ifndef _RGA_USER_IMPL_CW_HEADER_
#define _RGA_USER_IMPL_CW_HEADER_

#include <rockchip_rga.h>
#include <linux/videodev2.h>

class RgaUserImpl
{
public:
	RgaUserImpl(int height, int width, int rotate, int mirror, int src = 0);
	~RgaUserImpl();

	int Get(char* src, char* dest);

private:
	RockchipRga *mRga_ROTA;
	RockchipRga *mRga_FLIP;
	char* inner_buffer;
};


#endif