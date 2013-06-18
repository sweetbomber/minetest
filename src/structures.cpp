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

#include "biome.h"
#include "nodedef.h"
#include "log.h"
#include "main.h"
#include "util/timetaker.h"
#include "structures.h"
#include "mapblock.h"
#include "mapnode.h"
#include "map.h"

// TODO: Check for finished_registering_process on all registration functions!!!!

/*
 * Palette management
 */
StructurePalette::StructurePalette() {
	number_of_elements = 0;
	palette_compiled = true;
}

StructurePalette::~StructurePalette() {
}

/* Searches for a texture. If it does not exist, add it to the list
  and return its ID. Otherwise, just return the id */
u16 StructurePalette::addAndReturnId(std::string name) {
	std::map<std::string, u16> iterator it;
	it = id_list.find(name);
	if(it == id_list.end()) {
		id_list[name] = number_of_elements;
		number_of_elements++;
	}

	printf("Successfully added item to palette: %s (%d)\n", name.c_str(), id_list.find(name)->second);
	return id_list.find(name)->second;
}

void StructurePalette::compilePalette() {
}


void StructurePalette::translateData(MapNode *output, MapNode *input, u32 n) {
}


StructureSection::StructureSection() {
	volume = v3s16(0, 0, 0);
	finished_registering_process = false;
}

bool StructureSection::loadFromFile(std:ifstream file)
{
//	registerNode(v3s16(0,0,0), "default:stonebrick", 0, 0);
	return true;
}
void StructureSection::registerNode(v3s16 pos, std::string name,
									u8 param1 = 0, u8 param2 = 0) {
	if(finished_registering_process)
		return;

	if((pos.X >= 0 && pos.X < volume.X) &&
	   (pos.Y >= 0 && pos.X < volume.Y) &&
	   (pos.Z >= 0 && pos.Z < volume.Z)) {

		MapNode tmp_node = MapNode(this->structure->palette[0].addAndReturnId(name), param1, param2);
		node_data[pos.X + volume.X*pos.Y + pos.Z*volume.X*volume.Y] = tmp_node;
		delete tmp_node;
	}
	else {
		errorstream << "Tried to register section node outside volume. Ignoring..."
					<< std::endl;
	}
}

void StructureSection::setVolume(v3s16 vol) {
	if(volume.X * volume.Y * volume.Z > 0) {
		errorstream << "Tried to re-set a volume to a structure section. Ignoring..."
					<< std::endl;
		return;
	}

	if((vol.X < 0) || (vol.Y < 0) || (vol.Z < 0)) {
		errorstream << "Tried to set a negative volume. Ignoring..." << std::endl;
		return;
	}

	node_data = new MapNode(CONTENT_IGNORE, 0, 0) [vol.X * vol.Y * vol.Z];
	volume = vol;
}

v3s16 StructureSection::getVolume() {
	return volume;
}

void StructureSection::finishRegisteringProcess() {
	finished_registering_process = true;
}



Structure::Structure(std::string name) {
}

void Structure::addSection(std::string name, StructureSection *section) {
	std::map<std::string, StructureSection *> iterator section_it;
	section_it = sections_byname.find(name);

	/* If this fails, addSection was called directly.
	 * Registration of new sections must go trough StructDefMangr
	 */
	assert(section_it == sections_byname.end());

	sections_byname[name] = section;
	sections.push_back(section);
}

void Structure::addPalette(std::string name, StructurePalette *palette) {
	std::map<std::string, StructurePalette *> iterator palette_it;
	palette_it = palettes_byname.find(name);

	/* If this fails, addPalette was called directly.
	 * Registration of new palettes must go trough StructDefMangr
	 */
	assert(section_it == sections_byname.end());

	palettes_byname[name] = palette;
	palettes.push_back(palette);
}
/*
 * Manages structure definitions, such as adding new
 * or retrieving its data.
 */

StructureDefManager::StructureDefManager() {
	this->finished_registering_process = false;
}

StructureDefManager::~StructureDefManager() {
}

Structure *StructureDefManager::registerStructure(std::string structure_name) {
	std::map<std::string, Structure *> iterator structure_it;

	structure_it = structures_byname.find(structure_name);
	if(structure_it != structures_byname.end())
	{
		errorstream << "Structure already registered: " << structure_name <<
					<< std::endl;
		return NULL;
	}

	Structure structure = new Structure(structure_name);
	structures_byname[structure_name] = structure;
	structures.push_back(structure);
	printf("Structure registered! %s\n", structure_name.c_str());
	return structure;
}

/*
 * Assigns a section to a structure. If it does not exist, creates it.
 */
StructureSection *StructureDefManager::registerSection(std::string structure_name,
										  std::string section_name) {

	std::map<std::string, Structure *> iterator structure_it;
	std::map<std::string, StructureSection *> iterator section_it;

	structure_it = structures_byname.find(structure_name);
	if(structure_it == structures_byname.end())
	{
		errorstream << "Tried to add a section to a non-existing structure!"
					<< std::endl;
		return NULL;
	}

	section_it = sections_byname.find(section_name);
	if(section_it != sections_byname.end())
	{
		errorstream << "Section already exists! " << section_name <<
					<< std::endl;
		return NULL;
	}

	StructureSection section = new StructureSection(section_name);
	sections_byname[section_name] = section;
	sections.push_back(section);
	section_it = sections_byname.find(section_name);

	structure_it->addSection(section_name, *section_it);
	return section;
}

/*
 * Assigns a palette to a structure. If it does not exist, creates it.
 */
void StructureDefManager::registerPalette(std::string structure_name,
										  std::string palette_name) {
	std::map<std::string, Structure *> iterator structure_it;
	std::map<std::string, StructurePalette *> iterator palette_it;

	structure_it = structures_byname.find(structure_name);
	if(structure_it == structures_byname.end())
	{
		errorstream << "Tried to add a palette to a non-existing structure!"
					<< std::endl;
		return;
	}

	palette_it = palettes_byname.find(palette_name);
	if(palette_it == palettes_byname.end())
	{
		StructurePalette palette = new StructurePalette(palette_name);
		palettes_byname[palette_name] = palette;
		palettes.push_back(palette);
		palette_it = palettes_byname.find(palette_name);
	}

	structure_it->addPalette(palette_name, *palette_it);
}

void StructureDefManager::finishRegisteringProcess(INodeDefManager *ndef) {
	printf("Finish registering structures\n");
	if(structures.size()) {
		printf("Structure list:\n");
		std::list<Structure *>::iterator i;
		for(i = structures.begin(); i != structures.end(); i++) {
			printf("   %s\n", (*i)->name.c_str());
		}
	}
	else {
		printf("No structures added\n");
	}
}

/* Obtains the structure that best matches the given conditions */
void StructureDefManager::getMatchingStructure() {
}




