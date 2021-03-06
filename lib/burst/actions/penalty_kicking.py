'''
Created on Jun 14, 2009

@author: Alon & Eran
'''

from math import pi

from burst_util import (BurstDeferred, succeedBurstDeferred,
    Nameable, calculate_middle, calculate_relative_pos,
    polar2cart, cart2polar, nicefloats)

# local imports
import burst
from burst_events import (EVENT_OBSTACLE_SEEN, EVENT_OBSTACLE_LOST, EVENT_OBSTACLE_IN_FRAME)
import burst.actions
from burst.actions.target_finder import TargetFinder
import burst.moves as moves
import burst.moves.walks as walks
import burst.moves.poses as poses
from burst.behavior import Behavior
from burst_field import MIDFIELD_X

from burst.behavior_params import (calcTarget, getKickingType, MAX_FORWARD_WALK, MAX_SIDESTEP_WALK, BALL_IN_KICKING_AREA, BALL_BETWEEN_LEGS,
                                   BALL_FRONT_NEAR, BALL_FRONT_FAR, BALL_SIDE_NEAR, BALL_SIDE_FAR, BALL_DIAGONAL,
                                   MOVEMENT_PERCENTAGE_FORWARD, MOVEMENT_PERCENTAGE_SIDEWAYS, MOVEMENT_PERCENTAGE_TURN,
                                   MOVE_FORWARD, MOVE_ARC, MOVE_TURN, MOVE_SIDEWAYS, MOVE_CIRCLE_STRAFE, MOVE_KICK, MIN_FORWARD_WALK)
from burst_consts import (LEFT, RIGHT, IMAGE_CENTER_X, IMAGE_CENTER_Y,
    PIX_TO_RAD_X, PIX_TO_RAD_Y, DEG_TO_RAD)
import burst_consts
import random

#===============================================================================
#    Logic for Kicking behavior:
#
# 1. Scan for ball
# 2. Advance towards ball (while tracking ball)
# 3. When near ball, circle-strafe (to correct direction) till goal is seen and centered (goal is tracked)
# 4. Search ball down, align against ball and kick
#
# Handling Sonar data (obstacles):
# When obstacle encountered DURING move:
# * stop move only if it's a long walk (if it's a short walk to the ball, we prefer not to stop...)
# When obstacle encountered BEFORE move:
# * Kicking: change kick to inside-kick/angle-kick if obstacle is at center
#   (Goalie->inside kick towards opposite side, Kicker-> angle-kick towards field center)
# * Ball far: side-step to opposite side (or towards field center if at center)
#
# TODO's:
# * RESET self._aligned_to_goal when needed
#===============================================================================

GOAL_FAILURES_BEFORE_GIVEUP = 2 # count
MOVE_WHEN_GOAL_LOST_GOOD = 50.0 # [cm]

