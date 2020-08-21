#include "croyaplayer.h"
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/entities/character.h>
#include <infcroya/classes/class.h>
#include <infcroya/localization/localization.h>
#include <infcroya/entities/circle.h>
#include <infcroya/entities/inf-circle.h>
#include <limits>

CroyaPlayer::CroyaPlayer(int ClientID, CPlayer* pPlayer, CGameContext* pGameServer, CGameControllerMOD* pGameController, std::unordered_map<int, IClass*> Classes)
{
	m_ClientID = ClientID;
	m_pPlayer = pPlayer;
	m_pPlayer->SetCroyaPlayer(this);
	m_pGameServer = pGameServer;
	m_pGameController = pGameController;
	m_pCharacter = nullptr;
	m_Infected = false;
	m_HookProtected = true;
	m_Classes = Classes;
	m_Language = "english";

	m_RespawnPointsNum = 1;
	m_RespawnPointsDefaultNum = 1;
	m_RespawnPointPlaced = false;
	m_RespawnPointPos = vec2(0, 0);
	m_RespawnPointDefaultCooldown = 3;
	m_RespawnPointCooldown = 0;
	m_AirJumpCounter = 0;
	m_CanLockPosition = true;
	m_IsPositionLocked = false;
	m_InsideInfectionZone = false;
	m_InsideSafeZone = true;
	m_BeenOnRoundStart = false;
	m_InitialZombie = false;
}

CroyaPlayer::~CroyaPlayer()
{
}

void CroyaPlayer::SetBeenOnRoundStart() {
	m_BeenOnRoundStart = true;
}

