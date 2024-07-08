//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponAug C_WeaponAug
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponAug : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponAug, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponAug();

	
	virtual void SecondaryAttack();
	virtual void WeaponIdle();
	virtual void PrimaryAttack();

 	virtual float GetInaccuracy() const;
	virtual bool Reload();

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_AUG; }

#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

private:

	void AUGFire( float flSpread, bool bZoomed );
	
	CWeaponAug( const CWeaponAug & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAug, DT_WeaponAug )

BEGIN_NETWORK_TABLE( CWeaponAug, DT_WeaponAug )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponAug )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_aug, CWeaponAug );
PRECACHE_WEAPON_REGISTER( weapon_aug );



CWeaponAug::CWeaponAug()
{
}

void CWeaponAug::SecondaryAttack()
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


float CWeaponAug::GetInaccuracy() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return 0.0f;
	
		if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
			return 0.035f + 0.4f * m_flAccuracy;
	
		else if ( pPlayer->GetAbsVelocity().Length2D() > 140 )
			return 0.035f + 0.07f * m_flAccuracy;
		else
			return 0.02f * m_flAccuracy;
	}
	else
		return BaseClass::GetInaccuracy();
}

void CWeaponAug::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	bool bZoomed = pPlayer->GetFOV() < pPlayer->GetDefaultFOV();

	float flCycleTime = GetCSWpnData().m_flCycleTime;


	if ( !CSBaseGunFire( flCycleTime, m_weaponMode ) )
		return;

	// CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
	pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if(mSight)
	SendWeaponAnim( ACT_VM_PRIMARYATTACK_SPECIAL );
	else
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	if ( pPlayer->GetAbsVelocity().Length2D() > 5 )
		 pPlayer->KickBack ( 1, 0.45, 0.275, 0.05, 4, 2.5, 7 );
	
	else if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack ( 1.25, 0.45, 0.22, 0.18, 5.5, 4, 5 );
	
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack ( 0.575, 0.325, 0.2, 0.011, 3.25, 2, 8 );
	
	else
		pPlayer->KickBack ( 0.625, 0.375, 0.25, 0.0125, 3.5, 2.25, 8 );
}


bool CWeaponAug::Reload()
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


void CWeaponAug::WeaponIdle()
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
