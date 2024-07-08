//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponGalil C_WeaponGalil
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponGalil : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponGalil, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponGalil();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();

	virtual bool Reload();

 	virtual float GetInaccuracy() const;

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_GALIL; }
#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif


private:

	CWeaponGalil( const CWeaponGalil & );

	void GalilFire( float flSpread );

};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGalil, DT_WeaponGalil )

BEGIN_NETWORK_TABLE( CWeaponGalil, DT_WeaponGalil )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGalil )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_galil, CWeaponGalil );
PRECACHE_WEAPON_REGISTER( weapon_galil );



CWeaponGalil::CWeaponGalil()
{
}

float CWeaponGalil::GetInaccuracy() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return 0.0f;
	
		if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
			return 0.04f + 0.3f * m_flAccuracy;
		else if (pPlayer->GetAbsVelocity().Length2D() > 140)
			return 0.04f + 0.07f * m_flAccuracy;
		else
			return 0.0375f * m_flAccuracy;
	}
	else
		return BaseClass::GetInaccuracy();
}

void CWeaponGalil::SecondaryAttack()
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


void CWeaponGalil::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// don't fire underwater
	if (pPlayer->GetWaterLevel() == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		return;
	}
	
	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime, Primary_Mode ) )
		return;

	// CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
	pPlayer = GetPlayerOwner();

if(mSight)
	SendWeaponAnim( ACT_VM_PRIMARYATTACK_SPECIAL );
else
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	if ( !pPlayer )
		return;

	if (pPlayer->GetAbsVelocity().Length2D() > 5)
		pPlayer->KickBack (1.0, 0.45, 0.28, 0.045, 3.75, 3, 7);
	else if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack (1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack (0.6, 0.3, 0.2, 0.0125, 3.25, 2, 7);
	else
		pPlayer->KickBack (0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
}

bool CWeaponGalil::Reload()
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

void CWeaponGalil::WeaponIdle()
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

