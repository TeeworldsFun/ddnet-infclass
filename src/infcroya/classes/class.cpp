#include "class.h"
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <infcroya/croyaplayer.h>
#include <game/server/player.h>
#include <game/server/entities/projectile.h>
#include <game/generated/server_data.h>
#include <game/generated/protocol7.h>

IClass::~IClass()
{
}

void IClass::Tick(CCharacter* pChr)
{
	ItAntigravitates(pChr);
	ItInfiniteJumpsWhenSlowmo(pChr);
}

void IClass::GunShoot(CCharacter* pChr, vec2 ProjStartPos, vec2 Direction) {
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameContext* pGameServer = pChr->GameServer();
	CGameWorld* pGameWorld = pChr->GameWorld();

	new CProjectile(pGameWorld, WEAPON_GUN,
		ClientID,
		ProjStartPos,
		Direction,
		(int)(pChr->Server()->TickSpeed() * pGameServer->Tuning()->m_GunLifetime),
		g_pData->m_Weapons.m_Gun.m_pBase->m_Damage, false, 0, -1, WEAPON_GUN);

	pGameServer->CreateSound(pChr->GetPos(), SOUND_GUN_FIRE);
}

void IClass::ShotgunShoot(CCharacter* pChr, vec2 ProjStartPos, vec2 Direction) {
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameContext* pGameServer = pChr->GameServer();
	CGameWorld* pGameWorld = pChr->GameWorld();

	int ShotSpread = 3;
	float Force = 3.0f;

	for (int i = -ShotSpread; i <= ShotSpread; ++i)
	{
		//float Spreading[] = { -0.185f, -0.070f, 0, 0.070f, 0.185f, 0.1f, -0.1f};
		float Spreading[] = {
			 -0.140f,
			 0.140f,
			 0,
			 0.365f,
			 -0.365f,
			 0.2f,
			 -0.2f};
		float a = angle(Direction);
		a += Spreading[i + 3] * 2.0f * (0.25f + 0.75f * static_cast<float>(10 - pChr->m_aWeapons[WEAPON_SHOTGUN].m_Ammo) / 10.0f);
		float v = 1 - (absolute(i) / (float)ShotSpread);
		float Speed = mix((float)pGameServer->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
		float LifeTime = pGameServer->Tuning()->m_ShotgunLifetime + 0.1f * static_cast<float>(pChr->m_aWeapons[WEAPON_SHOTGUN].m_Ammo) / 10.0f;
		new CProjectile(pGameWorld, WEAPON_SHOTGUN,
			ClientID,
			ProjStartPos,
			vec2(cosf(a), sinf(a)) * Speed,
			(int)(pChr->Server()->TickSpeed() * LifeTime),
			g_pData->m_Weapons.m_Shotgun.m_pBase->m_Damage, false, Force, -1, WEAPON_SHOTGUN);
	}

	pGameServer->CreateSound(pChr->GetPos(), SOUND_SHOTGUN_FIRE);
}

void IClass::GrenadeShoot(CCharacter* pChr, vec2 ProjStartPos, vec2 Direction) {
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameContext* pGameServer = pChr->GameServer();
	CGameWorld* pGameWorld = pChr->GameWorld();

	new CProjectile(pGameWorld, WEAPON_GRENADE,
		ClientID,
		ProjStartPos,
		Direction,
		(int)(pChr->Server()->TickSpeed() * pGameServer->Tuning()->m_GrenadeLifetime),
		g_pData->m_Weapons.m_Grenade.m_pBase->m_Damage, true, 0, SOUND_GRENADE_EXPLODE, WEAPON_GRENADE);

		pGameServer->CreateSound(pChr->GetPos(), SOUND_GRENADE_FIRE);
}

bool IClass::WillItFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr) {
	return true;
}

void IClass::UnlockPosition(CCharacter* pChr) {
}

