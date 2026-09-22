#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "cglm/cglm.h"
#include <stdbool.h>

typedef struct Transform {
    vec3 translation;
    versor rotation;
    vec3 scale;
    mat4 world_transform;
    bool bDirty;
} Transform;

void Tr_SetTranslation(Transform* pTransform, vec3 translation);
void Tr_SetScale(Transform* pTransform, vec3 scale);
void Tr_SetRotation(Transform* pTransform, versor rotation);

#endif