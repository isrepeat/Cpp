#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl.h>
#include <cstdint>
#include <exception>
#include <string>
#include <vector>

#define HELPERS_NS_ALIAS HH
#include <MagicEnum/MagicEnum.h>
#include <Helpers/Logger.h>
#include <Helpers/Time.h>

#include "config.h"
#include "HSystem.h"
#include "HMath.h"
#include "Macros.h"
#include "Dx/Dx.h"
#include "Dx/DxDevice.h"
#include "Thread/critical_section.h"
#include "Thread/LockStack.h"