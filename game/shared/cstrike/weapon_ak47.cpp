//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )

	#define CAK47 C_AK47
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CAK47 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CAK47, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CAK47();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();

	virtual bool Reload();

	virtual float GetInaccuracy() const;

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_AK47; }
#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

private:
	CAK47( const CAK47 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( AK47, DT_WeaponAK47 )

BEGIN_NETWORK_TABLE( CAK47, DT_WeaponAK47 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CAK47 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_ak47, CAK47 );
PRECACHE_WEAPON_REGISTER( weapon_ak47 );

// ---------------------------------------------------------------------------- //
// CAK47 implementation.
// ---------------------------------------------------------------------------- //

CAK47::CAK47()
{
}


float CAK47::GetInaccuracy() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return 0.0f;

		if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
			return 0.04f + 0.4f * m_flAccuracy;
		else if (pPlayer->GetAbsVelocity().Length2D() > 140)
			return 0.04f + 0.07f * m_flAccuracy;
		else
			return 0.0275f * m_flAccuracy;
	}
	else
		return BaseClass::GetInaccuracy();
}

void CAK47::SecondaryAttack()
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

void CAK47::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime, Primary_Mode ) )
		return;

	// CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
	pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if(mSight)
	SendWeaponAnim( ACT_VM_PRIMARYATTACK_SPECIAL );
	else
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	if (pPlayer->GetAbsVelocity().Length2D() > 5 )
		pPlayer->KickBack ( 1.5, 0.45, 0.225, 0.05, 6.5, 2.5, 7 );
	else if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack ( 2, 1.0, 0.5, 0.35, 9, 6, 5 );
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack ( 0.9, 0.35, 0.15, 0.025, 5.5, 1.5, 9 );
	else
		pPlayer->KickBack ( 1, 0.375, 0.175, 0.0375, 5.75, 1.75, 8 );
}

bool CAK47::Reload()
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

void CAK47::WeaponIdle()
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
