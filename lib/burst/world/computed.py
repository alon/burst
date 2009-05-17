from math import cos, sin, sqrt, pi, fabs, atan, atan2

from ..events import (EVENT_BALL_IN_FRAME, EVENT_KP_CHANGED)

class Computed(object):
    """ place holder for any computed value, currently just the kicking point, that
    doesn't naturally belong to any other object, like ball speed etc
    """

    DEBUG_KP = False

    def __init__(self, world):
        self._world = world
        self._team = world.team
        # Kick Point
        self.kp = None
        self.kp_valid = False
        self.kp_k = 30.0

    def calc_events(self, events, deferreds):
        """
        we calculate:
          kick point
        """
        if (self._team.target_goal_seen_event in self._world._events
            and EVENT_BALL_IN_FRAME in self._world._events):
            new_kp = self.calculate_kp()
            if not self.kp_valid or (new_kp[0] - self.kp[0] > 1e-5 or new_kp[1] - self.kp[1] > 1e-5):
                events.add(EVENT_KP_CHANGED)
            self.kp_valid = True
            self.kp = new_kp
        else:
            # cache the kp value it self until a new one comes
            self.kp_valid = False

    def calculate_kp(self):
        """ Our kicking point, first iteration, is a point k distant from the ball
        along the line connecting the point in the middle of the target goal
        and the ball in the outward direction.

        The coordinate system is the standard: the x axis is to the front,
        the y axis is to the left of the robot. The bearing is measured from the x axis ccw.
        
        computation:
         c - goal center
         b - ball position
         r - robot 
         n - normal pointing from goal center to ball
         kp - kicking point (x, y, bearing)
        """
        # left - l, right - r, bearing - b, dist - d
        team = self._team
        left_post, right_post, ball = team.left_post, team.right_post, self._world.ball
        left_alpha, left_dist, right_alpha, right_dist = (
            left_post.bearing, left_post.dist, right_post.bearing, right_post.dist)
        
        ball_alpha, ball_dist = ball.bearing, ball.dist
        ball_x, ball_y = ball_dist * cos(ball_alpha), ball_dist * sin(ball_alpha)
        k = self.kp_k
        center_x = (right_dist * cos(right_alpha) + left_dist * cos(left_alpha)) / 2.0
        center_y = (right_dist * sin(right_alpha) + left_dist * sin(left_alpha)) / 2.0
        normal_x, normal_y = ball_x - center_x, ball_y - center_y # normal is a vector pointing from center to ball
        normal_norm = sqrt(normal_x**2 + normal_y**2)
        normal_x, normal_y = normal_x / normal_norm, normal_y / normal_norm
        kick_point_x, kick_point_y = ball_x + k * normal_x, ball_y + k * normal_y
        kick_point_norm = (kick_point_x**2 + kick_point_y**2)**0.5
        kick_point_bearing = atan2(-normal_y, -normal_x)
        if self.DEBUG_KP:
            print "KP: left post bearing/dist (%3.3f, %3.3f), right post bearing/dist (%3.3f, %3.3f)" % (
                left_post.bearing, left_post.dist, right_post.bearing, right_post.dist)
            print "KP: b (%3.3f, %3.3f), c(%3.3f, %3.3f), n(%3.3f, %3.3f), kp(%3.3f, %3.3f, %3.3f)" % (
                ball_x, ball_y, center_x, center_y, normal_x, normal_y,
                            kick_point_x, kick_point_y, kick_point_bearing)
        return kick_point_x, kick_point_y, kick_point_bearing

