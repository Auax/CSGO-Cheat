#pragma once
#include "World.h"
#include "Memory.h" 

class Entity
{
private:
    uintptr_t entity_ptr_;
    Memory* memory_;

public:
    //REFERENCE: https://www.unknowncheats.me/forum/counterstrike-global-offensive/195917-bone-matrix.html
    mat3x4 GetBoneMatrix() const;
    Vector3 GetBonePosition(int bone_index) const;

    Entity(Memory* memory, uintptr_t entity_ptr);
};
