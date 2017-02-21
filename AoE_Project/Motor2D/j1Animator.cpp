#include "j1Animator.h"

#include "j1App.h"
#include "j1FileSystem.h"
#include "j1Render.h"
#include "j1Textures.h"

#include "SDL/include/SDL_rect.h"
#include "p2Log.h"
#include "BaseEntities.h"

///Animation Class ------------------------------
//Constructor =========================
Animation::Animation()
{

}

//Destructor ==========================
Animation::~Animation()
{
	frames.clear();
	pivots.clear();
}

//Functionality =======================
void Animation::SetLoop(bool loop_state)
{
	loop = loop_state;
}

void Animation::SetSpeed(uint new_speed)
{
	speed = new_speed;
}

void Animation::SetId(uint id)
{
	enum_id = id;
}

bool Animation::GetLoop() const
{
	return loop;
}

uint Animation::GetSpeed() const
{
	return speed;
}

const SDL_Rect& Animation::GetCurrentFrame()
{
	if (current_frame == -1)return frames[0];

	current_frame = (float)floor(timer.Read() / speed);
	if (current_frame >= frames.size())
	{
		if (loop)
		{
			loops++;
			current_frame = 0;
			timer.Start();
		}
		else
		{
			loops = 0;
			current_frame = -1;
			return frames[0];
		}
	}

	return frames[(int)current_frame];
}

const std::vector<SDL_Rect>* Animation::GetAllFrames() const
{
	return &frames;
}

const iPoint& Animation::GetCurrentPivot() const
{
	return pivots.at((int)current_frame);
}

const std::vector<iPoint>* Animation::GetAllPivots() const
{
	return &pivots;
}

uint Animation::GetId() const
{
	return enum_id;
}

void Animation::AddFrame(const SDL_Rect & rect, const iPoint & point)
{
	frames.push_back(rect);
	pivots.push_back(point);
}
/// ---------------------------------------------


/// Animation Block Class -----------------------
//Constructor =========================
Animation_Block::Animation_Block(uint enum_id) :enum_id(enum_id)
{

}

//Destructor ==========================
Animation_Block::~Animation_Block()
{
	ClearAnimationBlocks();
}

//Functionality =======================
void Animation_Block::ClearAnimationBlocks()
{
	while (childs.size() > 0)
	{
		childs.back()->ClearAnimationBlocks();
		childs.pop_back();

	}

	if(animation != nullptr)delete animation;
}

void Animation_Block::SetId(uint id)
{
	enum_id = id;
}

uint Animation_Block::GetId() const
{
	return enum_id;
}

Animation* Animation_Block::GetAnimation() const
{
	return animation;
}

Animation_Block * Animation_Block::GetBlock(int index) const
{
	return childs.at(index);
}

uint Animation_Block::GetChildsNum() const
{
	return childs.size();
}

Animation_Block * Animation_Block::SearchId(uint id) const
{
	uint size = childs.size();
	for (uint k = 0; k < size; k++)
	{
		if (childs[k]->GetId() == id)return childs[k];
	}

	return nullptr;
}

void Animation_Block::SetAnimation(const Animation * new_animation)
{
	if(new_animation != nullptr)animation = (Animation*)new_animation;
}

void Animation_Block::AddAnimationBlock(Animation_Block* new_animation_block)
{
	if(new_animation_block != nullptr)childs.push_back(new_animation_block);
}
/// ---------------------------------------------


//Animator Module -------------------------------
//Constructor =========================
j1Animator::j1Animator()
{
	name = "animator";
}

//Destructor ==========================
j1Animator::~j1Animator()
{
}

//Game Loop ===========================
bool j1Animator::Awake(pugi::xml_node& config)
{
	//Load civilization folder from config.xml(this is temporal)
	pugi::xml_node folder_node = config.first_child();

	//Load the focused civilization
	//LoadCivilization(folder_node.attribute("folder").as_string());

	return true;
}

bool j1Animator::Start()
{
	//Load Civilization Test
	bool ret = LoadCivilization("Teutones.xml");

	return ret;
}

bool j1Animator::PostUpdate()
{
	return true;
}

bool j1Animator::CleanUp()
{
	//Clean the unit blocks
	uint size = unit_blocks.size();

	for (uint k = 0; k < size; k++)
	{
		unit_blocks[k]->ClearAnimationBlocks();
	}
	unit_blocks.clear();
	//Clean the building blocks
	size = building_blocks.size();

	for (uint k = 0; k < size; k++)
	{
		building_blocks[k]->ClearAnimationBlocks();
	}
	building_blocks.clear();
	//Clean the building blocks
	size = resource_blocks.size();

	for (uint k = 0; k < size; k++)
	{
		resource_blocks[k]->ClearAnimationBlocks();
	}
	resource_blocks.clear();

	//Just clear vector information of textures (module textures really unload the textures)
	textures.clear();
	return true;
}

