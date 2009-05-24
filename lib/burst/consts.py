import math

# Proxy names
BURST_SHARED_MEMORY_PROXY_NAME = "burstmem"
BURST_RECORDER_PROXY_NAME = "recorder"

# Naoqi Constants

BALANCE_MODE_OFF = 0
BALANCE_MODE_AUTO = 1
BALANCE_MODE_COM_CONTROL = 2

# support modes
SUPPORT_MODE_LEFT, SUPPORT_MODE_DOUBLE_LEFT, SUPPORT_MODE_RIGHT, SUPPORT_MODE_DOUBLE_RIGHT = 0, 1, 2, 3

INTERPOLATION_LINEAR, INTERPOLATION_SMOOTH = 0, 1

# Math constants

DEG_TO_RAD = math.pi / 180.0
RAD_TO_DEG = 180.0 / math.pi
CM_TO_METER = 100. # yeah, seems stupid, but probably better than using 100 throughout the code...

# Camera / Vision constants

IMAGE_WIDTH = 320.0
IMAGE_HEIGHT = 240.0

IMAGE_HALF_WIDTH = IMAGE_WIDTH / 2
IMAGE_HALF_HEIGHT = IMAGE_HEIGHT / 2

# Shared memory constants

MMAP_VARIABLES_FILENAME = "/home/root/burst/lib/etc/mmap_variables.txt"
MMAP_FILENAME           = "/home/root/burst/lib/etc/burstmem.mmap"
MMAP_LENGTH      = 4096

# Event Manager constants
EVENT_MANAGER_DT = 0.05 # seconds

# World constants

MISSING_FRAMES_MINIMUM = 5

MIN_BEARING_CHANGE = 1e-3 # TODO - ?
MIN_DIST_CHANGE = 1e-3

BALL_REAL_DIAMETER = 8.7 # cm
ROBOT_DIAMETER = 58.0    # this is from Appendix A of Getting Started - also, doesn't hands raised into account
GOAL_POST_HEIGHT = 80.0
GOAL_POST_DIAMETER = 80.0 # TODO: name? this isn't the radius*2 of the base, it is the diameter in the sense of longest line across an image of the post.

# Robot constants
MOTION_FINISHED_MIN_DURATION = EVENT_MANAGER_DT * 3

#############################################################
# Lists of variable names exported by us to ALMemory

