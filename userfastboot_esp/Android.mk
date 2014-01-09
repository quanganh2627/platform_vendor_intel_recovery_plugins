LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libufb_esp
LOCAL_SRC_FILES := ufb_esp.c
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_C_INCLUDES := bootable/userfastboot \
                    system/core/fs_mgr/include \
                    bootable/recovery
LOCAL_CFLAGS := -Wall -Werror -Wno-unused-parameter
ifneq ($(DROIDBOOT_NO_GUI),true)
LOCAL_CFLAGS += -DUSE_GUI
endif
include $(BUILD_STATIC_LIBRARY)

