#!/usr/bin/python
import os
from math import cos, sin
in_tree_dir = os.path.join(os.environ['HOME'], 'src/burst/lib/players')
if os.getcwd() == in_tree_dir:
    # for debugging only - use the local and not the installed burst
    print "DEBUG - using in tree burst.py"
    import sys
    sys.path.insert(0, os.path.join(os.environ['HOME'], 'src/burst/lib'))

from burst.player import Player
from burst.events import EVENT_BALL_IN_FRAME
import burst.moves as moves
from burst_util import polar2cart

class visionTesting(Player):
    
    def onStart(self):

        self._eventmanager.register(EVENT_BALL_IN_FRAME, self.trackBall)
        self._actions.initPoseAndStiffness().onDone(self.goalieInitPos)

    def goalieInitPos(self):
        self._actions.executeMove(moves.SIT_POS)
    
    def trackBall(self):
        self._actions.executeTracking(self._world.ball)
        ball_x = self._world.ball.dist * cos(self._world.ball.bearing)
        ball_y = self._world.ball.dist * sin(self._world.ball.bearing)
        print ball_x," , ", ball_y
        
        
        
if __name__ == '__main__':
    import burst
    from burst.eventmanager import MainLoop
    MainLoop(visionTesting).run()

