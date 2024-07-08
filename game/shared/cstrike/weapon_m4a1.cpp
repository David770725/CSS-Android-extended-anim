//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )

	#define CWeaponM4A1 C_WeaponM4A1
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponM4A1 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponM4A1, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	
	CWeaponM4A1();

	virtual void Spawn();
	virtual void Precache();

	virtual void PrimaryAttack();
	virtual void WeaponIdle();
	virtual void Drop( const Vector &vecVelocity );

	virtual void SecondaryAttack();
	virtual bool Reload();

 	virtual float GetInaccuracy() const;

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_M4A1; }

	// return true if this weapon has a silencer equipped
	virtual bool IsSilenced( void ) const				{ return true; }
#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

#ifdef CLIENT_DLL
	virtual int GetMuzzleFlashStyle( void );
#endif

	virtual const char		*GetWorldModel( void ) const;
	virtual int				GetWorldModelIndex( void );

private:

	CWeaponM4A1( const CWeaponM4A1 & );

	void DoFireEffects();

	CNetworkVar( bool, m_bSilencerOn );
	CNetworkVar( float, m_flDoneSwitchingSilencer );	// soonest time switching the silencer will be complete

	int m_silencedModelIndex;
	bool m_inPrecache;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM4A1, DT_WeaponM4A1 )

BEGIN_NETWORK_TABLE( CWeaponM4A1, DT_WeaponM4A1 )
	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bSilencerOn ) ),
		RecvPropTime( RECVINFO( m_flDoneSwitchingSilencer ) ),
	#else
		SendPropBool( SENDINFO( m_bSilencerOn ) ),
		SendPropTime( SENDINFO( m_flDoneSwitchingSilencer ) ),
	#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM4A1 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m4a1, CWeaponM4A1 );
PRECACHE_WEAPON_REGISTER( weapon_m4a1 );



CWeaponM4A1::CWeaponM4A1()
{
	m_bSilencerOn = true;
	m_flDoneSwitchingSilencer = 0.0f;
	m_inPrecache = false;
}


void CWeaponM4A1::Spawn( )
{
	BaseClass::Spawn();

	m_bSilencerOn = true;
	m_weaponMode = Primary_Mode;
	m_flDoneSwitchingSilencer = 0.0f;
	m_bDelayFire = true;
}


void CWeaponM4A1::Precache()
{
	m_inPrecache = true;
	BaseClass::Precache();

	m_silencedModelIndex = CBaseEntity::PrecacheModel( GetCSWpnData().m_szSilencerModel );
	m_inPrecache = false;
}


int CWeaponM4A1::GetWorldModelIndex( void )
{
	if ( !m_bSilencerOn || m_inPrecache )
	{
		return m_iWorldModelIndex;
	}
	else
	{
		return m_silencedModelIndex;
	}
}


const char * CWeaponM4A1::GetWorldModel( void ) const
{
	if ( !m_bSilencerOn || m_inPrecache )
	{
		return BaseClass::GetWorldModel();
	}
	else
	{
		return GetCSWpnData().m_szSilencerModel;
	}
}


void CWeaponM4A1::Drop( const Vector &vecVelocity )
{
	if ( gpGlobals->curtime < m_flDoneSwitchingSilencer )
	{
		// still switching the silencer.  Cancel the switch.
		m_bSilencerOn = !m_bSilencerOn;
		m_weaponMode = m_bSilencerOn ? Secondary_Mode : Primary_Mode;
		SetWeaponModelIndex( GetWorldModel() );
	}

	BaseClass::Drop( vecVelocity );
}

void CWeaponM4A1::SecondaryAttack()
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

float CWeaponM4A1::GetInaccuracy() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return 0.0f;

		if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		{
			return 0.035f + 0.4f * m_flAccuracy;
		}
		else if (pPlayer->GetAbsVelocity().Length2D() > 140)
		{
			return 0.035f + 0.07f * m_flAccuracy;
		}
		else
		{
			if ( m_bSilencerOn )
				return 0.025f * m_flAccuracy;
			else
				return 0.02f * m_flAccuracy;
		}
	}
	else
	{
		return BaseClass::GetInaccuracy();
	}
}


void CWeaponM4A1::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime, m_weaponMode ) )
		return;

	pPlayer = GetPlayerOwner();

	// CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
	if ( !pPlayer )
		return;

if(mSight)
	SendWeaponAnim( ACT_VM_PRIMARYATTACK_SPECIAL );
else
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	if (pPlayer->GetAbsVelocity().Length2D() > 5)
		pPlayer->KickBack (1.0, 0.45, 0.28, 0.045, 3.75, 3, 7);
	else if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack (1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack (0.6, 0.3, 0.2, 0.0125, 3.25, 2, 7);
	else
		pPlayer->KickBack (0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
}


void CWeaponM4A1::DoFireEffects()
{
	if ( !m_bSilencerOn )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( pPlayer )
		{
			pPlayer->DoMuzzleFlash();
		}
	}
}

bool CWeaponM4A1::Reload()
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


void CWeaponM4A1::WeaponIdle()
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


#ifdef CLIENT_DLL
int CWeaponM4A1::GetMuzzleFlashStyle( void )
{
	
		return CS_MUZZLEFLASH_NONE;

}
#endif
