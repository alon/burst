""" Personal file for a robot by the name of this file. """

## Behavior params
import burst.behavior_params as params
params.KICK_X_MIN = [14,14]
params.KICK_X_MAX = [20,20]
params.KICK_Y_MIN = [3.5,-4.5]
params.KICK_Y_MAX = [7,-8]

import burst.moves.walks as walks

walks.STRAIGHT_WALK = walks.Walk('STRAIGHT_WALK', walks.WalkParameters([
           100.0 * walks.DEG_TO_RAD, # ShoulderMedian
           20.0 * walks.DEG_TO_RAD,  # ShoulderAmplitude
           30.0 * walks.DEG_TO_RAD,  # ElbowMedian
           20.0 * walks.DEG_TO_RAD,  # ElbowAmplitude
           3.3,                   # LHipRoll(degrees)
           -5,                  # RHipRoll(degrees)
           0.19,                  # HipHeight(meters)
           -5.5,                   # TorsoYOrientation(degrees) - stopped adjusting to the negative direction - there is a possibility that a little bit more negative is better
           0.05,                  # StepLength
           0.014,                  # StepHeight
           0.04,                  # StepSide
           0.3,                   # MaxTurn
           0.013,                  # ZmpOffsetX
           0.015]),                  # ZmpOffsetY
           25          # 20ms count per step
    )



walks.TURN_WALK = walks.Walk('TURN_WALK', walks.WalkParameters([
           100.0 * walks.DEG_TO_RAD, # ShoulderMedian
           20.0 * walks.DEG_TO_RAD,  # ShoulderAmplitude
           30.0 * walks.DEG_TO_RAD,  # ElbowMedian
           20.0 * walks.DEG_TO_RAD,  # ElbowAmplitude
           3.3,                   # LHipRoll(degrees)
           -5,                  # RHipRoll(degrees)
           0.19,                  # HipHeight(meters)
           5.0,                   # TorsoYOrientation(degrees) - stopped adjusting to the negative direction - there is a possibility that a little bit more negative is better
           0.05,                  # StepLength
           0.014,                  # StepHeight
           0.04,                  # StepSide
           0.3,                   # MaxTurn
           0.013,                  # ZmpOffsetX
           0.015]),                  # ZmpOffsetY
           25          # 20ms count per step
    )
walks.SIDESTEP_WALK = walks.Walk('SIDESTEP_WALK', walks.WalkParameters([
           100.0 * walks.DEG_TO_RAD, # ShoulderMedian
           20.0 * walks.DEG_TO_RAD,  # ShoulderAmplitude
           30.0 * walks.DEG_TO_RAD,  # ElbowMedian
           20.0 * walks.DEG_TO_RAD,  # ElbowAmplitude
           4.5,                   # LHipRoll(degrees)
           -4.5,                  # RHipRoll(degrees)
           0.19,                  # HipHeight(meters)
           5.0,                   # TorsoYOrientation(degrees) - stopped adjusting to the negative direction - there is a possibility that a little bit more negative is better
           0.02,                  # StepLength
           0.015,                  # StepHeight
           0.04,                  # StepSide
           0.3,                   # MaxTurn
           0.015,                  # ZmpOffsetX
           0.02]),                  # ZmpOffsetY
           25          # 20ms count per step
    )
#walks.STRAIGHT_WALK.defaultSpeed = 100 # Eran: 80 seems to fall within ~40cm when running kicker, need to check 100 (150 works for sure)
#walks.SIDESTEP_WALK.defaultSpeed = 20 # 20 seems just fine