//Methods that transform strings to enums (used when loading data from xml)
UNIT_TYPE j1Animator::Str_to_UnitEnum(const char* str) const
{
	if (strcmp(str, "militia") == 0)	return MILITIA;
	if (strcmp(str, "arbalest") == 0)	return ARBALEST;
	return NO_UNIT;
}

ACTION_TYPE j1Animator::Str_to_ActionEnum(const char* str) const
{
	if (strcmp(str, "attack") == 0)		return ATTATCK;
	if (strcmp(str, "die") == 0)		return DIE;
	if (strcmp(str, "disapear") == 0)	return DISAPEAR;
	if (strcmp(str, "idle") == 0)		return IDLE;
	if (strcmp(str, "walk") == 0)		return WALK;
	return NO_ACTION;
}

DIRECTION_TYPE j1Animator::Str_to_DirectionEnum(const char* str) const
{
	if (strcmp(str, "north") == 0)			return NORTH;
	if (strcmp(str, "north-east") == 0)		return NORTH_EAST;
	if (strcmp(str, "south-east") == 0)		return EAST;
	if (strcmp(str, "east") == 0)			return SOUTH_EAST;
	if (strcmp(str, "south") == 0)			return SOUTH;
	if (strcmp(str, "south-west") == 0)		return SOUTH_WEST;
	if (strcmp(str, "west") == 0)			return WEST;
	if (strcmp(str, "north-west") == 0)		return NORTH_WEST;
	return NO_DIRECTION;
}

BUILDING_TYPE j1Animator::Str_to_BuildingEnum(const char* str) const
{
	if (strcmp(str, "town_center") == 0)	return TOWN_CENTER;
	return NO_BUILDING;
}

//Functionality =======================
bool j1Animator::LoadCivilization(const char* folder)
{
	j1Timer time;
	time.Start();
	LOG("---- Loading %s...", folder);

	//Load civilization data from loaded folder
	char* buffer = nullptr;
	std::string load_folder = name + "/" + folder;
	int size = App->fs->Load(load_folder.c_str(), &buffer);
	pugi::xml_document civilization_data;
	pugi::xml_parse_result result = civilization_data.load_buffer(buffer, size);
	RELEASE(buffer);

	//Check result of the buffer loaded
	if (result == NULL)
	{
		LOG("Error loading civilization data: %s", result.description());
		return false;
	}

	//Boolean to check the correct file loads
	bool ret = true;
	std::string tex_folder;
	std::string tex_file;
	//Load civilization units list
	pugi::xml_node unit_node = civilization_data.child("data").child("units").first_child();
	while (unit_node != NULL)
	{
		if (!ret)break;
		ret = LoadUnitBlock(unit_node.attribute("xml").as_string());
		tex_file = unit_node.attribute("spritesheet").as_string();
		tex_folder = name + "/" + tex_file;
		textures.push_back(App->tex->Load(tex_folder.c_str()));
		unit_node = unit_node.next_sibling();
	}
	//Load civilization buildings list
	pugi::xml_node building_node = civilization_data.child("data").child("buildings").first_child();
	while (building_node != NULL)
	{
		if (!ret)break;
		ret = LoadBuildingBlock(building_node.attribute("xml").as_string());
		tex_file = building_node.attribute("spritesheet").as_string();
		tex_folder = name + "/" + tex_file;
		textures.push_back(App->tex->Load(tex_folder.c_str()));
		building_node = building_node.next_sibling();
	}

	LOG("---- %s loaded in %.3f", folder, time.ReadSec());

	return ret;
}

