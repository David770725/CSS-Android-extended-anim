//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h" 
#include "decals.h" 
#include "cbase.h" 
#include "shake.h" 
#include "weapon_csbase.h"
#include "fx_cs_shared.h"


#if defined( CLIENT_DLL )

	#define CDEagle C_DEagle
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif



#define DEAGLE_WEIGHT   7
#define DEAGLE_MAX_CLIP 7

enum deagle_e {
	DEAGLE_IDLE1 = 0,
	DEAGLE_SHOOT1,
	DEAGLE_SHOOT2,
	DEAGLE_SHOOT_EMPTY,
	DEAGLE_RELOAD,	
	DEAGLE_DRAW,
};



class CDEagle : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CDEagle, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CDEagle();

	void Spawn();

	void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();

	virtual bool Reload();
	void MakeBeam ();
	void BeamUpdate ();
	virtual bool UseDecrement() {return true;};

 	virtual float GetInaccuracy() const;

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_DEAGLE; }
#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

public:
	float m_flLastFire;

private:
	CDEagle( const CDEagle & );
};



IMPLEMENT_NETWORKCLASS_ALIASED( DEagle, DT_WeaponDEagle )

BEGIN_NETWORK_TABLE( CDEagle, DT_WeaponDEagle )
END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CDEagle )
	DEFINE_FIELD( m_flLastFire, FIELD_FLOAT ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_deagle, CDEagle );
PRECACHE_WEAPON_REGISTER( weapon_deagle );



CDEagle::CDEagle()
{
	m_flLastFire = gpGlobals->curtime;
}


void CDEagle::Spawn()
{
	BaseClass::Spawn();
	m_flAccuracy = 0.9;
}


float CDEagle::GetInaccuracy() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return 0.0f;

		if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
			return 1.5f * (1 - m_flAccuracy);

		else if (pPlayer->GetAbsVelocity().Length2D() > 5)
			return 0.25f * (1 - m_flAccuracy);

		else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
			return 0.115f * (1 - m_flAccuracy);

		else
			return 0.13f * (1 - m_flAccuracy);
	}
	else
		return BaseClass::GetInaccuracy();
}
void CDEagle::SecondaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
	{
		pPlayer->SetFOV( pPlayer, 80, 0.2f );
		m_weaponMode = Secondary_Mode;
mSight = TRUE;
SendWeaponAnim( ACT_IDLE_AIM_RELAXED );

	}
	else 
	{
		//FIXME: This seems wrong
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), 0.2f );
		m_weaponMode = Primary_Mode;

mSight = FALSE;
SendWeaponAnim( ACT_IDLE_AIM_STIMULATED );
	}

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}
void CDEagle::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;
		
	// Mark the time of this shot and determine the accuracy modifier based on the last shot fired...
	m_flAccuracy -= (0.35)*(0.4 - ( gpGlobals->curtime - m_flLastFire ) );

	if (m_flAccuracy > 0.9)
		m_flAccuracy = 0.9;
	else if (m_flAccuracy < 0.55)
		m_flAccuracy = 0.55;

	m_flLastFire = gpGlobals->curtime;

	if (m_iClip1 <= 0)
	{
		if ( m_bFireOnEmpty )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f;
			m_bFireOnEmpty = false;
		}

		return;
	}

	pPlayer->m_iShotsFired++;

	m_iClip1--;

	pPlayer->DoMuzzleFlash();

if(mSight)
	SendWeaponAnim( ACT_VM_PRIMARYATTACK_SPECIAL );
else
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	//SetPlayerShieldAnim();
	
	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	//pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	//pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->EyeAngles() + 2.0f * pPlayer->GetPunchAngle(),
		GetWeaponID(),
		Primary_Mode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		GetInaccuracy(),
		GetSpread());

	m_flNextPrimaryAttack = gpGlobals->curtime + GetCSWpnData().m_flCycleTime;

	if ( !m_iClip1 && pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	SetWeaponIdleTime( gpGlobals->curtime + 1.8 );

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[Primary_Mode];

	QAngle punchAngle = pPlayer->GetPunchAngle();
	punchAngle.x -= 2;
	pPlayer->SetPunchAngle( punchAngle );

	//ResetPlayerShieldAnim();
}


bool CDEagle::Reload()
{
	CCSPlayer *pPlayer = GetPlayerOwner();

	m_weaponMode = Primary_Mode;

m_bWeaponIsLowered = false;

	m_flAccuracy = 0.9;
int Anim;

if(pPlayer->GetFOV() == 80)
{
SecondaryAttack();
if(m_iClip1 == 0)
Anim = ACT_PRIMARY_RELOAD_FINISH;//empty
else if(m_iClip1 > 0)
Anim = ACT_PRIMARY_RELOAD_START;
}
else
{
if(m_iClip1 == 0)
Anim = ACT_VM_RELOAD_EMPTY;
else if(m_iClip1 > 0)
Anim = ACT_VM_RELOAD;
}

return DefaultReload( GetCSWpnData().iDefaultClip1, 0, Anim );
}


void CDEagle::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	// only idle if the slid isn't back
	//if (m_iClip1 != 0)
	//{	
		SetWeaponIdleTime( gpGlobals->curtime + 10.0 ) ;
	//	SendWeaponAnim( ACT_VM_IDLE );
	//}
}

