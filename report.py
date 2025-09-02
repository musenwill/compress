import os
import csv
import sys
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt

files = []
idx = 2
while idx < len(sys.argv):
    files.append(sys.argv[idx])
    idx += 1

hasTimestampBase = False
timestampBase = 0
tsdatas = []

for f in files:
    with open(f, mode='r') as file:
        tsdata = {'timestamp': [], 'tsfield': [], 'delta': []}
        for line in file.readlines():
            line = line.strip()
            if len(line) == 0:
                continue
            sections = line.split('    ')
            row = []
            for section in sections:
                section = section.strip()
                row.append(section)

            timestamp = row[0]
            value = row[1]
            timestamp = int(timestamp)
            if not hasTimestampBase:
                hasTimestampBase = True
                timestampBase = timestamp
            timestamp = timestamp - timestampBase
            tsdata['timestamp'].append(timestamp)
            tsdata['tsfield'].append(float(value))
            if len(row) > 2:
                tsdata['delta'].append(float(row[2]))
        tsdatas.append(tsdata)

colors = ['blue', 'red', 'green']
plt.subplot(len(tsdatas) + 1, 1, 1)
plt.figure(figsize=(120, 35 * (len(tsdatas) + 1)), dpi=256)

picIdx = 1
for tsdata in tsdatas:
    plt.subplot(len(tsdatas) + 1, 1, picIdx)
    plt.plot(tsdata['timestamp'], tsdata['tsfield'], colors[picIdx-1])
    plt.title("picture-" + str(picIdx))
    plt.legend("_")
    picIdx += 1

plt.subplot(len(tsdatas) + 1, 1, picIdx)
datasetCount = 0
for tsdata in tsdatas:
    plt.plot(tsdata['timestamp'], tsdata['tsfield'], color=colors[datasetCount])
    datasetCount += 1
plt.title("lossy compress")
plt.legend("_")

outfile = sys.argv[1]
dir = os.path.dirname(files[0])
outfilepath = os.path.join(dir, outfile)
plt.savefig(outfilepath)
