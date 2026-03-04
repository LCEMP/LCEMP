#pragma once

#include <d3d11.h>

bool InitGammaPostProcess();
void ApplyGammaPostProcess();
void CleanupGammaPostProcess();
void SetGammaValue(float gamma);
