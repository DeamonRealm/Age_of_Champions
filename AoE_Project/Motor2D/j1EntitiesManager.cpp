#include "j1EntitiesManager.h"

#include "j1App.h"
#include "j1Render.h"
#include "j1Animator.h"

//Constructors ========================
j1EntitiesManager::j1EntitiesManager()
{

}

//Destructors =========================
j1EntitiesManager::~j1EntitiesManager()
{
}

//Game Loop ===========================
bool j1EntitiesManager::Awake(pugi::xml_node & config_node)
{
	return true;
}

bool j1EntitiesManager::Start()
{
	return true;
}

bool j1EntitiesManager::Update(float dt)
{
	std::list<Unit*>::const_iterator unit_item = units.begin();
	while (unit_item != units.end())
	{
		
		unit_item++;
	}
	return true;
}

bool j1EntitiesManager::CleanUp()
{
	//Clean Up Units List
	std::list<Unit*>::iterator units_item = units.begin();
	while (units_item != units.end())
	{
		RELEASE(units_item._Ptr->_Myval);
		units_item++;
	}
	units.clear();

	//Clean Up resources list
	std::list<Resource*>::iterator resources_item = resources.begin();
	while (resources_item != resources.end())
	{
		RELEASE(resources_item._Ptr->_Myval);
		resources_item++;
	}
	resources.clear();

	//Clean Up buildings list
	std::list<Building*>::iterator buildings_item = buildings.begin();
	while (buildings_item != buildings.end())
	{
		RELEASE(buildings_item._Ptr->_Myval);
		buildings_item++;
	}
	buildings.clear();

	//Clean Up units_defs vector
	uint size = units_defs.size();
	if (size > 0)
	{
		size--;
		for (uint k = size; k >= 0; k--)
		{
			RELEASE(units_defs[k]);
		}
		units_defs.clear();
	}

	//Clean Up resoureces_defs vector
	size = resoureces_defs.size();
	if (size > 0)
	{
		size--;
		for (uint k = size; k >= 0; k--)
		{
			RELEASE(resoureces_defs[k]);
		}
		resoureces_defs.clear();
	}
	//Clean Up buildings_defs vector
	size = buildings_defs.size();
	if (size > 0)
	{
		size--;
		for (uint k = size; k >= 0; k--)
		{
			RELEASE(buildings_defs[k]);
		}
		buildings_defs.clear();
	}

	return true;
}

//Functionality =======================
Unit * j1EntitiesManager::GenerateUnit(UNIT_TYPE type)
{
	Unit* new_unit = nullptr;

	switch (type)
	{
	case NO_UNIT: new_unit = new Unit("undefined"); break;

	case MILITIA:
		new_unit = new Unit("Militia");
		new_unit->SetUnitType(UNIT_TYPE::MILITIA);
		new_unit->SetAction(ACTION_TYPE::IDLE);
		new_unit->SetDirection(DIRECTION_TYPE::SOUTH);
		new_unit->SetTexture(App->animator->GetTextureAt(1));
		new_unit->SetAnimation(App->animator->UnitPlay(MILITIA, IDLE, WEST));
	 break;

	case ARBALEST:
		new_unit = new Unit("Arbalest");
		new_unit->SetUnitType(UNIT_TYPE::ARBALEST);
		new_unit->SetAction(ACTION_TYPE::IDLE);
		new_unit->SetDirection(DIRECTION_TYPE::SOUTH);
		new_unit->SetTexture(App->animator->GetTextureAt(0));
		new_unit->SetAnimation(App->animator->UnitPlay(ARBALEST, IDLE, WEST));
	 break;
	}

	//Add new unit at the manager list
	if(new_unit != nullptr)units.push_back(new_unit);
	
	//Return the generated unit
	return new_unit;
}

Building * j1EntitiesManager::GenerateBuilding(BUILDING_TYPE type)
{
	Building* new_buidling = nullptr;

	switch (type)
	{
	case NO_BUILDING:	new_buidling = new Building("undefined");		 break;
	case FARM:
		break;
	case TOWN_CENTER:
		new_buidling = new Building("Town Center");
		new_buidling->SetTexture(App->animator->GetTextureAt(2));
		new_buidling->SetAnimation(App->animator->BuildingPlay(TOWN_CENTER, IDLE, NO_DIRECTION));
		break;
	}

	//Add new unit at the manager list
	if (new_buidling != nullptr)buildings.push_back(new_buidling);

	//Return the generated unit
	return new_buidling;
}