#include "BaseEntities.h"

#include "j1App.h"
#include "j1Render.h"
#include "j1Animator.h"
#include "j1Pathfinding.h"
#include "j1Fonts.h"
#include "j1Map.h"
#include "p2Log.h"
#include "j1EntitiesManager.h"
#include "j1ActionManager.h"

///Class Entity ---------------------------------
//Constructors ========================
Entity::Entity() :name(""), action_worker(new ActionWorker())
{
	
}

Entity::Entity(const Entity& copy) : name(copy.name), position(copy.position), entity_type(copy.entity_type), entity_diplomacy(copy.entity_diplomacy), selection_rect(copy.selection_rect),
icon_rect(copy.icon_rect), max_life(copy.max_life), life(copy.life), current_animation(copy.current_animation), action_worker(new ActionWorker())
{

}

//Destructors =========================
Entity::~Entity()
{

}


//Functionality =======================
//Select/Deselect -
void Entity::Select()
{
	selected = true;
}

void Entity::Deselect()
{
	selected = false;
}

//Update ----------
bool Entity::Update()
{
	action_worker->Update();
	return true;
}

// Draw -----------
bool Entity::Draw(bool debug)
{
	bool ret = false;

	//Draw Entity Current animation frame
	const Sprite* sprite = current_animation.GetCurrentSprite();
	ret = App->render->CallBlit(current_animation.GetTexture(), position.x, position.y, sprite->GetFrame(), flip_sprite, -position.y - sprite->GetZ_cord(), sprite->GetXpivot(), sprite->GetYpivot());

	return ret;
}

//Add Action ------------
void Entity::AddAction(Action * action)
{
	action_worker->AddAction(action);
}

void Entity::AddPriorizedAction(Action * action)
{
	action_worker->AddPriorizedAction(action);
}

//Set Methods -----
void Entity::SetName(const char * name_str)
{
	name = name_str;
}

void Entity::SetPosition(float x, float y)
{
	position.x = x;
	position.y = y;
}

void Entity::SetEntityType(ENTITY_TYPE type)
{
	entity_type = type;
}

void Entity::SetDiplomacy(DIPLOMACY new_diplomacy)
{
	entity_diplomacy = new_diplomacy;
}

void Entity::SetMaxLife(uint full_life_val)
{
	max_life = full_life_val;
}

void Entity::SetLife(uint life_val)
{
	life = life_val;
}

void Entity::SetAnimation(Animation * anim)
{
	current_animation = *anim;
}

void Entity::SetFlipSprite(bool flip)
{
	flip_sprite = flip;
}

void Entity::SetSelectionRect(const SDL_Rect & rect)
{
	selection_rect = rect;
}

void Entity::SetIcon(const SDL_Rect & icon)
{
	icon_rect = icon;
}

// ----------------
//Get Methods -----
const char* Entity::GetName() const
{
	return name.c_str();
}

const fPoint& Entity::GetPosition() const
{
	return position;
}

iPoint Entity::GetPositionRounded() const
{
	return iPoint(position.x, position.y);
}

ENTITY_TYPE Entity::GetEntityType() const
{
	return entity_type;
}

DIPLOMACY Entity::GetDiplomacy() const
{
	return entity_diplomacy;
}

uint Entity::GetMaxLife() const
{
	return max_life;
}

uint Entity::GetLife() const
{
	return life;
}

Animation* Entity::GetAnimation() 
{
	return &current_animation;
}

bool Entity::GetFlipSprite() const
{
	return flip_sprite;
}

const SDL_Rect * Entity::GetSelectionRect() const
{
	return &selection_rect;
}

const SDL_Rect& Entity::GetIcon()const
{
	return icon_rect;
}

ActionWorker * Entity::GetWorker() const
{
	return action_worker;
}

// ----------------
///----------------------------------------------


///Class Unit -----------------------------------
//Constructors ========================
Unit::Unit() :Entity()
{

}