# To recreate under pynaoqi run:
#
#   refilter('/BURST/Vision',names)
#
vision_vars = ['/BURST/Vision/BGCrossbar/AngleXDeg',
 '/BURST/Vision/BGCrossbar/AngleYDeg',
 '/BURST/Vision/BGCrossbar/BearingDeg',
 '/BURST/Vision/BGCrossbar/CenterX',
 '/BURST/Vision/BGCrossbar/CenterY',
 '/BURST/Vision/BGCrossbar/Distance',
 '/BURST/Vision/BGCrossbar/ElevationDeg',
 '/BURST/Vision/BGCrossbar/FocDist',
 '/BURST/Vision/BGCrossbar/Height',
 '/BURST/Vision/BGCrossbar/LeftOpening',
 '/BURST/Vision/BGCrossbar/RightOpening',
 '/BURST/Vision/BGCrossbar/Width',
 '/BURST/Vision/BGCrossbar/X',
 '/BURST/Vision/BGCrossbar/Y',
 '/BURST/Vision/BGCrossbar/shotAvailable',
 '/BURST/Vision/BGLP/AngleXDeg',
 '/BURST/Vision/BGLP/AngleYDeg',
 '/BURST/Vision/BGLP/BearingDeg',
 '/BURST/Vision/BGLP/CenterX',
 '/BURST/Vision/BGLP/CenterY',
 '/BURST/Vision/BGLP/Distance',
 '/BURST/Vision/BGLP/DistanceCertainty',
 '/BURST/Vision/BGLP/ElevationDeg',
 '/BURST/Vision/BGLP/FocDist',
 '/BURST/Vision/BGLP/Height',
 '/BURST/Vision/BGLP/IDCertainty',
 '/BURST/Vision/BGLP/Width',
 '/BURST/Vision/BGLP/X',
 '/BURST/Vision/BGLP/Y',
 '/BURST/Vision/BGRP/AngleXDeg',
 '/BURST/Vision/BGRP/AngleYDeg',
 '/BURST/Vision/BGRP/BearingDeg',
 '/BURST/Vision/BGRP/CenterX',
 '/BURST/Vision/BGRP/CenterY',
 '/BURST/Vision/BGRP/Distance',
 '/BURST/Vision/BGRP/DistanceCertainty',
 '/BURST/Vision/BGRP/ElevationDeg',
 '/BURST/Vision/BGRP/FocDist',
 '/BURST/Vision/BGRP/Height',
 '/BURST/Vision/BGRP/IDCertainty',
 '/BURST/Vision/BGRP/Width',
 '/BURST/Vision/BGRP/X',
 '/BURST/Vision/BGRP/Y',
 '/BURST/Vision/Ball/bearing',
 '/BURST/Vision/Ball/centerX',
 '/BURST/Vision/Ball/centerY',
 '/BURST/Vision/Ball/confidence',
 '/BURST/Vision/Ball/dist',
 '/BURST/Vision/Ball/elevation',
 '/BURST/Vision/Ball/focDist',
 '/BURST/Vision/Ball/height',
 '/BURST/Vision/Ball/width',
 '/BURST/Vision/YGCrossbar/AngleXDeg',
 '/BURST/Vision/YGCrossbar/AngleYDeg',
 '/BURST/Vision/YGCrossbar/BearingDeg',
 '/BURST/Vision/YGCrossbar/CenterX',
 '/BURST/Vision/YGCrossbar/CenterY',
 '/BURST/Vision/YGCrossbar/Distance',
 '/BURST/Vision/YGCrossbar/ElevationDeg',
 '/BURST/Vision/YGCrossbar/FocDist',
 '/BURST/Vision/YGCrossbar/Height',
 '/BURST/Vision/YGCrossbar/LeftOpening',
 '/BURST/Vision/YGCrossbar/RightOpening',
 '/BURST/Vision/YGCrossbar/Width',
 '/BURST/Vision/YGCrossbar/X',
 '/BURST/Vision/YGCrossbar/Y',
 '/BURST/Vision/YGCrossbar/shotAvailable',
 '/BURST/Vision/YGLP/AngleXDeg',
 '/BURST/Vision/YGLP/AngleYDeg',
 '/BURST/Vision/YGLP/BearingDeg',
 '/BURST/Vision/YGLP/CenterX',
 '/BURST/Vision/YGLP/CenterY',
 '/BURST/Vision/YGLP/Distance',
 '/BURST/Vision/YGLP/DistanceCertainty',
 '/BURST/Vision/YGLP/ElevationDeg',
 '/BURST/Vision/YGLP/FocDist',
 '/BURST/Vision/YGLP/Height',
 '/BURST/Vision/YGLP/IDCertainty',
 '/BURST/Vision/YGLP/Width',
 '/BURST/Vision/YGLP/X',
 '/BURST/Vision/YGLP/Y',
 '/BURST/Vision/YGRP/AngleXDeg',
 '/BURST/Vision/YGRP/AngleYDeg',
 '/BURST/Vision/YGRP/BearingDeg',
 '/BURST/Vision/YGRP/CenterX',
 '/BURST/Vision/YGRP/CenterY',
 '/BURST/Vision/YGRP/Distance',
 '/BURST/Vision/YGRP/DistanceCertainty',
 '/BURST/Vision/YGRP/ElevationDeg',
 '/BURST/Vision/YGRP/FocDist',
 '/BURST/Vision/YGRP/Height',
 '/BURST/Vision/YGRP/IDCertainty',
 '/BURST/Vision/YGRP/Width',
 '/BURST/Vision/YGRP/X',
 '/BURST/Vision/YGRP/Y']