void IClass::ItDoubleJumps(CCharacter* pChr) {
	//Double jumps
	CroyaPlayer* cp = pChr->GetCroyaPlayer();
	if (pChr->IsGrounded()) cp->SetAirJumpCounter(0);
	if (pChr->GetCharacterCore().m_TriggeredEvents & protocol7::COREEVENTFLAG_AIR_JUMP && cp->GetAirJumpCounter() < 1)
	{
		pChr->GetCharacterCore().m_Jumped &= ~2;
		cp->SetAirJumpCounter(cp->GetAirJumpCounter() + 1);
	}
}

void IClass::ItSelfAntigravitates(CCharacter* pChr) {
	//Double jumps
	CroyaPlayer* cp = pChr->GetCroyaPlayer();
	if (pChr->IsGrounded()) cp->SetAirJumpCounter(0);
	if (pChr->GetCharacterCore().m_TriggeredEvents & protocol7::COREEVENTFLAG_AIR_JUMP && cp->GetAirJumpCounter() < 2)
	{
		pChr->GetCharacterCore().m_Jumped &= ~2;
		if (!cp->IsAntigravityOn() && cp->GetAirJumpCounter() == 1) {
			cp->AntigravityOn(false, 10);
			return;
		}
		if (cp->IsAntigravityOn()) { 
			cp->AntigravityOff();
		}
		cp->SetAirJumpCounter(cp->GetAirJumpCounter() + 1);
	}
}