bool j1Animator::LoadUnitBlock(const char* folder)
{
	//Load animations data from loaded folder
	LOG("Loading: %s", folder);
	char* buffer = nullptr;
	std::string load_folder = name + "/" + folder;
	int size = App->fs->Load(load_folder.c_str(), &buffer);
	pugi::xml_document animations_data;
	pugi::xml_parse_result result = animations_data.load_buffer(buffer, size);
	RELEASE(buffer);

	//Check result of the buffer loaded
	if (result == NULL)
	{
		LOG("Error loading %s data: %s",folder, result.description());
		return false;
	}

	//Load Animations data
	//Node focused at any unit node
	pugi::xml_node unit_node = animations_data.child("TextureAtlas").child("unit");
	std::string unit_enum;
	Animation_Block* unit_anim_block = nullptr;
	//Node focused at any unit action node
	pugi::xml_node action_node;
	std::string action_enum;
	Animation_Block* action_anim_block = nullptr;
	//Animation blocks to allocate the action different directions
	Animation_Block* dir_0_anim_block = nullptr;
	Animation_Block* dir_1_anim_block = nullptr;
	Animation_Block* dir_2_anim_block = nullptr;
	Animation_Block* dir_3_anim_block = nullptr;
	Animation_Block* dir_4_anim_block = nullptr;
	//Current sprite node 
	pugi::xml_node sprite;
	Animation* dir_0_anim = nullptr;
	Animation* dir_1_anim = nullptr;
	Animation* dir_2_anim = nullptr;
	Animation* dir_3_anim = nullptr;
	Animation* dir_4_anim = nullptr;

	//Build new unit animation block
	unit_anim_block = new Animation_Block();
	//Get unit enum
	unit_anim_block->SetId(Str_to_UnitEnum(unit_node.attribute("id").as_string()));

	//Iterate all unit action nodes
	action_node = unit_node.first_child();
	while (action_node != NULL)
	{
		//Build new action animation block
		action_anim_block = new Animation_Block();
		//Get current action enum
		action_anim_block->SetId(Str_to_ActionEnum(action_node.attribute("enum").as_string()));
		//Get current action animation speed
		uint speed = action_node.attribute("speed").as_uint();

		//Build new action direction animation blocks
		dir_0_anim_block = new Animation_Block(SOUTH);
		dir_1_anim_block = new Animation_Block(SOUTH_WEST);
		dir_2_anim_block = new Animation_Block(WEST);
		dir_3_anim_block = new Animation_Block(NORTH_WEST);
		dir_4_anim_block = new Animation_Block(NORTH);

		//Iterate all direction sprite nodes
		dir_0_anim = new Animation();
		dir_0_anim->SetSpeed(speed);
		dir_1_anim = new Animation();
		dir_1_anim->SetSpeed(speed);
		dir_2_anim = new Animation();
		dir_2_anim->SetSpeed(speed);
		dir_3_anim = new Animation();
		dir_3_anim->SetSpeed(speed);
		dir_4_anim = new Animation();
		dir_4_anim->SetSpeed(speed);
		sprite = action_node.first_child();

		while (sprite != NULL)
		{
			//Load sprite rect
			SDL_Rect rect = { sprite.attribute("x").as_int(),sprite.attribute("y").as_int(),sprite.attribute("w").as_int(),sprite.attribute("h").as_int() };
			//Load sprite pivot
			float pX = sprite.attribute("pX").as_float() * rect.w;
			pX = (pX > (floor(pX) + 0.5f)) ? ceil(pX) : floor(pX);
			float pY = sprite.attribute("pY").as_float() * rect.h;
			pY = (pY > (floor(pY) + 0.5f)) ? ceil(pY) : floor(pY);

			//Add sprite at correct animation
			switch (Str_to_DirectionEnum(sprite.attribute("direction").as_string()))
			{
			case SOUTH:			dir_0_anim->AddFrame(rect, iPoint(pX, pY));		break;
			case SOUTH_WEST:	dir_1_anim->AddFrame(rect, iPoint(pX, pY));		break;
			case WEST:			dir_2_anim->AddFrame(rect, iPoint(pX, pY));		break;
			case NORTH_WEST:	dir_3_anim->AddFrame(rect, iPoint(pX, pY));		break;
			case NORTH:			dir_4_anim->AddFrame(rect, iPoint(pX, pY));		break;
			}

			sprite = sprite.next_sibling();
		}
		//Add animations to direction blocks
		dir_0_anim_block->SetAnimation(dir_0_anim);
		dir_1_anim_block->SetAnimation(dir_1_anim);
		dir_2_anim_block->SetAnimation(dir_2_anim);
		dir_3_anim_block->SetAnimation(dir_3_anim);
		dir_4_anim_block->SetAnimation(dir_4_anim);
		//Add direction blocks to action block
		action_anim_block->AddAnimationBlock(dir_0_anim_block);
		action_anim_block->AddAnimationBlock(dir_1_anim_block);
		action_anim_block->AddAnimationBlock(dir_2_anim_block);
		action_anim_block->AddAnimationBlock(dir_3_anim_block);
		action_anim_block->AddAnimationBlock(dir_4_anim_block);

		//Add action block to unit block
		unit_anim_block->AddAnimationBlock(action_anim_block);

		action_node = action_node.next_sibling();
	}

	//Add unit animation block to module vector
	unit_blocks.push_back(unit_anim_block);

	//Release loaded document data
	animations_data.reset();
	return true;
}

