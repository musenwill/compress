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
plt.subplot(2,1,1)
plt.figure(figsize=(120, 90), dpi=256)

plt.subplot(2,1,1)
datasetCount = 0
for tsdata in tsdatas:
    plt.plot(tsdata['timestamp'], tsdata['tsfield'], color=colors[datasetCount])
    datasetCount += 1
plt.title("lossy compress")
plt.legend("_")

plt.subplot(2,1,2)
datasetCount = 0
for tsdata in tsdatas:
    if datasetCount <= 0:
        datasetCount += 1
        continue
    plt.plot(tsdata['timestamp'], tsdata['delta'], color=colors[datasetCount])
    datasetCount += 1
plt.title("delta")
plt.legend("_")

outfile = sys.argv[1]
dir = os.path.dirname(files[0])
outfilepath = os.path.join(dir, outfile)
plt.savefig(outfilepath)
