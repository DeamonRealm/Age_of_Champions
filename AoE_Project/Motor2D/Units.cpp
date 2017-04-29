#include "Units.h"
#include "j1App.h"
#include "j1EntitiesManager.h"
#include "j1ActionManager.h"
#include "p2Log.h"
#include "Hud_GamePanel.h"
#include "j1Player.h"

/// Class Villager --------------------
//Constructors ========================
Villager::Villager(): resource_collected_type(PLAYER_RESOURCES::GP_NO_RESOURCE)
{

}

Villager::Villager(const Villager & copy) :Unit(copy), item_type(copy.item_type),resource_collected_type(copy.resource_collected_type), resources_capacity(copy.resources_capacity), current_resources(copy.current_resources), recollect_capacity(copy.recollect_capacity), recollect_rate(copy.recollect_rate)
{

}

//Destructors =========================
Villager::~Villager()
{

}



//Functionality =======================
//Actions -----
bool Villager::Die()
{
	if (action_type != DIE && action_type != DISAPPEAR)
	{
		if (this->GetDiplomacy() == ALLY) App->player->game_panel->IncressPopulation(-1, false);
		App->buff_manager->RemoveTargetBuffs(this);
		action_type = DIE;

		App->entities_manager->AddDeathUnit(this);
		if (item_type == GOLD || item_type == STONE || item_type == MEAT)
		{
			item_type = NO_ITEM;
		}
		App->animator->UnitPlay(this);
	}
	else if (current_animation->IsEnd())
	{
		if (action_type == DIE)
		{
			action_type = DISAPPEAR;
			App->animator->UnitPlay(this);
		}
		else
		{
			//action_worker.HardReset();
			App->entities_manager->RemoveDeathUnit(this);
			App->entities_manager->DeleteEntity(this);
			return true;
		}
	}
	return false;
}

//Get Methods ----------
ITEM_TYPE Villager::GetItemType() const
{
	return this->item_type;
}

PLAYER_RESOURCES Villager::GetResourceCollectedType() const
{
	return resource_collected_type;
}

uint Villager::GetCurrentResources() const
{
	return current_resources;
}

//Set Methods ----------
void Villager::SetItemType(ITEM_TYPE type)
{
	item_type = type;
}

void Villager::SetResourceCollectedType(PLAYER_RESOURCES type)
{
	resource_collected_type = type;
}

void Villager::SetResourcesCapacity(uint value)
{
	resources_capacity = value;
}
void Villager::SetRecollectCapacity(uint value)
{
	recollect_capacity = value;
}
void Villager::SetRecollectRate(uint value)
{
	recollect_rate = value;
}
void Villager::SetCurrentResources(uint value)
{
	current_resources = value;
}
//Actions --------------
bool Villager::Recollect(Resource** target)
{

	//Check if the target resource is in the "attack" (in this case used for recollect) area
	if (!attack_area.Intersects((*target)->GetInteractArea()))
	{
		iPoint goal = attack_area.NearestPoint((*target)->GetInteractArea());
		this->AddPriorizedAction((Action*)App->action_manager->MoveAction(this, goal, (*target)->GetPositionRounded()));
		return false;
	}

	//Check the action rate
	if (action_timer.Read() < attack_rate) return false;

	//Get resources from the target resource
	uint recollect_value = MIN(recollect_capacity, resources_capacity - current_resources);

	//Extract resource material, if it fails return true to end the recollect action
	if (!(*target)->ExtractResources(&recollect_value))
	{
		*target = nullptr;
		if (current_resources > 0)
		{
			//Go to the nearest download point
			Building* save_point = App->entities_manager->SearchNearestSavePoint(GetPositionRounded());
			if (save_point == nullptr)return true;
			//Set the carry action animation type
			AddAction((Action*)App->action_manager->SaveResourcesAction(this, save_point), TASK_CHANNELS::PRIMARY);
			return true;
		}
		else
		{
			this->action_type = IDLE;
			App->animator->UnitPlay(this);
		}
		return true;
	}

	//Add extracted resources at the villager
	current_resources += recollect_value;

	//If villager is full let's find a place to download
	if (current_resources == resources_capacity)
	{
		//Go to the nearest download point
		Building* save_point = App->entities_manager->SearchNearestSavePoint(GetPositionRounded());
		if (save_point == nullptr)return true;
		//Set the carry action animation type
		AddPriorizedAction((Action*)App->action_manager->SaveResourcesAction(this, save_point));
		return false;
	}

	//Reset interaction timer
	action_timer.Start();

	return false;
}

bool Villager::SaveResources()
{
	//Check if the target building is in the "attack" (in this case used for save resources) area
	if (!attack_area.Intersects(((Building*)interaction_target)->GetInteractArea()))
	{
		iPoint intersect_point = attack_area.NearestPoint(((Building*)interaction_target)->GetInteractArea());
		this->AddPriorizedAction((Action*)App->action_manager->MoveAction(this, iPoint(intersect_point.x, intersect_point.y)));
		return false;
	}

	//Store all the resources collected in the player bag
	App->player->game_panel->AddResource(current_resources, resource_collected_type);

	//Reset all the resources data so the next action will not be affected for it
	this->ResetResourcesData();
	App->animator->UnitPlay(this);

	return true;
}

void Villager::CheckRecollectResource(RESOURCE_TYPE type)
{
	bool changed = false;
	switch (type)
	{
	case TREE:
	case TREE_CUT:
	case CHOP:
		if (resource_collected_type != GP_WOOD)
		{
			ResetResourcesData();
			resource_collected_type = GP_WOOD;
		}
		if (item_type != ITEM_TYPE::AXE)
		{
			item_type = AXE;
			changed = true;
		}
		break;
	case BERRY_BUSH:
		if (resource_collected_type != GP_MEAT)
		{
			ResetResourcesData();
			resource_collected_type = GP_MEAT;
		}
		if (item_type != ITEM_TYPE::BASKET)
		{
			item_type = BASKET;
			changed = true;
		}
		break;
	case GOLD_ORE:
	case TINY_GOLD_ORE:
		if (resource_collected_type != GP_GOLD)
		{
			ResetResourcesData();
			resource_collected_type = GP_GOLD;
		}
		if (item_type != ITEM_TYPE::PICK)
		{
			item_type = PICK;
			changed = true;
		}
		break;
	case STONE_ORE:
	case TINY_STONE_ORE:
		if (resource_collected_type != GP_STONE)
		{
			ResetResourcesData();
			resource_collected_type = GP_STONE;
		}
		if (item_type != ITEM_TYPE::PICK)
		{
			item_type = PICK;
			changed = true;
		}
		break;
	}

	action_type = ATTATCK;
	App->animator->UnitPlay(this);
}

void Villager::CheckCarryResource()
{
	switch (resource_collected_type)
	{
	case GP_NO_RESOURCE:
		item_type = NO_ITEM;
		break;
	case GP_WOOD:
		item_type = ITEM_TYPE::WOOD;
		break;
	case GP_MEAT:
		item_type = ITEM_TYPE::MEAT;
		break;
	case GP_GOLD:
		item_type = ITEM_TYPE::GOLD;
		break;
	case GP_STONE:
		item_type = ITEM_TYPE::STONE;
		break;
	}
}

void Villager::ResetResourcesData()
{
	item_type = NO_ITEM;
	resource_collected_type = GP_NO_RESOURCE;
	current_resources = 0;
	action_type = IDLE;
}

/// -----------------------------------