void IClass::HammerShoot(CCharacter* pChr, vec2 ProjStartPos) {
	int ClientID = pChr->GetPlayer()->GetCID();
	CGameContext* pGameServer = pChr->GameServer();

	// reset objects Hit
	pChr->SetNumObjectsHit(0);
	pChr->RegainBotControl();
	CCharacterCore *CCore = &pChr->GetCharacterCore();
	// bot control
	if (!CCore->m_IsBot) {
		CCore->m_BotInput.m_Direction = CCore->m_Input.m_Direction;
		CCore->m_BotInput.m_TargetX = CCore->m_Input.m_TargetX;
		CCore->m_BotInput.m_TargetY = CCore->m_Input.m_TargetY;
	}
	if (!CCore->m_IsBot)
		pGameServer->CreateSound(pChr->GetPos(), SOUND_HAMMER_FIRE);

	CCharacter *apEnts[MAX_CLIENTS];
	int Hits = 0;
	int Num = pChr->GameServer()->m_World.FindEntities(ProjStartPos, pChr->GetProximityRadius() * 0.5f, (CEntity **)apEnts,
													   MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

	for (int i = 0; i < Num; ++i)
	{
		CCharacter *pTarget = apEnts[i];

		if ((pTarget == pChr) || pGameServer->Collision()->IntersectLine(ProjStartPos, pTarget->GetPos(), NULL, NULL))
			continue;

		// don't hit owner
		if (pTarget->GetpCore() == pChr->GetpCore()->m_BotOwnerCore)
			continue;

		// don't hit bot
		if (pTarget->GetpCore()->m_BotOwnerCore == pChr->GetpCore())
			continue;

		int DAMAGE = 20;
		int HEAL = 4;
		int SELFHEAL = 1;
		bool ShouldHit = false;
		bool ShouldHeal = false;
		bool ShouldSelfHeal = false;
		bool ShouldUnfreeze = false;
		bool ShouldInfect = false;
		bool ShouldFreeze = false;
		bool ShouldGiveUpVelocity = false;
		bool ShouldAntigravitate = false;

		if (pChr->IsHuman()) {
			if (pTarget->IsZombie()) { // Human hits Zombie
				ShouldHit = true;
			} else {                   // Human hits Human
				ShouldUnfreeze = true;
				ShouldGiveUpVelocity = true;
			}
		}

		if (pChr->IsZombie()) {
			if (pTarget->IsHuman()) { // Zombie hits Human
				if (g_Config.m_SvFng)
					ShouldFreeze = true;
				else
					ShouldInfect = true;	
			} else {                  // Zombie hits Zombie
				ShouldHeal = true;
				HEAL = 4;
				ShouldSelfHeal = true;
				ShouldGiveUpVelocity = true;
				ShouldUnfreeze = true;
			}
		}

		if (pChr->IsHookProtected() || pTarget->IsHookProtected())
			ShouldGiveUpVelocity = false;

		CCharacterCore *CCore = &pChr->GetCharacterCore();
		CCharacterCore *TargetCCore = &pTarget->GetCharacterCore();
		if (CCore && TargetCCore) {
			if (CCore->m_TaxiPassengerCore && CCore->m_TaxiPassengerCore == TargetCCore)
				continue;

			if (CCore->m_TaxiDriverCore && CCore->m_TaxiDriverCore == TargetCCore)
				continue;
		}

		if (pChr->GetCroyaPlayer()->GetClassNum() == Class::MEDIC && (ShouldUnfreeze)) {
			HEAL = 2;
			ShouldHeal = true;
			ShouldSelfHeal = false;
		}

		if (ShouldHit && pChr->GetCroyaPlayer()->GetClassNum() == Class::MAGICIAN) {
			ShouldAntigravitate = true;
			ShouldHit = false;
		}


		if (pChr->GetCroyaPlayer()->GetClassNum() == Class::SPIDER && (ShouldFreeze || ShouldInfect)) {
			ShouldInfect = false;
			ShouldFreeze = false;
			ShouldHit = true;
			DAMAGE = 0;
		}

		if (pChr->GetCroyaPlayer()->GetClassNum() == Class::BAT && (ShouldFreeze || ShouldInfect)) {
			ShouldInfect = false;
			ShouldFreeze = false;
			ShouldHit = true;
			DAMAGE = 1;
		}

		if (pChr->GetCroyaPlayer()->GetClassNum() == Class::KING && (ShouldFreeze || ShouldInfect)) {
			ShouldInfect = false;
			ShouldFreeze = false;
			ShouldHit = true;
			DAMAGE = 9;
		}

		if (pChr->GetCroyaPlayer()->GetClassNum() == Class::WORKER && (ShouldFreeze || ShouldInfect)) {
			ShouldInfect = false;
			ShouldFreeze = false;
			ShouldHit = true;
			DAMAGE = 2;
		}

		if (pTarget->GetCroyaPlayer()->GetClassNum() == Class::HERO) {
			if (ShouldInfect) {
				ShouldInfect = false;
				ShouldHit = true;
				DAMAGE = 9;
			}
		}

		if (pTarget->GetCroyaPlayer()->GetClassNum() == Class::PSYCHO) {
			ShouldHeal = false;
			if (ShouldInfect) {
				ShouldInfect = false;
				ShouldHit = true;
				DAMAGE = 5;
			}
			// if (ShouldHeal) {
			// 	ShouldHeal = false;
			// }
		}

		if (length(pTarget->GetPos() - ProjStartPos) > 0.0f)
			pGameServer->CreateHammerHit(pTarget->GetPos() - normalize(pTarget->GetPos() - ProjStartPos) * pChr->GetProximityRadius() * 0.5f);
		else
			pGameServer->CreateHammerHit(ProjStartPos);

		vec2 Dir;
		if (length(pTarget->GetPos() - pChr->GetPos()) > 0.0f)
			Dir = normalize(pTarget->GetPos() - pChr->GetPos());
		else
			Dir = vec2(0.f, -1.f);


		if (ShouldHit)
		{
			pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f, Dir * -1, DAMAGE,
								ClientID, WEAPON_HAMMER);
		}

		if (ShouldUnfreeze) {
			pTarget->UnFreeze();
			if (pTarget->Stunned())
				pTarget->Unstun();
		}

		if (ShouldAntigravitate) {
			pTarget->GetCroyaPlayer()->AntigravityOn();
		}

		if (ShouldInfect) {
			pTarget->Infect(ClientID); 
			CPlayer *pPlayer = pChr->GetPlayer();
			if (pPlayer) {
			  //pPlayer->m_Infections += 1;
			  pTarget->GetPlayer()->m_Deaths += 1;
			}
		}

		if (ShouldFreeze) {
			pTarget->Freeze(5);
		}

		if (ShouldHeal) {
			pTarget->IncreaseOverallHp(HEAL);
			pTarget->SetEmote(EMOTE_HAPPY, pChr->Server()->Tick() + pChr->Server()->TickSpeed());
		}

		if (ShouldSelfHeal)
			pChr->IncreaseOverallHp(SELFHEAL);

		// set his velocity to fast upward (for now)
		if (ShouldGiveUpVelocity) {
			pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f, Dir * -1, 0,
								ClientID, WEAPON_HAMMER);
		}

	}

	// if we Hit anything, we have to wait for the reload
	if (Hits) {
		if (pChr->m_IsBot)
			pChr->SetReloadTimer(pChr->Server()->TickSpeed() / 10);
		else
			pChr->SetReloadTimer(pChr->Server()->TickSpeed() / 3);
	}
}

