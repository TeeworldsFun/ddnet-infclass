#pragma once

#include "class.h"

class CKing : public IClass {
public:
	CKing();

	void InitialWeaponsHealth(class CCharacter* pChr) override;

	void OnWeaponFire(vec2 Direction, vec2 ProjStartPos, int Weapon, class CCharacter* pChr) override;

};
