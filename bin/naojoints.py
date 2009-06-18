#!/usr/bin/python

# add path to burst lib
import os
import sys

if not 'AL_DIR' in os.environ:
    os.environ['AL_DIR'] = '/usr/local/nao'
    print "warning: $AL_DIR not defined, defining to %s" % os.environ['AL_DIR']

burst_lib = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), '../lib'))
sys.path.append(burst_lib)
from burst_consts import ROBOT_IP_TO_NAME
from burst_util import set_robot_ip_from_argv

def main():
    try:
        import pynaoqi.gui
        pynaoqi.gui.main()
    except Exception, e:
        print "oops, %s" % e
        import pdb; pdb.set_trace()

if __name__ == '__main__':
    set_robot_ip_from_argv()
    main()