Unit::Unit(const Unit& copy) : Entity(copy), unit_type(copy.unit_type), vision(copy.vision), mark(copy.mark),soft_collider(copy.soft_collider),hard_collider(copy.hard_collider), view_area(copy.view_area),
speed(copy.speed), action_type(copy.action_type), direction_type(copy.direction_type), attack_hitpoints(copy.attack_hitpoints), attack_bonus(copy.attack_bonus), siege_hitpoints(copy.siege_hitpoints),
attack_rate(copy.attack_rate), attack_type(copy.attack_type), attack_area(copy.attack_area), defense(copy.defense), defense_bonus(copy.defense_bonus), armor(copy.armor), armor_bonus(copy.armor_bonus),
food_cost(copy.food_cost), wood_cost(copy.wood_cost), gold_cost(copy.gold_cost), population_cost(copy.population_cost), train_time(copy.train_time)
{

}

//Destructors =========================
Unit::~Unit()
{

}

//Functionality =======================
//Draw ------------
bool Unit::Draw(bool debug)
{
	bool ret = false;

	//Draw Entity Mark
	if (selected)ret = mark.Draw();
	attack_area.Draw();
	/*
	if (debug) {
		if (selected)
			soft_collider.Draw();
			hard_collider.Draw();
			vision.Draw();
	}
	*/
	/*if (debug) {
	//Draw Entity Selection Rect
	App->render->DrawQuad({ (int)floor(position.x + selection_rect.x - selection_rect.w * 0.5f),(int)position.y + selection_rect.y, selection_rect.w,-selection_rect.h }, 50, 155, 255, 100, true);

	//Draw axis lines to check the center of the unit (tool used during the sprites allocation)
	int length = 55;
	iPoint p1 = { (int)position.x, (int)position.y - length };
	iPoint p2 = { (int)position.x, (int)position.y + length };
	SDL_Color color = { 50,255,200,255 };
	Line y_axis(p1, p2, color);
	p1.x -= length;
	p1.y += length;
	p2.x += length;
	p2.y -= length;
	Line x_axis(p1, p2, color);
	y_axis.Draw();
	x_axis.Draw();
	if (path != nullptr)DrawPath();

	}*/

	//Draw Entity Current animation frame
	const Sprite* sprite = current_animation.GetCurrentSprite();
	ret = App->render->CallBlit(current_animation.GetTexture(), position.x, position.y, sprite->GetFrame(), flip_sprite, -position.y - sprite->GetZ_cord(), sprite->GetOpacity(), sprite->GetXpivot(), sprite->GetYpivot());

	return ret;
}

bool Unit::DrawPath()
{
	if (path == nullptr) return false;

	uint size = path->size();
	for (uint k = 0; k < size; k++)
	{
		iPoint cell = path->at(k);
		App->render->CallBlit(App->pathfinding->path_texture, cell.x - App->map->data.tile_width * 0.5f, cell.y - App->map->data.tile_height * 0.5);
	}

	return true;
}

//Actions ---------
bool Unit::Move(std::vector<iPoint>* path) ///Returns true when it ends
{
	//Check if the unit have an assigned path
	if (path == nullptr)
	{
		LOG("Error path not found!");
		return true;
	}



	//Build goal path point
	iPoint goal = path->back();
	iPoint location = iPoint(position.x, position.y);
	//Update goal node and animation direction
	if (location.DistanceTo(goal) < 2)
	{
		if (path->size() == 1)
		{
			//Set unit at the goal pixel position
			SetPosition((float)goal.x, (float)goal.y);
			//Stop idle walk animation
			action_type = IDLE;
			App->animator->UnitPlay(this);

			//Delete unit path
			delete path;
			path = nullptr;

			return true;
		}
		//if we have a colision with other unit and we have lower priority reduction of spped

		//Look in the next update if there is an error
		iPoint next_update = *(path->rbegin() + 1);
		if (!App->pathfinding->IsWalkable(App->map->WorldToMap(next_update.x, next_update.y)))
		{
			std::vector<iPoint>* new_path;
			path->pop_back();

			iPoint next_goal;
			for (int i = path->size() - 1; i >= 0; i--) {
				if (App->pathfinding->IsWalkable(App->map->WorldToMap(path->at(i).x, path->at(i).y))) {
					next_goal = path->at(i);
					break;
				}
				else
					path->pop_back();

			}
			if (path->empty())
			{
				return true;
			}
			new_path=App->pathfinding->SimpleAstar(location, next_goal);
			
			path->insert(path->end(), new_path->begin(), new_path->end());
		}
		//Set the unit next tile goal

		path->pop_back();
		goal = path->back();
		
		//Focus the unit at the next goal
		Focus(goal);
	}

	//Check actor animation
	if (action_type != WALK)
	{
		action_type = WALK;
		Focus(goal);
	}

	//Calculate the X/Y values that the unit have to move 
	//checking the goal location and the unit movement speed
	int norm = location.DistanceTo(goal);
	float x_step = speed * (goal.x - location.x) / norm;
	float y_step = speed * (goal.y - location.y) / norm;

	//Add the calculated values at the unit & mark position
	SetPosition(position.x + x_step, position.y + y_step);

	return false;
}

