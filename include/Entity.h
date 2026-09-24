#ifndef ENTITY_H
#define ENTITY_H

#include "cglm/cglm.h"
#include <stdbool.h>
#include "Transform.h"

typedef struct Entity
{
    /*
        transform from previous frame
    */
    Transform previousTrans;
    Transform trans;
    struct Entity* pParent;
}Entity;

void GetWorldTransformForRender(Entity* pEntity, mat4 outMat4, float dt);

void GetWorldTransform(Entity* pEntity, mat4 outMat4);

#endif