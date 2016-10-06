#!/usr/bin/python

import sys

def main(filename):
    minLat = 48.06000
    maxLat = 48.24900
    deltaLat = maxLat - minLat

    minLon = 11.35800
    maxLon = 11.72400
    deltaLon = maxLon - minLon

    latPerTenMeter = 10.0/111191
    lonPerTenMeter = 10.0/74539
    height = (latPerTenMeter/deltaLat) * 1000  # height and width for a 10mX10m box on the grid
    width = (lonPerTenMeter/deltaLon) * 1000   #

    infile = open(filename, 'r')  # read the data file
    lines = infile.readlines()
    infile.close()

    outfile1 = open('newData.tsv', 'w') # new data for poi table
    outfile2 = open('rtreeData.tsv', 'w') # new data for rtree table
    for line in lines:
	line = line.strip()
        items = line.split('\t')
        newLat = ((float(items[2]) - minLat) / deltaLat) * 1000
        newLon = ((float(items[3]) - minLon) / deltaLon) * 1000
        minX = newLon
        maxX = newLon + width
        minY = newLat - height
        maxY = newLat
        outfile1.write("%s\t%s\t%s\t%s\t%f\t%f\t%f\t%f\t%f\t%f\n" % (items[0], items[1], items[2], items[3], newLat, newLon, minX, maxX, minY, maxY))
        outfile2.write("%s\t%f\t%f\t%f\t%f\n" % (items[0], minX, maxX, minY, maxY))

    outfile1.close()
    outfile2.close()

if len(sys.argv) == 2:
    main(sys.argv[1])
else:
    print('Usage: python q0.py <filename.tsv>')
