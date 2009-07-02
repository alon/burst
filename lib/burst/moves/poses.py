#constants file to store all our sweet ass-moves for the Nao # Marvelous XKCD reference!
#import MotionConstants
from burst_consts import DEG_TO_RAD
from ..world import World
from numpy import array

'''
    Angles:
        LEFT: ShoulderPitch,ShoulderRoll,ElbowYaw,ElbowRoll
        LEFT: HipYawPitch,HipRoll,HipPitch,KneePitch,AnklePitch,AnkleRoll
        RIGHT: HipYawPitch,-HipRoll,HipPitch,KneePitch,AnklePitch,-AnkleRoll
        RIGHT: ShoulderPitch,ShoulderRoll,ElbowYaw,ElbowRoll
    Note:
        To convert symmetric left/right movements, yaw and roll should also be * -1 - see mirrorChoreographMove
'''

def moveDegToRad(move):
    def convertPose(pose):
        ret = [tuple(x*DEG_TO_RAD for x in tup) for tup in pose[:-1]]
        ret.append(pose[-1])
        return ret
    return [convertPose(pose) for pose in move]

''' BODY POSITIONS '''
INITIAL_POS_DEG = [((0.,0.),
                (90.,20.,-80.,-45.),
                (0.,0.,-25.,40.,-20.,0.),
                (0.,0.,-25.,40.,-20.,0.),
                (90.,-20.,80.,45.),4.0)]
INITIAL_POS = moveDegToRad(INITIAL_POS_DEG)

#Angles measured pretty exactly from the robot w/gains off.
#might want to make them even different if we suspect the motors are weakening
#SIT_POS = [((0.,90.,0.,0.),
#            (0.,0.,-55.,125.7,-75.7,0.),
#            (0.,0.,-55.,125.7,-75.7,0.),
#            (0.,-90.,0.,0.),3.0),
#           ((90.,0.,-65.,-57.),
#            (0.,0.,-55.,125.7,-75.7,0.),
#            (0.,0.,-55.,125.7,-75.7,0.),
#            (90.,0.,65.,57.),1.5)]

SIT_POS = moveDegToRad([((0.,0.),
            (0.,90.,0.,0.),
            (-12.5, -5., -42.5, 125., -70., 6.),
            (-12.5, 5., -42.5, 125., -70., -6.),
            (0.,-90.,0.,0.),3.0),
           ((0.,0.),
            (55.,7.,0.,-30.),
            (-12.5, -5., -42.5, 125., -70., 6.),
            (-12.5, 5., -42.5, 125., -70., -6.),
            (55.,-7.,0.,30.),1.5)])

ZERO_POS = moveDegToRad([((0.,0.,0.,0.),(0.,0.,0.,0.,0.,0.),(0.,0.,0.,0.,0.,0.),(0.,0.,0.,0.),4.0),])

STAND = moveDegToRad([
         ((80.,40.,-50.,-70.),
          (0.,0.,-10.,20.,-10.,0.),
          (0.,0.,-10.,20.,-10.,0.),
          (80.,-40.,50.,70.),2.0),])


''' HEAD POSITIONS (for bottom camera) '''
HEAD_PITCH_DOWN_MAX = 30. * DEG_TO_RAD # NOTE: should have been 45 according to specs
HEAD_PITCH_UP_MAX = -45. * DEG_TO_RAD
HEAD_PITCH_UP_MOST = HEAD_PITCH_UP_MAX * 2 / 3
HEAD_YAW_RANGE_MAX = 90. * DEG_TO_RAD
HEAD_YAW_LEFT_MOST = HEAD_YAW_RANGE_MAX * 2 / 3
HEAD_YAW_RIGHT_MOST = -HEAD_YAW_RANGE_MAX * 2 / 3

HEAD_POS_FRONT_CLOSE = (0., HEAD_PITCH_UP_MAX) # look front - close
HEAD_POS_FRONT_FAR = (0., HEAD_PITCH_UP_MAX * 2 / 3) # look front - far
HEAD_POS_FRONT_BOTTOM = (0., HEAD_PITCH_DOWN_MAX) # look down


''' HEAD MOVES / SCANS (for bottom camera) '''
HEAD_MOVE_FRONT_FAR = ((HEAD_POS_FRONT_FAR, 0.5),)
#HEAD_MOVE_FRONT_CLOSE = ((HEAD_POS_FRONT_CLOSE, 0.5),)
HEAD_MOVE_FRONT_BOTTOM = ((HEAD_POS_FRONT_BOTTOM, 0.5),)