void IClass::OnMouseWheelDown(CCharacter *pChr)
{
}

void IClass::OnMouseWheelUp(CCharacter *pChr)
{
}

void IClass::OnCharacterSpawn(CCharacter* pChr)
{
	pChr->SetInfected(IsInfectedClass());
	pChr->ResetWeaponsHealth();
	pChr->m_EndlessHook = false;
	InitialWeaponsHealth(pChr);
}

int IClass::OnCharacterDeath(CCharacter* pVictim, CPlayer* pKiller, int Weapon)
{
	return 0;
}

const CSkin& IClass::GetSkin() const
{
	return m_Skin;
}

void IClass::Set06SkinColors(int SkinBodyColor06, int SkinFeetColor06)
{
	m_06SkinBodyColor = SkinBodyColor06;
	m_06SkinFeetColor = SkinFeetColor06;
}

void IClass::Set06SkinName(const char* name)
{
	char SkinName[64];
	str_format(SkinName, sizeof(SkinName), "%s", name);
	str_copy(m_06SkinName, SkinName, 64);
}

void IClass::SetSkin(CSkin skin)
{
	m_Skin = skin;
}

bool IClass::IsInfectedClass() const
{
	return m_Infected;
}

void IClass::SetInfectedClass(bool Infected)
{
	m_Infected = Infected;
}

std::string IClass::GetName() const
{
	return m_Name;
}

void IClass::SetName(std::string Name)
{
	m_Name = Name;
}

void IClass::AntigravityOn(CCharacter* pChr, bool LooseSpeed, int Duration) {
	dbg_msg("game", "Antigravity on by class %d", Duration);
	AntigravityTimeLeft = pChr->Server()->TickSpeed() * Duration; 
	if (LooseSpeed)
		pChr->GetCharacterCore().m_Vel = vec2(0, -4.0f);
}

void IClass::AntigravityOff(CCharacter* pChr) {
	dbg_msg("game", "Antigravity off by class");
	if (!pChr->GetCroyaPlayer())
		return;
	pChr->GetCroyaPlayer()->AntigravityOff();	
}

void IClass::ItInfiniteJumpsWhenSlowmo(CCharacter* pChr) {
	if (pChr->IsInSlowMotion() && pChr->IsHuman())
		ItInfiniteJumps(pChr);
}

void IClass::ItInfiniteJumps(CCharacter* pChr) {
	//Double jumps
	CroyaPlayer* cp = pChr->GetCroyaPlayer();
	if (pChr->GetCharacterCore().m_TriggeredEvents & protocol7::COREEVENTFLAG_AIR_JUMP)
	{
		pChr->GetCharacterCore().m_Jumped &= ~2;
		cp->SetAirJumpCounter(0);
	}
}

void IClass::ItAntigravitates(CCharacter* pChr) {
	CroyaPlayer* cp = pChr->GetCroyaPlayer();

	//if (AntigravityTimeLeft > 0) {
	if (cp && cp->IsAntigravityOn()) {
		float y = pChr->GetCharacterCore().m_Vel.y;
		y = clamp<float>(y - 1.0f, -4.0f, 20.0f);
		pChr->GetCharacterCore().m_Vel = vec2(pChr->GetCharacterCore().m_Vel.x, y); // antigravity
		AntigravityTimeLeft--;
	
		if(AntigravityTimeLeft < 0)
		{
			dbg_msg("game", "Antigravity timeout by class");
			AntigravityOff(pChr);
		}
	}	
}
