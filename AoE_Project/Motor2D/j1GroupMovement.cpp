#include "j1GroupMovement.h"
#include "j1App.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "j1ActionManager.h"
j1GroupMovement::j1GroupMovement()
{
	name = "group_movement";
}

j1GroupMovement::~j1GroupMovement()
{
}

void j1GroupMovement::Init()
{
	active = false;
	enabled = false;
}

iPoint j1GroupMovement::GetMiddlePoint()
{
	iPoint ret = { 0,0 };
	for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
	{
		Entity* single_unit = item._Ptr->_Myval;
		ret += single_unit->GetPositionRounded();
	}
	ret /= units->size();
	//Get all positions and add them on one ipoint and then divide them with the size of the units 
	// if that position is walkable return that position if is not look destination and get values 
	iPoint checker = App->map->WorldToMap(ret.x, ret.y);

	while (!App->pathfinding->IsWalkable(checker)) {
		if (checker.x < destination.x)	checker.x++;
		else checker.x--;
		if (checker.y < destination.y)	checker.y++;
		else checker.y--;
	}
	ret = App->map->MapToWorldCenter(checker.x, checker.y);
	return ret;
}

void j1GroupMovement::AttackOnGroup(std::list<Entity*>* units, Entity* target, bool building)
{
	if (units->empty()) 
		return;
	for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
	{
		
		Unit* current_unit = (Unit*)item._Ptr->_Myval;
		if (current_unit->GetUnitType() == MONK || current_unit->GetUnitType() == WIZARD_CHMP) 
			continue;	
		current_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
		current_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);
		if(building)
			current_unit->AddAction((Action*)App->action_manager->AttackToBuildingAction(current_unit, (Building*)target, PRIMARY));

		else
			current_unit->AddAction((Action*)App->action_manager->AttackToUnitAction(current_unit, (Unit*)target));
	}
}

void j1GroupMovement::HealOnGroup(std::list<Entity*>* units, Entity * target)
{
	if (units->empty())
		return;
	for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
	{

		Unit* current_unit = (Unit*)item._Ptr->_Myval;
		if (target->GetDiplomacy() != current_unit->GetDiplomacy())
			return;
		if (current_unit->GetUnitType() == MONK || current_unit->GetUnitType() == WIZARD_CHMP) 
		{
			current_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
			current_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);
			current_unit->AddAction((Action*)App->action_manager->HealAction(current_unit, (Unit*)target));
		}
	}
}

void j1GroupMovement::RecolectOnGroup(std::list<Entity*>* units, Resource* resource)
{

	if (units->empty())
		return;
	std::list<Entity*> no_villager_units;
	for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
	{
		Unit* current_unit = (Unit*)item._Ptr->_Myval;
		if (current_unit->GetUnitType() != VILLAGER)
		{
			no_villager_units.push_back(current_unit);
			continue;
		}
		current_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
		current_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);

		current_unit->AddAction((Action*)App->action_manager->RecollectAction((Villager*)item._Ptr->_Myval, resource));
	}
	if (!no_villager_units.empty())
	{
		iPoint resource_destination = resource->GetPositionRounded();
		GetGroupOfUnits(&no_villager_units, resource_destination.x, resource_destination.y, true);
	}
	no_villager_units.clear();
}

