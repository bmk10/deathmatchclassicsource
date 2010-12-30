#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "beam_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#define CWeapon_Shaft C_Weapon_Shaft
#endif

#define BEAM_DAMAGE		1.3f
#define DAMAGE_TICK		0.1f

class CWeapon_Shaft : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeapon_Shaft, CBaseHL2MPCombatWeapon )
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:

	CWeapon_Shaft()
	{
		m_flBeamTime   = FLT_MAX;
		m_flDamageTime = FLT_MAX;
	}

#ifndef CLIENT_DLL
	void Precache()
	{
		UTIL_PrecacheOther( "env_beam" ); 
		BaseClass::Precache();
	}
#endif

	bool Holster( CBaseCombatWeapon* pSwitchingTo )
	{
		if( BaseClass::Holster(pSwitchingTo) )
		{
			CancelPrimaryAttack();
			return true;
		}

		return false;
	}

	void ItemPostFrame()
	{
		CBasePlayer *pPlayer = dynamic_cast<CBasePlayer*>(GetOwner());
		if( !pPlayer )
			return;

		if ( pPlayer->m_afButtonReleased & IN_ATTACK )
		{
			CancelPrimaryAttack();
		}
					
		trace_t tr;
		Vector eyePos, eyeForward;
		pPlayer->EyePositionAndVectors( &eyePos, &eyeForward, NULL, NULL );

		CTraceFilterSkipTwoEntities traceFilter( pPlayer, this, COLLISION_GROUP_NONE );
		UTIL_TraceLine( eyePos, eyePos + eyeForward * MAX_TRACE_LENGTH, MASK_SHOT, &traceFilter, &tr );

		if( !m_hBeam )
		{
			if ( gpGlobals->curtime >= m_flBeamTime )
			{
#ifndef CLIENT_DLL
				m_hBeam = CREATE_ENTITY( CBeam, "env_beam" );

				if ( m_hBeam )
				{
					m_hBeam->BeamInit( "sprites/lgtning.vmt", 6.5f );
					m_hBeam->PointEntInit( tr.endpos, pPlayer->GetViewModel(0) );
					m_hBeam->SetScrollRate( -10.f );
					m_hBeam->SetNoise( 1 );
					m_hBeam->SetEndAttachment( LookupAttachment("muzzle") );
					m_hBeam->Spawn();
				}
#endif
				SendWeaponAnim( ACT_VM_PRIMARYATTACK );
			}
			
			if ( m_flBeamTime == FLT_MAX )
				BaseClass::ItemPostFrame();
		}
		else
		{
			if ( gpGlobals->curtime >= m_flDamageTime )
			{
				GetOwner()->RemoveAmmo( 1, GetPrimaryAmmoType() );
#ifndef CLIENT_DLL					
				if( tr.fraction != 1.0 && tr.m_pEnt )
				{
					ClearMultiDamage();
					Vector dir = tr.endpos - m_hBeam->GetAbsEndPos();
					VectorNormalize( dir );
	
					const float flDamage = BEAM_DAMAGE;

					CTakeDamageInfo info( m_hBeam, GetOwner(), flDamage, DMG_SHOCK );
					CalculateMeleeDamageForce( &info, dir, tr.endpos );
					tr.m_pEnt->DispatchTraceAttack( info, dir, &tr );
					ApplyMultiDamage();

					RadiusDamage( CTakeDamageInfo( m_hBeam, GetOwner(), flDamage * 0.25f, DMG_SHOCK ), tr.endpos, 16.0f, CLASS_NONE, NULL );
				}
#endif

				if ( !HasPrimaryAmmo() )
					CancelPrimaryAttack();

				m_flDamageTime = gpGlobals->curtime + DAMAGE_TICK;
			}

			m_hBeam->SetStartPos( tr.endpos );
		}
	}

	void PrimaryAttack()
	{
		 m_flNextPrimaryAttack = gpGlobals->curtime +  FLT_MAX;
		SetWeaponIdleTime( FLT_MAX );

		m_flBeamTime = gpGlobals->curtime + 0.2f;
		m_flDamageTime = gpGlobals->curtime + 0.2f;
	}

private:

	void CancelPrimaryAttack()
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.25f;
		SetWeaponIdleTime( gpGlobals->curtime );

#ifndef CLIENT_DLL
		if ( m_hBeam )
		{
			UTIL_Remove( m_hBeam );
		}
#endif
		m_flBeamTime = FLT_MAX;
	}

	CNetworkHandle( CBeam, m_hBeam );

	CNetworkVar( float, m_flBeamTime );
	CNetworkVar( float, m_flDamageTime );
};

LINK_ENTITY_TO_CLASS( weapon_shaft, CWeapon_Shaft );

IMPLEMENT_NETWORKCLASS_ALIASED( Weapon_Shaft, DT_Weapon_Shaft );
PRECACHE_WEAPON_REGISTER( weapon_shaft );

BEGIN_DATADESC( CWeapon_Shaft )
	DEFINE_FIELD( m_hBeam, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flBeamTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_flDamageTime, FIELD_FLOAT ),
END_DATADESC()

BEGIN_NETWORK_TABLE( CWeapon_Shaft, DT_Weapon_Shaft )
#ifndef CLIENT_DLL
	SendPropEHandle( SENDINFO(m_hBeam) ),
	SendPropFloat( SENDINFO(m_flBeamTime) ),
	SendPropFloat( SENDINFO(m_flDamageTime) ),
#else
	RecvPropEHandle( RECVINFO(m_hBeam) ),
	RecvPropFloat( RECVINFO(m_flBeamTime) ),
	RecvPropFloat( RECVINFO(m_flDamageTime) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeapon_Shaft )
END_PREDICTION_DATA()