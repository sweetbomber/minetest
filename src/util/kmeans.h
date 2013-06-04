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
	v3f point;
};

class Cluster {

public:
	v3f centroid;           // The centroid of the cluster (center of mass of all points)
	v3f accumulator;        // Accumulator used to update centroid. Centroid = accumulator/NumberOfPoints
	s32 number_of_points;
	std::list<ClusterDataPoint *> data_points; // Point data: Includes Vector + pointer to data

	Cluster(v2s16 x_limits, v2s16 y_limits, v2s16 z_limits);
	~Cluster();
	void resetAccumulator();
	void randomize(v2s16 x_limits, v2s16 y_limits, v2s16 z_limits);
};

class KMeans {
public:

	KMeans();
	~KMeans();

	void addDataPoint(void *data, v3f point);         // Add a new data Point
	void clusterize(s16 number_of_clusters);                                  // Calculate clusters' centroids
	void *getNearestDataPoint(v3f point); // Return the closest data from point


private:
	bool data_point_registration_finished;
	bool clusterization_finished;
	std::list<Cluster *> clusters;             // Forward list of all clusters
	std::list<ClusterDataPoint *> data_points;  // All point data
	v2s16 x_limits;                             // Min/Max of x, y, z.   in x_limits, 'X' denotes min and 'Y' denotes max
	v2s16 y_limits;
	v2s16 z_limits;

};

#endif