void j1GroupMovement::GetGroupOfUnits(std::list<Entity*>* get, int x, int y, bool active)
{
	map_destination = App->map->WorldToMap(x, y);
	destination = { x, y };

	
	
	//Get the list of units
	units = get;
	//get the size of units and set formation
	group_size = units->size();
	formation.SetFormation(group_size);
	//Get the middle point and set the lead of formation
	//	middle_point = GetMiddlePoint();
	lead = *(units->begin());
	if (!App->pathfinding->IsWalkable(map_destination))
	{
		destination = ((Unit*)lead)->FindSpawnCell(destination);
		if (destination == iPoint(-1, -1))
			return;
	}
	//Create the lead path
	std::vector<iPoint>* second_destination = nullptr;
	//	first_destination = CreateFirstDestination();
	second_destination = LeaderPath();
	if (second_destination == nullptr) return;
	//second_destination->insert(second_destination->end(), first_destination->begin(), first_destination->end());
	//push action
	lead->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
	lead->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);

	if (active)App->pathfinding->PushPath((Unit*)lead, destination,  false, PRIMARY);
	//	lead->AddAction((Action*)App->action_manager->MoveAction(second_destination, (Unit*)lead), TASK_CHANNELS::PRIMARY);
	else App->pathfinding->PushPath((Unit*)lead, destination,  false, SECONDARY);
	//	lead->AddAction((Action*)App->action_manager->MoveAction(second_destination, (Unit*)lead), TASK_CHANNELS::SECONDARY);
	lead_direcction = ((Unit*)lead)->LookDirection(*(second_destination->begin()), *(second_destination->begin() + 1));
	delete second_destination;
	second_destination = nullptr;
	if (units->size() > 1)
	{
		//do the other units path
		OtherUnitsPath(active);
	}
}

