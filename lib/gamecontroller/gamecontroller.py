#!/usr/bin/python

import socket, threading

#import burst
#from burst.consts import *
#from events import *


import messages, listener

__all__ = ['GameController']



class SynchronizedContainer(object): # TODO: Move to some util package.
    
    def __init__(self):
        self.lock = threading.Lock()
        self.listeners = set()

    def add(self, item):
        self.lock.acquire()
        self.listeners.add(item)
        self.lock.release()

    def __contains__(self, key):
        self.lock.acquire()
        result = key in self.listeners
        self.lock.release()
        return result
    
    def __getitem__(self, key):
        self.lock.acquire()
        result = self.listeners[key]
        self.lock.release()
        return result

    def __iter__(self):
        self.lock.acquire()
        result = list(self.listeners).__iter__() # Move to another container to avoid concurrent access.
        self.lock.release()
        return result

    def __len__(self):
        self.lock.acquire()
        result = len(self.listeners)
        self.lock.release()
        return result

    def remove(self, item):
        self.lock.acquire()
        self.listeners.remove(item)
        self.lock.release()



class ServerSocket(threading.Thread):

    def __init__(self, listeners, port=0):
        threading.Thread.__init__(self)
        self.listeners = listeners
        self.unreadyListeners = SynchronizedContainer()
        self.lock = threading.Lock()
        self.status = 'created'
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.settimeout(0.1)
        self.server.bind(('',port))
        self.server.listen(5)
        self.status = 'bound'
        print "Listening on ", self.server.getsockname()[1]

    def stop(self):
        self.lock.acquire()
        assert self.status == 'running'
        self.status = 'running and signaled to stop'
        self.lock.release()

    def stopped(self):
        self.lock.acquire()
        result = self.status == 'running and signaled to stop'
        self.lock.release()
        return result

    def run(self):
        self.status = 'running'
        while not self.stopped():
            # Check all of the listeners you've got so far. If any of them has finished handshaking, move them to the ready listeners pool.
            for previouslyUnreadyListener in self.unreadyListeners:
                if previouslyUnreadyListener.ready():
                    # See if a robot bearing the same identification is already connected. If so, eject that older instance, but report doing so.
                    ejected = False
                    for oldListener in self.listeners:
                        if oldListener.identification == previouslyUnreadyListener.identification:
                            ejected = True
                            self.listeners.remove(previouslyUnreadyListener)
                            break
                    # Either way, add the new listener.
                    self.listeners.add(previouslyUnreadyListener)
            # See if any more listeners are incoming.
            try:
                channel, details = self.server.accept()
                channel.setblocking(False)
                newListener = listener.Listener(channel, details)
                self.unreadyListeners.add(newListener)
            except socket.timeout:
                pass



class GameController(object):
    
    def __init__(self, port=0):
        self.listeners = SynchronizedContainer()
        self.serverSocket = ServerSocket(self.listeners, port)

    def send(self, message, recipient=None):
        messageSent = False        
        if recipient == None:
            messageSent = True # A message to be sent to everyone is considered sent even if "everyone" was an empty set.
            print "sending: ", message, "to", len(self.listeners), "users."
            for listener in self.listeners:
                listener.send(message)
        else:
            # TODO - use dictionary instead of iteration
            for listener in self.listeners:
                if listener.identification == recipient:
                    listener.send(message)
                    messageSent = True
        if not messageSent and DEBUGGING:
            print "[ERROR]: GameController.send - could not deliver a message."

    def run(self):
        self.serverSocket.start()



if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        GameController().run()
    else:
        GameController(int(sys.argv[1])).run()

