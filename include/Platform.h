#ifndef GLFW_PLATFORM_H
#define GLFW_PLATFORM_H

#include <stdbool.h>

double Platform_GetTime();

void Platform_PollInput();

int Platform_Init();

bool Platform_ShouldWindowClose();

#endif