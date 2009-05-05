
#constants file to store all our sweet ass-moves for the Nao # Marvelous XKCD reference!
#import MotionConstants
from burst.consts import DEG_TO_RAD

def getMoveTime(move):
    totalTime = 0.0
    for target in move:
        if len(target) == 6:
            totalTime += target[4]
        elif len(target) == 3:
            totalTime += target[1]
    return totalTime

OFF = None #OFF means the joint chain doesnt get enqueued during this motion


INITIAL_POS = (((80.,40.,-50.,-70.),(0.,0.,-50.,100.,-50.,0.),(0.,0.,-50.,100.,-50.,0.),(80.,-40.,50.,70.),4.0,1),)

#Angles measured pretty exactly from the robot w/gains off.
#might want to make them even different if we suspect the motors are weakening
SIT_POS = (((0.,90.,0.,0.),
            (0.,0.,-55.,125.7,-75.7,0.),
            (0.,0.,-55.,125.7,-75.7,0.),
            (0.,-90.,0.,0.),3.0,1),
           ((90.,0.,-65.,-57.),
            (0.,0.,-55.,125.7,-75.7,0.),
            (0.,0.,-55.,125.7,-75.7,0.),
            (90.,0.,65.,57.),1.5,1))

ZERO_POS = (((0.,0.,0.,0.),(0.,0.,0.,0.,0.,0.),(0.,0.,0.,0.,0.,0.),(0.,0.,0.,0.),4.0,1),)
PENALIZED_POS = INITIAL_POS

STAND = (((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0,1),)

SET_POS = INITIAL_POS

READY_POS = INITIAL_POS

ZERO_HEADS = (((0.0,0.0),1.0,1),)

NEUT_HEADS = (((0.,20.),2.0,1),)

PENALIZED_HEADS = (((0.0,25.0),0.5,1),)

# WALKS
FASTER_WALK = [110.0 * DEG_TO_RAD, # ShoulderMedian
           10.0 * DEG_TO_RAD,  # ShoulderAmplitude
           90.0 * DEG_TO_RAD,  # ElbowMedian 
           0.0 * DEG_TO_RAD,  # ElbowAmplitude 
           4.5,                   # LHipRoll(degrees) (2.5 original)
           -4.5,                  # RHipRoll(degrees) (-2.5 original)
           0.23,                  # HipHeight(meters)
           0.0,                   # TorsoYOrientation(degrees)
           0.04,                  # StepLength
           0.02,                  # StepHeight
           0.02,                  # StepSide
           0.3,                   # MaxTurn
           0.01,                  # ZmpOffsetX
           0.016,                 # ZmpOffsetY 
           0.5,                     # Distance
           18]#,                    # Speed
           #,0.68]                  # Angle 0.68

FAST_WALK = [100.0 * DEG_TO_RAD, # ShoulderMedian
           10.0 * DEG_TO_RAD,  # ShoulderAmplitude
           30.0 * DEG_TO_RAD,  # ElbowMedian 
           10.0 * DEG_TO_RAD,  # ElbowAmplitude 
           3.5,                   # LHipRoll(degrees) 
           -3.5,                  # RHipRoll(degrees)
           0.23,                  # HipHeight(meters)
           3.0,                   # TorsoYOrientation(degrees)
           0.04,                  # StepLength
           0.02,                  # StepHeight
           0.02,                  # StepSide
           0.3,                   # MaxTurn
           0.015,                 # ZmpOffsetX
           0.018,                 # ZmpOffsetY 
           0.5,                   # Distance
           25]                    # Speed

STD_WALK = [100.0 * DEG_TO_RAD, # ShoulderMedian
           10.0 * DEG_TO_RAD,  # ShoulderAmplitude
           30.0 * DEG_TO_RAD,  # ElbowMedian 
           10.0 * DEG_TO_RAD,  # ElbowAmplitude 
           2.5,                   # LHipRoll(degrees) 
           -2.5,                  # RHipRoll(degrees)
           0.23,                  # HipHeight(meters)
           0.0,                   # TorsoYOrientation(degrees)
           0.04,                  # StepLength
           0.02,                  # StepHeight
           0.03,                  # StepSide
           0.3,                   # MaxTurn
           0.01,                  # ZmpOffsetX
           0.018,                 # ZmpOffsetY 
           0.5,                   # Distance
           25]                    # Speed

SLOW_WALK = [100.0 * DEG_TO_RAD, # ShoulderMedian
           10.0 * DEG_TO_RAD,  # ShoulderAmplitude
           30.0 * DEG_TO_RAD,  # ElbowMedian 
           10.0 * DEG_TO_RAD,  # ElbowAmplitude 
           4.5,                   # LHipRoll(degrees) 
           -4.5,                  # RHipRoll(degrees)
           0.22,                  # HipHeight(meters)
           2.0,                   # TorsoYOrientation(degrees)
           0.05,                  # StepLength
           0.04,                  # StepHeight
           0.04,                  # StepSide
           0.4,                   # MaxTurn
           0.01,                  # ZmpOffsetX
           0.00,                  # ZmpOffsetY 
           0.05*4,                # Distance
           80]                    # Speed

#KICKS

KICK_STRAIGHT = (
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0,1),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1),
    #lift the left leg
    ((80.,40.,-50.,-70.),
     (0.,20.,-30.,60.,-30.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1),
    #kick the left leg
    ((80.,40.,-50.,-70.),
     (0.,20.,-55.,60.,-5.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),.08,1),
    #unkick the left leg
    ((80.,40.,-50.,-70.),
     (0.,20.,-30.,60.,-30.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),.08,1),
    #put the left leg back down the middle
    ((80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1))

HALF_KICK = (
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0,1),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1),
    #lift the left leg
    ((80.,40.,-50.,-70.),
     (0.,20.,-30.,60.,-30.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1))

