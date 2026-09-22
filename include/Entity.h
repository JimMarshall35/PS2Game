#ifndef ENTITY_H
#define ENTITY_H

#include "cglm/cglm.h"
#include <stdbool.h>
#include "Transform.h"

typedef struct Entity
{
    Transform trans;
    struct Entity* pParent;
}Entity;

void GetWorldTransform(Entity* pEntity, mat4 outMat4);

#endif