# ! /usr/bin/env python
# coding=utf-8
import pty
import os
# import select


def mkpty():
    master, slave = pty.openpty()
    slaveName = os.ttyname(slave)
    print('\nslave device names: ', slaveName)
    return master


if __name__ == "__main__":

    master = mkpty()
    while True:
        data = os.read(master, 128)
        print("The rec msg is: %s" % (data))