bool j1Animator::LoadBuildingBlock(const char * folder)
{
	//Load animations data from loaded folder
	LOG("Loading: %s", folder);
	char* buffer = nullptr;
	std::string load_folder = name + "/" + folder;
	int size = App->fs->Load(load_folder.c_str(), &buffer);
	pugi::xml_document build_anim_data;
	pugi::xml_parse_result result = build_anim_data.load_buffer(buffer, size);
	RELEASE(buffer);

	//Check result of the buffer loaded
	if (result == NULL)
	{
		LOG("Error loading %s data: %s", folder, result.description());
		return false;
	}

	//Focus building id
	pugi::xml_node build_node = build_anim_data.child("TextureAtlas").child("building");
	
	//Alloc building animation block data 
	Animation_Block* building_block = new Animation_Block();
	building_block->SetId((uint)Str_to_BuildingEnum(build_node.attribute("id").as_string()));

	//Iterate all building actions
	pugi::xml_node action_node = build_node.first_child();
	while (action_node != NULL)
	{
		//Alloc action animation block data
		Animation_Block* action_block = new Animation_Block();
		action_block->SetId((uint)Str_to_ActionEnum(action_node.attribute("enum").as_string()));

		//Build animation
		Animation* anim = new Animation();
		anim->SetSpeed(action_node.attribute("speed").as_uint());

		//Iterate all action sprites
		pugi::xml_node sprite = action_node.first_child();
		while (sprite != NULL)
		{
			//Load sprite rect
			SDL_Rect rect = { sprite.attribute("x").as_int(),sprite.attribute("y").as_int(),sprite.attribute("w").as_int(),sprite.attribute("h").as_int() };
			//Load sprite pivot
			float pX = sprite.attribute("pX").as_float() * rect.w;
			pX = (pX > (floor(pX) + 0.5f)) ? ceil(pX) : floor(pX);
			float pY = sprite.attribute("pY").as_float() * rect.h;
			pY = (pY > (floor(pY) + 0.5f)) ? ceil(pY) : floor(pY);

			//Add sprite at animation
			anim->AddFrame(rect, iPoint(pX, pY));

			//Focus next animation sprite
			sprite = sprite.next_sibling();
		}

		//Set animation of action block
		action_block->SetAnimation(anim);

		//Add buided action to building animation block
		building_block->AddAnimationBlock(action_block);
		
		//Focus next action node
		action_node = action_node.next_sibling();
	}

	//Add loaded building animation to buildings vector
	building_blocks.push_back(building_block);

	//Release loaded document data
	build_anim_data.reset();

	return true;
}

SDL_Texture * j1Animator::GetTextureAt(uint index) const
{
	if (index >= textures.size())return nullptr;
	return textures.at(index);
}

Animation * j1Animator::UnitPlay(const UNIT_TYPE unit, const ACTION_TYPE action, const DIRECTION_TYPE direction) const
{
	Animation_Block* block = nullptr;

	//Iterate all blocks of childs vector
	uint size = unit_blocks.size();
	for (uint k = 0; k < size; k++)
	{
		//Pointer to the current block
		block = unit_blocks[k];

		//Compare block unit id
		if (block->GetId() == unit)
		{
			//Compare block action id
			block = block->SearchId(action);
			//If action block is found search the correct direction block
			if (block != nullptr)block = block->SearchId(direction);
			//If direction block is found returns the block animation
			if (block != nullptr)return block->GetAnimation();
		}
	}
	
	return nullptr;
}

Animation * j1Animator::BuildingPlay(const BUILDING_TYPE unit, const ACTION_TYPE action, const DIRECTION_TYPE direction) const
{
	Animation_Block* block = nullptr;

	//Iterate all blocks of childs vector
	uint size = building_blocks.size();
	for (uint k = 0; k < size; k++)
	{
		//Pointer to the current block
		block = building_blocks[k];

		//Compare block unit id
		if (block->GetId() == unit)
		{
			//Compare block action id
			block = block->SearchId(action);
			//If action block is found search the correct direction block or return undirectional action
			if (direction == NO_DIRECTION)return block->GetAnimation(); 
			if (block != nullptr)block = block->SearchId(direction);
			//If direction block is found returns the block animation
			if (block != nullptr)return block->GetAnimation();
		}
	}

	return nullptr;
}