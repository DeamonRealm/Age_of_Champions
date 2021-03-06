#include "j1Player.h"

#include "p2Log.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Input.h"
#include "j1InputManager.h"
#include "j1Animator.h"

#include "j1App.h"
#include "j1Window.h"
#include "j1Gui.h"
#include "j1EntitiesManager.h"

#include "UI_Element.h"
#include "UI_Image.h"
#include "UI_String.h"

#include "Actions_Unit.h"
#include "Actions_Building.h"

//Hud Elements
#include "Hud_SelectionPanel.h"
#include "Hud_GamePanel.h"
#include "Hud_ActionPanel.h"
#include "Hud_MinimapPanel.h"


#include "j1AI.h"

//j1Player Constructor ============================================================
j1Player::j1Player()
{
	name = "player";
}

//j1Player Destructor ============================================================
j1Player::~j1Player()
{
}

void j1Player::Init()
{
	active = false;
	enabled = false;
}

bool j1Player::Enable()
{
	// Setting Game Panel Resources
	game_panel->AddResource(500, GP_WOOD);
	game_panel->AddResource(500, GP_MEAT);
	game_panel->AddResource(500, GP_GOLD);
	game_panel->AddResource(500, GP_STONE);
	game_panel->IncressPopulation(5, true);

	selection_panel->Enable();
	game_panel->Enable();
	action_panel->Enable();
	minimap_panel->Enable();

	return true;
}

void j1Player::Disable()
{
	active = false;
	enabled = false;
	selection_panel->Disable();
	game_panel->Disable();
	action_panel->Disable();
	minimap_panel->Disable();
}

void j1Player::Reset()
{
	game_panel->Reset();
	selection_panel->Reset();
	action_panel->Reset();
	minimap_panel->Reset();
}

bool j1Player::Awake(pugi::xml_node& config)
{
	LOG("Loading Player");
	bool ret = true;


	return ret;
}

bool j1Player::Start()
{
	App->gui->SetDefaultInputTarget(this);
	// Hud Background
	game_hud = (UI_Image*) App->gui->GenerateUI_Element(IMG);
	game_hud->ChangeTextureId(HUD);
	game_hud->SetLayer(1);


	// HUD Panels
	game_panel = new Game_Panel();
	selection_panel = new Selection_Panel();
	action_panel = new Action_Panel();
	minimap_panel = new Minimap_Panel();

	action_panel->SetSelectionPanelPointer(selection_panel);
	action_panel->SetGamePanelPointer(game_panel);

	game_hud->AddChild(selection_panel->GetViewport());
	game_hud->AddChild(action_panel->GetHeroSkillTree());
	action_panel->GetHeroSkillTree()->SetLayer(5);
	game_hud->AddChild(action_panel->GetActionScreen());
	game_hud->AddChild(game_panel->GetExitMenu());
	game_hud->AddChild(minimap_panel->minimap_background);
	minimap_panel->minimap_background->SetLayer(20);
	App->gui->PushScreen(game_hud);

	return true;
}

