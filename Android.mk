LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := Senator
LOCAL_SRC_FILES := jni/senator.c
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := liblog

include $(BUILD_EXECUTABLE)