void Unit::Focus(const iPoint & target)
{
	//Calculate the directional vector
	iPoint dir_point = target - iPoint(position.x, position.y);

	//Find the correct direction in relation of the goal and the location
	if (abs(dir_point.x) < 4)
	{
		if (dir_point.y > 0)direction_type = DIRECTION_TYPE::SOUTH;
		else direction_type = DIRECTION_TYPE::NORTH;
	}
	else if (abs(dir_point.y) < 4)
	{
		if (dir_point.x > 0)direction_type = DIRECTION_TYPE::EAST;
		else direction_type = DIRECTION_TYPE::WEST;
	}
	else if (dir_point.x >= 0 && dir_point.y >= 0)
	{
		direction_type = DIRECTION_TYPE::SOUTH_EAST;
	}
	else if (dir_point.x <= 0 && dir_point.y >= 0)
	{
		direction_type = DIRECTION_TYPE::SOUTH_WEST;
	}
	else if (dir_point.x >= 0 && dir_point.y <= 0)
	{
		direction_type = DIRECTION_TYPE::NORTH_EAST;
	}
	else if (dir_point.x <= 0 && dir_point.y <= 0)
	{
		direction_type = DIRECTION_TYPE::NORTH_WEST;
	}

	//Set the unit animation with the new direction
	App->animator->UnitPlay(this);
}
DIRECTION_TYPE Unit::LookDirection(const iPoint & from, const iPoint & to)
{
	//Calculate the directional vector
	iPoint dir_point = from - to;

	//Find the correct direction in relation of the goal and the location
	if (abs(dir_point.x) < 4)
	{
		if (dir_point.y > 0)return SOUTH;
		else return NORTH;
	}
	else if (abs(dir_point.y) < 4)
	{
		if (dir_point.x > 0)return EAST;
		else return WEST;
	}
	else if (dir_point.x >= 0 && dir_point.y >= 0)
	{
		return SOUTH_EAST;
	}
	else if (dir_point.x <= 0 && dir_point.y >= 0)
	{
		return SOUTH_WEST;
	}
	else if (dir_point.x >= 0 && dir_point.y <= 0)
	{
		return NORTH_EAST;
	}
	else if (dir_point.x <= 0 && dir_point.y <= 0)
	{
		return NORTH_WEST;
	}
	else {
		return NO_DIRECTION;
	}
}
bool Unit::AttackUnit()
{

	//Check if the target is in the attack area
	if (!attack_area.Intersects(&interaction_target->GetPositionRounded()))
	{
		iPoint goal = attack_area.NearestPoint(((Unit*)interaction_target)->GetAttackArea());
		this->AddPriorizedAction((Action*)App->action_manager->MoveAction(this, goal.x, goal.y));
		return false;
	}

	//Control action rate
	if (action_timer.Read() < attack_rate)return false;
	
	//Set unit attack animation
	if (action_type != ATTATCK)
	{
		action_type = ATTATCK;
		Focus(interaction_target->GetPositionRounded());
		App->animator->UnitPlay(this);
	}

	if (interaction_target->GetLife() == 0)
	{
		ACTION_TYPE act = ((Unit*)interaction_target)->action_type;
		if (this->action_type == ATTATCK)
		{
			action_type = IDLE;
			App->animator->UnitPlay(this);
		}
		if(act != DIE && act != DISAPPEAR)interaction_target->AddAction((Action*)App->action_manager->DieAction((Unit*)interaction_target));
		return true;
	}
	//Calculate the attack & apply the value at the target life points
	((Unit*)interaction_target)->life -= MIN(((Unit*)interaction_target)->life, attack_hitpoints);

	//Reset action timer
	action_timer.Start();

	return false;
}

