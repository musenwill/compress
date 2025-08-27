import os
import csv
import sys
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt


tsfile = sys.argv[1]
compressedFile = sys.argv[2]
dir = os.path.dirname(tsfile)
basename = os.path.basename(tsfile)

hasTimestampBase = False
timestampBase = 0
tsdata = {'timestamp': [], 'tsfield': []}
tsCompressedData = {'timestamp': [], 'tsfield': []}

fileCount = 0
files = [tsfile, compressedFile]
for f in files:
    with open(f, mode='r') as file:
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
            if fileCount > 0:
                tsCompressedData['timestamp'].append(timestamp)
                tsCompressedData['tsfield'].append(float(value))
            else:
                tsdata['timestamp'].append(timestamp)
                tsdata['tsfield'].append(float(value))
    fileCount += 1

plt.figure(figsize=(120, 60), dpi=256)

plt.plot(tsdata['timestamp'], tsdata['tsfield'], label='origin', color='blue')
plt.plot(tsCompressedData['timestamp'], tsCompressedData['tsfield'], label='lossy', color='red')
plt.title("lossy compress")
plt.xlabel("timestamp")
plt.ylabel("tsfield")
plt.legend("lossy compress")

filename = os.path.join(dir, basename+".png")
plt.savefig(filename)

