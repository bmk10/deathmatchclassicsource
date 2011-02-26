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
extern ConVar sk_no_delay_time;

class CWeaponShotgundouble : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponShotgundouble, CBaseHL2MPCombatWeapon );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

private:
	CNetworkVar( bool,	m_bNeedPump );
	CNetworkVar( bool,  NeedDelay );

public:
	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone = VECTOR_CONE_10DEGREES;
		return cone;
	}

	virtual int				GetMinBurst() { return 1; }
	virtual int				GetMaxBurst() { return 3; }

	void Pump( void );
	void ItemHolsterFrame( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );
	void DryFire( void );
	bool Deploy( void );
	virtual float GetFireRate( void ) { return 0.1; };

	DECLARE_ACTTABLE();

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

acttable_t	CWeaponShotgundouble::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_SHOTGUN,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_SHOTGUN,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_SHOTGUN,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_SHOTGUN,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_SHOTGUN,					false },
};

IMPLEMENT_ACTTABLE(CWeaponShotgundouble);

void CWeaponShotgundouble::Pump( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if ( pOwner == NULL )
		return;

	m_bNeedPump = false;

	WeaponSound( SPECIAL1 );

	// Finish reload animation
	SendWeaponAnim( ACT_SHOTGUN_PUMP );

	pOwner->m_flNextAttack	= gpGlobals->curtime + 0.7f;
	m_flNextPrimaryAttack	= gpGlobals->curtime + 0.7f;
}
void CWeaponShotgundouble::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	Pump();
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

	FireBulletsInfo_t info( 14, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_pAttacker = pPlayer;

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( info );
	
	QAngle punch;
	punch.Init( SharedRandomFloat( "shotgunpax", -3, 3 ), SharedRandomFloat( "shotgunpay", -3, 3 ), 0 );
	pPlayer->ViewPunch( punch );

	m_bNeedPump = true;
}

bool CWeaponShotgundouble::Deploy( void )
{
	CHL2MP_Player *pPlayer = assert_cast<CHL2MP_Player*>( GetOwner() );
	DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_IDLE_LOWERED, (char*)GetAnimPrefix() );
	if (NeedDelay = false)
	{
		pPlayer->SetNextAttack( gpGlobals->curtime);
		m_flNextPrimaryAttack = gpGlobals->curtime;
	}
	else
	{
		pPlayer->SetNextAttack( gpGlobals->curtime + 0.4 );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.4;
	}
		return true;
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
	NeedDelay = false;

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

	// If it's been longer than one second, no delay
	if ( ( gpGlobals->curtime - m_flHolsterTime ) > 1 )
		NeedDelay = false;
	else
		NeedDelay = true;
}

void CWeaponShotgundouble::DryFire( void )
{
	WeaponSound(EMPTY);
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flNextPrimaryAttack = gpGlobals->curtime + 1;
}