void CroyaPlayer::Tick() // todo cleanup INF circles and safezones are mixed
{
	SetInsideInfectionZone(false);

	if (m_pCharacter && m_pClass && m_pCharacter->GameWorld()) { // so many null checks, not sure which are necessary
		m_pClass->Tick(m_pCharacter);
	}

	if (IsHuman() && m_pCharacter && m_pCharacter->GameWorld()) {
		// Infect when inside infection zone circle
		auto circle = GetClosestInfCircle();
		if (circle) {
			float dist = distance(m_pCharacter->GetPos(), circle->GetPos());
			if (dist < circle->GetRadius()) {
				m_pCharacter->Infect(-1);
			}
		}

		// Take damage when outside of safezone circle
		if (!m_InsideSafeZone) {
			int Dmg = 1;
			if (m_pGameServer->Server()->Tick() % m_pGameServer->Server()->TickSpeed() == 0)
			{ // each second
				if (m_pCharacter->GetCroyaPlayer()->GetClassNum() == Class::HERO)
				{
					if (m_pGameServer->Server()->Tick() % (m_pGameServer->Server()->TickSpeed() * 3) == 0)
					  m_pCharacter->TakeDamage(vec2(0, 0), m_pCharacter->GetPos(), Dmg, m_ClientID, WEAPON_WORLD);
				} else {
					m_pCharacter->TakeDamage(vec2(0, 0), m_pCharacter->GetPos(), Dmg, m_ClientID, WEAPON_WORLD);
				}
			}
		}
	}

	if (IsZombie() && m_pCharacter) {
		// Increase health when inside infection zone circle
		auto inf_circle = GetClosestInfCircle();
		if (inf_circle) {
			float dist = distance(m_pCharacter->GetPos(), inf_circle->GetPos());
			if (dist < inf_circle->GetRadius() && m_pGameServer->Server()->Tick() % m_pGameServer->Server()->TickSpeed() == 0) { // each second
				m_pCharacter->IncreaseHealth(2);
				m_RespawnPointsNum = m_RespawnPointsDefaultNum;
			}
			if (dist < inf_circle->GetRadius()) {
				SetInsideInfectionZone(true);
			}
		}
	}

	if (m_RespawnPointCooldown > 0) {
		if (m_pGameServer->Server()->Tick() % m_pGameServer->Server()->TickSpeed() == 0) { // each second
			m_RespawnPointCooldown -= 1;
		}
	}

	if (IsZombie() && m_pCharacter && m_pCharacter->GameWorld()) {
		if (m_pCharacter->GetCharacterCore().m_HookedPlayer >= 0) {
			CCharacter* VictimChar = m_pGameServer->GetPlayerChar(m_pCharacter->GetCharacterCore().m_HookedPlayer);
			if (VictimChar)
			{
				float Rate = 1.0f;
				int Damage = 1;

				if (GetClassNum() == Class::SMOKER)
				{
					Rate = 0.5f;
					Damage = g_Config.m_InfSmokerHookDamage;
				}

				if (GetClassNum() == Class::MOTHER)
				{
					Rate = 1.0f;
					Damage = 0;
				}

				if (GetClassNum() == Class::BAT)
				{
					Rate = 1.0f;
					Damage = 1;
				}


				if (GetClassNum() == Class::FREEZER)
				{
					Rate = 1.0f;
					Damage = 0;
					if (VictimChar->IsHuman()) {
						VictimChar->Freeze(1.0f, m_ClientID, FREEZEREASON_FLASH);
						//VictimChar->Die(m_ClientID, WEAPON_HAMMER);
					}
				}			

				if (GetClassNum() == Class::PARASITE){
					Rate = 1.0f;
					Damage = 0;
					if (VictimChar->IsHuman()) {
						const CSkin skin = VictimChar->GetCroyaPlayer()->GetClass()->GetSkin();
						CPlayer *pPlayer = GetPlayer();

				        //str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_BODY], skin.GetBodyName(), 24);
				        //pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_BODY] = true;
				        //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_BODY] = skin.GetBodyColor();

				        //str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_MARKING], skin.GetMarkingName(), 24);
				        //pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_MARKING] = true;
				        //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_MARKING] = skin.GetMarkingColor();

				        //str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_DECORATION], skin.GetDecorationName(), 24);
				        //pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_DECORATION] = true;
				        //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_DECORATION] = skin.GetDecorationColor();

				        //str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_HANDS], skin.GetHandsName(), 24);
				        //pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_HANDS] = true;
				        //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_HANDS] = skin.GetHandsColor();

				        //str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_FEET], skin.GetFeetName(), 24);
				        //pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_FEET] = true;
				        //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_FEET] = skin.GetFeetColor();

						//if (VictimChar->GetCroyaPlayer() && (
						//    VictimChar->GetCroyaPlayer()->GetClassNum() == Class::SNIPER ||
						//    VictimChar->GetCroyaPlayer()->GetClassNum() == Class::MEDIC ||
						//    VictimChar->GetCroyaPlayer()->GetClassNum() == Class::SOLDIER )
						//) {
				  //      	str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_EYES], "negative", 24);
				  //          pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_EYES] = 130944;
						//} else {
				  //      	str_copy(pPlayer->m_TeeInfos.m_aaSkinPartNames[SKINPART_EYES], "negative", 24);
				  //          pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_EYES] = 128104;
						//}
				  //      pPlayer->m_TeeInfos.m_aUseCustomColors[SKINPART_EYES] = true;
				  //      //pPlayer->m_TeeInfos.m_aSkinPartColors[SKINPART_EYES] = skin.GetEyesColor();

						//m_pGameServer->SendAllClientsSkinChange(GetClientID());
					}
				}

				if (m_pCharacter->m_HookDmgTick + m_pGameServer->Server()->TickSpeed() * Rate < m_pGameServer->Server()->Tick())
				{
					m_pCharacter->m_HookDmgTick = m_pGameServer->Server()->Tick();
					if (Damage > 0)
					  VictimChar->TakeDamage(vec2(0.0f, 0.0f), m_pCharacter->GetPos(), Damage, m_pPlayer->GetCID(), WEAPON_NINJA);
					if (GetClassNum() == Class::SMOKER) {
						m_pCharacter->IncreaseOverallHp(Damage); // blood licking
					}
				}
			}
		}
	}
	
	// set this at the end of CroyaPlayer tick so entities could overwrite this
	if (IsHuman()) // we don't care about zombies
		SetInsideSafeZone(IsInsideSafeZone());

}

bool CroyaPlayer::IsInsideSafeZone() {
	auto safe_circle = GetClosestCircle(); // closest only
	if (safe_circle)
	{
		float dist = distance(m_pCharacter->GetPos(), safe_circle->GetPos());
		if (dist > safe_circle->GetRadius())
			return false;
	}
	return true;
}

