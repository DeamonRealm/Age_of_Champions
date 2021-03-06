#include "j1FogOfWar.h"

#include "j1App.h"
#include "j1Map.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1EntitiesManager.h"

#include "j1Player.h"
#include "Hud_MinimapPanel.h"

//Constructors ========================
j1FogOfWar::j1FogOfWar()
{
	name = "fog_of_war";
}

//Destructors =========================
j1FogOfWar::~j1FogOfWar()
{

}


//Game Loop ===========================
void j1FogOfWar::Disable()
{
	active = false;
	enabled = false;

	entities_dinamic_update.clear();
	entities_static_update.clear();
	entities_release.clear();
	buildings_to_spawn.clear();
	cells_in_screen.clear();

	//Reset fog of war alpha layer
	for (uint y = 0; y < alpha_layer_height; y++)
	{
		for (uint x = 0; x < alpha_layer_width; x++)
		{
			AlphaCell* current_cell = &alpha_layer[y * alpha_layer_width + x];
			current_cell->alpha = ALPHA_LIMIT;
			current_cell->locked = false;
		}
	}

	//Reset fog of war logic layer
	uint size = App->map->data.width * App->map->data.height;
	for (uint k = 0; k < size; k++)
	{
		fog_layer[k].locked = false;
		fog_layer[k].type = DARK_FOG;
	}

}

void j1FogOfWar::Reset()
{
	entities_dinamic_update.clear();
	entities_static_update.clear();
	entities_release.clear();
	buildings_to_spawn.clear();
	cells_in_screen.clear();

	//Reset fog of war alpha layer
	for (uint y = 0; y < alpha_layer_height; y++)
	{
		for (uint x = 0; x < alpha_layer_width; x++)
		{
			AlphaCell* current_cell = &alpha_layer[y * alpha_layer_width + x];
			current_cell->alpha = ALPHA_LIMIT;
			current_cell->locked = false;
		}
	}

	//Reset fog of war logic layer
	uint size = App->map->data.width * App->map->data.height;
	for (uint k = 0; k < size; k++)
	{
		fog_layer[k].locked = false;
		fog_layer[k].type = DARK_FOG;
	}

}

void j1FogOfWar::Init()
{
	active = false;
	enabled = false;
}

bool j1FogOfWar::PostUpdate()
{
	if (update_timer.Read() > UPDATE_RATE)
	{
		std::vector<Unit*> units;
		uint size = App->entities_manager->units_quadtree.CollectCandidates(units, App->render->camera_viewport);
		for (uint k = 0; k < size; k++)
		{
			if (units[k]->GetDiplomacy() == ALLY)entities_static_update.push_back(units[k]);
		}
		update_timer.Start();
		CollectFogCells();
		ResetFogTilesInCamera();
	}

	for (uint k = 0; k < entities_release.size(); k++)
	{
		entities_release[k]->ResetFogAround();
	}
	entities_release.clear();

	j1Timer timer;
	while (timer.Read() < UPDATE_TIME  && !entities_static_update.empty())
	{
		entities_static_update.back()->CheckFogAround();
		entities_static_update.pop_back();
	}

	if (App->map_debug_mode)return true;

	uint size = cells_in_screen.size();
	for (uint k = 0; k < size; k++)
	{
		if (!cells_in_screen[k]->locked && cells_in_screen[k]->alpha < MID_ALPHA && cells_in_screen[k]->unlock_timer.Read() > UNLOCK_TIME)cells_in_screen[k]->alpha = MID_ALPHA;
		if(!cells_in_screen[k]->locked)App->render->FogBlit(cells_in_screen[k]->position, alpha_cell_size, cells_in_screen[k]->alpha);
	}

	return true;
}

bool j1FogOfWar::CleanUp()
{
	//Delete fog cells & tile information
	delete[] fog_layer;
	delete[] alpha_layer;
	fog_quadtree.Clear();

	//Clear fog entities lists
	entities_dinamic_update.clear();
	entities_static_update.clear();
	entities_release.clear();

	//Clear cells in screen vector
	cells_in_screen.clear();

	return true;
}