# Start from bottom part (closer is probably more important), continue with middle, finish with top
# TODO: check what's the fastest time for scanning where ball/goal is still detected (can save lots of time)
HEAD_SCAN_FRONT = (
    (HEAD_POS_FRONT_FAR, 0.3),
    ((HEAD_YAW_RIGHT_MOST, HEAD_PITCH_DOWN_MAX), 0.3),
    ((HEAD_YAW_LEFT_MOST, HEAD_PITCH_DOWN_MAX), 3.0),
    ((HEAD_YAW_LEFT_MOST, 0), 0.3),
    ((HEAD_YAW_RIGHT_MOST, 0), 3.0),
    ((HEAD_YAW_RIGHT_MOST, HEAD_PITCH_UP_MOST), 0.3),
    ((HEAD_YAW_LEFT_MOST, HEAD_PITCH_UP_MOST), 4.0),
    (HEAD_POS_FRONT_FAR, 0.3),
    )

HEAD_SCAN_QUICK = (
    (HEAD_POS_FRONT_BOTTOM, 0.5),
    (HEAD_POS_FRONT_FAR, 2.0),
    )
#    ((0.0, HEAD_PITCH_DOWN_MAX), 1.0),
#    ((0.0, HEAD_PITCH_UP_MAX), 2.0),
#    (HEAD_POS_FRONT_FAR, 0.3),


''' KICKS '''
#KICK_STR_OFFSET_FROM_BODY = 4 # in cm
#KICK_STR_DISTANCE = 17 # in cm

#===============================================================================
#    Usage:
#    ------
#    KICK_RIGHT = mirrorMove(KICK_LEFT)
#===============================================================================
def mirrorMove(positions):
    return list(
          list(((RShoulderPitch, -RShoulderRoll, -RElbowYaw, -RElbowRoll),
                (RHipYawPitch, -RHipRoll, RHipPitch, RKneePitch, RAnklePitch, -RAnkleRoll),
                (LHipYawPitch, -LHipRoll, LHipPitch, LKneePitch, LAnklePitch, -LAnkleRoll),
                (LShoulderPitch, -LShoulderRoll, -LElbowYaw, -LElbowRoll),
                interp_time))
                  for
              ((LShoulderPitch, LShoulderRoll, LElbowYaw, LElbowRoll),
               (LHipYawPitch, LHipRoll, LHipPitch, LKneePitch, LAnklePitch, LAnkleRoll),
               (RHipYawPitch, RHipRoll, RHipPitch, RKneePitch, RAnklePitch, RAnkleRoll),
               (RShoulderPitch, RShoulderRoll, RElbowYaw, RElbowRoll),
                interp_time) in positions)

GREAT_KICK_LEFT_EXTD = moveDegToRad((
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,40.,-25.,0.),
     (0.,0.,-10.,40.,-25.,0.),
     (80.,-40.,50.,70.),3.0),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,25.,-10.,40.,-30.,-20.),
     (0.,20.,-20.,40.,-25.,-30.),
     (80.,-40.,50.,70.),1.8),
    #lift the left leg
    ((80.,40.,-50.,-70.),
     (0.,25.,-20.,80.,-45.,-20.),
     (0.,20.,-30.,40.,-20.,-30.),
     (80.,-40.,50.,70.),1.5),
    #Get ready
    ((80.,40.,-50.,-70.),
     (-10.,35.,-20.,120.,-20.,0.),
     (0.,35.,-10.,40.,-25.,-30.),
     (80.,-40.,50.,70.),1.2),
    #Kick
    ((80.,40.,-50.,-70.),
     (0.,25.,-70.,60.,-20.,0.),
     (0.,20.,-10.,45.,-25.,-25.),
     (80.,-40.,50.,70.),0.20),
    #make leg go further away
    ((80.,40.,-50.,-70.),
     (0.,25.,-50.,10.,120.,0.),
     (0.,20.,-10.,45.,-25.,-25.),
     (80.,-40.,50.,70.),0.18),
    #lift the left leg
    ((80.,40.,-50.,-70.),
     (0.,25.,-20.,80.,-45.,-20.),
     (0.,20.,-30.,40.,-20.,-30.),
     (80.,-40.,50.,70.),1.0),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,25.,-10.,40.,-30.,-20.),
     (0.,20.,-20.,40.,-25.,-30.),
     (80.,-40.,50.,70.),0.8),
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,40.,-25.,0.),
     (0.,0.,-10.,40.,-25.,0.),
     (80.,-40.,50.,70.),2.0)
))