CCircle* CroyaPlayer::GetClosestCircle()
{
	float min_distance = std::numeric_limits<float>::max();
	CCircle* closest = nullptr;
	for (CCircle* circle : m_pGameController->GetCircles()) {
		if (!m_pCharacter)
			break;
		if (!circle)
			continue; // maybe unnecessary, not sure

		float dist = distance(m_pCharacter->GetPos(), circle->GetPos()) - circle->GetRadius();
		if (dist < min_distance) {
			min_distance = dist;
			closest = circle;
		}
	}
	if (closest) {
		return closest;
	}
	return nullptr;
}

CInfCircle* CroyaPlayer::GetClosestInfCircle()
{
	float min_distance = std::numeric_limits<float>::max();
	CInfCircle* closest = nullptr;
	for (CInfCircle* circle : m_pGameController->GetInfCircles()) {
		if (!m_pCharacter)
			break;
		if (!circle)
			continue; // maybe unnecessary, not sure

		float dist = distance(m_pCharacter->GetPos(), circle->GetPos()) - circle->GetRadius();
		if (dist < min_distance) {
			min_distance = dist;
			closest = circle;
		}
	}
	if (closest) {
		return closest;
	}
	return nullptr;
}

int CroyaPlayer::GetClassNum()
{
	int ClassNum;
	for (const auto& c : m_Classes) {
		if (m_pClass == c.second) {
			ClassNum = c.first;
			break;
		}
	}
	return ClassNum;
}

void CroyaPlayer::SetClassNum(int Class, bool DrawPurpleThing, bool ShowInfo, bool destroyChildEntities)
{
	SetClass(m_Classes[Class], DrawPurpleThing, destroyChildEntities);
	//m_pGameServer->SendClassSelectorByClassId(Class, GetClientID(), ShowInfo);
	//TBD
}

bool CroyaPlayer::SetZombieClassNumPlease(int Class, bool DrawPurpleThing, bool Force)
{
	if (!Force) {
		if (!IsZombie())
			return false;

		if (!IsInsideInfectionZone()) {
			return false;
		}
	}

	SetClassNum(Class, DrawPurpleThing);
	ResetRespawnPoint();
	return true;
}

bool CroyaPlayer::SetHumanClassNumPlease(int Class, bool DrawPurpleThing, bool Force)
{
	if (!Force) {
		if (!m_pGameController->IsCroyaWarmup())
		return false;
	}

	SetClassNum(Class, DrawPurpleThing, true);
	return true;
}

CCharacter* CroyaPlayer::GetCharacter() {
	return m_pCharacter;
}

void CroyaPlayer::SetCharacter(CCharacter* pCharacter)
{
	m_pCharacter = pCharacter;
}

CPlayer* CroyaPlayer::GetPlayer()
{
	return m_pPlayer;
}

int CroyaPlayer::GetClientID() const
{
	return m_ClientID;
}

void CroyaPlayer::OnCharacterSpawn(CCharacter* pChr)
{
	m_pClass->OnCharacterSpawn(pChr);
	m_pCharacter->SetCroyaPlayer(this);
}

void CroyaPlayer::OnCharacterDeath(CCharacter* pVictim, CPlayer* pKiller, int Weapon)
{
	m_pClass->OnCharacterDeath(pVictim, pKiller, Weapon);
	if (IsHuman()) {
		SetOldClassNum(GetClassNum());
		TurnIntoRandomZombie();
	}
	m_pCharacter = nullptr;
}

void CroyaPlayer::OnKill(int Victim)
{
	int64 Mask = CmaskOne(m_ClientID);
	m_pGameServer->CreateSound(m_pPlayer->m_ViewPos, SOUND_CTF_GRAB_PL, Mask);
	if (IsZombie()) {
		m_pPlayer->m_Score += 3;
	}
	else {
		m_pPlayer->m_Score++;
	}
}

bool CroyaPlayer::WillItFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	m_pCharacter = pChr;
	return m_pClass->WillItFire(Direction, ProjStartPos, Weapon, m_pCharacter);
}

