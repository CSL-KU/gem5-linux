#
# Makefile for the linux kernel.
#

ifeq ($(CONFIG_ARCH_VEXPRESS_GEM5),y)
ccflags-$(CONFIG_ARCH_MULTIPLATFORM) := -I$(srctree)/$(src)/include \
	-I$(srctree)/arch/arm/plat-versatile/include \
	-I$(srctree)/arch/arm/mach-vexpress/include
else
ccflags-$(CONFIG_ARCH_MULTIPLATFORM) := -I$(srctree)/$(src)/include \
	-I$(srctree)/arch/arm/plat-versatile/include
endif

obj-y					:= v2m.o
obj-$(CONFIG_ARCH_VEXPRESS_CA9X4)	+= ct-ca9x4.o
obj-$(CONFIG_ARCH_VEXPRESS_GEM5)	+= ct-ca9x4.o
obj-$(CONFIG_ARCH_VEXPRESS_DCSCB)	+= dcscb.o	dcscb_setup.o
CFLAGS_dcscb.o				+= -march=armv7-a
CFLAGS_REMOVE_dcscb.o			= -pg
obj-$(CONFIG_ARCH_VEXPRESS_SPC)		+= spc.o
CFLAGS_REMOVE_spc.o			= -pg
obj-$(CONFIG_ARCH_VEXPRESS_TC2_PM)	+= tc2_pm.o
CFLAGS_tc2_pm.o				+= -march=armv7-a
CFLAGS_REMOVE_tc2_pm.o			= -pg
ifeq ($(CONFIG_ARCH_VEXPRESS_TC2_PM),y)
obj-$(CONFIG_ARM_PSCI)			+= tc2_pm_psci.o
CFLAGS_REMOVE_tc2_pm_psci.o		= -pg
endif
obj-$(CONFIG_SMP)			+= platsmp.o
obj-$(CONFIG_HOTPLUG_CPU)		+= hotplug.o
obj-$(CONFIG_PCI)			+= pcie_gem5.o
obj-$(CONFIG_ARCH_GEM5_ENERGY_CTRL)		+= gem5-energy-ctrl.o
