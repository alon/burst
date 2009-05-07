#
# Event enumeration
#

from events import FIRST_EVENT_NUM, LAST_EVENT_NUM, EVENT_STEP

EVENT_MANAGER_DT = 0.1

class SuperEvent(object):
    
    def __init__(self, eventmanager, events):
        self._events = events
        self._waiting = set(events)
        self._eventmanager = eventmanager

class AndEvent(SuperEvent):

    def __init__(self, eventmanager, events, cb):
        super(AndEvent, self).__init__(eventmanager, events, cb)
        self._cb = cb
        for event in events:
            eventmanager.register(event, lambda self=self, event=event: self.onEvent(event))
            
    def onEvent(self, event):
        self._waiting.remove(event)
        if len(self._waiting) == 0:
            for event in self.events:
                self._eventmanager.unregister(event)
            self._cb()

class SerialEvent(SuperEvent):
    
    def __init__(self, eventmanager, events, callbacks):
        super(SerialEvent, self).__init__(eventmanager, events)
        self._callbacks = callbacks
        self._i = 0
        eventmanager.register(self._events[0], self.onEvent)
            
    def onEvent(self, event):
        self._eventmanager.unregister(self._events[self._i])
        self._callbacks[self._i]()
        self._i += 1
        if len(self._events) == self._i:
            return
        self._eventmanager.register(self._events[self._i], self.onEvent)

def expected_argument_count(f):
    if hasattr(f, 'im_func'):
        return f.im_func.func_code.co_argcount - 1 # to account for self
    return f.func_code.co_argcount

class Deferred(object):
    
    """ A Deferred is a promise to call you when some operation is complete.
    It is also concatenatable. What that means for implementation, is that
    when the operation is done we need to call a deferred we stored and gave
    the user when he gave us a callback. That deferred 
    """

    def __init__(self, data):
        self._data = data
        self._ondone = None
        self._completed = False # we need this for concatenation to work
    
    def onDone(self, cb):
        # will be called by cb's return deferred, if any
        deferred = Deferred(data = None)
        self._ondone = (cb, deferred)
        return deferred

    def callOnDone(self):
        self._completed = True
        if self._ondone:
            cb, chain_deferred = self._ondone
            if expected_argument_count(cb) == 0:
                ret = cb()
            else:
                ret = cb(self._data)
            # is it a deferred? if so tell it to execute the deferred
            # we handed out once it is done.
            if isinstance(ret, Deferred):
                ret.onDone(deferred.callOnDone)

class EventManager(object):

    def __init__(self, world):
        """ In charge of computing when certain events happen, keeping track
        of callbacks, and calling them.
        """
        self._events = dict([(event, None) for event in
                xrange(FIRST_EVENT_NUM, LAST_EVENT_NUM)])
        self._world = world

    def register(self, event, callback):
        """ set a callback on an event.
        """
        self._events[event] = callback

    def unregister(self, event):
        self._events[event] = None

    def unregister_all(self):
        for k in self._events.keys():
            self._events[k] = None

    def runonce(self):
        """ Call all callbacks registered based on the new events
        stored in self._world
        """
        events, deferreds = self._world.getEventsAndDeferreds()
        for event in events:
            if self._events[event] != None:
                self._events[event]()
        if self._events[EVENT_STEP]:
            self._events[EVENT_STEP]()
        # TODO: EVENT_TIME_EVENT
        for deferred in deferreds:
            deferred.callOnDone()

class EventManagerLoop(object):
    
    def __init__(self, playerclass):
        """ Must be called after burst.init() - so that any proxies can be
        created at will.
        """
        import actions
        import world
        self._world = world.World()
        self._eventmanager = EventManager(world = self._world)
        self._actions = actions.Actions(world = self._world)
        self._player = playerclass(world = self._world, eventmanager = self._eventmanager,
            actions = self._actions)

    def run(self):
        # a sanity check
        import burst
        try:
            man = burst.ALProxy("Man")
        except:
            print "you are missing the Man proxy - run naoload and uncomment man"
            raise SystemExit
        from time import sleep, time
        # TODO: this should be called from the gamecontroller, this is just
        # a temporary measure. The gamecontroller should keep track of the game state,
        # and when it is changed call the player.
        self._player.onStart()
        while True:
            sleep(EVENT_MANAGER_DT)
            self._world.update()
            self._eventmanager.runonce()
