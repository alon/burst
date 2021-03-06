import time
import burst
burst.init()

def do():
	ALMotion_Proxy = burst.getMotionProxy()

	#________________________________
	# Remote procedure call
	#________________________________

	TimeInterpolation = 0.05
	ALMotion_Proxy.setBodyStiffness(1.0,TimeInterpolation)
	time.sleep(TimeInterpolation)


	joints = ["HeadPitch","HeadYaw","LAnklePitch","LAnkleRoll","LElbowRoll","LElbowYaw","LHipPitch","LHipRoll","LHipYawPitch","LKneePitch","LShoulderPitch","LShoulderRoll","RAnklePitch","RAnkleRoll","RElbowRoll","RElbowYaw","RHipPitch","RHipRoll","RKneePitch","RShoulderPitch","RShoulderRoll"];
	angles = [[0,0],
	[3.37175e-007,3.37175e-007],
	[0.523599,0.523599],
	[0,0],
	[0,0],
	[0,0],
	[0,0],
	[0,0],
	[0,0],
	[0,0],
	[0,0],
	[1.5708,1.5708],
	[0.523599,0.523599],
	[0,0],
	[0,0],
	[0,0],
	[0,0],
	[0,0],
	[0,0],
	[0,0],
	[-1.5708,-1.5708],
	];

	times = [[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	[1.9,2.9],
	];


	try:
	  moveId = ALMotion_Proxy.doMove(joints, angles, times, 1);
	except Exception,e:
	  print "get up from back test Failed"
	  exit(1)


	# get up from front

	joints = ["HeadPitch","LAnklePitch","LAnkleRoll","LElbowRoll","LElbowYaw","LHipPitch","LHipRoll","LHipYawPitch","LKneePitch","LShoulderPitch","LShoulderRoll","RAnklePitch","RAnkleRoll","RElbowRoll","RElbowYaw","RHipPitch","RHipRoll","RHipYawPitch","RKneePitch","RShoulderPitch","RShoulderRoll"];
	angles = [[-0.575959,0,-0.349066,-0.488692,0,0.279253],
	[-1.13446,-1.13446,-0.783653,0.0872665,-0.312414,-0.715585,-1.0472,-0.174533],
	[0,0,-0.680678,-0.555015,-0.296706,-0.10472,0,0],
	[0,-0.610865,-1.65806,-0.139626,-0.715585,-1.29154,-1.39626,-1.25664],
	[-1.5708,-1.5708,-1.5708,-1.5708,-0.244346,-0.925025,-1.5708,-1.23918],
	[0,-1.5708,-1.5708,-1.5708,-1.5708,-1.06989,-1.0472,-0.174533],
	[1.56923e-007,1.56923e-007,1.56923e-007,1.56923e-007,0.0872665,0.10472,-0.010472,-0.010472],
	[0,0,-0.872665,-0.872665,-0.965167,-0.785398,0,0],
	[2.0944,2.0944,1.0472,1.01229,2.15548,2.16421,2.0944,0.349066],
	[-1.5708,-0.872665,-0.174533,0,0.610865,1.11701,1.62316,1.8326],
	[0,0,0,0,0.0349066,0.1309,0.174533,0.191986],
	[-1.13446,-1.13446,-0.783653,0.0872665,-0.312414,-0.715585,-1.0472,-0.174533],
	[8.63852e-008,8.63852e-008,0.680678,0.555015,0.296706,0.10472,0,0],
	[0,0.610865,1.65806,0.139626,0.715585,1.29154,1.39626,1.25664],
	[1.5708,1.5708,1.5708,1.5708,0.244346,0.925025,1.5708,1.23918],
	[1.44878e-007,-1.5708,-1.5708,-1.5708,-1.5708,-1.06989,-1.0472,-0.174533],
	[0,0,0,0,-0.0872665,-0.10472,0.010472,0.010472],
	[8.54618e-008,9.7389e-008,-0.872665],
	[2.0944,2.0944,1.0472,1.01229,2.15548,2.16421,2.0944,0.349066],
	[-1.5708,-0.872665,-0.174533,0,0.610865,1.11701,1.62316,1.8326],
	[0,0,0,0,-0.0349066,-0.1309,-0.174533,-0.191986],
	];
	times = [[1.4,2.4,3.7,5.2,6.2,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	[1.4,2.4,3.7,4.4,5.2,6.2,7.4,8.4],
	];

	try:
		moveId = ALMotion_Proxy.doMove(joints, angles, times, 1);
	except Exception,e:
	  print "get up from back test Failed"
	  exit(1)
