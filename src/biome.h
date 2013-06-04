/*
Minetest
Copyright (C) 2010-2013 kwolekr, Ryan Kwolek <kwolekr@minetest.net>

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

#ifndef BIOME_HEADER
#define BIOME_HEADER

#include <string>
#include "nodedef.h"
#include "gamedef.h"
#include "mapnode.h"
#include "noise.h"
#include "mapgen.h"
#include "util/kmeans.h"

enum BiomeTerrainType
{
	BIOME_TERRAIN_NORMAL,
	BIOME_TERRAIN_LIQUID,
	BIOME_TERRAIN_NETHER,
	BIOME_TERRAIN_AETHER,
	BIOME_TERRAIN_FLAT
};

extern NoiseParams nparams_biome_def_heat;
extern NoiseParams nparams_biome_def_humidity;

class Biome {
public:
	u8 id;
	std::string name;
	u32 flags;
	
	std::string top_nodename;
	std::string filler_nodename;

	content_t c_top;
	s16 top_depth;

	content_t c_filler;
	s16 filler_height;
	
	float height_point;
	float heat_point;
	float humidity_point;
};

struct BiomeNoiseInput {
	v2s16 mapsize;
	float *heat_map;
	float *humidity_map;
	s16 *height_map;
};

class BiomeDefManager {
public:
	std::vector<Biome *> biomes;
	KMeans *kMeans;
	bool biome_registration_finished;
	NoiseParams *np_heat;
	NoiseParams *np_humidity;

	BiomeDefManager();
	~BiomeDefManager();
	
	Biome *createBiome(BiomeTerrainType btt);
	void  calcBiomes(BiomeNoiseInput *input, u8 *biomeid_map);
	Biome *getBiome(float heat, float humidity, s16 y);

	void addBiome(Biome *b);
	void resolveNodeNames(INodeDefManager *ndef);
};

#endif