bool Unit::AttackBuilding()
{
	return true;
}

bool Unit::Cover()
{
	return ((HabitableBuilding*)interaction_target)->CoverUnit(this);
}

bool Unit::Die()
{
	if (action_type != DIE && action_type != DISAPPEAR)
	{
		action_type = DIE;
		App->animator->UnitPlay(this);
	}
	else if (current_animation.IsEnd())
	{
		if (action_type == DIE)
		{
			action_type = DISAPPEAR;
			App->animator->UnitPlay(this);
		}
		else
		{
			App->entities_manager->DeleteEntity(this);
			return true;
		}
	}
	return false;
}

//Bonus -----------
void Unit::AddBonus(BONUS_TYPE type, uint type_id, uint bonus, bool defence)
{
	Bonus* new_bonus = new Bonus(type, type_id, bonus);
	if (defence) defence_bonuses.push_back(new_bonus);
	else defence_bonuses.push_back(new_bonus);
}

//Set Methods -----
void Unit::SetPosition(float x, float y)
{
	//Extract the units to push it with the new position later
	if (!App->entities_manager->units_quadtree.Exteract(&position))
	{
		int k = 0;
		k++;
	}

	//Set unit position
	position.x = x;
	position.y = y;
	//Set unit vision position
	vision.SetPosition(iPoint(position.x, position.y));
	//Set unit mark position
	mark.SetPosition(iPoint(position.x, position.y));
	//Set soft_collider mark position
	soft_collider.SetPosition(iPoint(position.x, position.y));
	//Set hard_collider mark position
	hard_collider.SetPosition(iPoint(position.x, position.y));
	//Set unit attack area position
	attack_area.SetPosition(iPoint(position.x, position.y));

	//Add the unit with the correct position in the correct quad tree
	App->entities_manager->units_quadtree.Insert(this, &position);
}

void Unit::SetAttackBuff(float atk_buff)
{
	attack_buff = atk_buff;
}

void Unit::SetUnitType(UNIT_TYPE type)
{
	unit_type = type;
}

void Unit::SetInteractionTarget(const Entity * target)
{
	interaction_target = (Entity*)target;
}

void Unit::SetVision(const Circle & new_vision)
{
	vision = new_vision;
}

void Unit::SetMark(const Circle & new_mark)
{
	mark = new_mark;
}

void Unit::SetSoftCollider(const Circle & new_soft_collider)
{
	soft_collider = new_soft_collider;
}

void Unit::SetHardCollider(const Circle & new_hard_collider)
{
	hard_collider = new_hard_collider;
}

void Unit::SetViewArea(uint area_val)
{
	view_area = area_val;
}

void Unit::SetSpeed(float speed_val)
{
	speed = speed_val;
}

void Unit::SetAction(ACTION_TYPE action_val)
{
	action_type = action_val;
}

void Unit::SetDirection(DIRECTION_TYPE direction_val)
{
	direction_type = direction_val;
}

void Unit::SetAttackDelay(uint atk_delay)
{
	attack_delay = atk_delay;
}

void Unit::SetAttackHitPoints(uint atk_val)
{
	attack_hitpoints = atk_val;
}

void Unit::SetAttackBonus(uint atk_bonus)
{
	attack_bonus = atk_bonus;
}

void Unit::SetSiegeHitPoints(uint siege_val)
{
	siege_hitpoints = siege_val;
}