bool j1Player::PreUpdate()
{
	App->gui->CalculateUpperElement(game_hud);
	if (App->paused)return true;
	
	int x, y;
	App->input->GetMousePosition(x, y);

	selection_panel->PreUpdate();
	action_panel->PreUpdate();
	game_panel->PreUpdate();
	minimap_panel->PreUpdate();

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		if (selection_panel->GetInViewport() && !action_panel->GetOnAction())
		{
			selection_panel->Handle_Input(MOUSE_LEFT_BUTTON_DOWN);
		}
		action_panel->Handle_Input(MOUSE_LEFT_BUTTON_DOWN);
	}
	
	else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		if (selection_panel->GetInViewport() && !action_panel->GetOnAction())
		{
			selection_panel->Handle_Input(MOUSE_RIGHT_BUTTON);
		}
		action_panel->Handle_Input(MOUSE_RIGHT_BUTTON);
	}

	//MAP MOVEMENT-----------------------------------------
	if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
	{
		selection_panel->WindowsMove(C_MOVE_UP);
	}

	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
	{
		selection_panel->WindowsMove(C_MOVE_DOWN);
	}

	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
	{
		selection_panel->WindowsMove(C_MOVE_LEFT);
	}

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
	{
		selection_panel->WindowsMove(C_MOVE_RIGHT);
	}

	// Debug Keys To Produce Entities in Game
	if (App->debug_mode)
	{
		//Generate a town center in the mouse coordinates
		if (App->input->GetKey(SDL_SCANCODE_F4) == KEY_DOWN)
		{
			Building* center = App->entities_manager->GenerateBuilding(BUILDING_TYPE::TOWN_CENTER, ALLY);
			center->SetPosition((float)x - App->render->camera.x, (float)y - App->render->camera.y);
			center->SetDiplomacy(ALLY);

			game_panel->IncressPopulation(15, true);
		}
		//Generate Villager in the mouse coordinates
		if (App->input_manager->GetEvent(GENERATE_VILLAGER) == INPUT_DOWN)
		{
			Unit* new_unit = App->entities_manager->GenerateUnit(VILLAGER, ALLY);
			new_unit->SetPosition((float)x - App->render->camera.x,(float) y - App->render->camera.y);

			game_panel->IncressPopulation(1, false);
		}
		//Generate a God unit in the mouse coordinates
		if (App->input_manager->GetEvent(GENERATE_GOD) == INPUT_DOWN)
		{
			Unit* new_unit = App->entities_manager->GenerateUnit(GOD_UNIT, ALLY);
			new_unit->SetPosition((float)x - App->render->camera.x, (float)y - App->render->camera.y);
		}
		//Generate Tree resource in mouse coordinates
		if (App->input_manager->GetEvent(GENERATE_TREE) == INPUT_DOWN)
		{
			Entity* new_unit = App->entities_manager->GenerateResource(TREE);
			new_unit->SetPosition((float)x - App->render->camera.x, (float)y - App->render->camera.y);
		}
		//Generate BerryBush resource in mouse coordinates
		if (App->input_manager->GetEvent(GENERATE_BUSH) == INPUT_DOWN)
		{
			Entity* new_unit = App->entities_manager->GenerateResource(BERRY_BUSH);
			new_unit->SetPosition((float)x - App->render->camera.x, (float)y - App->render->camera.y);
		}
		//Generate Stone Ore resource in mouse coordinates
		if (App->input_manager->GetEvent(GENERATE_STONE) == INPUT_DOWN)
		{
			Entity* new_unit = App->entities_manager->GenerateResource(STONE_ORE);
			new_unit->SetPosition((float)x - App->render->camera.x, (float)y - App->render->camera.y);
		}
		//Generate Gold Ore resource in mouse coordinates
		if (App->input_manager->GetEvent(GENERATE_GOLD) == INPUT_DOWN)
		{
			Entity* new_unit = App->entities_manager->GenerateResource(GOLD_ORE);
			new_unit->SetPosition((float)x - App->render->camera.x, (float)y - App->render->camera.y);
		}
		//Add Player resources
		if (App->input_manager->GetEvent(ADD_RESOURCES) == INPUT_DOWN)
		{
			game_panel->AddResource(200, PLAYER_RESOURCES::GP_GOLD);
			game_panel->AddResource(200, PLAYER_RESOURCES::GP_MEAT);
			game_panel->AddResource(200, PLAYER_RESOURCES::GP_STONE);
			game_panel->AddResource(200, PLAYER_RESOURCES::GP_WOOD);
		}
		//Generate Warrior unit in mouse coordinates
		if (App->input_manager->GetEvent(GENERATE_WARRIOR) == INPUT_DOWN)
		{
			Unit* new_unit = App->entities_manager->GenerateUnit(WARRIOR_CHMP, ALLY);
			((Champion*)new_unit)->LevelUp(false);
			((Champion*)new_unit)->LevelUp(false);
			new_unit->SetPosition((float)x - App->render->camera.x, (float)y - App->render->camera.y);
		}
		//Generate Wizard unit in mouse coordinates
		if (App->input_manager->GetEvent(GENERATE_WIZARD) == INPUT_DOWN)
		{
			Unit* new_unit = App->entities_manager->GenerateUnit(WIZARD_CHMP, ALLY);
			((Champion*)new_unit)->LevelUp(false);
			((Champion*)new_unit)->LevelUp(false);
			new_unit->SetPosition((float)x - App->render->camera.x, (float)y - App->render->camera.y);
		}
		//Generate Archer unit in mouse coordinates
		if (App->input_manager->GetEvent(GENERATE_HUNTER) == INPUT_DOWN)
		{
			Unit* new_unit = App->entities_manager->GenerateUnit(ARCHER_CHMP, ALLY);
			((Champion*)new_unit)->LevelUp(false);
			((Champion*)new_unit)->LevelUp(false);
			new_unit->SetPosition((float)x - App->render->camera.x, (float)y - App->render->camera.y);
		}

		if (App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		{
			App->AI->GenerateDebugVillager();
		}

		/*if (App->input->GetKey(SDL_SCANCODE_L) == KEY_REPEAT)
		{
			Circle del_area;
			del_area.SetPosition(iPoint(x - App->render->camera.x, y - App->render->camera.y));
			del_area.SetRad(35);
			std::vector<Resource*> coll_res;
			uint size = App->entities_manager->resources_quadtree.CollectCandidates(coll_res, del_area);
			for (uint k = 0; k < size; k++)
			{
				if (coll_res[k]->GetResourceType() == TREE)
				{
					App->entities_manager->DeleteEntity(coll_res[k]);
					App->entities_manager->resources_quadtree.Exteract(&coll_res[k]->GetPosition());
					App->entities_manager->resources.remove(coll_res[k]);
					coll_res[k]->CleanMapLogic();
				}
			}
			
		}*/
	}

	// Skills
	if (App->input_manager->GetEvent(SET_HABILITY_A) == INPUT_DOWN)
	{
		action_panel->ActivateSkill(0);
	}
	if (App->input_manager->GetEvent(SET_HABILITY_B) == INPUT_DOWN)
	{
		action_panel->ActivateSkill(1);
	}
	if (App->input_manager->GetEvent(SET_HABILITY_C) == INPUT_DOWN)
	{
		action_panel->ActivateSkill(2);
	}

	// Villager Buildings Shortcuts
	if (App->input_manager->GetEvent(GENERATE_HOUSE) == INPUT_DOWN)
	{
		action_panel->BuildShortcut(0, 1);
	}
	else if (App->input_manager->GetEvent(GENERATE_MINING_CAMP) == INPUT_DOWN)
	{
		action_panel->BuildShortcut(2, 1);
	}
	else if (App->input_manager->GetEvent(GENERATE_LUMBER_CAMP) == INPUT_DOWN)
	{
		action_panel->BuildShortcut(3, 1);
	}
	else if (App->input_manager->GetEvent(GENERATE_BLACKSMITH) == INPUT_DOWN)
	{
	action_panel->BuildShortcut(6, 1);
	}
	else if (App->input_manager->GetEvent(GENERATE_UNIVERSITY) == INPUT_DOWN)
	{
		action_panel->BuildShortcut(8, 1);
	}
	else if (App->input_manager->GetEvent(GENERATE_MONASTERY) == INPUT_DOWN)
	{
		action_panel->BuildShortcut(9, 1);
	}
	else if (App->input_manager->GetEvent(GENERATE_TOWN_CENTER) == INPUT_DOWN)
	{
		action_panel->BuildShortcut(10, 1);
	}
	else if (App->input_manager->GetEvent(GENERATE_BARRACK) == INPUT_DOWN)
	{
		action_panel->BuildShortcut(0, 2);
	}
	else if (App->input_manager->GetEvent(GENERATE_ARCHERY_RANGE) == INPUT_DOWN)
	{
		action_panel->BuildShortcut(1, 2);
	}
	else if (App->input_manager->GetEvent(GENERATE_STABLE) == INPUT_DOWN)
	{
		action_panel->BuildShortcut(2, 2);
	}

	
	return true;
}

