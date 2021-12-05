LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := fm-bridge
LOCAL_SRC_FILES := \
        main.c \

LOCAL_C_INCLUDES := \

LOCAL_SHARED_LIBRARIES := \

LOCAL_CFLAGS += -O3 --std=c99
LOCAL_VENDOR_MODULE := true

include $(BUILD_EXECUTABLE)