ALMOST_KICK = (
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0,1),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1),
    #lift the left leg
    ((80.,40.,-50.,-70.),
     (0.,20.,-30.,60.,-30.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1),
    #Get ready
    ((80.,40.,-50.,-70.),
     (0.,30.,-20.,120.,-40.,0.),
     (-10.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1),
    #Kick
    ((80.,40.,-50.,-70.),
     (0.,20.,-70.,60.,-30.,0.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.25,1))

KICK_A = (
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0,1),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1),
    #lift the left leg
    ((80.,40.,-50.,-70.),
     (0.,20.,-30.,60.,-30.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1),
    #kick the left leg
    ((80.,40.,-50.,-70.),
     (0.,20.,-55.,60.,-5.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),.05,1),
    #unkick the left leg
    ((80.,40.,-50.,-70.),
     (0.,20.,-30.,60.,-30.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),.08,1),
    #put the left leg back down the middle
    ((80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0,1),
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0,1))

KICK_STRAIGHT_RIGHT = (
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0,1),
    #swing to the left
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-20.,-10.,20.,-10.,20.),
     (80.,-40.,50.,70.),2.0,1),
    #lift the right leg
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-20.,-30.,60.,-30.,20.),
     (80.,-40.,50.,70.),2.0,1),
    #kick the right leg
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-20.,-55.,60.,-5.,20.),
     (80.,-40.,50.,70.),.08,1),
    #unkick the right leg
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-20.,-30.,60.,-30.,20.),
     (80.,-40.,50.,70.),.08,1),
    #put the right leg back down the middle
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-20.,-10.,20.,-10.,20.),
     (80.,-40.,50.,70.),2.0,1))


#HEAD SCANS

LOC_PANS = (
    (( 65.0, 10.0),1.5, 1),
    ((65.,-25.),1.0,  1),
    ((-65.,-25.),2.5, 1),
    ((-65.0, 10.0) ,1., 1),
    (( 0.0, 10.0),1.5,  1),)

SCAN_BALL= (
    (( 65.0, 20.0),1.0, 1),
    ((65.,-30.),0.75,  1),
    ((-65.,-30.),2.0, 1),
    ((-65.0, 20.0) ,0.75, 1),
    (( 0.0, 20.0),1.0,  1),)

POST_SCAN = (
    ((65.,-25.),2.0,  1),
    ((-65.,-25.),2.0, 1))

PAN_LEFT = (
    (( 65.0, 20.0),2.0, 1),
    ((0.0,20.),2.0,1))
PAN_RIGHT = (
    (( -65.0, 20.0),2.0, 1),
    ((0.0,20.),2.0,1))

