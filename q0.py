#!/usr/bin/python

import sys

def main(filename):
    minLat = 48.06000
    maxLat = 48.24900
    deltaLat = maxLat - minLat

    minLon = 11.35800
    maxLon = 11.72400
    deltaLon = maxLon - minLon

    infile = open(filename, 'r')
    lines = infile.readlines()
    infile.close()

    outfile = open('newData.tsv', 'w')
    for line in lines:
	line = line.strip()
        items = line.split('\t')
        newLat = ((float(items[2]) - minLat) / deltaLat) * 1000
        newLon = ((float(items[3]) - minLon) / deltaLon) * 1000
        outfile.write("%s\t%s\t%s\t%s\t%f\t%f\n" % (items[0], items[1], items[2], items[3], newLat, newLon))

    outfile.close()

if len(sys.argv) == 2:
    main(sys.argv[1])
else:
    print('Usage: python q0.py <filename.tsv>')
