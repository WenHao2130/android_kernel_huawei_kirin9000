EXTRA_CFLAGS += -I$(srctree)/drivers/hwspinlock
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea

obj-$(CONFIG_HUAWEI_THP) += thp.o
thp-objs += huawei_thp_attr.o huawei_thp_core.o huawei_thp_mt_wrapper.o
obj-$(CONFIG_HUAWEI_THP_FOCALTECH) += focaltech_thp.o
obj-$(CONFIG_HUAWEI_THP_NOVATECH) += novatek_thp.o
obj-$(CONFIG_HUAWEI_THP_SYNAPTICS) += synaptics_thp.o
obj-$(CONFIG_JDI_HOST_TS_KIT) += bu21150_thp.o
obj-$(CONFIG_HUAWEI_THP_HIMAX) += himax_thp.o
obj-$(CONFIG_HUAWEI_THP_GOODIX) += goodix_thp.o
obj-$(CONFIG_HUAWEI_THP_SOLOMON) += ssl_thp.o
obj-$(CONFIG_HUAWEI_SHB_THP) += huawei_thp_log.o
obj-$(CONFIG_HUAWEI_THP_ILITEK) += ilitek_thp.o
obj-$(CONFIG_HUAWEI_THP_ST) += stmicro_thp.o

EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/tzdriver
EXTRA_CFLAGS += -I$(srctree)/drivers/tzdriver
EXTRA_CFLAGS += -I$(srctree)/drivers/devkit/lcdkit/lcdkit1.0/core/common
EXTRA_CFLAGS += -I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/common/include
ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)),)
EXTRA_CFLAGS += -I$(srctree)/drivers/huawei_platform/inputhub/default
else
EXTRA_CFLAGS += -I$(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)
endif

ifeq ($(TARGET_BOARD_PLATFORM), $(TRINKET))
EXTRA_CFLAGS += -I$(srctree)/drivers/gpu/drm/msm
EXTRA_CFLAGS += -I$(srctree)/drivers/gpu/drm/msm/dsi-staging
EXTRA_CFLAGS += -I$(srctree)/drivers/hwsensor/pssensor
endif

EXTRA_CFLAGS += -I$(srctree)/drivers/huawei_platform/sensor2_0/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/mediatek/sensor/2.0/core

EXTRA_CFLAGS += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/alsps/alspshub