GREAT_KICK_LEFT = moveDegToRad([
    #Stand up more fully
    [(80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0],
    #swing to the right
    [(80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.8],
    #lift the left leg
    [(80.,40.,-50.,-70.),
     (0.,20.,-30.,80.,-30.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),1.0],
    #Get ready
    [(80.,40.,-50.,-70.),
     (-10.,30.,-20.,120.,-40.,0.),
     (0.,30.,-10.,10.,-10.,-20.),
     (80.,-40.,50.,70.),1.2],
    #Kick
    [(80.,40.,-50.,-70.),
     (0.,20.,-70.,60.,-30.,0.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.18],
    #make leg go further away
    [(80.,40.,-50.,-70.),
     (0.,20.,-50.,10.,120.,0.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.18],
    #lift the left leg
    [(80.,40.,-50.,-70.),
     (0.,20.,-30.,60.,-30.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0],
    #swing to the right
    [(80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0],
    #Stand up more fully
    [(80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0]
])


#Personalizible
GREAT_KICK_LEFT_OFFSET = moveDegToRad((
    #Stand up more fully
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #swing to the right
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #lift the left leg
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #Get ready
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #Kick
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #make leg go further away
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #lift the left leg
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #swing to the right
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #Stand up more fully
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0)
))


#Personalizible
GREAT_KICK_RIGHT_OFFSET = moveDegToRad((
    #Stand up more fully
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #swing to the right
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #lift the left leg
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #Get ready
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #Kick
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #make leg go further away
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #lift the left leg
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #swing to the right
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0),
    #Stand up more fully
    ((0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0,0.0,0.0),
     (0.0,0.0,0.0,0.0),0.0)
))

INSIDE_KICK_LEFT = moveDegToRad((
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,22.,-10.,20.,-10.,-22.),
     (0.,22.,-10.,20.,-10.,-22.),
     (80.,-40.,50.,70.),0.8),
    #Lift left leg
    ((80.,40.,-50.,-70.),
     (0.,40.,-10.,20.,-10.,-22.),
     (0.,22.,-10.,20.,-10.,-22.),
     (80.,-90.,50.,0.),2.0),
    #left leg forward
    ((80.,40.,-50.,-70.),
     (0.,45.,-40.,20.,10.,30.),
     (0.,22.,-10.,20.,-10.,-22.),
     (80.,-90.,50.,0.),2.5),
    #kick, poo, kick
    ((80.,40.,-50.,-70.),
     (0.,17.,-60.,40.,10.,-20.),
     (0.,22.,-10.,20.,-10.,-22.),
     (80.,-90.,50.,0.),0.18),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,22.,-10.,20.,-10.,-22.),
     (0.,22.,-10.,20.,-10.,-22.),
     (80.,-40.,50.,70.),0.8),
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),1.0)))


GREAT_KICK_RIGHT = mirrorMove(GREAT_KICK_LEFT)
GREAT_KICK_RIGHT_EXTD = mirrorMove(GREAT_KICK_LEFT_EXTD)
INSIDE_KICK_RIGHT = mirrorMove(INSIDE_KICK_LEFT)

def makesym(x):
    return (list(x[0])+[0.0,0.0],x[1],x[2],list(x[3])+[0.0,0.0],(x[4],0.0,0.0,0.0,0.0,0.0))

def makeback(x):
    return (x[0][:4],x[1],x[2],x[3][:4],x[4][0])

def getGreatKickLeft(kick_side_offset):
    return map(makeback, (    ( array(map(makesym,GREAT_KICK_LEFT_EXTD))  +array(map(makesym,GREAT_KICK_LEFT_OFFSET))  )*(1-kick_side_offset)  +   array(map(makesym,GREAT_KICK_LEFT))*kick_side_offset    ).tolist())

def getGreatKickRight(kick_side_offset):
    return map(makeback, (    ( array(map(makesym,GREAT_KICK_RIGHT_EXTD))  +array(map(makesym,GREAT_KICK_RIGHT_OFFSET))  )*(1-kick_side_offset)  +   array(map(makesym,GREAT_KICK_RIGHT))*kick_side_offset    ).tolist())

SHPAGAT = moveDegToRad((
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),2.0)))


ALMOST_INSIDE_KICK = moveDegToRad((
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.8),
    #Lift left leg
    ((80.,40.,-50.,-70.),
     (0.,40.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-90.,50.,0.),1.5),
    #left leg forward
    ((80.,40.,-50.,-70.),
     (0.,45.,-40.,20.,10.,30.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-90.,50.,0.),2.5)))

