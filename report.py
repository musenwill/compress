import os
import csv
import sys
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt


tsfile = sys.argv[1]
files = [tsfile]
dir = os.path.dirname(tsfile)
basename = os.path.basename(tsfile)

idx = 2
while idx < len(sys.argv):
    files.append(sys.argv[idx])
    idx += 1

hasTimestampBase = False
timestampBase = 0
tsdatas = []

for f in files:
    with open(f, mode='r') as file:
        tsdata = {'timestamp': [], 'tsfield': []}
        for line in file.readlines():
            line = line.strip()
            if len(line) == 0:
                continue
            sections = line.split('    ')
            row = []
            for section in sections:
                section = section.strip()
                row.append(section)
            timestamp, value = row
            timestamp = int(timestamp)
            if not hasTimestampBase:
                hasTimestampBase = True
                timestampBase = timestamp
            timestamp = timestamp - timestampBase
            tsdata['timestamp'].append(timestamp)
            tsdata['tsfield'].append(float(value))
        tsdatas.append(tsdata)

colors = ['blue', 'red', 'green']
plt.figure(figsize=(120, 60), dpi=256)

datasetCount = 0
for tsdata in tsdatas:
    plt.plot(tsdata['timestamp'], tsdata['tsfield'], color=colors[datasetCount])
    datasetCount += 1

plt.title("lossy compress")
plt.xlabel("timestamp")
plt.ylabel("tsfield")
plt.legend("lossy compress")

filename = os.path.join(dir, basename+".png")
plt.savefig(filename)

