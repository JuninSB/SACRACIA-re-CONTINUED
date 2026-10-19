#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <process.h>
#include <dxgi.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <shared_mutex>

#include <third_party/haxsdk/unity/hax_unity.h>
#include <third_party/haxsdk/gui/hax_gui_dx11.h>
#include <third_party/safetyhook/safetyhook.hpp>