SIDE_KICK_LEFT = moveDegToRad((
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.8),
    #Lift left leg
    ((80.,40.,-50.,-70.),
     (0.,20.,-50.,60.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.8),
    #Ready
    ((80.,40.,-50.,-70.),
     (0.,15.,-50.,60.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.8),
    #Kick
    ((80.,40.,-50.,-70.),
     (0.,50.,-50.,60.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.15),
    #Lift left leg
    ((80.,40.,-50.,-70.),
     (0.,20.,-50.,60.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.8),
    #swing to the right
    ((80.,40.,-50.,-70.),
     (0.,20.,-10.,20.,-10.,-20.),
     (0.,20.,-10.,20.,-10.,-20.),
     (80.,-40.,50.,70.),0.8),
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),1.0)))

SIDE_KICK_RIGHT = moveDegToRad((
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),2.0),
    #swing to the left
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-20.,-10.,20.,-10.,20.),
     (80.,-40.,50.,70.),0.8),
    #lift the right leg
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-20.,-50.,60.,-10.,20.),
     (80.,-40.,50.,70.),1.0),
    #Ready
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-15.,-50.,60.,-10.,20.),
     (80.,-40.,50.,70.),1.0),
    #Kick
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-50.,-50.,60.,-10.,20.),
     (80.,-40.,50.,70.),0.15),
    #lift the right leg
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-20.,-50.,60.,-10.,20.),
     (80.,-40.,50.,70.),1.0),
    #swing to the left
    ((80.,40.,-50.,-70.),
     (0.,-20.,-10.,20.,-10.,20.),
     (0.,-20.,-10.,20.,-10.,20.),
     (80.,-40.,50.,70.),0.8),
    #Stand up more fully
    ((80.,40.,-50.,-70.),
     (0.,0.,-10.,20.,-10.,0.),
     (0.,0.,-10.,20.,-10.,0.),
     (80.,-40.,50.,70.),1.0)))

# STAND UPS
STAND_UP_FRONT = moveDegToRad((
                   ((90,50,0,0),
                    (0,8,0,120,-65,0),
                    (0,0,8,120,-65,4),
                    (90,-50,0,0 ),1.0),

                   ((90,90,0,0),
                    (0,8,0,120,-65,0),
                    (0,0,8,120,-65,4),
                    (90,-90,0,0 ),1.0),

                   ((-90,90,0,0),
                    (0,8,0,120,-65,0),
                    (0,0,8,120,-65,4),
                    (-90,-90,0,0 ),0.5),

                   ((-90,0,0,0),
                    (0,8,0,120,-65,0),
                    (0,0,8,120,-65,4),
                    (-90,0,0,0 ),0.75),

                   ((-90,0,-90,0),
                    (0,8,0,120,-65,0),
                    (0,0,8,120,-65,4),
                    (-90,0,90,0 ),0.3),

                   ((-50,0,-90,-35),
                    (5,8,-90,120,-65,0),
                    (5,0,-90,120,-65,4),
                    (-50,0,90,35),2.0),

                   ((-10,0,-90,-95),
                    (-50,8,-90,60,-44,-39),
                    (-50,0,-90,60,-44,39),
                    (-10,0,90,95),1.3),

                   ((0,0,-90,-8),
                    (-50,8,-90,58,5,-31),
                    (-50,0,-90,58,5,31),
                    (0,0,90,8),1.7),

                   ((35,2,-14,-41),
                    (-55,5,-90,123,-17,-17),
                    (-55,-5,-90,123,-17,17),
                    (35,2,14,41),0.8),

                   ((64,7,-53,-74),
                    (-45,6,-61,124,-41,-6),
                    (-45,-6,-61,124,-41,6),
                    (64,-7,53,74),1.2),

                   ((93,10,-90,-80),
                    (0,0,-60,120,-60,0),
                    (0,0,-60,120,-60,0),
                    (93,-10,90,80),1.0),

                   ( INITIAL_POS_DEG[0][1], # start from [1] to skip head
                     INITIAL_POS_DEG[0][2],
                     INITIAL_POS_DEG[0][3],
                     INITIAL_POS_DEG[0][4],0.5)))

