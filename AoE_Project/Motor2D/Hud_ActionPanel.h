#ifndef HUD_ACTION_PANEL
#define HUD_ACTION_PANEL

#include "p2Defs.h"
#include "p2Point.h"
#include <vector>

#include "SDL\include\SDL_rect.h"


#include "Hud_SelectionPanel.h"

#define MAX_PANEL_CELLS 15
#define PANEL_COLUMNS	5
#define CELL_WIDTH		40
#define CELL_HEIGHT		41
#define MAX_SKILLS_LEARNED 6

//class Selection_Panel;
class Game_Panel;

class UI_Image;
class UI_Button;
class UI_Fixed_Button;

class Entity;

enum ENTITY_TYPE;
enum BUILDING_TYPE;
enum UNIT_TYPE;
enum GUI_INPUT;

// Base Action Panel ------------------------------------------------------------------------------------------

class Action_Panel_Elements
{
public:
	Action_Panel_Elements();
	~Action_Panel_Elements();
	
	// Reset Panel
	virtual void ResetPanel();

	virtual bool ActivateCell(int i) { return false; };

	void AddIcon(SDL_Rect icon_rect, uint position);
	virtual void ChangePanelIcons(std::vector<UI_Image*> & actual_panel);

	virtual void ChangePanelTarget(Entity* new_target) { entitis_panel = new_target; };
	Entity* GetActualEntity();

	virtual void ChangePlayerGamePanel(Game_Panel* game_panel);

	void LoadPanel() {};

protected:
	std::vector<SDL_Rect>		panel_icons;

	Entity*						entitis_panel = nullptr;
	
	Game_Panel*					player_game_panel_resources = nullptr;
};

// BUILDINGS ============================================================================================

// TownCenter ------------------------------------------------------------------------------------------

class TownCenterPanel : public Action_Panel_Elements
{
public:
	TownCenterPanel();
	~TownCenterPanel() {};

	// Reset Panel
	void ResetPanel();

	bool ActivateCell(int i);
	void ChampionIsDead(UNIT_TYPE type);

private:
	bool		got_melechmp = false;
};

// Barrack -----------------------------------------------------------------------------------------------

class BarrackPanel : public Action_Panel_Elements
{
public:
	BarrackPanel();
	~BarrackPanel() {};

	// Reset Panel
	void ResetPanel();

	bool ActivateCell(int i);

private:

};

// General Units -------------------------------------------------------------------------------------------

class UnitPanel : public Action_Panel_Elements
{
public:
	UnitPanel();
	~UnitPanel() {};

	// Reset Panel
	void ResetPanel();

	bool ActivateCell(int i);

private:

};

// Villager ----------------------------------------------------------------------------------------------

enum VILLAGER_IS_BULIDING
{
	VP_NOT_BUILDING,
	VP_NORMAL,
	VP_MILITARY
};

class VillagerPanel : public Action_Panel_Elements
{
public:
	VillagerPanel();
	~VillagerPanel() {};

	// Pre Update
	void PreUpdate();

	// Reset Panel
	void ResetPanel();

	bool ActivateCell(int i);

	void ChangePanelIcons(std::vector<UI_Image*> & actual_panel);
	bool Villager_Handle_input(GUI_INPUT input);

	bool GetIsBuilding()const;

private:
	VILLAGER_IS_BULIDING	villagerisbuilding = VP_NOT_BUILDING;

	std::vector<SDL_Rect>	v_normal_panel;
	std::vector<SDL_Rect>	v_militari_panel;

	bool					isbuilding = false;
	Entity*					buildingthis = nullptr;
};

// Hero Panel	-------------------------------------------------------------------------------------------

class HeroPanel : public Action_Panel_Elements
{
public:
	HeroPanel();
	~HeroPanel();

	// Reset Panel
	void ResetPanel();

	bool ActivateCell(int i);
	bool Hero_Handle_input(UI_Element* ui_element, GUI_INPUT ui_input);
	bool Handle_input(GUI_INPUT input);

	void LearnSkill(int i);
	void ChangePanelIcons(std::vector<UI_Image*> & actual_panel);
	void ChangePanelTarget(Entity* new_target);
	

public:
	UI_Image*						skill_tree = nullptr;
private:

	UNIT_TYPE						champion_selected;

	std::vector<UI_Image*>			skills;
	std::vector<UI_Fixed_Button*>	skills_buttons;

	// Champions Skills;
	std::vector<SDL_Rect>			mele_champion;
	int								mele_learned[3];
	int								activate_skill = -1;
};


// Action Panel --------------------------------------------------------------------------------------------
class Action_Panel
{
public:
	Action_Panel();
	~Action_Panel();

public:
	// Called before quitting
	bool CleanUp();

	// Disable/Enable Panels
	void Enable();
	void Disable();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool PostUpdate();

	// Handle Input
	void Handle_Input(GUI_INPUT newevent);

	// Draw Game Panel
	bool Draw();

	// Get Cell Clicked
	void ActivateCell(int i);

	// Get isin bool
	bool GetIsIn() const;

	// Handle_input_GUI
	void Handle_Input(UI_Element* ui_element, GUI_INPUT ui_input);

	// Return Hero Panel Skil Tree;
	UI_Element* GetHeroSkillTree() const;
	void HeroIsDead(UNIT_TYPE type);

	// Set Pointer To Selection Panel
	void SetSelectionPanelPointer(Selection_Panel* selection_panel);
	void GetEntitySelected();

	// Set Pointer To Game Panel
	void SetGamePanelPointer(Game_Panel* game_panel);

	// Panel Settings
	void SetPanelType();
	void SetButtons();
	void CheckSelected(int size);
	bool GetOnAction();

	// Return ActionPanel Screens
	UI_Element* GetActionScreen()const;

private:
	
	// HUD Panels Pointer
	Selection_Panel*			player_selection_panel = nullptr;
	Game_Panel*					player_game_panel = nullptr;

	// Action Panel Detection
	bool						isin;
	SDL_Point					mouse_pos;
	SDL_Rect					action_rect;
	UI_Element*					action_screen = nullptr;

	// Panell Settings 
	int							cell;
	iPoint						panel_pos;
	std::vector<UI_Image*>		panel_cells;
	std::vector<UI_Button*>		panel_buttons;
	Entity*						actual_entity = nullptr;

	// Entityes Panels
	Action_Panel_Elements*		actualpanel = nullptr;
	
	// Building Panels
	TownCenterPanel*			towncenterpanel = nullptr;
	BarrackPanel*				barrackpanel = nullptr;
	
	// Units Panels
	VillagerPanel*				villagerpanel = nullptr;
	UnitPanel*					unitpanel = nullptr;

	// Unit On Action   
	bool						on_action = false;


public:
	// Hero Panel
	HeroPanel*					heropanel = nullptr;


};
#endif // !HUD_ACTION_PANEL