void CroyaPlayer::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	m_pCharacter = pChr;
	m_pClass->OnWeaponFire(Direction, ProjStartPos, Weapon, m_pCharacter);
}

void CroyaPlayer::OnButtonF3()
{
	SetHookProtected(!IsHookProtected());
}

void CroyaPlayer::SetSpawnPointAt(vec2 NewPos)
{
	if (IsHuman()) {
		return;
	}

	if (!m_pGameController->IsCroyaWarmup()) {
		m_RespawnPointPos = NewPos;
		m_RespawnPointPlaced = true;
	}
}

void CroyaPlayer::OnMouseWheelDown(CCharacter* pChr)
{
	m_pCharacter = pChr;
	if (m_pGameController->IsCroyaWarmup()) {
		TurnIntoNextHumanClass();
		m_pClass->OnMouseWheelDown();
		return;
	}
	if (IsZombie() && IsInsideInfectionZone()) {
		TurnIntoNextZombieClass();
		return;
	}
	if (IsZombie() && m_pCharacter && m_RespawnPointCooldown == 0) {
		if (m_RespawnPointPlaced) {
			m_RespawnPointPos = m_pCharacter->GetPos();
		}
		else {
			m_RespawnPointPos = m_pCharacter->GetPos();
			m_RespawnPointPlaced = true;
		}
		m_RespawnPointCooldown = m_RespawnPointDefaultCooldown;
	}
}

void CroyaPlayer::OnMouseWheelUp(CCharacter* pChr)
{
	m_pCharacter = pChr;
	if (m_pGameController->IsCroyaWarmup()) {
		TurnIntoPrevHumanClass();
		m_pClass->OnMouseWheelUp();
		return;
	}

	if (IsZombie() && IsInsideInfectionZone()) {
		TurnIntoPrevZombieClass();
	}
}

void CroyaPlayer::ResetRespawnPoint()
{
	m_RespawnPointsNum = 1;
	m_RespawnPointPos = vec2(0, 0);
	m_RespawnPointPlaced = false;
}


vec2 CroyaPlayer::GetRespawnPointPos() const
{
	return m_RespawnPointPos;
}

int CroyaPlayer::GetRespawnPointsNum() const
{
	return m_RespawnPointsNum;
}

void CroyaPlayer::SetRespawnPointsNum(int Num)
{
	m_RespawnPointsNum = Num;
}

bool CroyaPlayer::IsRespawnPointPlaced() const
{
	return m_RespawnPointPlaced;
}

void CroyaPlayer::SetRespawnPointPlaced(bool Placed)
{
	m_RespawnPointPlaced = Placed;
}

int CroyaPlayer::GetRespawnPointDefaultCooldown() const
{
	return m_RespawnPointDefaultCooldown;
}

int CroyaPlayer::GetRespawnPointCooldown()
{
	return m_RespawnPointCooldown;
}

void CroyaPlayer::SetRespawnPointCooldown(int Cooldown)
{
	m_RespawnPointCooldown = Cooldown;
}

bool CroyaPlayer::IsHuman() const
{
	return !m_Infected;
}

bool CroyaPlayer::IsZombie() const
{
	return m_Infected;
}

bool CroyaPlayer::IsInitialZombie()
{
	return m_InitialZombie;
}


std::unordered_map<int, class IClass*>& CroyaPlayer::GetClasses()
{
	return m_Classes;
}

void CroyaPlayer::TurnIntoNextHumanClass()
{
	int NextClass = GetClassNum() + 1;
	int FirstClass = Class::HUMAN_CLASS_START + 1;
	bool NotInRange = !(NextClass > HUMAN_CLASS_START && NextClass < HUMAN_CLASS_END);

	if (NextClass == Class::HUMAN_CLASS_END || NotInRange)
		NextClass = FirstClass;
	SetClassNum(NextClass, false, true);
}

void CroyaPlayer::TurnIntoPrevHumanClass()
{
	int PrevClass = GetClassNum() - 1;
	int LastClass = Class::HUMAN_CLASS_END - 1;
	bool NotInRange = !(PrevClass > HUMAN_CLASS_START && PrevClass < HUMAN_CLASS_END);

	if (PrevClass == Class::HUMAN_CLASS_START || NotInRange)
		PrevClass = LastClass;
	SetClassNum(PrevClass, false, true);
}

