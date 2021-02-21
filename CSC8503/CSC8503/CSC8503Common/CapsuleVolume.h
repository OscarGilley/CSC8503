#pragma once
#include "CollisionVolume.h"

namespace NCL {
    class CapsuleVolume : public CollisionVolume
    {
    public:
		CapsuleVolume(float halfHeight, float radius, bool isPlayer = false) {
            this->halfHeight    = halfHeight;
            this->radius        = radius;
            this->type          = VolumeType::Capsule;
			this->isPlayer = isPlayer;
        };
        ~CapsuleVolume() {

        }
        float GetRadius() const {
            return radius;
        }

        float GetHalfHeight() const {
            return halfHeight;
        }
		bool IsPlayer() const {
			return isPlayer;
		}
		

    protected:
        float radius;
        float halfHeight;
		bool isPlayer;
    };
}

