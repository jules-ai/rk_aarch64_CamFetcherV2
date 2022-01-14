/*
 * CamFetcherV2 -- Face Pad Cam hard-Decode lib.
 * Copyright (C) 2021 - 2022, CloudWalk Technology Co., Ltd..
 * All Rights Reserved.
 *
 * Author: Jules (zhulei@cloudwalk.com)
 * Developed at CloudWalk (ShangHai China).
 */

#ifndef _VIDEO_CAM_FETCHER_CW_HEADER_
#define _VIDEO_CAM_FETCHER_CW_HEADER_

#define FetcherApi __attribute__((visibility ("default")))

#ifdef __cplusplus
extern "C" {
#endif

	namespace rk_aarch64_driver
	{
		typedef enum _CamRotation {
			CAM_ROTATE_NONE = 0,
			CAM_ROTATE_90,
			CAM_ROTATE_180,
			CAM_ROTATE_270,
		} CamRotation;

		typedef enum _RgaRotate {
			CAM_FLIP_NONE = 0,
			CAM_FLIP_HOR,
			CAM_FLIP_VER,
		} RgaRotate;

		class FetcherApi CamFetcher
		{
		public:
			CamFetcher();
			~CamFetcher();

			// 以下接口返回0为正确执行，否则请查看屏幕所打印的错误信息
			int Init(int device_id = 0, int fps = 25, int height = 640, int width = 480, int rotate = CAM_ROTATE_NONE , int mirror = CAM_FLIP_NONE);
			int Start();
			int Get(char * buffer);
			int Stop();

		private:
			void* inner_handle;
			void* outter_handle;
			void* mid_handle;
		};

	}

#ifdef __cplusplus
}
#endif

#endif