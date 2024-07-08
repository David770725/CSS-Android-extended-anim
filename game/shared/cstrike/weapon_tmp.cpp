//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponTMP C_WeaponTMP
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponTMP : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponTMP, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponTMP();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();

	virtual bool Reload();
	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_TMP; }
	virtual bool IsSilenced( void ) const				{ return true; }

 	virtual float GetInaccuracy() const;
#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

private:

	CWeaponTMP( const CWeaponTMP & );

	void DoFireEffects( void );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponTMP, DT_WeaponTMP )

BEGIN_NETWORK_TABLE( CWeaponTMP, DT_WeaponTMP )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponTMP )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_tmp, CWeaponTMP );
PRECACHE_WEAPON_REGISTER( weapon_tmp );


CWeaponTMP::CWeaponTMP()
{
}


float CWeaponTMP::GetInaccuracy() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return 0.0f;
	
		if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
			return 0.25f * m_flAccuracy;
		else
			return 0.03f * m_flAccuracy;
	}
	else
		return BaseClass::GetInaccuracy();
}

void CWeaponTMP::SecondaryAttack()
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

void CWeaponTMP::PrimaryAttack( void )
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

	if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack (1.1, 0.5, 0.35, 0.045, 4.5, 3.5, 6);
	else if (pPlayer->GetAbsVelocity().Length2D() > 5)
		pPlayer->KickBack (0.8, 0.4, 0.2, 0.03, 3, 2.5, 7);
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack (0.7, 0.35, 0.125, 0.025, 2.5, 2, 10);
	else
		pPlayer->KickBack (0.725, 0.375, 0.15, 0.025, 2.75, 2.25, 9);
}

bool CWeaponTMP::Reload()
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

void CWeaponTMP::WeaponIdle()
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
void CWeaponTMP::DoFireEffects( void )
{
	// TMP is silenced, so do nothing
}
