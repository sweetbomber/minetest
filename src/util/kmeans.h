/*
Minetest
Copyright (C) 2013 sweetbomber <ffrogger0@yahoo.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef KMEANS_HEADER
#define KMEANS_HEADER

#define KMEANS_MAX_IT 100
#define KMEANS_DISTANCE(A, B) (((A).X - (B).X)*((A).X - (B).X) + ((A).Y - (B).Y)*((A).Y - (B).Y) + ((A).Z - (B).Z)*((A).Z - (B).Z))

class ClusterDataPoint{
  public:
  void *data;
  v3s16 point;
};

class Cluster {

public:
	v3s16 centroid;           // The centroid of the cluster (center of mass of all points)
  v3s16 accumulator;        // Accumulator used to update centroid. Centroid = accumulator/NumberOfPoints
  s32 numberOfPoints;
  std::list<ClusterDataPoint *> dataPoints; // Point data: Includes Vector + pointer to data

	Cluster(v2s16 xLimits, v2s16 yLimits, v2s16 zLimits);
	~Cluster();
  void resetAccumulator();
  void randomize(v2s16 xLimits, v2s16 yLimits, v2s16 zLimits);
};

class KMeans {
public:

	KMeans();
	~KMeans();

  void addDataPoint(void *data, v3s16 point);         // Add a new data Point
  void clusterize(s16 numberOfClusters);                                  // Calculate clusters' centroids
//  Cluster *classifyPoint(v3s16 point);                // Return the closest cluster to the given point
  void *getNearestDataPoint(v3s16 point); // Return the closest data from point

  void listPoints();
private:
	bool dataPointRegistrationFinished;
	bool clusterizationFinished;
	std::list<Cluster *> clusters;             // Forward list of all clusters
  std::list<ClusterDataPoint *> dataPoints;  // All point data
  v2s16 xLimits;                             // Min/Max of x, y, z.   in xLimits, 'X' denotes min and 'Y' denotes max
  v2s16 yLimits;
  v2s16 zLimits;

};

#endif
