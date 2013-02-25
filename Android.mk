LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := Senator
LOCAL_SRC_FILES := senator.c
LOCAL_LDLIBS := -llog

include $(BUILD_EXECUTABLE)
