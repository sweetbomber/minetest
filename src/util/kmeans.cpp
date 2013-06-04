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

#include "main.h"
#include "numeric.h"
#include "kmeans.h"

// TODO: Replace all v3s16 for v3s32
KMeans::KMeans() {
	dataPointRegistrationFinished = false;
	clusterizationFinished = false;
	printf("Class initialized\n");
}


KMeans::~KMeans() {
	printf("Class destroyed\n");
}

void KMeans::addDataPoint(void *data, v3s16 point) {
	if(dataPointRegistrationFinished)
		return;

	ClusterDataPoint *dataPoint = new ClusterDataPoint;
	dataPoint->data = data;
	dataPoint->point = point;

	printf("Added new dataPoint\n");
	dataPoints.push_back(dataPoint);

	// Calculate min and max limits over X,Y and Z. in xLimits, 'X' denotes min and 'Y' denotes max
	if(dataPoints.size() == 1) {
		// For the first point, min and max are the same
		xLimits = v2s16(point.X, point.X);
		yLimits = v2s16(point.Y, point.Y);
		zLimits = v2s16(point.Z, point.Z);
	}
	else {
		// Regular min/max finding
		xLimits = v2s16((xLimits.X > point.X)?point.X:xLimits.X, (xLimits.Y < point.X)?point.X:xLimits.Y);
		yLimits = v2s16((yLimits.X > point.Y)?point.Y:yLimits.X, (yLimits.Y < point.Y)?point.Y:yLimits.Y);
		zLimits = v2s16((zLimits.X > point.Z)?point.Z:zLimits.X, (zLimits.Y < point.Z)?point.Z:zLimits.Y);
	}
}

void KMeans::clusterize(s16 numberOfClusters) {
	dataPointRegistrationFinished = true;

	if(dataPoints.size() == 0) {
		// No points added!
		return;
	}

	// Create clusters
	int i;
	for(i = 0; i < numberOfClusters; i++) {
		Cluster *cluster = new Cluster(xLimits, yLimits, zLimits); // Create cluster. The limits are used to randomize its initial centroid
//		printf("acc: %d %d %d\n", cluster->accumulator.X, cluster->accumulator.Y, cluster->accumulator.Z);
		clusters.push_back(cluster);
	}

	// Now the k-means method is applied. The stopping criteria is stability or KMEANS_MAX_IT
	bool clustersStable = false;
	for(i = 0; (i < KMEANS_MAX_IT) && (!clustersStable); i++) {

		// Iterate over each dataPoint, and classify it according to distance
		std::list<ClusterDataPoint *>::iterator i_dataPoint;
		std::list<Cluster *>::iterator i_cluster;

		for(i_dataPoint = dataPoints.begin(); i_dataPoint != dataPoints.end(); i_dataPoint++) {

			// Iterate over all clusters to search for the nearest
			Cluster *nearestCluster = *(clusters.begin());
			s32 nearestDistance = KMEANS_DISTANCE(nearestCluster->centroid, (*i_dataPoint)->point);
			for(i_cluster = clusters.begin(); i_cluster != clusters.end(); i_cluster++) {
				s32 distance = KMEANS_DISTANCE((*i_cluster)->centroid, (*i_dataPoint)->point);
				if(distance < nearestDistance) {
					nearestDistance = distance;
					nearestCluster = *i_cluster;
				}
			}
			
			// Now that we have the nearest cluster, add this point to its accumulator, and increment numberOfPoints
			nearestCluster->accumulator += (*i_dataPoint)->point;
			nearestCluster->numberOfPoints ++;
	//		printf("Point: (%d, %d, %d) Nearest cluster centroid: (%d, %d, %d) Acc: (%d, %d, %d)\n", (*i_dataPoint)->point.X, (*i_dataPoint)->point.Y, (*i_dataPoint)->point.Z, nearestCluster->centroid.X, nearestCluster->centroid.Y, nearestCluster->centroid.Z, nearestCluster->accumulator.X, nearestCluster->accumulator.Y, nearestCluster->accumulator.Z );
		}
		
		// Now that all points have been classified, the new centroids can be calculated for each cluster.
		// Their accumulators will be reset, and stability will be checked.
		clustersStable = true;
		for(i_cluster = clusters.begin(); i_cluster != clusters.end(); i_cluster++) {

			// Checks if the cluster is empty
			if((*i_cluster)->numberOfPoints > 0) {
				v3s16 newCenter = v3s16((*i_cluster)->accumulator.X/(*i_cluster)->numberOfPoints, (*i_cluster)->accumulator.Y/(*i_cluster)->numberOfPoints, (*i_cluster)->accumulator.Z/(*i_cluster)->numberOfPoints);
//				printf("no %d\n", (*i_cluster)->numberOfPoints);
				
				// Check for stability
				if(newCenter != (*i_cluster)->centroid)
					clustersStable = false;

				(*i_cluster)->centroid = newCenter;
				(*i_cluster)->resetAccumulator();
					
				printf("acc: %d %d %d new center %d %d %d\n", (*i_cluster)->accumulator.X, (*i_cluster)->accumulator.Y, (*i_cluster)->accumulator.Z, newCenter.X, newCenter.Y, newCenter.Z);
			}
			else {
				printf("Overlap!\n");
				// If the cluster has no points, its because it is overlapping with another one. Place it somewhere else.
				(*i_cluster)->randomize(xLimits, yLimits, zLimits);
				(*i_cluster)->resetAccumulator();
			}
			
		}
		
	}	
	if(clustersStable) {
		printf("Stabilized!!\n");
	}
	else {
		printf("KMEANS_MAX_IT reached!\n");
	}
	
	// Now that the clusters are placed, add each of its points to their respective lists
	std::list<ClusterDataPoint *>::iterator i_dataPoint;
	std::list<Cluster *>::iterator i_cluster;

	for(i_dataPoint = dataPoints.begin(); i_dataPoint != dataPoints.end(); i_dataPoint++) {

		// Iterate over all clusters to search for the nearest
		Cluster *nearestCluster = *(clusters.begin());
		s32 nearestDistance = KMEANS_DISTANCE(nearestCluster->centroid, (*i_dataPoint)->point);
		for(i_cluster = clusters.begin(); i_cluster != clusters.end(); i_cluster++) {
			s32 distance = KMEANS_DISTANCE((*i_cluster)->centroid, (*i_dataPoint)->point);
			if(distance < nearestDistance) {
				nearestDistance = distance;
				nearestCluster = *i_cluster;
			}
		}
		
		// Now that we have the nearest cluster, add this point to its list
		nearestCluster->dataPoints.push_back(*i_dataPoint);
	}
	
	clusterizationFinished = true;
	// Done. Time to sleep...or have breakfast??!	
}