void j1GroupMovement::OtherUnitsPath(bool active)
{
	//iPoint new_middle_point;
	iPoint new_destination = { 0,0 };
	int current_width = 0;
	int current_height = 0;
	int i = 0;
	bool left = true;
	int loop = 0;
	int radius = ((Unit*)lead)->GetSoftCollider().GetRad();
	std::vector<iPoint> locations_takens;
	iPoint copy_destination = destination;
	switch (lead_direcction)
	{
	case NORTH:
		//iterate list of units

		for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
		{
			loop = 0;
			//get the unit if is the lead continue
			Entity* single_unit = item._Ptr->_Myval;
			current_width++;
			if (single_unit == lead) {
				continue;
			}
			if (left == true) {
				i++;
			}

			//set path for the other units
			new_destination.create(copy_destination.x + radius * i, copy_destination.y);
			iPoint check = App->map->WorldToMap(new_destination.x, new_destination.y);
			while (!App->pathfinding->IsWalkable(check) && !PlaceTaken(locations_takens, new_destination)) {
				new_destination.create(destination.x - radius * loop, destination.y);		
				loop++;
				check = App->map->WorldToMap(new_destination.x, new_destination.y);
			}
			locations_takens.push_back(new_destination);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);
			
				if (active)	App->pathfinding->PushPath((Unit*)single_unit, new_destination,  false, PRIMARY);
				//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::PRIMARY);
				else App->pathfinding->PushPath((Unit*)single_unit, new_destination, false, SECONDARY);
				//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::SECONDARY);
			
			left = !left;
			i *= -1;
			if (current_width == formation.width) {
				current_height++;
				copy_destination.create(destination.x, destination.y + radius * current_height);
				i = 0;
				current_width = 0;

			}
		}
		break;
	case NORTH_EAST:
		//iterate list of units
		for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
		{
			loop = 0;
			//get the unit if is the lead continue
			Entity* single_unit = item._Ptr->_Myval;
			current_width++;
			if (single_unit == lead) {
				continue;
			}
			if (left == true) {
				i++;
			}

			//set path for the other units
			new_destination.create(copy_destination.x + radius * i, copy_destination.y + radius * i);
			iPoint check = App->map->WorldToMap(new_destination.x, new_destination.y);
			while (!App->pathfinding->IsWalkable(check) && !PlaceTaken(locations_takens, new_destination)) {
				new_destination.create(destination.x - radius * loop, destination.y + radius * loop);
				loop++;
				check = App->map->WorldToMap(new_destination.x, new_destination.y);
			}
			locations_takens.push_back(new_destination);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);


			if (active)	App->pathfinding->PushPath((Unit*)single_unit, new_destination,  false, PRIMARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::PRIMARY);
			else App->pathfinding->PushPath((Unit*)single_unit, new_destination,  false, SECONDARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::SECONDARY);

			left = !left;
			i *= -1;
			if (current_width == formation.width) {
				current_height++;
				copy_destination.create(destination.x - radius * current_height, destination.y + radius * current_height);
				i = 0;
				current_width = 0;
			}
		}
		break;
	case EAST:
		//iterate list of units
		for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
		{
			loop = 0;
			//get the unit if is the lead continue
			Entity* single_unit = item._Ptr->_Myval;
			current_width++;
			if (single_unit == lead) {
				continue;
			}
			if (left == true) {
				i++;
			}

			//set path for the other units
			new_destination.create(copy_destination.x , copy_destination.y + radius * i);
			iPoint check = App->map->WorldToMap(new_destination.x, new_destination.y);
			while (!App->pathfinding->IsWalkable(check) && !PlaceTaken(locations_takens, new_destination)) {
				new_destination.create(destination.x - radius * loop, destination.y );
				loop++;
				check = App->map->WorldToMap(new_destination.x, new_destination.y);
			}
			locations_takens.push_back(new_destination);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);

			if (active)	App->pathfinding->PushPath((Unit*)single_unit, new_destination,  false, PRIMARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::PRIMARY);
			else App->pathfinding->PushPath((Unit*)single_unit, new_destination,  false, SECONDARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::SECONDARY);

			left = !left;
			i *= -1;
			if (current_width == formation.width) {
				current_height++;
				copy_destination.create(destination.x - radius * current_height, destination.y);
				i = 0;
				current_width = 0;

			}
		}
		break;
	case SOUTH_EAST:
		//iterate list of units
		for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
		{
			loop = 0;
			//get the unit if is the lead continue
			Entity* single_unit = item._Ptr->_Myval;
			current_width++;
			if (single_unit == lead) {
				continue;
			}
			if (left == true) {
				i++;
			}

			//set path for the other units
			new_destination.create(copy_destination.x - radius * i, copy_destination.y + radius * i);
			iPoint check = App->map->WorldToMap(new_destination.x, new_destination.y);
			while (!App->pathfinding->IsWalkable(check) && !PlaceTaken(locations_takens, new_destination)) {
				new_destination.create(destination.x - radius * loop, destination.y - radius * loop);
				loop++;
				check = App->map->WorldToMap(new_destination.x, new_destination.y);
			}
			locations_takens.push_back(new_destination);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);

			if (active)	App->pathfinding->PushPath((Unit*)single_unit, new_destination, false, PRIMARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::PRIMARY);
			else App->pathfinding->PushPath((Unit*)single_unit, new_destination,  false, SECONDARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::SECONDARY);

			left = !left;
			i *= -1;
			if (current_width == formation.width) {
				current_height++;
				copy_destination.create(destination.x - radius * current_height, destination.y - radius * current_height);
				i = 0;
				current_width = 0;

			}
		}
		break;
	case SOUTH:
		for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
		{
			loop = 0;
			//get the unit if is the lead continue
			Entity* single_unit = item._Ptr->_Myval;
			current_width++;
			if (single_unit == lead) {
				continue;
			}
			if (left == true) {
				i++;
			}

			//set path for the other units
			new_destination.create(copy_destination.x + radius * i, copy_destination.y);
			iPoint check = App->map->WorldToMap(new_destination.x, new_destination.y);
			while (!App->pathfinding->IsWalkable(check) && !PlaceTaken(locations_takens, new_destination)) {
				new_destination.create(destination.x, destination.y - radius * loop);
				loop++;
				check = App->map->WorldToMap(new_destination.x, new_destination.y);
			}
			locations_takens.push_back(new_destination);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);

			if (active)	App->pathfinding->PushPath((Unit*)single_unit, new_destination,  false, PRIMARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::PRIMARY);
			else App->pathfinding->PushPath((Unit*)single_unit, new_destination,  false, SECONDARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::SECONDARY);

			left = !left;
			i *= -1;
			if (current_width == formation.width) {
				current_height++;
				copy_destination.create(destination.x, destination.y - radius * current_height);
				i = 0;
				current_width = 0;

			}
		}
		break;
	case SOUTH_WEST:
		for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
		{

			loop = 0;
			//get the unit if is the lead continue
			Entity* single_unit = item._Ptr->_Myval;
			current_width++;
			if (single_unit == lead) {
				continue;
			}
			if (left == true) {
				i++;
			}

			//set path for the other units
			new_destination.create(copy_destination.x - radius * i, copy_destination.y - radius * i);
			iPoint check = App->map->WorldToMap(new_destination.x, new_destination.y);
			while (!App->pathfinding->IsWalkable(check) && !PlaceTaken(locations_takens, new_destination)) {
				new_destination.create(destination.x - radius * loop, destination.y + radius * loop);	
				loop++;
				check = App->map->WorldToMap(new_destination.x, new_destination.y);
			}
			locations_takens.push_back(new_destination);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);

			if (active)	App->pathfinding->PushPath((Unit*)single_unit, new_destination,  false, PRIMARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::PRIMARY);
			else App->pathfinding->PushPath((Unit*)single_unit, new_destination,  false, SECONDARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::SECONDARY);

			left = !left;
			i *= -1;
			if (current_width == formation.width) {
				current_height++;
				copy_destination.create(destination.x + radius * current_height, destination.y - radius * current_height);
				i = 0;
				current_width = 0;

			}
		}
		break;
	case WEST:
		for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
		{
			loop = 0;
			//get the unit if is the lead continue
			Entity* single_unit = item._Ptr->_Myval;
			current_width++;
			if (single_unit == lead) {
				continue;
			}
			if (left == true) {
				i++;
			}

			//set path for the other units
			new_destination.create(copy_destination.x, copy_destination.y + radius * i);
			iPoint check = App->map->WorldToMap(new_destination.x, new_destination.y);
			while (!App->pathfinding->IsWalkable(check) && !PlaceTaken(locations_takens, new_destination)) {
				new_destination.create(destination.x + radius * loop, destination.y);
				loop++;
				check = App->map->WorldToMap(new_destination.x, new_destination.y);
			}
			locations_takens.push_back(new_destination);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);

			if (active)	App->pathfinding->PushPath((Unit*)single_unit, new_destination, false, PRIMARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::PRIMARY);
			else App->pathfinding->PushPath((Unit*)single_unit, new_destination, false, SECONDARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::SECONDARY);

			left = !left;
			i *= -1;
			if (current_width == formation.width) {
				current_height++;
				copy_destination.create(destination.x + radius * current_height, destination.y);
				i = 0;
				current_width = 0;

			}
		}
		break;
	case NORTH_WEST:
		for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
		{
			loop = 0;
			//get the unit if is the lead continue
			Entity* single_unit = item._Ptr->_Myval;
			current_width++;
			if (single_unit == lead) {
				continue;
			}
			if (left == true) {
				i++;
			}

			//set path for the other units
			new_destination.create(copy_destination.x + radius * i, copy_destination.y - radius * i);
			iPoint check = App->map->WorldToMap(new_destination.x, new_destination.y);
			while (!App->pathfinding->IsWalkable(check) && !PlaceTaken(locations_takens, new_destination)) {
				new_destination.create(destination.x + radius * loop, destination.y + radius * loop);
				loop++;
				check = App->map->WorldToMap(new_destination.x, new_destination.y);
			}
			locations_takens.push_back(new_destination);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::PRIMARY);
			single_unit->GetWorker()->ResetChannel(TASK_CHANNELS::SECONDARY);

			if (active)	App->pathfinding->PushPath((Unit*)single_unit, new_destination, false, PRIMARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::PRIMARY);
			else App->pathfinding->PushPath((Unit*)single_unit, new_destination, false, SECONDARY);
			//	single_unit->AddAction((Action*)App->action_manager->MoveAction(unit_destination, (Unit*)single_unit), TASK_CHANNELS::SECONDARY);

			left = !left;
			i *= -1;
			if (current_width == formation.width) {
				current_height++;
				copy_destination.create(destination.x + radius * current_height, destination.y + radius * current_height);
				i = 0;
				current_width = 0;

			}
		}
		break;
	default:
		break;
	}
	/*
	//iterate list of units
	for (std::list<Entity*>::iterator item = units->begin(); item != units->end(); item++)
	{
	//get the unit if is the lead continue
	Entity* single_unit = item._Ptr->_Myval;
	if (single_unit == lead) {
	continue;
	}
	//set path for the other units
	std::vector<iPoint>* first_destination = nullptr;
	std::vector<iPoint>* second_destination = nullptr;
	//get closer new middle point STILL NEED IMPROVEMENT
	//	new_middle_point.create(middle_point.x + 30 * i, middle_point.y + 30 * i);
	//	first_destination = App->pathfinding->SimpleAstar(single_unit->GetPositionRounded(), new_middle_point);
	//get closer new destination STILL NEED IMPROVEMENT
	new_destination.create(destination.x + 20 * i, destination.y + 20 * i);
	second_destination = App->pathfinding->SimpleAstar(single_unit->GetPositionRounded(), new_destination);
	//second_destination->insert(second_destination->end(), first_destination->begin(), first_destination->end());
	//push action
	single_unit->AddAction((Action*)App->action_manager->MoveAction(second_destination, (Unit*)single_unit));
	i++;

	}
	*/
}

