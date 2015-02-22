#!/usr/bin/env python

from matplotlib import pyplot

times = []

with open('/home/ahota/workspace/monstermash/src/test/output.log') as log:
    for line in log.readlines():
        minutes = int(line[0])
        seconds = float(line[2:7])
        times.append(minutes*60 + seconds)

pyplot.plot(times)
pyplot.xlabel('import size in KB')
pyplot.ylabel('seconds')
pyplot.savefig('test.png', bbox_inches='tight')