//void CroyaPlayer::TurnIntoInitialZombie()
//{
//	m_InitialZombie = true;
//	TurnIntoRandomZombie();
//}

void CroyaPlayer::TurnIntoRandomZombie()
{
	if (m_pGameController->GetRealPlayerNum() >= 2 && m_pGameController->GetIZombieCount() < 1)
		m_InitialZombie = true;

	int RandomZombieClass = random_int_range(Class::ZOMBIE_CLASS_START + 1, Class::BOOMER - 1);
	SetClassNum(RandomZombieClass, true);
}

void CroyaPlayer::TurnIntoRandomHuman()
{
	// Class::HUMAN_CLASS_START + 2 because of DEFAULT. DEFAULT comes right after HUMAN_CLASS_START
	int RandomHumanClass = random_int_range(Class::HUMAN_CLASS_START + 2, Class::HUMAN_CLASS_END - 1);
	SetClassNum(RandomHumanClass, true);
	ResetRespawnPoint();
}

void CroyaPlayer::TurnIntoNextZombieClass()
{
	int NextClass = GetClassNum() + 1;
	int FirstClass = Class::ZOMBIE_CLASS_START + 1;
	//int LastSelectableInGeneral = m_pGameServer->IsDevServer() ? PARASITE : MOTHER;
	int LastSelectableInGeneral = MOTHER;
	int SelectableZombieEnd = m_InitialZombie ? LastSelectableInGeneral + 1: MOTHER;
	bool NotInRange = !(NextClass > ZOMBIE_CLASS_START && NextClass < SelectableZombieEnd);

	if (NextClass == Class::ZOMBIE_CLASS_END || NotInRange)
		NextClass = FirstClass;
	SetClassNum(NextClass, false, true);
	ResetRespawnPoint();
}

void CroyaPlayer::TurnIntoPrevZombieClass()
{
	int PrevClass = GetClassNum() - 1;
	//int LastSelectableInGeneral = m_pGameServer->IsDevServer() ? PARASITE : MOTHER;
	//TBD
	int LastSelectableInGeneral = MOTHER;
	int SelectableZombieEnd = m_InitialZombie ? LastSelectableInGeneral + 1: MOTHER;
	bool NotInRange = !(PrevClass > ZOMBIE_CLASS_START && PrevClass < SelectableZombieEnd);

	if (PrevClass == Class::ZOMBIE_CLASS_START || NotInRange)
		PrevClass = SelectableZombieEnd - 1;
	SetClassNum(PrevClass, false, true);
	ResetRespawnPoint();
}

bool CroyaPlayer::IsInsideInfectionZone() const
{
	return m_InsideInfectionZone;
}

void CroyaPlayer::SetInsideInfectionZone(bool InsideInfectionZone)
{
	m_InsideInfectionZone = InsideInfectionZone;
}

void CroyaPlayer::SetInsideSafeZone(bool InsideSafeZone)
{
	m_InsideSafeZone = InsideSafeZone;
}

int CroyaPlayer::GetAirJumpCounter() const
{
	return m_AirJumpCounter;
}

void CroyaPlayer::SetAirJumpCounter(int AirJumpCounter)
{
	m_AirJumpCounter = AirJumpCounter;
}

vec2 CroyaPlayer::GetLockPosition() {
	return m_PositionLockVec;
}

bool CroyaPlayer::IsPositionLocked() {
	return m_IsPositionLocked;
}

void CroyaPlayer::ResetCanLockPositionAbility() {
	m_CanLockPosition = true;
}

void CroyaPlayer::LockPosition(vec2 Pos)
{
	if (m_pGameController->IsCroyaWarmup())
	  return;

	if (m_IsPositionLocked || !m_CanLockPosition)
		return;
	m_IsPositionLocked = true;
	m_CanLockPosition = false;
	m_PositionLockVec = Pos;
}

void CroyaPlayer::UnlockPosition()
{
	m_IsPositionLocked = false;
}