# STAND UPS
STAND_UP_FRONT = ( ((90,50,0,0),
                    (0,8,0,120,-65,0),
                    (0,0,8,120,-65,4),
                    (90,-50,0,0 ),1.0,1 ),

                   ((90,90,0,0),
                    (0,8,0,120,-65,0),
                    (0,0,8,120,-65,4),
                    (90,-90,0,0 ),1.0,1 ),

                   ((-90,90,0,0),
                    (0,8,0,120,-65,0),
                    (0,0,8,120,-65,4),
                    (-90,-90,0,0 ),0.5,1 ),

                   ((-90,0,0,0),
                    (0,8,0,120,-65,0),
                    (0,0,8,120,-65,4),
                    (-90,0,0,0 ),0.75,1 ),

                   ((-90,0,-90,0),
                    (0,8,0,120,-65,0),
                    (0,0,8,120,-65,4),
                    (-90,0,90,0 ),0.3,1 ),

                   ((-50,0,-90,-35),
                    (5,8,-90,120,-65,0),
                    (5,0,-90,120,-65,4),
                    (-50,0,90,35),2.0,1),

                   ((-10,0,-90,-95),
                    (-50,8,-90,60,-44,-39),
                    (-50,0,-90,60,-44,39),
                    (-10,0,90,95),1.3,1),

                   ((0,0,-90,-8),
                    (-50,8,-90,58,5,-31),
                    (-50,0,-90,58,5,31),
                    (0,0,90,8),1.7,1),

                   ((35,2,-14,-41),
                    (-55,5,-90,123,-17,-17),
                    (-55,-5,-90,123,-17,17),
                    (35,2,14,41),0.8, 1),

                   ((64,7,-53,-74),
                    (-45,6,-61,124,-41,-6),
                    (-45,-6,-61,124,-41,6),
                    (64,-7,53,74),1.2, 1),

                   ((93,10,-90,-80),
                    (0,0,-60,120,-60,0),
                    (0,0,-60,120,-60,0),
                    (93,-10,90,80),1.0,1),

                   ( INITIAL_POS[0][0],
                     INITIAL_POS[0][1],
                     INITIAL_POS[0][2],
                     INITIAL_POS[0][3],0.5,1))

STAND_UP_BACK = ( ((0,90,0,0),
                   (0,0,0,0,0,0),
                   (0,0,0,0,0,0),
                   (0,-90,0,0),1.0,1),

                  ((120,46,9,0),
                   (0,8,10,96,14,0),
                   (0,0,10,96,14,4),
                   (120,-46,-9,0), 1.0, 1),

                  ((120,25,10,-95),
                   (-2,8,0,70,18,0),
                   (-2,0,0,70,18,4),
                   (120,-25,-10,95),0.7,1),

                  ((120,22,15,-30),
                   (-38,8,-90,96,14,0),
                   (-38, 0,-90, 96, 14, 4),
                   ( 120,-22,-15, 30), 0.7, 1),


                  ((120,0,5,0),
                   (-38,31,-90,96,45,0),
                   (-38,-31,-90,96,45,4),
                   (120,0,-5,0), 1.0,1),

                  ((40,60,4,-28),
                   (-28,8,-49,126,-32,-22),
                   (-28,-31,-87,70,45,0),
                   (120,-33,-4,4),1.0,1 ),

                  ((42,28,5,-47),
                   (-49,-16,22,101,-70,-5),
                   (-49,-32,-89,61,39,-7),
                   (101,-15,-4,3),0.9,1 ),

                  ((42,28,4,-46),
                   (-23,11,-49,126,-70,6),
                   (-23,-17,-51,50,23,38),
                   (51,-50,0,26), 1.0,1),

                  ((42,28,4,-46),
                   (-23,21,-47,126,-70,5),
                   (-23,-1,-51,101,-33,15),
                   (51,-39,0,32), 0.5,1),

                  ((98,2,-72,-65),
                   (0,0,-50,120,-70,0),
                   (0,0,-50,120,-70,0),
                   (98,-2,72,65), 1.1,1),

                  ( INITIAL_POS[0][0],
                    INITIAL_POS[0][1],
                    INITIAL_POS[0][2],
                    INITIAL_POS[0][3],0.5,1))
