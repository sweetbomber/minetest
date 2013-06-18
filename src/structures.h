/*
Minetest
Copyright (C) 2013 sweetbomber, <ffrogger0@yahoo.com>

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

#ifndef STRUCTGEN_HEADER
#define STRUCTGEN_HEADER

#include <string>
#include "nodedef.h"
#include "gamedef.h"
#include "mapnode.h"
#include "noise.h"
#include "mapgen.h"


/*
 * Structure palette stores node names, node IDs
 * and structure nodes IDs.
 * This is used so that the same structure could
 * be built with different materials
 * (like the same houses built with either stone
 * or sandstone)
 */
struct PaletteElement {
	u8 type;
	std::string name;
	MapNode mapNode;
};

class StructurePalette {
public:
	StructurePalette();
	~StructurePalette();

	/*
	 * 'elements' must contain the types and names. 'mapNode' field should be null
	 */
	void addElement(struct PaletteElement *element);
	u16 addAndReturnId(std::string name);
	void compilePalette();

	/* Translates input node types to MapNode. n is quantity */
	void translateData(MapNode *output, MapNode *input, u32 n);
private:
	bool palette_compiled;
	std::map<std::string, u16> id_list;
	u16 number_of_elements;
};

class Structure;
/*
 * A structure is composed of several sections.
 * This class stores all section-related data.
 */
class StructureSection {
public:
	void addPalette(std::string name);
	bool loadFromFile(std::ifstream file);
	void registerNode(v3s16 pos, std::string name, u8 param1 = 0, u8 param2 = 0);
	void setVolume(v3s16 vol);
	v3s16 getVolume();
	void finishRegistration();

	std::string name;
	Structure *structure;

	MapNode *node_data;
private:
	v3s16 volume;
	bool finished_registering_process;
};

/*
 * Structure definition class.
 * Used to store all structure-related data and manage its sections
 */
class Structure {
public:
	std::string name;
	void addSection(std::string name, StructureSection *section);
	void addPalette(std::string palette_name, StructurePalette *palette);
private:
	std::list<StructureSection *> sections;
	std::list<StructurePalette *> palettes;
	std::map<std::string, StructureSection *> sections_byname;
	std::map<std::string, StructurePalette *> palettes_byname;
};

class StructurePlacer {
};

/*
 * Manages structure definitions, such as adding new
 * or retrieving its data.
 */

class StructureDefManager {
public:
	StructureDefManager();
	~StructureDefManager();

	Structure *registerStructure(Structure *structure);
	StructureSection *registerSection(StructureSection *section);
	void registerPalette(std::string structure_name, std::string palette_name);
	void finishRegisteringProcess(INodeDefManager *ndef);

	/* Obtains the structure that best matches the given conditions */
	void getMatchingStructure();

private:
	std::list<Structure *> structures;
	std::list<StructureSection *> sections;
	std::map<std::string, Structure *> structures_byname;
	std::map<std::string, StructureSection *> sections_byname;
	std::map<std::string, StructurePalette *> palettes_byname;

	bool finished_registering_process;
};

#endif
