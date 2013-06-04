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

KMeans::KMeans() {
	data_point_registration_finished = false;
	clusterization_finished = false;
	printf("Class initialized\n");
}


KMeans::~KMeans() {
	printf("Class destroyed\n");
}

void KMeans::addDataPoint(void *data, v3f point) {
	if(data_point_registration_finished)
		return;

	ClusterDataPoint *dataPoint = new ClusterDataPoint;
	dataPoint->data = data;
	dataPoint->point = point;

	printf("Added new dataPoint\n");
	data_points.push_back(dataPoint);

	// Calculate min and max limits over X,Y and Z. in x_limits, 'X' denotes min and 'Y' denotes max
	if(data_points.size() == 1) {
		// For the first point, min and max are the same
		x_limits = v2s16(point.X, point.X);
		y_limits = v2s16(point.Y, point.Y);
		z_limits = v2s16(point.Z, point.Z);
	}
	else {
		// Regular min/max finding
		x_limits = v2s16((x_limits.X > point.X)?point.X:x_limits.X, (x_limits.Y < point.X)?point.X:x_limits.Y);
		y_limits = v2s16((y_limits.X > point.Y)?point.Y:y_limits.X, (y_limits.Y < point.Y)?point.Y:y_limits.Y);
		z_limits = v2s16((z_limits.X > point.Z)?point.Z:z_limits.X, (z_limits.Y < point.Z)?point.Z:z_limits.Y);
	}
}