void Unit::SetAttackRate(uint atk_rate)
{
	attack_rate = atk_rate;
}

void Unit::SetAttackType(ATTACK_TYPE atk_type)
{
	attack_type = atk_type;
}

void Unit::SetAttackArea(const Circle & atk_area)
{
	attack_area = atk_area;
}

void Unit::SetDefense(uint def)
{
	defense = def;
}

void Unit::SetDefenseBonus(uint def_bonus)
{
	defense_bonus = def_bonus;
}

void Unit::SetDefenseBuff(float def_buff)
{
	defense_buff = def_buff;
}

void Unit::SetArmor(uint arm)
{
	armor = arm;
}

void Unit::SetArmorBonus(uint arm_bonus)
{
	armor_bonus = arm_bonus;
}

void Unit::SetArmorBuff(float arm_buff)
{
	armor_buff = arm_buff;
}

void Unit::SetFoodCost(uint food_cst)
{
	food_cost = food_cst;
}

void Unit::SetWoodCost(uint wood_cst)
{
	wood_cost = wood_cst;
}

void Unit::SetGoldCost(uint coin_cst)
{
	gold_cost = coin_cst;
}

void Unit::SetPopulationCost(uint poblation_cst)
{
	population_cost = poblation_cst;
}

void Unit::SetTrainTime(uint train_time_val)
{
	train_time = train_time_val;
}
void Unit::SetExp(uint experience)
{
	exp = experience;
}
void Unit::SetPath(const std::vector<iPoint>* new_path)
{
	path = (std::vector<iPoint>*)new_path;
}
// ----------------
//Get Methods -----
UNIT_TYPE Unit::GetUnitType()const
{
	return unit_type;
}

const Circle& Unit::GetMark() const
{
	return mark;
}

const Entity * Unit::GetInteractionTarget()
{
	return interaction_target;
}

uint Unit::GetViewArea()const
{
	return view_area;
}

float Unit::GetSpeed()const
{
	return speed;
}

ACTION_TYPE Unit::GetAction()const
{
	return action_type;
}

DIRECTION_TYPE Unit::GetDirection()const
{
	return direction_type;
}

uint Unit::GetAttackDelay() const
{
	return attack_delay;
}

uint Unit::GetAttackHitPoints()const
{
	return attack_hitpoints;
}

uint Unit::GetAttackBonus()const
{
	return attack_bonus;
}

float Unit::GetAttackBuff() const
{
	return attack_buff;
}

uint Unit::GetSiegeHitPoints()const
{
	return siege_hitpoints;
}

uint Unit::GetAttackRate()const
{
	return attack_rate;
}

ATTACK_TYPE Unit::GetAttackType()const
{
	return attack_type;
}

uint Unit::GetAttackRange()const
{
	return attack_area.GetRad();
}

const Circle * Unit::GetAttackArea() const
{
	return &attack_area;
}

uint Unit::GetDefense()const
{
	return defense;
}

uint Unit::GetDefenseBonus() const
{
	return defense_bonus;
}

float Unit::GetDefenseBuff() const
{
	return defense_buff;
}

uint Unit::GetArmor() const
{
	return armor;
}

uint Unit::GetArmorBonus() const
{
	return armor_bonus;
}

float Unit::GetArmorBuff() const
{
	return armor_buff;
}

uint Unit::GetFoodCost() const
{
	return food_cost;
}

uint Unit::GetWoodCost() const
{
	return wood_cost;
}

uint Unit::GetGoldCost() const
{
	return gold_cost;
}

uint Unit::GetPopulationCost() const
{
	return population_cost;
}

uint Unit::GetTrainTime() const
{
	return train_time;
}
uint Unit::GetExp() const
{
	return exp;
}
// ----------------
///----------------------------------------------


///Class Resource -------------------------------
//Constructors ========================
Resource::Resource() :Entity()
{

}

Resource::Resource(const Resource& copy) : Entity(copy), resource_type(copy.resource_type), mark(copy.mark),interact_area(copy.interact_area)
{

}

