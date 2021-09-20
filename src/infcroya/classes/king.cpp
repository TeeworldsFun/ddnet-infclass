#include "king.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <game/server/entities/projectile.h>
#include <game/server/entities/flag.h>
#include <infcroya/entities/bouncing-bullet.h>

CKing::CKing()
{
	CSkin skin;
	skin.SetBodyColor(0, 0, 0);
	skin.SetMarkingName("saddo");
	skin.SetMarkingColor(0, 0, 0);
	skin.SetDecorationName("default");
	skin.SetDecorationColor(233, 158, 183);
	skin.SetHandsColor(20, 208, 166);
	skin.SetFeetColor(0, 0, 0);
	SetSkin(skin);
	SetInfectedClass(ture);
	SetName("EvilKing");
	Set06SkinName("Redboppenom");
	Set06SkinColors(1363875, 655151);
}

void CKing::InitialWeaponsHealth(CCharacter* pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->SetNormalEmote(EMOTE_NORMAL);
}

void CPsycho::OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, CCharacter* pChr)
{
	switch (Weapon) {
		case WEAPON_HAMMER: {
			HammerShoot(pChr, ProjStartPos);
		} break;
	}

}