STAND_UP_BACK = moveDegToRad((
                  ((0,90,0,0),
                   (0,0,0,0,0,0),
                   (0,0,0,0,0,0),
                   (0,-90,0,0),1.0),

                  ((120,46,9,0),
                   (0,8,10,96,14,0),
                   (0,0,10,96,14,4),
                   (120,-46,-9,0), 1.0),

                  ((120,25,10,-95),
                   (-2,8,0,70,18,0),
                   (-2,0,0,70,18,4),
                   (120,-25,-10,95),0.7),

                  ((120,22,15,-30),
                   (-38,8,-90,96,14,0),
                   (-38, 0,-90, 96, 14, 4),
                   ( 120,-22,-15, 30), 0.7),


                  ((120,0,5,0),
                   (-38,31,-90,96,45,0),
                   (-38,-31,-90,96,45,4),
                   (120,0,-5,0), 1.0),

                  ((40,60,4,-28),
                   (-28,8,-49,126,-32,-22),
                   (-28,-31,-87,70,45,0),
                   (120,-33,-4,4),1.0),

                  ((42,28,5,-47),
                   (-49,-16,22,101,-70,-5),
                   (-49,-32,-89,61,39,-7),
                   (101,-15,-4,3),0.9),

                  ((42,28,4,-46),
                   (-23,11,-49,126,-70,6),
                   (-23,-17,-51,50,23,38),
                   (51,-50,0,26), 1.0),

                  ((42,28,4,-46),
                   (-23,21,-47,126,-70,5),
                   (-23,-1,-51,101,-33,15),
                   (51,-39,0,32), 0.5),

                  ((98,2,-72,-65),
                   (0,0,-50,120,-70,0),
                   (0,0,-50,120,-70,0),
                   (98,-2,72,65), 1.1),

                  ( INITIAL_POS_DEG[0][1], # Start from [1] to skip head
                    INITIAL_POS_DEG[0][2],
                    INITIAL_POS_DEG[0][3],
                    INITIAL_POS_DEG[0][4],0.5)))

#STABLE_WALK_INITIAL_POSE = [
#                (HEAD_POS_FRONT_BOTTOM,
#                 (1.7471840000000001,0.25460203999999997,-1.5662560000000001,-0.33130205000000001),
#                 (0.0061779618000000003,-0.0076280384999999999,-0.78536605999999998,1.5431621,-0.78238200999999996,0.016915962),
#                 (0.0061779618000000003,0.072139964000000001,-0.77931397999999996,1.53711,-0.79303604000000005,-0.073590039999999995),
#                 (1.734996,-0.25008397999999998,1.5646381,0.36053199000000002),
#                 1.0),
#                ]

# FOR NEWER/FASTER walk
STRAIGHT_WALK_INITIAL_POSE = [
                (HEAD_POS_FRONT_BOTTOM,
                (1.734912, 0.25460203999999997, -1.563188, -0.52918803999999997),
                (-0.010696038999999999, -0.088930041000000001, -0.65957807999999996, 1.5416281000000001, -0.76857597, 0.078275964000000003),
                (-0.010696038999999999, 0.0061779618000000003, -0.66272998000000005, 1.5432459999999999, -0.78229808999999995, -0.0030260384000000001),
                (1.747268, -0.27309397000000002, 1.5661721, 0.50626194000000002),
                 1.5),
                ]

# SLOW walk
#STRAIGHT_WALK_INITIAL_POSE = [
#                (HEAD_POS_FRONT_BOTTOM,
#                 (1.7655921000000001, 0.27914604999999998, -1.558586, -0.50157607000000004),
#                 (0.010779962000000001, -0.047512039999999998, -0.57214003999999996, 1.0384761, -0.52160196999999997, 0.042993963000000003),
#                 (0.010779962000000001, 0.039925963000000002, -0.58449596000000004, 1.0308900000000001, -0.51845001999999996, -0.041376039000000003),
#                 (1.7380640999999999, -0.25468596999999998, 1.5615699999999999, 0.54307795000000003),
#                 1.0),
#                ]

SIDE_WALK_INITIAL_POSE = [
                (HEAD_POS_FRONT_BOTTOM,
                (1.7303101000000001, 0.25460203999999997, -1.563188, -0.53072202000000002),
                (-0.0014920383999999999, -0.078192040000000004, -0.85899806000000001, 1.5416281000000001, -0.771644, 0.081343964000000005),
                (-0.0014920383999999999, 0.018449962, -0.87135397999999997, 1.5432459999999999, -0.77156007000000004, -0.015298038999999999),
                (1.753404, -0.25468596999999998, 1.5646381, 0.51086396000000001),
                1.5)
]
def getMoveTime(move):
    totalTime = 0.0
    for target in move:
        if len(target) == 6:
            totalTime += target[4]
        elif len(target) == 3:
            totalTime += target[1]
    return totalTime