void * KMeans::getNearestDataPoint(v3s16 point) {
	
	s32 nearestDistance;
	// Have we clusterized points?
	if(!clusterizationFinished)
		return NULL;

	// Are there any clusters?
	if(clusters.size() == 0)
		return NULL;

	// Search among known clusters for the nearest one
	std::list<Cluster *>::iterator i_cluster;
	Cluster *nearestCluster = *(clusters.begin());
	nearestDistance = KMEANS_DISTANCE(nearestCluster->centroid, point);
	
	for(i_cluster = clusters.begin(); i_cluster != clusters.end(); i_cluster++) {
		s32 distance = KMEANS_DISTANCE((*i_cluster)->centroid, point);
		if(distance < nearestDistance) {
			nearestDistance = distance;
			nearestCluster = *i_cluster;
		}
	}

	// Are there points in the list?
	if(nearestCluster->dataPoints.size() == 0)
		return NULL;

	// Now search within the points belonging to that cluster
	std::list<ClusterDataPoint *>::iterator i_dataPoint;
	ClusterDataPoint *nearestDataPoint = *(nearestCluster->dataPoints.begin());
	nearestDistance = KMEANS_DISTANCE(nearestDataPoint->point, point);

	for(i_dataPoint = nearestCluster->dataPoints.begin(); i_dataPoint != nearestCluster->dataPoints.end(); i_dataPoint++) {
		s32 distance = KMEANS_DISTANCE((*i_dataPoint)->point, point);
		if(distance < nearestDistance) {
			nearestDistance = distance;
			nearestDataPoint = *i_dataPoint;
		}
	}

	printf("Alright! Min Distance: %d\n", nearestDistance);	
	return nearestDataPoint->data;

}

void KMeans::listPoints() {
	
	std::list<ClusterDataPoint *>::iterator i_dataPoint;
	
	for(i_dataPoint=dataPoints.begin(); i_dataPoint != dataPoints.end(); i_dataPoint++) {
		printf("point: (%d, %d, %d)\n", (*i_dataPoint)->point.X, (*i_dataPoint)->point.Y, (*i_dataPoint)->point.Z);
	}
	printf("X:(%d, %d) Y:(%d, %d) Z:(%d, %d)\n", xLimits.X, xLimits.Y, yLimits.X, yLimits.Y, zLimits.X, zLimits.Y);
}

Cluster::Cluster(v2s16 xLimits, v2s16 yLimits, v2s16 zLimits) {

	resetAccumulator();

	// To initialize a cluster, two methods exist: place the centroid over a random point, or over a random (bounded) location. The later is used.
	randomize(xLimits, yLimits, zLimits);
	printf("Cluster added. Centroid: (%d, %d, %d) (%d, %d, %d)\n", centroid.X, centroid.Y, centroid.Z, accumulator.X, accumulator.Y, accumulator.Z);
}

void Cluster::resetAccumulator() {
	accumulator = v3s16(0, 0, 0);
	numberOfPoints = 0;
}

void Cluster::randomize(v2s16 xLimits, v2s16 yLimits, v2s16 zLimits) {
	centroid = v3s16(myrand_range(xLimits.X, xLimits.Y), myrand_range(yLimits.X, yLimits.Y), myrand_range(zLimits.X, zLimits.Y));
}