bool j1Player::PostUpdate()
{
	if (App->input_manager->GetEvent(ESCAPE) == INPUT_DOWN)
	{
		game_panel->ActivatePauseMenu();
	}

	// Draw Selected Entities Life
	selection_panel->DrawLife();

	game_hud->Draw(false);

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
	{
		if(!action_panel->GetOnAction()) selection_panel->Handle_Input(MOUSE_LEFT_BUTTON_REPEAT);
		action_panel->Handle_Input(MOUSE_LEFT_BUTTON_REPEAT);
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
	{
		
		if(!action_panel->GetOnAction()) selection_panel->Handle_Input(MOUSE_LEFT_BUTTON_UP);
		action_panel->Handle_Input(MOUSE_LEFT_BUTTON_UP);
		action_panel->SetPanelType();
	}
	
	// Draw Action Panel
	action_panel->Draw();

	// Draw Minimap
	minimap_panel->Draw();

	// Draw Selected Units
	selection_panel->Draw();

	// Draw Game Panel (HUD)
	game_panel->Draw();

	//Draw Mouse Last one
	if (SDL_ShowCursor(-1) == 0) App->gui->DrawMouseTexture();

	return true;
}

bool j1Player::CleanUp()
{
	//Delete HUD
	delete game_panel;
	delete selection_panel;
	delete action_panel;
	delete minimap_panel;

	return true;
}

bool j1Player::Load(pugi::xml_node & data)
{
	// Player Load ---------------------------
	//Player node
	pugi::xml_node player_node = data.child("player_data");

	if (player_node == NULL) return true;

	game_panel->Load(player_node);
	selection_panel->Load(player_node);
	action_panel->Load(player_node);
	minimap_panel->Load(player_node);

	return true;
}

bool j1Player::Save(pugi::xml_node &data) const
{
	// Player Save ---------------------------
	//Player node
	pugi::xml_node player_node = data.append_child("player_data");

	game_panel->Save(player_node);
	selection_panel->Save(player_node);
	action_panel->Save(player_node);
	minimap_panel->Save(player_node);

	return true;
}

void j1Player::GUI_Input(UI_Element* target, GUI_INPUT input)
{
	game_panel->Handle_Input(target, input);

	switch (input)
	{
	case UP_ARROW:
		break;
	case DOWN_ARROW:
		break;
	case LEFT_ARROW:
		break;
	case RIGHT_ARROW:
		break;
	case MOUSE_LEFT_BUTTON_DOWN:
		{
			minimap_panel->Handle_Input(target, input);
			action_panel->Handle_Input(target, input);
		}
		break;
	case MOUSE_LEFT_BUTTON_REPEAT:
		break;
	case MOUSE_LEFT_BUTTON_UP:
		{
			minimap_panel->Handle_Input(target, input);
			action_panel->Handle_Input(target, input);
		}
		break;
	case MOUSE_RIGHT_BUTTON:
		{
			minimap_panel->Handle_Input(target, input);
		}
		break;
	case MOUSE_IN:
		{
			if (App->gui->upper_element == selection_panel->GetViewport()->GetLayer() && selection_panel->GetInViewport() != true)
			{
				selection_panel->Handle_Input(target, input);
			}
			action_panel->Handle_Input(target, input);
			minimap_panel->Handle_Input(target, input);
		}
		break;
	case MOUSE_OUT: 
		{
			if (App->gui->upper_element != selection_panel->GetViewport()->GetLayer() && selection_panel->GetInViewport() == true)
			{
				selection_panel->Handle_Input(selection_panel->GetViewport(), input);
			}
			action_panel->Handle_Input(target, input);
			minimap_panel->Handle_Input(target, input);
		}
	case SUPR:
		break;
	case BACKSPACE:
		break;
	case ENTER:
		break;
	case TAB:
		break;
	default:
		break;
	}
}

void j1Player::UpgradeCivilization(RESEARCH_TECH type) 
{
	switch (type)
	{
	case NO_TECH:
		break;
	case TC_FEUDAL: {
		action_panel->UpgradeCivilizationAge(2);
		action_panel->UpgradeTecnology(type);
		return;
		}
		break;
	case TC_CASTLE:	{
		action_panel->UpgradeCivilizationAge(3);
		action_panel->UpgradeTecnology(type);
		return;
		}
		break;
	case TC_IMPERIAL: {
		action_panel->UpgradeCivilizationAge(4);
		action_panel->UpgradeTecnology(type);
		return;
		}
		break;
	default: action_panel->UpgradeTecnology(type);
		break;
	}
	selection_panel->UpdateSelected();
}