//Destructors =========================
Resource::~Resource()
{

}

//Functionality =======================
bool Resource::Draw(bool debug)
{
	bool ret = false;
	//Draw Resource Mark
	ret = mark.Draw();

	/*if (debug) {
	//Draw Entity Selection Rect
	App->render->DrawQuad({ (int)floor(position.x + selection_rect.x - selection_rect.w * 0.5f),(int)position.y + selection_rect.y, selection_rect.w,-selection_rect.h }, 50, 155, 255, 100, true);

	//Draw axis lines to check the center of the unit (tool used during the sprites allocation)
	int length = 55;
	iPoint p1 = { (int)position.x, (int)position.y - length };
	iPoint p2 = { (int)position.x, (int)position.y + length };
	SDL_Color color = { 50,255,200,255 };
	Line y_axis(p1, p2, color);
	p1.x -= length;
	p1.y += length;
	p2.x += length;
	p2.y -= length;
	Line x_axis(p1, p2, color);
	y_axis.Draw();
	x_axis.Draw();
	}*/

	const std::vector<Sprite>* sprites = current_animation.GetAllSprites();

	uint size = sprites->size();
	for (uint k = 0; k < size; k++)
	{
		ret = App->render->CallBlit(current_animation.GetTexture(), position.x - sprites->at(k).GetXpivot(), position.y - sprites->at(k).GetYpivot(), sprites->at(k).GetFrame(), false, -position.y - sprites->at(k).GetZ_cord(), sprites->at(k).GetOpacity());
		if (!ret)break;
	}

	return ret;
}

bool Resource::ExtractResources(uint* value)
{
	if (life == 0)return false;
	if (life <= *value)
	{
		*value = life;
		life = 0;
		App->entities_manager->DeleteEntity(this);
		App->entities_manager->resources_quadtree.Exteract(&position);
		return true;
	}
	else
	{
		life -= *value;
		if (resource_type == GOLD_ORE && life < 50)
		{
			resource_type = TINY_GOLD_ORE;
			App->animator->ResourcePlay(this);
		}
		else if (resource_type == STONE_ORE && life < 50)
		{
			resource_type = TINY_STONE_ORE;
			App->animator->ResourcePlay(this);
		}

		return true;
	}
}

void Resource::SetPosition(float x, float y)
{
	//Set resource position fixing it in the tiles coordinates
	iPoint coords = App->map->WorldToMap(x, y);
	coords = App->map->MapToWorld(coords.x, coords.y);
	position.x = coords.x;
	position.y = coords.y;

	//Set resource interaction area position
	interact_area.SetPosition(iPoint(position.x, position.y));

	//Set resource mark position
	mark.SetPosition(iPoint(position.x, position.y));

	//Add Resource at the correct quad tree
	App->entities_manager->resources_quadtree.Insert(this, &position);
}

void Resource::SetMark(const Rectng & rectangle)
{
	mark = rectangle;
}

void Resource::SetInteractArea(const Circle & area)
{
	interact_area = area;
}

void Resource::SetResourceType(RESOURCE_TYPE type)
{
	resource_type = type;
}

const Rectng& Resource::GetMark() const
{
	return mark;
}

const Circle * Resource::GetInteractArea() const
{
	return &interact_area;
}

RESOURCE_TYPE Resource::GetResourceType() const
{
	return resource_type;
}
///----------------------------------------------


///Class Building -------------------------------
//Constructors ========================
Building::Building() :Entity()
{

}

Building::Building(const Building& copy) : Entity(copy), mark(copy.mark), building_type(copy.building_type), max_life(copy.max_life),
life(copy.life), width_in_tiles(copy.width_in_tiles), height_in_tiles(copy.height_in_tiles), interact_area(copy.interact_area)
{

}

//Destructors =========================
Building::~Building()
{

}

//Functionality =======================

