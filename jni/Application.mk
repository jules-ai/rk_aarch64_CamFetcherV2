NDK_TOOLCHAIN_VERSION := 4.9
APP_STL := gnustl_static
APP_CPPFLAGS +=-std=c++11 -frtti -fexceptions -pie -fPIE

APP_ABI := arm64-v8a 

APP_PLATFORM := android-21
APP_OPTIM:= release


# 编exe时开启
ifeq ($(PRODUCT_TYPE),EXE)
	APP_PIE := true 
endif