bool CroyaPlayer::IsHookProtected() const
{
	return m_HookProtected;
}

void CroyaPlayer::SetHookProtected(bool HookProtected)
{
	//m_HookProtected = HookProtected;
	//if (m_pPlayer) {
	//	m_pPlayer->SetHookProtected(HookProtected);
	//	if (IsHookProtected()) {
	//		m_pGameServer->SendChatTarget(m_ClientID, "Hook protection enabled");
	//	} else {
	//		m_pGameServer->SendChatTarget(m_ClientID, "Hook protection disabled");

	//	}
	//}
}

int CroyaPlayer::GetOldClassNum() const
{
	return m_OldClassNum;
}

void CroyaPlayer::SetOldClassNum(int Class)
{
	if (Class == Class::DEFAULT) {
		Class = Class::MEDIC;
	}
	m_OldClassNum = Class;
}

const char* CroyaPlayer::GetLanguage() const
{
	return m_Language.c_str();
}

void CroyaPlayer::SetLanguage(const char* Language)
{
	m_Language = Language;
}

CGameControllerMOD* CroyaPlayer::GetGameControllerMOD()
{
	return m_pGameController;
}

IClass* CroyaPlayer::GetClass()
{
	return m_pClass;
}

void CroyaPlayer::SetClass(IClass* pClass, bool DrawPurpleThing, bool destroyChildEntities)
{
	if (m_pCharacter && DrawPurpleThing) {
		vec2 PrevPos = m_pCharacter->GetPos();
		m_pGameServer->CreatePlayerSpawn(PrevPos); // draw purple thing
	}
	//for (int& each : m_pPlayer->m_TeeInfos.m_aUseCustomColors) {
	//	each = 1;
	//}
	//TBD

	m_pClass = pClass;
	const CSkin& skin = m_pClass->GetSkin();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[0] = skin.GetBodyColor();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[1] = skin.GetMarkingColor();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[2] = skin.GetDecorationColor();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[3] = skin.GetHandsColor();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[4] = skin.GetFeetColor();
	m_pPlayer->m_TeeInfos.m_aSkinPartColors[5] = skin.GetEyesColor();
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[0], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[0]), "%s", skin.GetBodyName());
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[1], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[1]), "%s", skin.GetMarkingName());
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[2], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[2]), "%s", skin.GetDecorationName());
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[3], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[3]), "%s", skin.GetHandsName());
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[4], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[4]), "%s", skin.GetFeetName());
	str_format(m_pPlayer->m_TeeInfos.m_apSkinPartNames[5], sizeof(m_pPlayer->m_TeeInfos.m_apSkinPartNames[5]), "%s", skin.GetEyesName());

	if (m_pClass->IsInfectedClass()) {
		m_Infected = true;
	}
	else {
		m_InitialZombie = false;
		m_Infected = false;
	}

	for (const CPlayer* each : m_pGameServer->m_apPlayers) {
		if (each) {
			m_pGameServer->SendSkinChange(m_pPlayer->GetCID(), each->GetCID());
			//if (m_pClass->IsInfectedClass()) {
			//	if (m_InitialZombie)
			//	  m_pGameServer->SendClanChange(m_pPlayer->GetCID(), each->GetCID(), "iZombie");
			//	else
			//	  m_pGameServer->SendClanChange(m_pPlayer->GetCID(), each->GetCID(), "Zombie");
			//} else {
			//	m_pGameServer->SendClanChange(m_pPlayer->GetCID(), each->GetCID(), "Human");
			//}
		}
	}
	if (!destroyChildEntities)
	  return;

	if (m_pCharacter) {
		m_pCharacter->DestroyChildEntities();
		m_pCharacter->SetInfected(m_pClass->IsInfectedClass());
		m_pCharacter->ResetWeaponsHealth();
		m_pClass->InitialWeaponsHealth(m_pCharacter);
	}

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s", m_pClass->GetName().c_str());
	//m_pGameServer->SendBroadcast(localize(aBuf, GetLanguage()).c_str(), m_pPlayer->GetCID());
	m_pGameServer->SendBroadcast(aBuf, m_pPlayer->GetCID());
}
