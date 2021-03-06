#ifndef __J1ACTION_MANAGER_H__
#define __J1ACTION_MANAGER_H__

#include "j1Module.h"
#include <queue>
#include "p2Point.h"
#include "j1Timer.h"

class Entity;
class Unit;
class Resource;
class Building;
class ProductiveBuilding;
class Villager;
class TeleportUnitAction;
class MoveUnitAction;
class AttackUnitAction;
class AttackBuildingAction;
class AttackResourceAction;
class DieUnitAction;
class DieBuildingAction;
class RecollectVillagerAction;
class SpawnUnitAction;
class ResearchTecAction;
class SaveResourcesVillagerAction;
class StunUnitAction;
class ScannAction;
class AutoAttackPassiveAction;
class AutoHealPassiveAction;
class HealUnitAction;

enum UNIT_TYPE;
enum BUILDING_TYPE;
enum RESOURCE_TYPE;
enum DIPLOMACY;
enum RESEARCH_TECH;


enum TASK_TYPE
{
	TASK_NONE = 0,
	TASK_U_MOVE,
	TASK_U_TELEPORT,
	TASK_U_ATTACK_U,
	TASK_U_ATTACK_B,
	TASK_U_HEAL_U,
	TASK_U_DIE,
	TASK_B_DIE,
	TASK_U_STUN,
	TASK_U_RECOLLECT,
	TASK_U_SAVE_RESOURCES,
	TASK_U_SCANN,
	TASK_U_AA,  //Auto attack
	TASK_U_AH,	//Auto heal
	TASK_B_SPAWN_UNITS,
	TASK_B_RESEARCH,
	TASK_U_HU //Heal Unit
};


enum TASK_CHANNELS
{
	NO_CHANNEL = 0,
	PRIMARY,
	SECONDARY,
	PASSIVE
};


/// Class Action --------------------------------
//Action virtual class (are commands but not for the console)
class Action
{
public:

	//Set all the Actions to a list on the Entities manager to clean them up at closing the app.
	Action(Entity* actor, TASK_TYPE type, TASK_CHANNELS channel = PRIMARY);
	virtual ~Action();

protected:

	Entity*					actor = nullptr;
	TASK_TYPE				type = TASK_NONE;

public:
	TASK_CHANNELS			task_channel = NO_CHANNEL;

public:

	//This function defines the action taking place
	//Returns false if Action was unable to initialize
	virtual bool Activation() { return true; }
	//Returns TRUE when execute is finished
	virtual bool Execute() { return true; }

	virtual void CleanUp() {}
	///Each different action inheriting from this class should have custom
	///properties to do its actions.

	//Get methods -----------
	TASK_TYPE GetType();

	//Set Methods -----------
	virtual void SetPausedTime(uint time);
	
};
/// ---------------------------------------------


///Action Worker---------------------------------
class ActionWorker
{
public:

	ActionWorker();
	~ActionWorker();

private:

	//Diferent action list
	std::list<Action*> primary_action_queue;
	std::list<Action*> secondary_action_queue;
	std::list<Action*> passive_action_queue;

	//Each current action
	Action* current_primary_action = nullptr;
	Action* current_secondary_action = nullptr;
	Action* current_passive_action = nullptr;

	//Refresh rate in ms		Default = 0
	j1Timer refresh_timer;
	uint	refresh_rate = 0;

	//Timer that counts the time that the action has been paused
	j1Timer pause_timer;
	uint	paused_time = 0;
	bool	paused = false;

public:

	//Updates every list
	void Update();

	void AddAction(Action* action, TASK_CHANNELS channel = TASK_CHANNELS::PRIMARY);
	void AddPriorizedAction(Action* action, TASK_CHANNELS channel = TASK_CHANNELS::PRIMARY);
	void PopAction(Action* action);

	//Clean all actions of the worker
	void HardReset();
	void ResetChannel(TASK_CHANNELS channel);

	//Get Methods -----------
	TASK_TYPE	GetCurrentActionType() const;

	Action*		GetCurrentPrimaryAction() const;

	bool IsBusy(TASK_CHANNELS channel);

	std::list<Action*> GetActionList(TASK_CHANNELS channel);
	std::list<Action*>* GetActionListPointer(TASK_CHANNELS channel);

	void Pause();
	void Restart();

	uint GetPausedTime()const;

private:

	///These are called by the public functions
	//Makes the Actions do their Execute and Activation
	//Returns true if the current action has been executed correctly
	bool DoWork(std::list<Action*>* queue, Action** current);
	void AddPriorized(Action* action, std::list<Action*>* queue, Action** current);

	//Resets a list and their current  action
	void ResetQueue(std::list<Action*>* queue, Action** current);
};
///----------------------------------------------

/// Action Manager ------------------------------
//Action Manager Module
class j1ActionManager : public j1Module
{
public:

	j1ActionManager();
	~j1ActionManager();


	void Init();
	// Called before quitting
	bool CleanUp();

public:

	//Action Calls --------------------
	//Move Functions
	MoveUnitAction*				MoveAction(Unit* actor, const iPoint& destination);
	MoveUnitAction*				MoveAction(std::vector<iPoint>* path, Unit* actor, Unit* unit_target =nullptr);
	TeleportUnitAction*			TeleportAction(Unit* actor, const iPoint& displacement);

	//Attack Functions
	AttackUnitAction*			AttackToUnitAction(Unit* actor, Unit* target);
	AttackBuildingAction*		AttackToBuildingAction(Unit* actor, Building* target, TASK_CHANNELS channel);
	DieUnitAction*				DieAction(Unit* actor);
	DieBuildingAction*			DieBuildngAction(Building* actor);
	StunUnitAction*				StunAction(Unit* actor, uint time);
	HealUnitAction*				HealAction(Unit* actor, Unit* target);

	//Recollect Functions
	RecollectVillagerAction*	RecollectAction(Villager* actor, Resource* target);
	SaveResourcesVillagerAction*SaveResourcesAction(Villager* actor, Building* target);

	// Building Functions
	SpawnUnitAction*			SpawnAction(ActionWorker* worker, ProductiveBuilding* actor, UNIT_TYPE type, DIPLOMACY diplomacy, uint runned_time = 0);
	ResearchTecAction*			ResearchAction(ActionWorker* worker, RESEARCH_TECH type, uint r_time, DIPLOMACY diplomacy);

	//Passive actions------------------
	//Scann for units
	AutoAttackPassiveAction*	AutoAttackAction(Entity* actor);
	AutoHealPassiveAction*		AutoHealAction(Entity* actor);


	//Set the passive actions----------
	void						SetUnitAutoPassive(Unit* actor);

	//Methods used in Load/Save -------
	bool	SaveTask(pugi::xml_node& node, Action* action);
	bool	LoadTask(pugi::xml_node& node, Entity* actor, TASK_CHANNELS channel);

private:

	//List of all action JUST for CLEANUP 
	std::list<Action*> all_actions;

};
/// ---------------------------------------------
#endif // __ACTION_MANAGER_H__

