//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponMAC10 C_WeaponMAC10
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponMAC10 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMAC10, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMAC10();

	virtual void Spawn();
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();

	virtual bool Reload();

 	virtual float GetInaccuracy() const;

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_MAC10; }

#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

private:
	CWeaponMAC10( const CWeaponMAC10 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMAC10, DT_WeaponMAC10 )

BEGIN_NETWORK_TABLE( CWeaponMAC10, DT_WeaponMAC10 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMAC10 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mac10, CWeaponMAC10 );
PRECACHE_WEAPON_REGISTER( weapon_mac10 );



CWeaponMAC10::CWeaponMAC10()
{
}


void CWeaponMAC10::Spawn( )
{
	BaseClass::Spawn();

	m_flAccuracy = 0.15;
}


bool CWeaponMAC10::Reload()
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

float CWeaponMAC10::GetInaccuracy() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return 0.0f;

		if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
			return 0.375f * m_flAccuracy;
		else
			return 0.03f * m_flAccuracy;
	}
	else
		return BaseClass::GetInaccuracy();
}
void CWeaponMAC10::SecondaryAttack()
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
void CWeaponMAC10::PrimaryAttack()
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

	if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )	// jumping
		pPlayer->KickBack (1.3, 0.55, 0.4, 0.05, 4.75, 3.75, 5);
	else if (pPlayer->GetAbsVelocity().Length2D() > 5)				// running
		pPlayer->KickBack (0.9, 0.45, 0.25, 0.035, 3.5, 2.75, 7);
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )	// ducking
		pPlayer->KickBack (0.75, 0.4, 0.175, 0.03, 2.75, 2.5, 10);
	else														// standing
		pPlayer->KickBack (0.775, 0.425, 0.2, 0.03, 3, 2.75, 9);
}

void CWeaponMAC10::WeaponIdle()
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
