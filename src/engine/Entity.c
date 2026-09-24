/*
    Copyright Jim Marshall 2026.
    File: Entity.c
    Description: Game entity
*/
/////////////////////////////////////////////////////////////////////////////////////////// Standard Library Includes

#include <string.h>

/////////////////////////////////////////////////////////////////////////////////////////// SDK Includes

/////////////////////////////////////////////////////////////////////////////////////////// Third Party Includes

#include <cglm/cglm.h>

/////////////////////////////////////////////////////////////////////////////////////////// First Party Includes

#include "Entity.h"

/////////////////////////////////////////////////////////////////////////////////////////// Typedefs

/////////////////////////////////////////////////////////////////////////////////////////// Defines

/////////////////////////////////////////////////////////////////////////////////////////// Enums

/////////////////////////////////////////////////////////////////////////////////////////// Structs

/////////////////////////////////////////////////////////////////////////////////////////// Private Globals

/////////////////////////////////////////////////////////////////////////////////////////// Public Globals

/////////////////////////////////////////////////////////////////////////////////////////// Private Functions

/////////////////////////////////////////////////////////////////////////////////////////// Public Functions

void GetWorldTransformForRender(Entity* pEntity, mat4 outMat4, float dt)
{
    if(!pEntity->trans.bDirty)
    {
        glm_mat4_copy(pEntity->trans.world_transform, outMat4);
        return;
    }

    glm_mat4_identity(pEntity->trans.world_transform);
    vec3 pos;
    glm_vec3_lerp(pEntity->previousTrans.translation, pEntity->trans.translation, dt, pos);

    glm_translate(pEntity->trans.world_transform, pos);

    versor rot;
    glm_quat_slerp(pEntity->previousTrans.rotation, pEntity->trans.rotation, dt, rot);
    glm_quat_rotate(pEntity->trans.world_transform, rot, pEntity->trans.world_transform);

    vec3 scale;
    glm_vec3_lerp(pEntity->previousTrans.scale, pEntity->trans.scale, dt, pos);
    glm_scale(pEntity->trans.world_transform, pEntity->trans.scale);
    
    
    if(pEntity->pParent)
    {
        mat4 parentWorld;
        GetWorldTransformForRender(pEntity->pParent, parentWorld, dt);
        glm_mat4_mul(parentWorld, pEntity->trans.world_transform, pEntity->trans.world_transform);
    }

    pEntity->trans.bDirty = false;
    glm_mat4_copy(pEntity->trans.world_transform, outMat4);
    return;
}

void GetWorldTransform(Entity* pEntity, mat4 outMat4)
{
    if(!pEntity->trans.bDirty)
    {
        glm_mat4_copy(pEntity->trans.world_transform, outMat4);
        return;
    }

    glm_mat4_identity(pEntity->trans.world_transform);

    glm_translate(pEntity->trans.world_transform, pEntity->trans.translation);
    glm_quat_rotate(pEntity->trans.world_transform, pEntity->trans.rotation, pEntity->trans.world_transform);
    glm_scale(pEntity->trans.world_transform, pEntity->trans.scale);
    
    
    if(pEntity->pParent)
    {
        mat4 parentWorld;
        GetWorldTransform(pEntity->pParent, parentWorld);
        glm_mat4_mul(parentWorld, pEntity->trans.world_transform, pEntity->trans.world_transform);
    }

    pEntity->trans.bDirty = false;
    glm_mat4_copy(pEntity->trans.world_transform, outMat4);
    return;

}