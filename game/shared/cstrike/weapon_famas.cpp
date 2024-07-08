//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "fx_cs_shared.h"


#if defined( CLIENT_DLL )

	#define CWeaponFamas C_WeaponFamas
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponFamas : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponFamas, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponFamas();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();

	virtual bool Reload();

 	virtual float GetInaccuracy() const;

	void FamasFire( float flSpread, bool bFireBurst );

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_FAMAS; }
#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif


private:
	
	CWeaponFamas( const CWeaponFamas & );
	CNetworkVar( bool, m_bBurstMode );
	CNetworkVar( int, m_iBurstShotsRemaining );	
	float	m_fNextBurstShot;			// time to shoot the next bullet in burst fire mode
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponFamas, DT_WeaponFamas )

BEGIN_NETWORK_TABLE( CWeaponFamas, DT_WeaponFamas )
	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bBurstMode ) ),
		RecvPropInt( RECVINFO( m_iBurstShotsRemaining ) ),
	#else
		SendPropBool( SENDINFO( m_bBurstMode ) ),
		SendPropInt( SENDINFO( m_iBurstShotsRemaining ) ),
	#endif
END_NETWORK_TABLE()

#if defined(CLIENT_DLL)
BEGIN_PREDICTION_DATA( CWeaponFamas )
DEFINE_PRED_FIELD( m_iBurstShotsRemaining, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_fNextBurstShot, FIELD_FLOAT, 0 ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_famas, CWeaponFamas );
PRECACHE_WEAPON_REGISTER( weapon_famas );


const float kFamasBurstCycleTime = 0.075f;


CWeaponFamas::CWeaponFamas()
{
	m_bBurstMode = false;
}



// Secondary attack could be three-round burst mode
void CWeaponFamas::SecondaryAttack()
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

float CWeaponFamas::GetInaccuracy() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
	{
		float fAutoPenalty = m_bBurstMode ? 0.0f : 0.01f;

		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return 0.0f;
	
		if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )	// if player is in air
			return 0.03f + 0.3f * m_flAccuracy + fAutoPenalty;
	
		else if ( pPlayer->GetAbsVelocity().Length2D() > 140 )	// if player is moving
			return 0.03f + 0.07f * m_flAccuracy + fAutoPenalty;
		/* new code */
		else
			return 0.02f * m_flAccuracy + fAutoPenalty;
	}
	else
		return BaseClass::GetInaccuracy();
}


void CWeaponFamas::PrimaryAttack()
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

	pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	float flCycleTime = GetCSWpnData().m_flCycleTime;

	// change a few things if we're in burst mode
	if ( m_bBurstMode )
	{
		flCycleTime = 0.55f;
		m_iBurstShotsRemaining = 2;
		m_fNextBurstShot = gpGlobals->curtime + kFamasBurstCycleTime;
	}

	if ( !CSBaseGunFire( flCycleTime, m_weaponMode ) )
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

bool CWeaponFamas::Reload()
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

void CWeaponFamas::WeaponIdle()
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