bool j1FogOfWar::Load(pugi::xml_node& data)
{

	if (fog_layer == nullptr || alpha_layer == nullptr)
	{
		LOG("Can't load fog of war!");
		return true;
	}

	//Node where alpha layer data is saved
	pugi::xml_node alpha_layer_node = data.child("alpha_layer");

	//First alpha cell node from alpha layer
	pugi::xml_node alpha_cell_node = alpha_layer_node.first_child();
	
	//Iterate all alpha cells saved
	uint k = 0;
	while (alpha_cell_node != NULL)
	{
		//Get saved cell alpha
		alpha_layer[k].alpha = alpha_cell_node.attribute("alpha").as_uint(255);
		
		//Focus next saved cell
		k++;
		alpha_cell_node = alpha_cell_node.next_sibling();
	}

	//Node where logic layer is saved
	pugi::xml_node logic_layer_node = data.child("logic_layer");

	//Node where tile characteristics are saved
	pugi::xml_node fog_tile_node = logic_layer_node.first_child();

	k = 0;
	while (fog_tile_node != NULL)
	{
		fog_layer[k].type = (FOG_TYPE)fog_tile_node.attribute("id").as_int(2);
		if (fog_layer[k].type != DARK_FOG) App->player->minimap_panel->PushTilestoClear(k);

		k++;
		fog_tile_node = fog_tile_node.next_sibling();
	}

	return true;
}

bool j1FogOfWar::Save(pugi::xml_node& data) const
{
	if (fog_layer == nullptr || alpha_layer == nullptr)
	{
		LOG("Can't save fog of war!");
		return true;
	}
	
	//Node where alpha layer data is saved
	pugi::xml_node alpha_layer_node = data.append_child("alpha_layer");

	//Iterate all the alpha layer to save alpha values
	uint size = alpha_layer_width * alpha_layer_height;
	for (uint k = 0; k < size; k++)
	{
		//First alpha cell node from alpha layer
		pugi::xml_node alpha_cell_node = alpha_layer_node.append_child("cell");

		//Save current alpha cell alpha value
		alpha_cell_node.append_attribute("alpha") = alpha_layer[k].alpha;
	}

	//Node where logic layer is saved
	pugi::xml_node logic_layer_node = data.append_child("logic_layer");

	//Iterate all the logic layer to save values
	size = App->map->data.width * App->map->data.height;

	for (uint k = 0; k < size; k++)
	{
		//Node where current tile characteristics are saved
		pugi::xml_node fog_tile_node = logic_layer_node.append_child("tile");

		//Save tile id
		fog_tile_node.append_attribute("id") = fog_layer[k].type;
	}

	return true;
}

//Functionality =======================
void j1FogOfWar::GenerateFogOfWar()
{
	//Calculate alpha cells size
	alpha_cell_size = ceil(1000 / DIVISIONS_PER_PIXELS);

	//Check if alpha cell size is a multiple of 1000 to don't have pixels without fog
	while (1000 % (alpha_cell_size - MARGIN) != 0)alpha_cell_size++;

	//Calculate fog alpha layer size with the map data & alpha cells size
	alpha_layer_width = ceil((App->map->data.width * (App->map->data.tile_width + MARGIN)) / alpha_cell_size) + MARGIN;

	alpha_layer_height = ceil((App->map->data.height * (App->map->data.tile_height + MARGIN)) / (alpha_cell_size - MARGIN)) + MARGIN;

	int mid_map_lenght = ceil(App->map->data.width * (App->map->data.tile_width)) * -0.5;

	//Build fog quadtree boundaries & limit
	fog_quadtree.SetBoundaries({ (int)mid_map_lenght, 0, (int)alpha_cell_size * (int)alpha_layer_width, (int)alpha_cell_size * (int)alpha_layer_height });
	fog_quadtree.SetMaxObjects(800);
	fog_quadtree.SetDebugColor({ 255,255,0,255 });

	j1Timer time;

	//Build fog alpha layer
	//Allocate alpha layer cells
	alpha_layer = new AlphaCell[alpha_layer_width * alpha_layer_height];
	float mid_alpha_cell_size = alpha_cell_size * 0.5f;
	for (uint y = 0; y < alpha_layer_height; y++)
	{
		for (uint x = 0; x < alpha_layer_width; x++)
		{
			AlphaCell* current_cell = &alpha_layer[y * alpha_layer_width + x];
			current_cell->position = { (int)mid_map_lenght + (int)alpha_cell_size * (int)x, (int)(int)alpha_cell_size * (int)y };
			fog_quadtree.Insert(current_cell, &iPoint(current_cell->position.x + mid_alpha_cell_size, current_cell->position.y + mid_alpha_cell_size));
		}
	}

	fog_layer = new FogTile[App->map->data.width * App->map->data.height];



	LOG("%f", time.ReadSec());
}

