#!/usr/bin/python


from events import *

class Player(object):

    def __init__(self, world, eventmanager, actions):
        self._world = world
        self._eventmanager = eventmanager
        self._actions = actions
        self._eventmanager.register(EVENT_FALLEN_DOWN, self.onFallenDown)
        self._eventmanager.register(EVENT_ON_BELLY, self.onOnBelly)
        self._eventmanager.register(EVENT_ON_BACK, self.onOnBack)
    
    def onGCPlaying(self):
        """ only state player needs to deal with, the rest are done
        by the events object """
        self.onStart()

    def onStart(self):
        self.onInitial()

    def onStop(self):
        """ implemented by inheritor from Player. Called whenever player
        moves from action to no action. (game controller moves from playing
        state to another, or when changing behaviors)

        Needs to take care of cleaning up: stop any action you were in the middle of,
        i.e. clearFootsteps.
        """
        self._actions.clearFootsteps()

    def onInitial(self):
        self._actions.say("ENTERING INITIAL")
        def onLeftBumperPressed(self=self):
            self._world.playerSettings.toggleteamColor();
            if debug:
                print "Team number: %d. Player number: %d." % (self._world.playerSettings.teamColor, self._world.playerSettings.playerNumber)
        def onRightBumperPressed(self=self):
            self._world.playerSettings.togglePlayerNumber();
            if debug:
                print "Team number: %d. Player number: %d." % (self._world.playerSettings.teamColor, self._world.playerSettings.playerNumber)
        def onChestButtonPressed(self=self):
            if debug:
                print "Leaving initial."
                print "Team number: %d. Player number: %d." % (self._world.playerSettings.teamColor, self._world.playerSettings.playerNumber)
            for event in [EVENT_LEFT_BUMPER_PRESSED, EVENT_RIGHT_BUMPER_PRESSED, EVENT_CHEST_BUTTON_PRESSED]:
                self._eventmanager.unregister(event)
            self.onConfigured()
        self._eventmanager.register(EVENT_LEFT_BUMPER_PRESSED, onLeftBumperPressed)
        self._eventmanager.register(EVENT_RIGHT_BUMPER_PRESSED, onRightBumperPressed)
        self._eventmanager.register(EVENT_CHEST_BUTTON_PRESSED, onChestButtonPressed)

    def onConfigured(self):
        pass # TODO: Go to the state that's associated with the current GameState.

    def onPlay(self):
        pass

    def onReady(self):
        pass

    def onSet(self):
        pass

    def onPenalized(self):
        pass

    def onFallenDown(self):
        print "I'm down!"
        self._eventmanager.unregister(EVENT_FALLEN_DOWN)

    def onOnBack(self):
        print "I'm on my back."
        self._eventmanager.unregister(EVENT_ON_BACK)
        # temporarily removed
        #self._actions.executeGettingUpBack().onDone(self.gettingUpDoneBack)
    
    def gettingUpDoneBack(self):
        print "Getting up done!"
        self._eventmanager.register(EVENT_ON_BACK, self.onOnBack)

    def onOnBelly(self):
        print "I'm on my belly."
        self._eventmanager.unregister(EVENT_ON_BELLY)
        # temporarily removed
        #self._actions.executeGettingUpBelly().onDone(self.gettingUpDoneBelly)
        
    def gettingUpDoneBelly(self):
        print "Getting up done!"
        self._eventmanager.register(EVENT_ON_BELLY, self.onOnBelly)

    # Utilities

    def registerDecoratedEventHandlers(self):
        """
        Intended usage: if you have a clear method -> event in your player, you can
        mark each such method with the event it handles like so:
        @eventhandler(EVENT_YGRP_POSITION_CHANGED)
        def my_method(self):
            ...

        You will then have access to self.my_method.event as the event you gave,
        and this utility method uses that to register all of those events without
        having to redclare the event name -> method pairing.
        """
        # register to events - see singletime
        for fname in [fname for fname in dir(self) if hasattr(getattr(self, fname), 'event')]:
            f = getattr(self, fname)
            self._eventmanager.register(f.event, f)


if __name__ == '__main__':
    print "Welcome to the player module's testing procedure. Have fun."
    from eventmanager import MainLoop
    mainloop = MainLoop(GameControllerTester)
    mainloop.run()