void KMeans::clusterize(s16 number_of_clusters) {
	data_point_registration_finished = true;

	if(data_points.size() == 0) {
		// No points added!
		return;
	}

	// Create clusters
	int i;
	for(i = 0; i < number_of_clusters; i++) {
		Cluster *cluster = new Cluster(x_limits, y_limits, z_limits); // Create cluster. The limits are used to randomize its initial centroid
//		printf("acc: %f %f %f\n", cluster->accumulator.X, cluster->accumulator.Y, cluster->accumulator.Z);
		clusters.push_back(cluster);
	}

	// Now the k-means method is applied. The stopping criteria is stability or KMEANS_MAX_IT
	bool clustersStable = false;
	for(i = 0; (i < KMEANS_MAX_IT) && (!clustersStable); i++) {

		// Iterate over each dataPoint, and classify it according to distance
		std::list<ClusterDataPoint *>::iterator i_dataPoint;
		std::list<Cluster *>::iterator i_cluster;

		for(i_dataPoint = data_points.begin(); i_dataPoint != data_points.end(); i_dataPoint++) {

			// Iterate over all clusters to search for the nearest
			Cluster *nearest_cluster = *(clusters.begin());
			float shortest_distance = KMEANS_DISTANCE(nearest_cluster->centroid, (*i_dataPoint)->point);
			for(i_cluster = clusters.begin(); i_cluster != clusters.end(); i_cluster++) {
				float distance = KMEANS_DISTANCE((*i_cluster)->centroid, (*i_dataPoint)->point);
				if(distance < shortest_distance) {
					shortest_distance = distance;
					nearest_cluster = *i_cluster;
				}
			}
			
			// Now that we have the nearest cluster, add this point to its accumulator, and increment number_of_points
			nearest_cluster->accumulator += (*i_dataPoint)->point;
			nearest_cluster->number_of_points ++;
	//		printf("Point: (%f, %f, %f) Nearest cluster centroid: (%f, %f, %f) Acc: (%f, %f, %f)\n", (*i_dataPoint)->point.X, (*i_dataPoint)->point.Y, (*i_dataPoint)->point.Z, nearest_cluster->centroid.X, nearest_cluster->centroid.Y, nearest_cluster->centroid.Z, nearest_cluster->accumulator.X, nearest_cluster->accumulator.Y, nearest_cluster->accumulator.Z );
		}
		
		// Now that all points have been classified, the new centroids can be calculated for each cluster.
		// Their accumulators will be reset, and stability will be checked.
		clustersStable = true;
		for(i_cluster = clusters.begin(); i_cluster != clusters.end(); i_cluster++) {

			// Checks if the cluster is empty
			if((*i_cluster)->number_of_points > 0) {
				v3f newCenter = v3f((*i_cluster)->accumulator.X/(*i_cluster)->number_of_points, (*i_cluster)->accumulator.Y/(*i_cluster)->number_of_points, (*i_cluster)->accumulator.Z/(*i_cluster)->number_of_points);
//				printf("no %d\n", (*i_cluster)->number_of_points);
				
				// Check for stability
				if(newCenter != (*i_cluster)->centroid)
					clustersStable = false;

				(*i_cluster)->centroid = newCenter;
				(*i_cluster)->resetAccumulator();
					
				printf("acc: %f %f %f new center %f %f %f\n", (*i_cluster)->accumulator.X, (*i_cluster)->accumulator.Y, (*i_cluster)->accumulator.Z, newCenter.X, newCenter.Y, newCenter.Z);
			}
			else {
				printf("Overlap!\n");
				// If the cluster has no points, its because it is overlapping with another one. Place it somewhere else.
				(*i_cluster)->randomize(x_limits, y_limits, z_limits);
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

	for(i_dataPoint = data_points.begin(); i_dataPoint != data_points.end(); i_dataPoint++) {

		// Iterate over all clusters to search for the nearest
		Cluster *nearest_cluster = *(clusters.begin());
		float shortest_distance = KMEANS_DISTANCE(nearest_cluster->centroid, (*i_dataPoint)->point);
		for(i_cluster = clusters.begin(); i_cluster != clusters.end(); i_cluster++) {
			float distance = KMEANS_DISTANCE((*i_cluster)->centroid, (*i_dataPoint)->point);
			if(distance < shortest_distance) {
				shortest_distance = distance;
				nearest_cluster = *i_cluster;
			}
		}
		
		// Now that we have the nearest cluster, add this point to its list
		nearest_cluster->data_points.push_back(*i_dataPoint);
	}
	
	clusterization_finished = true;
	// Done. Time to sleep...or have breakfast??!	
}

void * KMeans::getNearestDataPoint(v3f point) {
	
	float shortest_distance;
	float distance;
	// Have we clusterized points?
/*	if(!clusterization_finished)
		return NULL;

	// Are there any clusters?
	if(clusters.size() == 0)
		return NULL;
*/
	// Search among known clusters for the nearest one
	std::list<Cluster *>::iterator i_cluster;
	Cluster *nearest_cluster = *(clusters.begin());
	shortest_distance = KMEANS_DISTANCE(nearest_cluster->centroid, point);
	
	for(i_cluster = clusters.begin(); i_cluster != clusters.end(); i_cluster++) {
		distance = KMEANS_DISTANCE((*i_cluster)->centroid, point);
		if(distance < shortest_distance) {
			shortest_distance = distance;
			nearest_cluster = *i_cluster;
		}
	}

	// Are there points in the list?
//	if(nearest_cluster->data_points.size() == 0)
//		return NULL;

	// Now search within the points belonging to that cluster
	std::list<ClusterDataPoint *>::iterator i_dataPoint;
	ClusterDataPoint *nearest_data_point = *(nearest_cluster->data_points.begin());
	shortest_distance = KMEANS_DISTANCE(nearest_data_point->point, point);

	for(i_dataPoint = nearest_cluster->data_points.begin(); i_dataPoint != nearest_cluster->data_points.end(); i_dataPoint++) {
		distance = KMEANS_DISTANCE((*i_dataPoint)->point, point);
		if(distance < shortest_distance) {
			shortest_distance = distance;
			nearest_data_point = *i_dataPoint;
		}
	}

	//printf("Alright! Min Distance: %f\n", shortest_distance);	
	return nearest_data_point->data;

}

Cluster::Cluster(v2s16 x_limits, v2s16 y_limits, v2s16 z_limits) {

	resetAccumulator();

	// To initialize a cluster, two methods exist: place the centroid over a random point, or over a random (bounded) location. The later is used.
	randomize(x_limits, y_limits, z_limits);
	printf("Cluster added. Centroid: (%f, %f, %f) (%f, %f, %f)\n", centroid.X, centroid.Y, centroid.Z, accumulator.X, accumulator.Y, accumulator.Z);
}

void Cluster::resetAccumulator() {
	accumulator = v3f(0, 0, 0);
	number_of_points = 0;
}

void Cluster::randomize(v2s16 x_limits, v2s16 y_limits, v2s16 z_limits) {
	centroid = v3f(myrand_range(x_limits.X, x_limits.Y), myrand_range(y_limits.X, y_limits.Y), myrand_range(z_limits.X, z_limits.Y));
}