void j1FogOfWar::CollectFogCells()
{
	cells_in_screen.clear();
	uint size = fog_quadtree.CollectCandidates(cells_in_screen, App->render->camera_viewport);
	for (uint k = 0; k < size; k++)
	{
		if (!cells_in_screen[k]->locked && cells_in_screen[k]->alpha < MID_ALPHA && cells_in_screen[k]->unlock_timer.Read() > UNLOCK_TIME)cells_in_screen[k]->alpha = MID_ALPHA;
	}
	/*for (uint k = 0; k < size; k++)
	{
		SoftAlphaCell(cells_in_screen[k]);
	}*/
}

void j1FogOfWar::ResetFogTilesInCamera()
{
	std::vector<iPoint> collected_points;
	uint size = App->map->map_quadtree.CollectCandidates(collected_points, App->render->camera_viewport);

	for (uint k = 0; k < size; k++)
	{
		FogTile* target = &fog_layer[collected_points[k].x + collected_points[k].y * App->map->data.width];
		if (!target->locked)
		{
			if (target->type == NO_FOG) target->type = GRAY_FOG;
		}
	}
}

FOG_TYPE j1FogOfWar::GetFogID(int x, int y) const
{
	return fog_layer[y * App->map->data.width + x].type;
}

std::vector<AlphaCell*> j1FogOfWar::ClearAlphaLayer(const Circle zone, unsigned short alpha, bool lock)
{
	std::vector<AlphaCell*> fog_cells;
	std::vector<AlphaCell*> definitive;

	uint size = fog_quadtree.CollectCandidates(fog_cells, zone);
	for (uint k = 0; k < size; k++)
	{
		if (!fog_cells[k]->locked)
		{
			fog_cells[k]->alpha = alpha;
			fog_cells[k]->locked = lock;
			fog_cells[k]->unlock_timer.Start();
			definitive.push_back(fog_cells[k]);
		}
	}

	return definitive;
}

std::vector<FogTile*> j1FogOfWar::ClearFogLayer(const Circle zone, FOG_TYPE type, bool lock)
{
	std::vector<iPoint> tiles_in;
	uint size = App->map->map_quadtree.CollectCandidates(tiles_in, zone);

	std::vector<FogTile*> resulting_tiles;

	for (uint k = 0; k < size; k++)
	{
		uint index = tiles_in[k].y * App->map->data.width + tiles_in[k].x;

		if (!fog_layer[index].locked)
		{
			if (fog_layer[index].type == DARK_FOG) 
				App->player->minimap_panel->PushTilestoClear(index);

			fog_layer[index].locked = lock;
			fog_layer[index].type = type;
			
			resulting_tiles.push_back(&fog_layer[index]);
		}
	}

	return resulting_tiles;
}

void j1FogOfWar::CheckEntityFog(Entity * target)
{
	entities_static_update.push_back(target);
}

void j1FogOfWar::ReleaseEntityFog(Entity * target)
{
	entities_release.push_back(target);
}

void j1FogOfWar::SoftAlphaCell(AlphaCell * target)
{
	iPoint coordinates(target->position.x / alpha_cell_size, target->position.y / alpha_cell_size);

	if (coordinates.x == 0 || coordinates.x == alpha_layer_width || coordinates.y == 0 || coordinates.y ==  alpha_layer_height)
	{
		LOG("CAN'T SOFT ALPHA");
	}
	else
	{
		uint index = coordinates.x * coordinates.y;
		target->alpha = alpha_layer[index + 1].alpha + alpha_layer[index - 1].alpha + alpha_layer[index + alpha_layer_width].alpha + alpha_layer[index + alpha_layer_width].alpha;
	}
}

