LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := updater_esp.c
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := bootable/recovery bootable/iago/include
LOCAL_MODULE := libupdater_esp
LOCAL_CFLAGS := -Wall -Werror
include $(BUILD_STATIC_LIBRARY)