class BallKicker(Behavior):

    def __init__(self, actions, target_left_right_posts, align_to_target=True):
        super(BallKicker, self).__init__(actions = actions, name = 'BallKicker')

        self.verbose = True

        self._align_to_target = align_to_target

        self._sonar = self._world.robot.sonar
        self._eventmanager.register(self.onObstacleSeen, EVENT_OBSTACLE_SEEN)
        self._eventmanager.register(self.onObstacleLost, EVENT_OBSTACLE_LOST)
        self._eventmanager.register(self.onObstacleInFrame, EVENT_OBSTACLE_IN_FRAME)

        self._ballFinder = TargetFinder(actions=actions, targets=[self._world.ball], start=False)
        self._ballFinder.setOnTargetFoundCB(self._approachBall)
        self._ballFinder.setOnTargetLostCB(self._stopOngoingMovement)
        self._ballFinder.setOnSearchFailedCB(self._onBallSearchFailed)
        
        self.target_left_right_posts = target_left_right_posts
        self._goalFinder = TargetFinder(actions=actions, targets=self.target_left_right_posts, start=False)
        self._goalFinder.setOnTargetFoundCB(self.onGoalFound)
        self._goalFinder.setOnSearchFailedCB(self._onGoalSearchFailed)
        self._currentFinder = None

    def _start(self, firstTime=False):
        self._aligned_to_goal = True
        self._diag_kick_tested = False
        self._movement_deferred = None
        self._movement_type = None
        self._movement_location = None

        self._is_strafing = False
        self._is_strafing_init_done = False

        self._obstacle_in_front = None
        self._target = self._world.ball

        self._initBallMovements()

        self._numGoalFailedStrafes = GOAL_FAILURES_BEFORE_GIVEUP # do this number, then giveup, walk 1 m, return to approachBall

        # kicker initial position
        self._actions.executeMove(poses.STRAIGHT_WALK_INITIAL_POSE).onDone(
            lambda: self.switchToFinder(to_goal_finder=False))

    def _stop(self):
        print "KICKING STOPS!!!"
        self._clearMovement(clearFootsteps = True)

        stop_bd = succeedBurstDeferred(self)
        if self._currentFinder:
            print "STOPPING CURRENT FINDER: %s" % self._currentFinder.name
            stop_bd = self._currentFinder.stop()
        return stop_bd

    ################################################################################
    # Handling movements
    #
    def _clearMovement(self, clearFootsteps = False):
        if self._movement_deferred:
            self._movement_deferred.clear()
            if clearFootsteps and self._movement_type in (MOVE_FORWARD, MOVE_SIDEWAYS, MOVE_TURN, MOVE_ARC):
                print "CLEARING FOOTSTEPS!"
                self._actions.clearFootsteps()
        self._movement_deferred = None
        self._movement_type = None
        self._movement_location = None

    def _onMovementFinished(self, nextAction):
        print "Movement DONE!"
        self._clearMovement(clearFootsteps = False)
        nextAction()

    def _stopOngoingMovement(self, forceStop = False):
        # stop movement if we're forced or if it's a long walk-forward move
        shouldStopMovement = forceStop or (self._movement_type == MOVE_FORWARD and self._movement_location == BALL_FRONT_FAR)
        if shouldStopMovement:
            self._clearMovement(clearFootsteps = True)

        print "Kicking: _stopOngoingMovement: current movement %s (forceStop = %s)" % (shouldStopMovement and "STOPPED" or "CONTINUES", forceStop)

    ################################################################################
    # Sonar callbacks
    #
    def onObstacleSeen(self):
        self._obstacle_in_front = self._sonar.getLastReading()
        print "Obstacle seen (on %s, distance of %f)!" % (self._obstacle_in_front)

        if self._movement_deferred:
            # if walking forward and ball is far, stop
            if self._movement_type == MOVE_FORWARD and self._movement_location == BALL_FRONT_FAR:
                print "NOTE: Obstacle seen while a movement is in progress, movement STOPPED"
                self._stopOngoingMovement(forceStop = True)
                self._eventmanager.callLater(0.5, self._approachBall)
            else:
                print "NOTE: Obstacle seen while a movement is in progress, movement CONTINUES"

    def onObstacleLost(self):
        print "Obstacle lost!"
        self._obstacle_in_front = None

    def onObstacleInFrame(self):
        #print "Obstacle in frame!"
        self._obstacle_in_front = self._sonar.getLastReading()
        #print "Obstacle seen (on %s, distance of %f)!" % (self._obstacle_in_front)

    def getObstacleOppositeSide(self):
        if self._obstacle_in_front == None:
            print "NO OBSTACLE DATA?"
            opposite_side_from_obstacle = 0
        elif self._obstacle_in_front[0] == "center":
            opposite_side_from_obstacle = random.choice((-1,1))
        elif self._obstacle_in_front[0] == "left":
            opposite_side_from_obstacle = -1
        elif self._obstacle_in_front[0] == "right":
            opposite_side_from_obstacle = 1
        return opposite_side_from_obstacle

    ################################################################################
    # Ball finding functionality - better then just plain old search!
    #
    #
    
    def _initBallMovements(self):
        self._ball_search_first_failure = True

    def _onBallSearchFailed(self):
        # we are here after searching for a ball. Just make sure it did actually fail.
        if self._world.ball in self._actions.searcher.seen_objects:
            print "@ _onBallSearchFailed called, but ball is visible - proceeding as usual"
            return self._approachBall()
        print "@ Ball Search Failed!!!! Stopping ball Finder"
        self._ballFinder.stop()
        if self._movement_deferred:
            print "@ Movement Deferred - waiting for it to complete and not doing anything here"
            return
        if self._ball_search_first_failure:
            self._ball_search_first_failure = False
            print "@ Turning, and rerunning search"
            self._movement_deferred = self._actions.turn(pi)
            self._movement_type = MOVE_TURN
            self._movement_deferred.onDone(lambda: self._onMovementFinished(self._ballSearch_restart))
        else:
            # TODO
            self._onSecondSearchFail_LocalizeStrategy()
            # TODO
            #self._onSecondSearchFail_GotoGoalStrategy()

    def _ballSearch_restart(self):
        print "@ Ball search: restarting search"
        self.switchToFinder(to_goal_finder=False)

    # Own Goal Seen Strategy: Turn towards closest goal, walk forward
    # according to dist Strategy
    def _onSecondSearchFail_GotoGoalStrategy(self):
        print "TODO - starting ball finder again."
        self._ballFinder.start()

    # Localization strategy: localize, then move to location
    def _onSecondSearchFail_LocalizeStrategy(self):
        # not first time, localize
        print "@ Calling localize"
        self._actions.localize().onDone(self._onSecondSearchFail_LocalizeStrategy_LocalizeOver)

    def _onSecondSearchFail_LocalizeStrategy_LocalizeOver():
        # so we are localized.
        # Let's see where we are:
        print "In SecondSearchFaile, LocalizeStrategy, Localize Over"
        x, y = self.robot.world_x, self.robot.world_y
        if x > MIDFIELD_X:
            target_x, target_y = 180.0, 0.0 # Our penalty
        else:
            target_x, target_y = MIDFIELD_X + 120.0, 0.0 # Their penalty
        import approacher
        dx, dy, dh = approacher.getTargetPosition(target_x, target_y, 0.0) # heading towards opposite goal regardles of penalty.
        self._actions.changeLocationRelative(dx, dy, dh).onDone(self._ballSearch_restart)

    ################################################################################
    # _approachBall helpers (XXX - should they be submethods of _approachBall? would
    # make it cleared to understand the relationship, not require this comment)
    #
    def _approachBall(self):
        print ("\nApproaching %s: (recently seen %s, dist: %3.3f, distSmoothed: %3.3f, bearing: %3.3f)"+"\n"+"-"*100) % (
                  self._target.name, self._target.recently_seen, self._target.dist, self._target.distSmoothed, self._target.bearing)

        # TODO: we probably need a better solution? this can happen after we're aligned,
        # when ball tracker finds the ball while a previous movement is still ON.
        if self._movement_deferred:
            #import pdb; pdb.set_trace()
            if not self._actions.isMotionInProgress() and not self._actions.isWalkInProgress():
                print "ignoring current movement deferred and erasing it"
                self._clearMovement(clearFootsteps = False)
            else:
                print "LAST MOVEMENT STILL ON!!!"
                return

        if not self._target.recently_seen:
            if self._ballFinder.stopped:
                print "TARGET LOST, STARTING BALL FINDER"
                self.switchToFinder(to_goal_finder=False)
            else:
                print "TARGET LOST, RESTARTING BALL FINDER"
                self._ballFinder.stop() # TODO - onDone
                self.switchToFinder(to_goal_finder=False)
            # TODO: searcher / searcher CB should take care of finding target, behavior should take care of turning when search fails
            return

        (side, kp_x, kp_y, kp_dist, kp_bearing, target_location, kick_side_offset) = calcTarget(self._target.distSmoothed, self._target.bearing)

        ### DECIDE ON NEXT MOVEMENT ###
        
        # Ball inside kicking area, kick it
        if target_location == BALL_IN_KICKING_AREA: # and (self._aligned_to_goal or not self._align_to_target):
            # TODO: diagonalize the kick. It might be off target even if we think we are aligned
            
            print "DIAGONAL KICK: Searching goal post 1"
            self._diag_kick_tested = True
            self._diag_kick_forced = True
            (ball_x, ball_y) = polar2cart(self._target.distSmoothed, self._target.bearing)
            self._ballY_lastseen = ball_y
            self._side_last = side
            self._kick_side_offset = kick_side_offset
            #if self._ballFinder:
            #    self._ballFinder.stop().onDone(self.searchGoalPost)
            #    return
            #else:
            #    self.log("NO BALL FINDER 1???")
            self.doKick(side, kick_side_offset)
            return

        if target_location in (BALL_FRONT_NEAR, BALL_FRONT_FAR):
            self.logverbose("Walking straight!")
            self._movement_type = MOVE_FORWARD
            self._movement_location = target_location
            if self._obstacle_in_front and target_location == BALL_FRONT_FAR:
                opposite_side_from_obstacle = self.getObstacleOppositeSide()
                print "opposite_side_from_obstacle: %d" % opposite_side_from_obstacle
                # if we do a significant side-stepping, our goal-alignment isn't worth much anymore...
                self._aligned_to_goal = False
                self._movement_type = MOVE_SIDEWAYS
                self._movement_deferred = self._actions.changeLocationRelativeSideways(
                    0.0, 30.0*opposite_side_from_obstacle, walk=walks.SIDESTEP_WALK)

