LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := Liberator
LOCAL_SRC_FILES := jni/liberator.c
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := liblog

include $(BUILD_EXECUTABLE)