bool j1GroupMovement::GroupCanWalk(const iPoint & position)
{
	iPoint check = position;
	for (int i = 0; i < group_size; i++)
	{

	}
	return false;
}

bool j1GroupMovement::PlaceTaken(const std::vector<iPoint> positions, const iPoint& check)
{
	if (!positions.empty()) {
		for (int i = 0; i < positions.size(); i++)
		{
			if (positions[i] == check)
			{
				return true;
			}
		}
	}
	return false;
}

std::vector<iPoint>* j1GroupMovement::CreatePath()
{
	std::vector<iPoint>* first_destination = nullptr;
	std::vector<iPoint>* second_destination = nullptr;

	first_destination = CreateFirstDestination();
	second_destination = LeaderPath();
	second_destination->insert(second_destination->end(), first_destination->begin(), first_destination->end());
	return second_destination;
}

std::vector<iPoint>* j1GroupMovement::CreateFirstDestination()
{
	std::vector<iPoint>* path = nullptr;
	path = App->pathfinding->SimpleAstar(lead->GetPositionRounded(), middle_point);

	//first leader do the path to the middle point
	//then other units do the path
	//if leader last position look north and south add on middle positive x on width and positive y on height
	//if leader last position look north-east and south-west add on middle positive x and y on width and negative x positive y on height
	//if leader last position look east and west add on middle 0 on x and y on width and negative y on height
	//if leader last position look south-east north-west add on middle negative on x and positive y on width and negative x negative y on height
	return path;
}

std::vector<iPoint>* j1GroupMovement::LeaderPath()
{
	std::vector<iPoint>* path = nullptr;
	path = App->pathfinding->SimpleAstar(lead->GetPositionRounded(), destination);

	//first leader do the path to the middle point
	//then other units COPY the path with changed values
	//if leader last position look north and south add on middle positive x on width and positive y on height
	//if leader last position look north-east and south-west add on middle positive x and y on width and negative x positive y on height
	//if leader last position look east and west add on middle 0 on x and y on width and negative y on height
	//if leader last position look south-east north-west add on middle negative on x and positive y on width and negative x negative y on height
	return path;

}

void Formation::SetFormation(int size)
{
	if (size <= 60 && size > 40) {
		width = 15;
		height = 4;
	}
	if (size <= 40 && size > 27) {
		width = 10;
		height = 3;
	}
	if (size <= 27 && size > 10) {
		width = 7;
		height = 3;
	}
	if (size <= 10 && size > 5) {
		width = 5;
		height = 1;
	}
	else if (size <= 5) {
		width = size;
		height = 0;
	}
}
