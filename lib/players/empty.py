#!/usr/bin/python

import player_init
import burst
from burst.player import Player


class Empty(Player):
    pass




if __name__ == '__main__':
    print "Welcome to empty.py's testing thingie. Have fun."
    from burst.eventmanager import MainLoop
    mainloop = MainLoop(Empty)
    mainloop.run()