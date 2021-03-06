#
# Makefile for the libimops.so shared library that is used from pynaoqi
# and also should be usable from gstreamer / directly with /dev/video0
# later or as a naoqi module.
#

# Rules explanation:
#
# There are three build directories for the four targets:
#  host 32bit <- webots 32 bit + pynaoqi
#  host 64bit <- pynaoqi 64 bit
#  robot      <- robot
#
# This works by calling my self from the main targets after chdir
# to the build dir, and setting VPATH to contain the original directory.
#
# The main targets are:
#  robot - cross compile and install for the robot (cross is the same as robot)
#  pynaoqi - compile and install the pynaoqi module, regardless of word size
#  webots - compile and install the webots module, will complain if in 64 bit environment.
#
# compilation results will always end up in the BUILD dir, which is
#  build_host32, build_host64 or build_robot
#

# Possible debug flags:
# TODO - document function of each
#-DBURST_DEBUG_VISION -DOFFLINE -DDEBUG_IMAGE

# BURST_DEBUG_VISION - print vision related stuff
# BURST_DEBUG_VISION_THREAD - prints time per loop, and frame count
# DEBUG_IMAGE_REQUESTS - ALImageTranscriber

# Burst flags:
# BURST_DO_LOCALIZATION_IN_MODULE - not working right now (or ever?)

ifndef ROOT
# XXX we assume we are called from the Makefile directory if no ROOT is supplied
ROOT=../../
endif
include $(ROOT)/Makefile.local

################################################################################

TARGET_LIB=libimops.so

SRC=ConcreteCorner.cpp       ConcreteLine.cpp  Kinematics.cpp       objrec.cpp    Threshold.cpp  VisualBall.cpp      VisualDetection.cpp    VisualLine.cpp \
ConcreteFieldObject.cpp  FieldLines.cpp    NaoPose.cpp          Profiler.cpp  Utility.cpp    VisualCorner.cpp    VisualFieldObject.cpp  VisualRobot.cpp \
ConcreteLandmark.cpp     imops.cpp         ObjectFragments.cpp  Sensors.cpp   Vision.cpp     VisualCrossbar.cpp  VisualLandmark.cpp     Zlib.cpp \
imopsmodule.cpp ALImageTranscriber.cpp synchro.cpp

OBJ_PYNAOQI=ConcreteCorner.o       ConcreteLine.o  Kinematics.o       objrec.o    Threshold.o  VisualBall.o      VisualDetection.o    VisualLine.o \
ConcreteFieldObject.o  FieldLines.o    NaoPose.o          Profiler.o  Utility.o    VisualCorner.o    VisualFieldObject.o  VisualRobot.o \
ConcreteLandmark.o     imops.o         ObjectFragments.o  Sensors.o   Vision.o     VisualCrossbar.o  VisualLandmark.o     Zlib.o

HOST_32_LIB=imops_host_32.so
PYNAOQI_32_LIB=imops_pynaoqi_32.so
HOST_64_LIB=imops_host_64.so
PYNAOQI_64_LIB=imops_pynaoqi_64.so

# Turn on any debug flags - same for all targets
#CFLAGS_DEBUG=-DBURST_DEBUG_VISION_THREAD -DDEBUG_IMAGE_REQUESTS
CFLAGS_DEBUG=
#OFFLINE_DEBUG_FLAGS=-DDEBUG_IMAGE
OFFLINE_DEBUG_FLAGS=-DOFFLINE
CFLAGS_DEBUG_BUILD_32=$(OFFLINE_DEBUG_FLAGS)
CFLAGS_DEBUG_BUILD_64=$(OFFLINE_DEBUG_FLAGS)

ifeq ($(USE_ANYBALL), 1)
$(info =================================== Building ANYBALLS imops ==================================)
CFLAGS+=-DUSE_ANYBALL
LDFLAGS_WEBOTS+=-lcv -lhighgui
LDFLAGS_PYNAOQI+=-lcv -lhighgui
# TODO - 64 bit
LDFLAGS_NAO+=-lcv
endif

################################################################################
# This part must be copied as is

ifdef ROOT
else
ROOT=../../
endif

include $(ROOT)/Makefile.local

# See this for details
include $(ROOT)/src/Makefile.base


