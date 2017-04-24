#ifndef _CHAMPIONS_
#define _CHAMPIONS_

#include "Units.h"
#include "j1BuffManager.h"

///Class Champion -------------------------------
//Base class that define the champions bases
class Champion : public Unit
{
public:

	Champion();
	Champion(const Champion& copy);
	~Champion();

protected:

	//Hero level
	uint	level = 2;
	bool	ability[3];
	bool	actived[3];
	bool	ability_lvl_2_prepare_mode = false;
	bool	ability_lvl_3_prepare_mode = false;

	//Buff area
	Circle			buff_area;
	PassiveBuff*	buff_to_apply = nullptr;

	//Attack area lvl 2
	j1Timer			ability_lvl_2_timer;
	uint			ability_lvl_2_cooldown = 0;
	//Attack area lvl 3
	j1Timer			ability_lvl_3_timer;
	uint			ability_lvl_3_cooldown = 0;
	//Stats bonus for level
	uint	attack_for_level = 0;
	uint	range_for_level = 0;
	float	defense_for_level = 0;
	float	armor_for_level = 0;
	float	speed_for_level = 0;
	uint	view_area_for_level = 0;

	//List of all units buffed by the hero
	std::list<Unit*>	buffed_units;

public:

	//Functionality ---------
	//Actions -----
	void		 CleanBuffedUnits();
	//Ability A methods
	virtual void SetAbility_lvl_1(bool choosed);
	virtual void Hability_lvl_1();
	virtual void CheckHability_lvl_1();
	//Ability B methods
	virtual void SetAbility_lvl_2(bool choosed);
	virtual void PrepareAbility_lvl_2();
	virtual void Hability_lvl_2(int x = 0, int y = 0);
	virtual void CheckHability_lvl_2();

	//Set Methods -
	void	SetPosition(float x, float y, bool insert = true);
	void	SetBuffArea(const Circle& area);
	void	SetBuffToApply(const PassiveBuff* buff);
	void	SetAbility_lvl_2_Cooldown(uint value);
	void	SetLevel(uint lvl);
	void	SetAttackForLevel(uint atk_for_lvl);
	void	SetRangeForLevel(uint rng_for_lvl);
	void	SetDefenseForLevel(float def_for_lvl);
	void	SetArmorForLevel(float arm_for_lvl);
	void	SetSpeedForLevel(float spd_for_lvl);
	void	SetViewAreaForLevel(uint view_for_level);

	//Get Methods -
	Circle			GetBuffArea()const;
	PassiveBuff*	GetBuffToApply()const;
	uint			GetLevel()const;
	uint			GetAttackForLevel()const;
	uint			GetRangeForLevel()const;
	float			GetDefenseForLevel()const;
	float			GetArmorForLevel()const;
	float			GetSpeedForLevel()const;
	uint			GetViewAreaForLevel()const;

};
/// ---------------------------------------------


/// Class Warrior -------------------------------
// Class that defines the warrior champion states
class Warrior : public Champion
{
public:

	Warrior();
	Warrior(const Warrior& copy);
	~Warrior();

private:

	/* extra data */
	std::vector<Unit*>	protected_units;

	j1Timer				taunt_timer;
	uint				taunt_max_time = 0;

	Triangle			special_attack_area;
	Circle				area_ability_lvl_3;

	Particle			ability_lvl_2_particle;
	Particle			ability_lvl_3_particle;

	uint				ability_lvl_2_attack_value = 0;
	uint				ability_lvl_2_stun_value = 0;
	uint				ability_lvl_3_attack_value = 0;

public:

	//Functionality ---------
	void ClearProtectedUnits();
	//Game Loop ---
	bool Update();
	//Draw --------
	bool	Draw(bool debug);
	//Actions -----
	void	SetAbility_lvl_1(bool choosed);
	void	Hability_lvl_1();
	void	CheckHability_lvl_1();
	void	SetAbility_lvl_2(bool choosed);
	void	PrepareAbility_lvl_2();
	void	Hability_lvl_2(int x = 0, int y = 0);
	void	CheckHability_lvl_2();

	void	SetAbility_lvl_3(bool choosed);
	void	PrepareAbility_lvl_3();
	void	Hability_lvl_3(int x = 0, int y = 0);
	void	CheckHability_lvl_3();
	iPoint	GetiPointFromDirection(DIRECTION_TYPE direction)const;
	void	CalculateSpecialAttackArea(const iPoint& base);
	bool	Die();
	//Set Methods -
	void	SetPosition(float x, float y, bool insert = true);
	void	SetSpecialAttackArea(const Triangle& tri);
	void	SetAbility_lvl_2_AttackValue(uint atk);
	void	SetAbility_lvl_2_StunValue(uint stun);


};
/// ---------------------------------------------

/// Class Warrior -------------------------------
// Class that defines the warrior champion states
class Wizard : public Champion
{
public:

	Wizard();
	Wizard(const Wizard& copy);
	~Wizard();

private:

	/* extra data */
	Circle				area_attack_spell_2;
	Circle				area_attack_spell_3;

	Circle				area_limit_spell_2;
	Circle				area_limit_spell_3;

	Particle			ability_lvl_2_particle;
	Particle			ability_lvl_3_particle;

	uint				ability_lvl_2_heal_value = 0;
	uint				ability_lvl_2_cooldown = 0;

	uint				ability_lvl_3_attack_value = 0;
	uint				ability_lvl_3_cooldown = 0;


public:

	//Functionality ---------
	//Game Loop ---
	bool Update();
	//Draw --------
	bool	Draw(bool debug);
	//Actions -----
	void	SetAbility_lvl_1(bool choosed);
	void	Hability_lvl_1();
	void	CheckHability_lvl_1();
	void	SetAbility_lvl_2(bool choosed);
	void	PrepareAbility_lvl_2();
	void	Hability_lvl_2(int x = 0, int y = 0);
	void	CheckHability_lvl_2();
	iPoint	GetiPointFromDirection(DIRECTION_TYPE direction)const;
	void	CalculateSpecialAttackArea(const iPoint& base,bool attack_lvl_2);
	bool	Die();
	//Set Methods -
	void	SetPosition(float x, float y, bool insert = true);
	void	SetSpecialAttackArea(const Circle& circle,const char* name);
	void	SetAbility_lvl_2_HealValue(uint heal);
	void	SetAbility_lvl_3_AttackValue(uint attack);


};
/// ---------------------------------------------
#endif // _CHAMPIONS_

