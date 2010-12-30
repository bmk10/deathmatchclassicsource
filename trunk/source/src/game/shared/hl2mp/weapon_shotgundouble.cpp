// Super Shotgun, take lolololol
// By Jethro

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CWeaponShotgundouble C_WeaponShotgundouble
#endif

extern ConVar sk_auto_reload_time;
extern ConVar sk_plr_num_shotgun_pellets;

class CWeaponShotgundouble : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponShotgundouble, CBaseHL2MPCombatWeapon );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

private:
	CNetworkVar( bool,	m_bNeedPump );

public:
	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone = VECTOR_CONE_15DEGREES;
		return cone;
	}

	virtual int				GetMinBurst() { return 1; }
	virtual int				GetMaxBurst() { return 3; }

	void Pump( void );
	void ItemHolsterFrame( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );
	void DryFire( void );
	virtual float GetFireRate( void ) { return 0.1; };

	CWeaponShotgundouble(void);

private:
	CWeaponShotgundouble( const CWeaponShotgundouble & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponShotgundouble, DT_WeaponShotgundouble )

BEGIN_NETWORK_TABLE( CWeaponShotgundouble, DT_WeaponShotgundouble )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponShotgundouble )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_shotgundouble, CWeaponShotgundouble );
PRECACHE_WEAPON_REGISTER(weapon_shotgundouble);

void CWeaponShotgundouble::Pump( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if ( pOwner == NULL )
		return;

	m_bNeedPump = false;

	WeaponSound( SPECIAL1 );

	// Finish reload animation
	SendWeaponAnim( ACT_SHOTGUN_PUMP );

	pOwner->m_flNextAttack	= gpGlobals->curtime + 1;
	m_flNextPrimaryAttack	= gpGlobals->curtime + 1;
}
void CWeaponShotgundouble::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	GetOwner()->RemoveAmmo( 2, GetPrimaryAmmoType() );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	ToHL2MPPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );


	Vector	vecSrc		= pPlayer->Weapon_ShootPosition( );
	Vector	vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );	

	FireBulletsInfo_t info( 21, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_pAttacker = pPlayer;

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( info );
	
	QAngle punch;
	punch.Init( SharedRandomFloat( "shotgunpax", -5, 5 ), SharedRandomFloat( "shotgunpay", -5, 5 ), 0 );
	pPlayer->ViewPunch( punch );

	m_bNeedPump = true;
}

void CWeaponShotgundouble::ItemPostFrame( void )
{
CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
	{
		return;
	}

	if ( pOwner->m_nButtons & IN_ATTACK )
	{
		if ( !HasPrimaryAmmo() )
		{
			DryFire();
			return;
		}
		else
		{
		PrimaryAttack();
		Pump();
		WeaponIdle();
		}
		
		return;
	}
	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2)))
	{
		// no fire buttons down
		if ( !ReloadOrSwitchWeapons()  )
		{
			WeaponIdle();
		}
	}

}

CWeaponShotgundouble::CWeaponShotgundouble( void )
{
	m_bNeedPump		= false;

	m_fMinRange1		= 0.0;
	m_fMaxRange1		= 500;
	m_fMinRange2		= 0.0;
	m_fMaxRange2		= 200;
}

void CWeaponShotgundouble::ItemHolsterFrame( void )
{
	// Must be player held
	if ( GetOwner() && GetOwner()->IsPlayer() == false )
		return;

	// We can't be active
	if ( GetOwner()->GetActiveWeapon() == this )
		return;
}

void CWeaponShotgundouble::DryFire( void )
{
	WeaponSound(EMPTY);
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flNextPrimaryAttack = gpGlobals->curtime + 1;
}