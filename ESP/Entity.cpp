#include "Entity.h"
#include "Offsets.h"

Entity::Entity(Memory* memory, const uintptr_t entity_ptr) {
    this->memory_ = memory;
    this->entity_ptr_ = entity_ptr;
}

mat3x4 Entity::GetBoneMatrix() const
{
	const auto BoneMatrixPtr = memory_->RPM<DWORD>(entity_ptr_ + m_dwBoneMatrix);
    return memory_->RPM<mat3x4>(BoneMatrixPtr);
}

Vector3 Entity::GetBonePosition(const int bone_index) const
{
	const auto BoneMatrixPtr = memory_->RPM<DWORD>(entity_ptr_ + m_dwBoneMatrix);
	const mat3x4 BoneMatrix = memory_->RPM<mat3x4>(BoneMatrixPtr + (0x30 * bone_index));
    Vector3 vec{};
    vec.x = BoneMatrix.c[0][3];
    vec.y = BoneMatrix.c[1][3];
    vec.z = BoneMatrix.c[2][3];
    return vec;
}
