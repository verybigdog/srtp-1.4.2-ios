LOCAL_PATH 			:= $(call my-dir)

XRTP_SRC_DIR		:= src
XRTP_PORT_DIR		:= ports/arm/linuxu/src
XRTP_INC_DIR		:= $(LOCAL_PATH)/include
XRTP_PORT_INC_DIR	:= $(LOCAL_PATH)/ports/arm/linuxu/include
XRTP_SRTP_DIR		:= 

SRTP_ON				:= 1

include $(CLEAR_VARS)

LOCAL_MODULE    :=	srtp
LOCAL_CFLAGS	:= 	-I$(XRTP_INC_DIR) \
					-I$(XRTP_PORT_INC_DIR) \
					-DNDK_COMPATIBLE
ifeq ($(SRTP_ON), 1)
LOCAL_CFLAGS	+=	-I$(LOCAL_PATH)/include \
					-I$(LOCAL_PATH)/crypto/include
endif					
					
ifeq ($(SRTP_ON), 1)
LOCAL_SRC_FILES += 	crypto/cipher/cipher.c \
					crypto/cipher/null_cipher.c \
					crypto/cipher/aes.c \
					crypto/cipher/aes_icm.c \
					crypto/cipher/aes_cbc.c \
					crypto/hash/null_auth.c \
					crypto/hash/sha1.c \
					crypto/hash/hmac.c \
					crypto/hash/auth.c \
					crypto/replay/rdb.c \
					crypto/replay/rdbx.c \
					crypto/replay/ut_sim.c \
					crypto/math/datatypes.c \
					crypto/math/stat.c \
					crypto/rng/rand_source.c \
					crypto/rng/prng.c \
					crypto/rng/ctr_prng.c \
					crypto/kernel/err.c \
					crypto/kernel/crypto_kernel.c \
					crypto/kernel/alloc.c \
					crypto/kernel/key.c \
					srtp/srtp.c
endif					

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