//Draw ----------------------
bool Building::Draw(bool debug)
{
	bool ret = false;

	//Debug Draw
	ret = mark.Draw();

	/*if (debug) {
	//Draw Entity Selection Rect
	App->render->DrawQuad({ (int)floor(position.x + selection_rect.x - selection_rect.w * 0.5f),(int)position.y + selection_rect.y, selection_rect.w,-selection_rect.h }, 50, 155, 255, 100, true);

	//Draw axis lines to check the center of the unit (tool used during the sprites allocation)
	int length = 200;
	iPoint p1 = { (int)position.x, (int)position.y - length };
	iPoint p2 = { (int)position.x, (int)position.y + length };
	SDL_Color color = { 50,255,200,255 };
	Line y_axis(p1, p2, color);
	p1.x -= length;
	p1.y += length;
	p2.x += length;
	p2.y -= length;
	Line x_axis(p1, p2, color);
	y_axis.Draw();
	x_axis.Draw();
	}*/

	//Get all sprites of the current animation
	const std::vector<Sprite>*	sprites = current_animation.GetAllSprites();

	uint size = sprites->size();
	for (uint k = 0; k < size; k++)
	{
		ret = App->render->CallBlit(current_animation.GetTexture(), position.x - sprites->at(k).GetXpivot(), position.y - sprites->at(k).GetYpivot(), sprites->at(k).GetFrame(), false, -position.y - sprites->at(k).GetZ_cord(), sprites->at(k).GetOpacity());
		if (!ret)break;
	}

	return ret;
}

void Building::SetPosition(float x, float y)
{
	//Set building position fixing it in the tiles coordinates ( center position in the selected tile)
	iPoint map_coords = App->map->WorldToMap(x, y);
	iPoint world_coords = App->map->MapToWorld(map_coords.x, map_coords.y);
	position.x = world_coords.x;
	position.y = world_coords.y - (App->map->data.tile_height + 1) * 0.5f;

	//Set interaction area rectangle position
	interact_area.SetPosition(iPoint(position.x,position.y));
	
	//Set building mark position
	mark.SetPosition(iPoint(position.x, position.y));

	//Calculate the upper tile of the building zone
	iPoint upper_tile(map_coords.x - 2, map_coords.y - 1);

	//Update the logic & construction map
	//Check if the building is a town center to respect the build exception
	if (building_type == TOWN_CENTER)
	{
		App->map->ChangeLogicMap(upper_tile, width_in_tiles - 2, height_in_tiles - 2);
	}
	else
	{
		App->map->ChangeLogicMap(upper_tile, width_in_tiles, height_in_tiles);
	}

	App->map->ChangeConstructionMap(upper_tile, width_in_tiles, height_in_tiles);

	//Add building at the correct quad tree
	App->entities_manager->buildings_quadtree.Insert(this, &position);
}

//Set Methods ---------------
void Building::SetMark(const Rectng& rectangle)
{
	mark = rectangle;
}

void Building::SetInteractArea(const Rectng& rectangle)
{
	interact_area = rectangle;
}

void Building::SetWidthInTiles(uint width)
{
	width_in_tiles = width;
}

void Building::SetHeightInTiles(uint height)
{
	height_in_tiles = height;
}

void Building::SetBuildingType(BUILDING_TYPE type)
{
	building_type = type;
}

void Building::SetActionType(ACTION_TYPE type)
{
	action_type = type;
}

void Building::SetDirectionType(DIRECTION_TYPE type)
{
	direction_type = type;
}

void Building::SetMaxLife(uint max_life_val)
{
	max_life = max_life_val;
}

void Building::SetLife(uint life_val)
{
	life = life_val;
}

const Rectng & Building::GetMark() const
{
	return mark;
}

const Rectng & Building::GetInteractArea() const
{
	return interact_area;
}

//Get Methods ---------------
BUILDING_TYPE Building::GetBuildingType() const
{
	return building_type;
}

ACTION_TYPE Building::GetActionType() const
{
	return action_type;
}

DIRECTION_TYPE Building::GetDirectionType() const
{
	return direction_type;
}

uint Building::GetMaxLife() const
{
	return max_life;
}

uint Building::GetLife() const
{
	return life;
}
///----------------------------------------------