#                        self._movement_type = MOVE_CIRCLE_STRAFE
#                        if opposite_side_from_obstacle == -1:
#                            strafeMove = self._actions.executeCircleStrafeCounterClockwise
#                        else:
#                            strafeMove = self._actions.executeCircleStrafeClockwise
#                        self._movement_deferred = self._actions.executeCircleStraferInitPose().onDone(strafeMove)

            else:
                self._movement_deferred = self._actions.changeLocationRelative(min(max(kp_x*MOVEMENT_PERCENTAGE_FORWARD,MIN_FORWARD_WALK),MAX_FORWARD_WALK))
        elif target_location in (BALL_BETWEEN_LEGS, BALL_SIDE_NEAR):
            self.logverbose("Side-stepping!")
            movementAmount = min(kp_y*MOVEMENT_PERCENTAGE_SIDEWAYS,MAX_SIDESTEP_WALK)
            # if we do a significant side-stepping, our goal-alignment isn't worth much anymore...
            if movementAmount > 20:
                self._aligned_to_goal = False
            self._movement_type = MOVE_SIDEWAYS
            self._movement_location = target_location
            # TODO: change numbers for side stepping. Does that 4 or 5 times.                            
            self._movement_deferred = self._actions.changeLocationRelativeSideways(
                0.0, movementAmount, walk=walks.SIDESTEP_WALK)
        elif target_location in (BALL_DIAGONAL, BALL_SIDE_FAR):
            self.logverbose("Turning!")
            self._aligned_to_goal = False
            movementAmount = kp_bearing*MOVEMENT_PERCENTAGE_TURN
            # if we do a significant turn, our goal-alignment isn't worth much anymore...
            if movementAmount > 10*DEG_TO_RAD:
                self._aligned_to_goal = False
            self._movement_type = MOVE_TURN
            self._movement_location = target_location
            self._movement_deferred = self._actions.turn(movementAmount)
        else:
            self.logverbose("!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR!!! ball location problematic!")
            #import pdb; pdb.set_trace()

        print "Movement STARTING!"
        self._movement_deferred.onDone(lambda _, nextAction=self._approachBall: self._onMovementFinished(nextAction))

    def doKick(self, side, kick_side_offset):
        # Look for goal, decide if can skip adjustments and kick ball diagonally
        self.logverbose("Kicking!")
        if self._currentFinder:
            self._currentFinder.stop()
            self._currentFinder = None
        self.logverbose("kick_side_offset: %3.3f" % (kick_side_offset))
        self._movement_type = MOVE_KICK
        self._movement_location = BALL_IN_KICKING_AREA

        if self._obstacle_in_front and self._obstacle_in_front[0] == "center":
            # TODO: Change to angle-kick towards left/right side of goal (except for Goalie)
            self._movement_deferred = self._actions.inside_kick(burst.actions.KICK_TYPE_INSIDE, side)
        else:
            self._movement_deferred = self._actions.adjusted_straight_kick(side, kick_side_offset)
        self._movement_deferred.onDone(self.stop)

    def switchToFinder(self, to_goal_finder=False):
        from_finder, to_finder = self._goalFinder, self._ballFinder
        if to_goal_finder:
            from_finder, to_finder = self._ballFinder, self._goalFinder
        else:
            # switch to bottom camera when we look for the ball
            # --- DONT DO THIS UNTIL IMOPS CODE DOES THE SWITCHING, or segfault for you ---
            #self._actions.setCamera(burst_consts.CAMERA_WHICH_BOTTOM_CAMERA)
            pass
        stop_bd = from_finder.stop()
        print "SwitchToFinder: calling %s.start" % (to_finder.name)
        self._currentFinder = to_finder
        # TODO: Yet More Hacks
        doit = lambda f: self._eventmanager.callLater(0.0, f) if not stop_bd.completed() else stop_bd.onDone(f)
        if not to_finder.stopped:
            print "SwitchToFinder: Target finder not stopped. Stopping it."
            to_finder.stop()
            self._eventmanager.callLater(0.5, to_finder.start)
        else:
            doit(to_finder.start)


    ################################################################################
    # Checking goal position before kicking

    def searchGoalPost(self):
        self._currentFinder = None
        self._actions.searcher.search_one_of(self.target_left_right_posts, center_on_targets=False).onDone(self.onSearchGoalPostOver)

    def onSearchGoalPostOver(self):
        self._actions.say('onSearchGoalPostOver')
        
        # calculate target bearing (position inside goal)
        nearestGoalpost = None
        if self.target_left_right_posts[0].centered_self.sighted:
            nearestGoalpost = self.target_left_right_posts[0]
        if self.target_left_right_posts[1].centered_self.sighted:
            if not nearestGoalpost or abs(nearestGoalpost.bearing) > self.target_left_right_posts[1].bearing:
                nearestGoalpost = self.target_left_right_posts[1]
        
        if nearestGoalpost is None:
            self.logverbose("LOST GOAL WHILE TRYING TO KICK?! (switching to goal finder)")
            self._aligned_to_goal = False
            self._diag_kick_tested = True
            if self._diag_kick_forced:
                self.logverbose("SUPPOSED TO BE ALIGNED BUT CAN'T SEE GOAL! KICKING ANYWAY!")
                self.doKick(self._side_last, self._kick_side_offset)
                return

            self._onGoalSearchFailed()
            #self._eventmanager.callLater(0.0, self._approachBall)
            #TODO: RESTART GOAL/Ball TARGET_FINDER
            return

        self._actions.headTowards(nearestGoalpost).onDone(lambda _, nearestGoalpost=nearestGoalpost:
            self.onSearchGoalPostOverAfterHeadTowards(nearestGoalpost))

    def onSearchGoalPostOverAfterHeadTowards(self, nearestGoalpost):
        print "AFTER HEAD MOVE TOWARDS %s" % (nearestGoalpost.name) 
        # Add offset to the diagonal kick (so we'll align not on the actual goalpost, but on about 1/4 of the goal)
        targetBearing = nearestGoalpost.bearing
        if nearestGoalpost == self._world.opposing_lp:
            targetBearing = targetBearing + 0.5/3
        elif nearestGoalpost == self._world.opposing_rp:
            targetBearing = targetBearing - 0.5/3 # TODO: Move to const, calibrate value (cover half-goal? goal? differs for different distances?)

        # check if diagonal kick is viable
        # use self._ballY_lastseen
        kick_side_offset = getKickingType(self, targetBearing, self._ballY_lastseen, self._side_last, margin=0)
        if kick_side_offset is None:
            if self._diag_kick_forced:
                self.logverbose("SUPPOSED TO BE ALIGNED BUT CAN'T SEE GOAL! KICKING ANYWAY!")
                self.doKick(self._side_last, self._kick_side_offset)
                return
            self._eventmanager.callLater(0.0, self._approachBall)
        else:
            # do diagonal kick
            self.doKick(self._side_last, kick_side_offset)
 
    ################################################################################
    # Strafing

    def onGoalFound(self):
        self.logverbose('onGoalFound')
        if not self._is_strafing:
            self.goalpost_to_track = self._goalFinder.getTargets()[0]

            # Add offset to the goalpost align (so we'll align not on the actual goalpost, but on about 1/4 of the goal)
            if self.goalpost_to_track == self._world.opposing_lp:
                self.alignLeftLimit = -0.5
                self.alignRightLimit = 0
            elif self.goalpost_to_track == self._world.opposing_rp:
                self.alignLeftLimit = 0
                self.alignRightLimit = 0.5 # TODO: Move to const, calibrate value (cover half-goal? goal? differs for different distances?)

            g = self.goalpost_to_track
            self.logverbose('onGoalFound: found %s at %s, %s (%s)' % (g.name, g.centerX, g.centerY, g.seen))
            self.strafe()

    def _onGoalSearchFailed(self):
        self._numGoalFailedStrafes -= 1
        if self._numGoalFailedStrafes <= 0:
            print "@ Goal not found, but limit of strafes reached - 1 m forward and back to approach"
            self._numGoalFailedStrafes = GOAL_FAILURES_BEFORE_GIVEUP
            self.switchToFinder(to_goal_finder=False)
            self._movement_type = MOVE_FORWARD
            self._movement_deferred = self._actions.changeLocationRelative(MOVE_WHEN_GOAL_LOST_GOOD)
            self._movement_deferred.onDone(lambda _, nextAction=self._approachBall: self._onMovementFinished(nextAction))
            return
        print "@ Goal not found - doing some strafing (how do I do this for 180 degrees?) - %s left" % self._numGoalFailedStrafes
        #self._actions.searcher.stop()
        
        self._movement_type = MOVE_TURN
        #self._actions.turn(pi)
        num_circle_strafes = 8
        self._movement_deferred = self._actions.executeCircleStraferInitPose()
        for i in xrange(num_circle_strafes):
            self._movement_deferred = self._movement_deferred.onDone(self._actions.executeCircleStrafeCounterClockwise)
        #self._movement_deferred = self._actions.turn(pi)
        self._movement_deferred.onDone(lambda: self._onMovementFinished)
        self._movement_deferred.onDone(self.restartGoalFinderAfterFailure)

    def restartGoalFinderAfterFailure(self):
        self.log("Restarting goal finder")
        self._eventmanager.callLater(0.5, lambda: self.switchToFinder(to_goal_finder=True))

    count_strafe = 1

    def strafe(self):
        self._is_strafing = True

        if not self.goalpost_to_track.seen:
            self.logverbose("strafe: goal post not seen")
            # Eran: Needed? won't goal-post searcher wake us up? Can't this create a case where strafe is called twice?
            self._eventmanager.callLater(self._eventmanager.dt, self.strafe)
            return
        self.logverbose("strafe: goal post seen")
        self.logverbose("%s bearing is %s (seen %s, all %s). Left is %s, Right is %s" % (
                self.goalpost_to_track.name,
                self.goalpost_to_track.bearing,
                self.goalpost_to_track.seen, str(['%3.2f (c %3.2f) %s %s' % (x.bearing,
                    x.centered_self.bearing,
                    'seen' if x.seen else 'not seen', 'centered' if x.centered else 'not centered')
                    for x in self._world.opposing_goal.left_right_unknown]),
                self.alignLeftLimit,
                self.alignRightLimit))
        # TODO: Add align-to-goal-center support
        if self.goalpost_to_track.bearing < self.alignLeftLimit:
            #strafeMove = lambda: self._actions.executeCircleStrafeClockwise().onDone(self._actions.executeCircleStrafeClockwise)
            strafeMove = self._actions.executeCircleStrafeClockwise
            print "#### About to do a clockwise strafe"
        elif self.goalpost_to_track.bearing > self.alignRightLimit:
            #strafeMove = lambda: self._actions.executeCircleStrafeCounterClockwise().onDone(self._actions.executeCircleStrafeCounterClockwise)
            strafeMove = self._actions.executeCircleStrafeCounterClockwise
            print "#### About to do a counter clockwise strafe"
        else:
            self._is_strafing = False
            self._is_strafing_init_done = False
            self.logverbose("Aligned position reached! (starting ball search)")
            self._aligned_to_goal = True
            if self._goalFinder:
                self._goalFinder.stop().onDone(self.refindBall)
            else:
                self.refindBall()
            return

        self._movement_type = MOVE_CIRCLE_STRAFE
        self._movement_location = BALL_BETWEEN_LEGS
        if not self._is_strafing_init_done:
            self.logverbose("Aligning and strafing...")
            self._is_strafing_init_done = True
            self._movement_deferred = self._actions.executeCircleStraferInitPose().onDone(strafeMove)
        else:
            self.logverbose("Strafing...")
            self._movement_deferred = strafeMove()

        # We use call later to allow the strafing to handle the correct image (otherwise we get too much strafing)
        nextAction = lambda _: self._onMovementFinished(lambda: self._eventmanager.callLater(0.2, self.strafe))

        print "Movement STARTING! (strafing)"
        self._movement_deferred.onDone(nextAction)

    def refindBall(self):
        self._currentFinder = None
        self._actions.executeHeadMove(poses.HEAD_MOVE_FRONT_BOTTOM).onDone(
            # TODO: Fix distSmooth after moving head - this is just a workaround
            lambda: self._eventmanager.callLater(0.5,
                 lambda: self.switchToFinder(to_goal_finder=False)))

