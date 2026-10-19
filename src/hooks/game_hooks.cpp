#include <src/pch.h>

#include <cmath>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

#include <src/cheat.h>
#include <src/game_classes.h>
#include <src/resource.h>
#include <src/overlay/esp/esp.h>
#include <src/overlay/menu/tabs/tabs.h>

using namespace System;
using namespace Unity;

namespace Unity::Photon
{
    bool PhotonNetwork::RaiseEvent(System::Byte code, System::Object data, RaiseEventOptions options, SendOptions sendOptions)
    {
        static auto method = typeof().GetMethod("RaiseEvent",
            "System.Boolean(System.Byte,System.Object,Photon.Realtime.RaiseEventOptions,ExitGames.Client.Photon.SendOptions)", true).Wrap();
        return method.Call<bool>(code, data, options, System::Box(sendOptions));
    }
}

namespace Cheat
{
    extern void Hook(void* ptr, void* detour, SafetyHookInline& out, const char* name);
    extern void HookModuleProc(HMODULE module, const char* procName, void* procHook, SafetyHookInline& out);

    static void* Hooked_MonoRuntimeInvoke(void* a1, void* a2, void* a3, void* a4);
    static void Hooked__EventSystem_Update(Unity::EventSystem __this);
    static void Hooked__PlayerHealth_Hurt(PlayerHealth __this, int damage, bool savingGrace, int enemyIndex);
    static void Hooked__PlayerAvatar_PlayerDeath(PlayerAvatar __this, int enemyIndex);
    static void Hooked__PlayerController_FixedUpdate(PlayerController __this);
    static void Hooked__PlayerController_Update(PlayerController __this);
    static void Hooked__PlayerTumble_TumbleRequest(PlayerTumble __this, bool isTumbling, bool playerInput);
    static void Hooked__FlashlightController_Update(FlashlightController __this);
    static void Hooked__ItemUpgrade_Update(ItemUpgrade __this);
    static void Hooked__ItemHealthPack_Update(ItemHealthPack __this);
    static void Hooked__ItemAttributes_Update(ItemAttributes __this);
    static void Hooked__InventorySpot_Update(InventorySpot __this);
    static void Hooked__SemiUI_Hide(SemiUI __this);
    static void Hooked__RunManager_ChangeLevel(RunManager __this, bool completedLevel, bool levelFailed, int changeLevelType);
    static void Hooked__UpdateLoop();
    static void Hooked__FixedPhysicsLoop();
    static void Hooked__FixedScriptsLoop();
    static void Hooked__LateScriptsLoop();
    static void Hooked__PhysGrabObject_PhysicsGrabbingManipulation(PhysGrabObject __this);
    static void Hooked__PostProcessLayer_BuildCommandBuffers(Unity::PostProcessLayer __this);
    static void Hooked__PlayerAvatarVisuals_Update(PlayerAvatarVisuals __this);
    static void Hooked__PhysGrabber_PhysGrabLogic(PhysGrabber __this);
    static void Hooked__PhysGrabber_RayCheck(PhysGrabber __this, bool grab);
    static void Hooked__MusicBoxTrap_Update(MusicBoxTrap __this);
    static bool Hooked__Physics_Raycast(void* v1, void* v2, void* v3, float maxDistance, int v5, int v6);
    static void Hooked__PhysGrabObjectImpactDetector_Break(PhysGrabObjectImpactDetector __this, float a1, void* a2, int a3, bool a4);
    static void Hooked__PhysGrabObjectImpactDetector_DestroyObject(PhysGrabObjectImpactDetector __this, bool a1);
    static void Hooked__ItemBattery_Update(ItemBattery __this);
    static void Hooked__Camera_FireOnPreRender(Unity::Camera __this);
    static void Hooked__ItemGun_Update(ItemGun __this);
    static void Hooked__RunManager_SetRunLevel(RunManager __this);
    static void Hooked__PhysGrabber_PhysGrabOverCharge(PhysGrabber __this, float _amount, float _multiplier);
    static void Hooked__SpectateCamera_HeadEnergyLogic(SpectateCamera __this);
    static void Hooked__PostLateUpdateLoop();
    static void Hooked__PresentAfterDrawLoop();
    static void Hooked__EnemyRigidbody_FixedUpdate(EnemyRigidbody __this);

    static bool IsInGame();
    static void* GetPlayerLoopPtr(System::Type type, System::Type subType);
    static void Heal(PhysGrabObjectImpactDetector obj, float value);
    static void SpawnItem(Item item);
    static void SpawnValuablePrefab(PrefabRef prefab, int overrideValue = 0);
    static void SpawnCosmeticBox(int rarity);
    static void ParseEnemies();
    static void ParseItems();
    static void ParseValuables();
    static void ParseLevels();
    static void DrawPlayerChams(PlayerAvatar player, Unity::CommandBuffer cb, Unity::Material aliveMat, Unity::Material deadMat);

    static void UpdateNameChangerConfig(const wchar_t* name)
    {
        wchar_t appData[MAX_PATH]{};
        DWORD length = GetEnvironmentVariableW(L"APPDATA", appData, _countof(appData));
        if (!length || length >= _countof(appData))
            return;

        std::error_code ec;
        std::filesystem::path profiles = std::filesystem::path(appData) / L"Thunderstore Mod Manager" / L"DataFolder" / L"REPO" / L"profiles";
        std::filesystem::path selected;
        std::filesystem::file_time_type newest = std::filesystem::file_time_type::min();
        for (const auto& entry : std::filesystem::directory_iterator(profiles, ec))
        {
            if (ec) break;
            const auto config = entry.path() / L"BepInEx" / L"config" / L"NameChangeREPO.cfg";
            const auto plugin = entry.path() / L"BepInEx" / L"plugins" / L"I_am_not_choco-NameChanger" / L"NameChanger.dll";
            if (!std::filesystem::exists(config, ec) || !std::filesystem::exists(plugin, ec))
                continue;
            auto log = entry.path() / L"BepInEx" / L"LogOutput.log";
            auto timestamp = std::filesystem::exists(log, ec) ? std::filesystem::last_write_time(log, ec) : std::filesystem::last_write_time(config, ec);
            if (!ec && timestamp >= newest)
            {
                newest = timestamp;
                selected = config;
            }
        }

        if (!selected.empty())
            WritePrivateProfileStringW(L"User", L"Name", name, selected.c_str());
    }

    static void UpdatePerformanceSettings()
    {
        if (!G->IsInGame)
            return;

        struct Baseline
        {
            bool Ready = false;
            int Shadows = 0;
            float ShadowDistance = 0.f;
            int AntiAliasing = 0;
            int PixelLightCount = 0;
            float LodBias = 1.f;
            bool Fog = true;
            bool SoftParticles = true;
            bool SoftVegetation = true;
            bool ReflectionProbes = true;
        };
        struct ComponentState { Unity::Behaviour Component; bool WasEnabled; };
        static Baseline original;
        static std::vector<ComponentState> postProcessLayers;
        static ULONGLONG nextApply = 0;
        static ULONGLONG nextPostProcessScan = 0;

        const ULONGLONG now = GetTickCount64();
        if (now < nextApply)
            return;
        nextApply = now + 250;

        if (!original.Ready)
        {
            original.Shadows = QualitySettings::Shadows();
            original.ShadowDistance = QualitySettings::ShadowDistance();
            original.AntiAliasing = QualitySettings::AntiAliasing();
            original.PixelLightCount = QualitySettings::PixelLightCount();
            original.LodBias = QualitySettings::LodBias();
            original.Fog = ::RenderSettings::Fog();
            original.SoftParticles = QualitySettings::SoftParticles();
            original.SoftVegetation = QualitySettings::SoftVegetation();
            original.ReflectionProbes = QualitySettings::RealtimeReflectionProbes();
            original.Ready = true;
        }

        QualitySettings::Shadows(G->PerfDisableShadows ? 0 : original.Shadows);
        QualitySettings::ShadowDistance(G->PerfDisableShadows ? 0.f : original.ShadowDistance);
        QualitySettings::AntiAliasing(G->PerfDisableAntiAliasing ? 0 : original.AntiAliasing);
        QualitySettings::PixelLightCount(G->PerfDisablePixelLights ? 0 : original.PixelLightCount);
        QualitySettings::LodBias(G->PerfDisableLodBias ? 0.f : original.LodBias);
        ::RenderSettings::Fog(G->PerfDisableFog ? false : original.Fog);
        QualitySettings::SoftParticles(G->PerfDisableSoftParticles ? false : original.SoftParticles);
        QualitySettings::SoftVegetation(G->PerfDisableSoftVegetation ? false : original.SoftVegetation);
        QualitySettings::RealtimeReflectionProbes(G->PerfDisableReflectionProbes ? false : original.ReflectionProbes);

        if (G->PerfDisablePostProcessing && now >= nextPostProcessScan)
        {
            nextPostProcessScan = now + 2000;
            static constexpr const char* effectTokens[] = { "post", "color", "grade", "grain", "vignette", "bloom", "blur", "tone" };
            auto sceneCameras = Unity::Object::FindObjectsOfType<Unity::Camera>();
            for (int c = 0; c < sceneCameras.Length(); ++c)
            {
                Unity::Camera sceneCamera = sceneCameras[c];
                if (!sceneCamera)
                    continue;
                auto components = sceneCamera.GetComponentsInChildren(Unity::Behaviour::typeof(), true);
                for (int i = 0; i < components.Length(); ++i)
                {
                    Unity::Behaviour component = std::bit_cast<Unity::Behaviour>(components[i]);
                    if (!component)
                        continue;
                    std::string typeName = component.GetType().Name();
                    std::transform(typeName.begin(), typeName.end(), typeName.begin(), [](unsigned char ch) { return (char)std::tolower(ch); });
                    bool isEffect = false;
                    for (const char* token : effectTokens)
                        if (typeName.find(token) != std::string::npos) { isEffect = true; break; }
                    if (!isEffect)
                        continue;

                    auto found = std::find_if(postProcessLayers.begin(), postProcessLayers.end(), [component](const ComponentState& entry) { return entry.Component == component; });
                    if (found == postProcessLayers.end())
                        postProcessLayers.push_back({component, component.GetEnabled()});
                    component.SetEnabled(false);
                }
            }
        }

        for (size_t i = postProcessLayers.size(); i-- > 0;)
        {
            if (!postProcessLayers[i].Component)
            {
                postProcessLayers.erase(postProcessLayers.begin() + i);
                continue;
            }
            if (!G->PerfDisablePostProcessing)
            {
                postProcessLayers[i].Component.SetEnabled(postProcessLayers[i].WasEnabled);
                postProcessLayers.erase(postProcessLayers.begin() + i);
            }
        }

        // Do not retain or access Camera wrappers from this hook. Unity tears
        // down the old scene asynchronously during level changes; a wrapper may
        // still be truthy after its native Camera has been destroyed. Reading
        // clip planes from that stale object can crash the process.
    }

    void HookMonoRuntimeInvoke()
    {
        HookModuleProc((HMODULE)Hax::Unity::GetUvmHandle(), "mono_runtime_invoke", Hooked_MonoRuntimeInvoke, G->MonoRuntimeInvokeHook);
        ::FlushInstructionCache(::GetCurrentProcess(), nullptr, 0);
        ::Sleep(20);
    }

    void HookGameFuncs()
    {
        Hax::LogDebug(G->Logger, L"Installing game hooks...");

        #define HOOK(ptr, n) Hook(ptr, Hooked__ ## n, G-> ## n ## _Hook, #n)
        HOOK(::EventSystem::s_Update.Address, EventSystem_Update);
        HOOK(PlayerHealth::s_Hurt.Address, PlayerHealth_Hurt);
        HOOK(PlayerAvatar::s_PlayerDeath.Address, PlayerAvatar_PlayerDeath);
        HOOK(PlayerController::s_FixedUpdate.Address, PlayerController_FixedUpdate);
        HOOK(PlayerController::s_Update.Address, PlayerController_Update);
        HOOK(PlayerTumble::s_TumbleRequest.Address, PlayerTumble_TumbleRequest);
        HOOK(FlashlightController::s_Update.Address, FlashlightController_Update);
        HOOK(ItemUpgrade::s_Update.Address, ItemUpgrade_Update);
        HOOK(ItemHealthPack::s_Update.Address, ItemHealthPack_Update);
        HOOK(ItemAttributes::s_Update.Address, ItemAttributes_Update);
        HOOK(InventorySpot::s_Update.Address, InventorySpot_Update);
        HOOK(SemiUI::s_Hide.Address, SemiUI_Hide);
        HOOK(RunManager::s_ChangeLevel.Address, RunManager_ChangeLevel);

        HOOK(PhysGrabObject::s_PhysicsGrabbingManipulation.Address, PhysGrabObject_PhysicsGrabbingManipulation);
        HOOK(::PostProcessLayer::s_BuildCommandBuffers.Address, PostProcessLayer_BuildCommandBuffers);
        HOOK(PlayerAvatarVisuals::s_Update.Address, PlayerAvatarVisuals_Update);
        HOOK(PhysGrabber::s_PhysGrabLogic.Address, PhysGrabber_PhysGrabLogic);
        HOOK(PhysGrabber::s_RayCheck.Address, PhysGrabber_RayCheck);
        HOOK(MusicBoxTrap::s_Update.Address, MusicBoxTrap_Update);
        HOOK(::Physics::s_Raycast.Address, Physics_Raycast);
        HOOK(PhysGrabObjectImpactDetector::s_Break.Address, PhysGrabObjectImpactDetector_Break);
        HOOK(PhysGrabObjectImpactDetector::s_DestroyObject.Address, PhysGrabObjectImpactDetector_DestroyObject);
        HOOK(ItemBattery::s_Update.Address, ItemBattery_Update);
        HOOK(::Camera::s_FireOnPreRender.Address, Camera_FireOnPreRender);
        HOOK(ItemGun::s_Update.Address, ItemGun_Update);
        HOOK(RunManager::s_SetRunLevel.Address, RunManager_SetRunLevel);
        HOOK(PhysGrabber::s_PhysGrabOverCharge.Address, PhysGrabber_PhysGrabOverCharge);
        HOOK(SpectateCamera::s_HeadEnergyLogic.Address, SpectateCamera_HeadEnergyLogic);
        HOOK(EnemyRigidbody::s_FixedUpdate.Address, EnemyRigidbody_FixedUpdate);
        #undef HOOK

        ::FlushInstructionCache(::GetCurrentProcess(), nullptr, 0);
    }

    void HookPlayerLoop()
    {
        HAX_PANIC(G->PlayerLoop.ReadyToHook, &G->Logger, L"LoopSystem not ready");

        Hook(G->PlayerLoop.UpdatePtr, Hooked__UpdateLoop, G->UpdateHook, "LoopSystem.Update");
        if (G->PlayerLoop.FixedPhysicsPtr) Hook(G->PlayerLoop.FixedPhysicsPtr, Hooked__FixedPhysicsLoop, G->FixedPhysicsLoop_Hook, "LoopSystem.FixedUpdate.Physics");
        if (G->PlayerLoop.FixedScriptsPtr) Hook(G->PlayerLoop.FixedScriptsPtr, Hooked__FixedScriptsLoop, G->FixedScriptsLoop_Hook, "LoopSystem.FixedUpdate.Scripts");
        if (G->PlayerLoop.LateScriptsPtr) Hook(G->PlayerLoop.LateScriptsPtr, Hooked__LateScriptsLoop, G->LateScriptsLoop_Hook, "LoopSystem.LateUpdate.Scripts");
        Hook(G->PlayerLoop.PostLateUpdatePtr, Hooked__PostLateUpdateLoop, G->PostLateUpdateHook, "LoopSystem.PostLateUpdate");
        Hook(G->PlayerLoop.PresentAfterDrawPtr, Hooked__PresentAfterDrawLoop, G->PresentAfterDrawHook, "LoopSystem.PresentAfterDrawLoop");
    }

    static void* Hooked_MonoRuntimeInvoke(void* a1, void* a2, void* a3, void* a4)
    {
        using GetMethodName = const char*(*)(void*);
        using GetMethodClass = void*(*)(void*);
        using GetClassName = const char*(*)(void*);
        static HMODULE mono = (HMODULE)Hax::Unity::GetUvmHandle();
        static GetMethodName getMethodName = (GetMethodName)::GetProcAddress(mono, "mono_method_get_name");
        static GetMethodClass getMethodClass = (GetMethodClass)::GetProcAddress(mono, "mono_method_get_class");
        static GetClassName getClassName = (GetClassName)::GetProcAddress(mono, "mono_class_get_name");

        const bool benchmark = IsBenchmarkCaptureEnabled() && G && G->IsInGame;
        const bool waitingForUnity = G && G->UnityLoadedEvent != nullptr;
        const char* methodName = (benchmark || waitingForUnity) && getMethodName && a1 ? getMethodName(a1) : nullptr;
        const bool trackMethod = benchmark && methodName &&
            (strcmp(methodName, "Update") == 0 || strcmp(methodName, "FixedUpdate") == 0 || strcmp(methodName, "LateUpdate") == 0);
        LARGE_INTEGER start{}, end{};
        if (trackMethod) ::QueryPerformanceCounter(&start);
        void* ret = G->MonoRuntimeInvokeHook.unsafe_call<void*>(a1, a2, a3, a4);
        if (trackMethod)
        {
            ::QueryPerformanceCounter(&end);
            const char* className = nullptr;
            if (!BenchmarkManagedMethodKnown(a1) && getMethodClass && getClassName)
            {
                void* methodClass = getMethodClass(a1);
                className = methodClass ? getClassName(methodClass) : nullptr;
            }
            BenchmarkRecordManagedMethod(a1, methodName, className,
                end.QuadPart > start.QuadPart ? (unsigned long long)(end.QuadPart - start.QuadPart) : 0);
        }

        // mono_runtime_invoke writes the managed exception object through its
        // fourth argument. Record it even when caught by game/plugin code.
        void* managedException = a4 ? *reinterpret_cast<void**>(a4) : nullptr;
        if (managedException)
        {
            if (!methodName && getMethodName && a1) methodName = getMethodName(a1);
            static LONG loggedManagedExceptions = 0;
            const LONG sequence = ::InterlockedIncrement(&loggedManagedExceptions);
            if (sequence <= 256 || (sequence % 128) == 0)
                Hax::LogError(G->Logger, L"Managed exception #%ld in %hs; object=%p (later repeats sampled)",
                    sequence, methodName ? methodName : "<unknown method>", managedException);
        }

        if (G && G->UnityLoadedEvent != nullptr)
        {
            if (!methodName && getMethodName && a1) methodName = getMethodName(a1);
            if (methodName && strcmp(methodName, "Update") == 0)
            {
                ::SetEvent(G->UnityLoadedEvent);
                G->UnityLoadedEvent = nullptr;
            }
        }
        return ret;
    }

    static void Hooked__EventSystem_Update(Unity::EventSystem __this)
    {
        static bool hasLastState = false;
        static bool lastInGame = false;
        static bool lastInShop = false;
        static bool lastIsClient = false;
        try
        {
            if (__this != Unity::EventSystem::GetCurrent())
                return;

            PlayerAvatar avatar = PlayerAvatar::instance();
            PlayerHealth health = avatar ? avatar.playerHealth() : nullptr;

            G->IsClient = !SemiFunc::IsMasterClientOrSingleplayer();
            G->IsInGame = IsInGame();
            if (RunManager run = RunManager::instance()) G->IsInShop = run.levelIsShop();
            else G->IsInShop = false;
            if (!hasLastState || lastInGame != G->IsInGame || lastInShop != G->IsInShop || lastIsClient != G->IsClient)
            {
                Hax::Log(G->Logger, L"State changed: inGame=%d shop=%d client=%d hostOrOffline=%d",
                    G->IsInGame, G->IsInShop, G->IsClient, !G->IsClient);
                hasLastState = true;
                lastInGame = G->IsInGame;
                lastInShop = G->IsInShop;
                lastIsClient = G->IsClient;
            }
            UpdateRuntimeSnapshot();
            UpdatePerformanceSettings();

            if (G->LoadIdentityName)
            {
                G->LoadIdentityName = false;
                PlayerController controller = PlayerController::instance();
                if (controller && controller.playerName() != nullptr && !G->ApplyIdentityName && wcscmp(G->IdentityName, L"Player") == 0)
                    wcsncpy_s(G->IdentityName, _countof(G->IdentityName), controller.playerName().GetRawStringData(), _TRUNCATE);
            }

            if (G->ApplyIdentityName)
            {
                G->ApplyIdentityName = false;
                PlayerController controller = PlayerController::instance();
                PlayerAvatar localAvatar = PlayerAvatar::instance();
                size_t len = 0;
                while (len < _countof(G->IdentityName) && G->IdentityName[len]) ++len;
                if (len > 0)
                {
                    UpdateNameChangerConfig(G->IdentityName);
                    if (controller && localAvatar)
                        controller.PlayerSetName(System::String::New(Hax::WStringView{G->IdentityName, len}), SemiFunc::PlayerGetSteamID(localAvatar));
                }
            }

            if (G->RandomizeAvatar)
            {
                G->RandomizeAvatar = false;
                static bool randomSeeded = false;
                if (!randomSeeded)
                {
                    std::srand(GetTickCount());
                    randomSeeded = true;
                }
                if (MetaManager meta = MetaManager::instance())
                {
                    auto equipped = meta.cosmeticEquipped();
                    auto assets = meta.cosmeticAssets();
                    for (int i = 0; equipped != nullptr && i < equipped.Count(); ++i)
                    {
                        int index = equipped[i];
                        if (index >= 0 && index < assets.Count() && assets[index])
                            meta.CosmeticUnequip(assets[index], false, false);
                    }

                    auto unlocked = meta.cosmeticUnlocks();
                    for (int type = 0; type <= 32; ++type)
                    {
                        int chosen = -1;
                        int seen = 0;
                        for (int i = 0; unlocked != nullptr && i < unlocked.Count(); ++i)
                        {
                            int index = unlocked[i];
                            if (index < 0 || index >= assets.Count() || !assets[index] || assets[index].type() != type)
                                continue;
                            if ((std::rand() % ++seen) == 0)
                                chosen = index;
                        }
                        if (chosen >= 0)
                            meta.CosmeticEquip(assets[chosen], false);
                    }

                    auto colors = meta.colors();
                    auto equippedColors = meta.colorsEquipped();
                    for (int i = 0; colors != nullptr && equippedColors != nullptr && i < equippedColors.Length(); ++i)
                        if (colors.Count() > 0)
                            meta.CosmeticColorSet(i, std::rand() % colors.Count());

                    meta.CosmeticPreviewSet(false);
                    meta.CosmeticPlayerUpdateLocal(false, false);
                }
            }

            if (!G->PlayerLoop.ReadyToHook)
            {
                G->PlayerLoop.UpdatePtr = GetPlayerLoopPtr(Unity::PlayerLoop::Update::typeof(), Unity::PlayerLoop::ScriptRunBehaviourUpdate::typeof());
                Hax::LogDebug(G->Logger, L"m_UpdatePtr ptr = %p", G->PlayerLoop.UpdatePtr);

                const char* loopAssembly = "UnityEngine.CoreModule";
                const char* loopNamespace = "UnityEngine.PlayerLoop";
                G->PlayerLoop.FixedPhysicsPtr = GetPlayerLoopPtr(
                    System::Type::GetType(loopAssembly, loopNamespace, "FixedUpdate", false),
                    System::Type::GetType(loopAssembly, loopNamespace, "FixedUpdate/PhysicsFixedUpdate", false));
                G->PlayerLoop.FixedScriptsPtr = GetPlayerLoopPtr(
                    System::Type::GetType(loopAssembly, loopNamespace, "FixedUpdate", false),
                    System::Type::GetType(loopAssembly, loopNamespace, "FixedUpdate/ScriptRunBehaviourFixedUpdate", false));
                G->PlayerLoop.LateScriptsPtr = GetPlayerLoopPtr(
                    System::Type::GetType(loopAssembly, loopNamespace, "PreLateUpdate", false),
                    System::Type::GetType(loopAssembly, loopNamespace, "PreLateUpdate/ScriptRunBehaviourLateUpdate", false));
                Hax::LogDebug(G->Logger, L"Benchmark phase hooks fixedPhysics=%p fixedScripts=%p lateScripts=%p",
                    G->PlayerLoop.FixedPhysicsPtr, G->PlayerLoop.FixedScriptsPtr, G->PlayerLoop.LateScriptsPtr);

                G->PlayerLoop.PostLateUpdatePtr = GetPlayerLoopPtr(Unity::PlayerLoop::PostLateUpdate::typeof(), Unity::PlayerLoop::PlayerSendFrameStarted::typeof());
                Hax::LogDebug(G->Logger, L"m_PostLateUpdatePtr ptr = %p", G->PlayerLoop.PostLateUpdatePtr);

                G->PlayerLoop.PresentAfterDrawPtr = GetPlayerLoopPtr(Unity::PlayerLoop::PostLateUpdate::typeof(), Unity::PlayerLoop::PresentAfterDraw::typeof());
                Hax::LogDebug(G->Logger, L"m_PresentAfterDrawPtr ptr = %p", G->PlayerLoop.PresentAfterDrawPtr);

                G->PlayerLoop.ReadyToHook = true;
            }

            if (G->HealToMax)
            {
                G->HealToMax = false;
                if (health)
                    health.HealOther(99999, true);
            }

            if (G->UpgradeToToggle)
            {
                ItemUpgrade item = G->UpgradeToToggle;
                G->UpgradeToToggle = nullptr;

                ItemToggle toggle = item.itemToggle();
                if (toggle)
                {
                    toggle.photonView() = item.photonView();
                    toggle.ToggleItem(true, SemiFunc::PhotonViewIDPlayerAvatarLocal());
                }
            }

            if (G->AidToToggle)
            {
                ItemHealthPack item = G->AidToToggle;
                G->AidToToggle = nullptr;

                ItemToggle toggle = item.itemToggle();
                if (toggle)
                {
                    toggle.photonView() = item.photonView();
                    toggle.ToggleItem(true, SemiFunc::PhotonViewIDPlayerAvatarLocal());
                }
            }

            if (G->ItemToGrab)
            {
                ItemAttributes attributes = G->ItemToGrab;
                if (!G->IsInGame || (G->IsInShop && !G->ShopInventory) ||
                    GetTickCount64() >= G->ItemGrabDeadline || !attributes)
                {
                    G->ItemToGrab = nullptr;
                    G->ItemGrabReplacingSlot = false;
                    G->ItemGrabDeadline = 0;
                }
                else if (Inventory inventory = Inventory::instance(); inventory && inventory.spotsFeched())
                {
                    ItemEquippable equippable = attributes.GetComponent<ItemEquippable>();
                    if (!equippable || equippable.IsEquipped())
                    {
                        G->ItemToGrab = nullptr;
                        G->ItemGrabReplacingSlot = false;
                        G->ItemGrabDeadline = 0;
                    }
                    else
                    {
                        const int freeSpot = inventory.GetFirstFreeInventorySpotIndex();
                        if (freeSpot >= 0)
                        {
                            equippable.RequestEquip(freeSpot, SemiFunc::PhotonViewIDPlayerAvatarLocal());
                            Hax::Log(G->Logger, L"Grab item: requested inventory slot %d", freeSpot + 1);
                            G->ItemToGrab = nullptr;
                            G->ItemGrabReplacingSlot = false;
                            G->ItemGrabDeadline = 0;
                        }
                        else if (!G->ItemGrabReplacingSlot)
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                InventorySpot spot = inventory.GetSpotByIndex(i);
                                if (!spot || !spot.IsOccupied()) continue;
                                ItemEquippable oldItem = spot.CurrentItem();
                                if (!oldItem) continue;
                                oldItem.RequestUnequip();
                                G->ItemGrabReplacingSlot = true;
                                Hax::Log(G->Logger, L"Grab item: inventory full; dropping slot %d before equipping selected item", i + 1);
                                break;
                            }
                        }
                    }
                }
            }

            if (G->EnemiesPool.Empty())
                ParseEnemies();

            if (G->EnemyAction)
            {
                const int action = G->EnemyAction;
                G->EnemyAction = 0;
                EnemyParent parent = G->SelectedManageEnemy;
                Enemy enemy{};
                if (parent) enemy = parent.enemy();
                EnemyHealth enemyHealth = enemy ? enemy.Health() : nullptr;
                if (G->IsInGame && parent && parent.Spawned() && enemy && enemyHealth && !enemyHealth.dead())
                {
                    switch (action)
                    {
                    case 1:
                        if (!G->IsClient) enemyHealth.Hurt(Hax::Max(1, enemyHealth.healthCurrent()), Unity::Vector3{});
                        break;
                    case 2:
                        if (!G->IsClient && avatar && enemy.Rigidbody() && enemy.Rigidbody().followTarget())
                        {
                            EnemyRigidbody body = enemy.Rigidbody();
                            Unity::Vector3 target = avatar.GetTransform().GetPosition() + avatar.GetTransform().GetForward() * 2.f;
                            body.followTarget().SetPosition(target - Unity::Vector3(0.f, body.yOffset(), 0.f));
                            body.Teleport();
                        }
                        break;
                    case 3: if (!G->IsClient) enemyHealth.healthCurrent() = enemyHealth.health(); break;
                    case 4: if (!G->IsClient) enemyHealth.healthCurrent() = 1; break;
                    case 5:
                        if (!G->IsClient)
                        {
                            enemy.SetEnabled(false);
                            if (enemy.Rigidbody()) enemy.Rigidbody().SetEnabled(false);
                        }
                        break;
                    case 6:
                        if (!G->IsClient)
                        {
                            enemy.SetEnabled(true);
                            if (enemy.Rigidbody()) enemy.Rigidbody().SetEnabled(true);
                        }
                        break;
                    case 7:
                    {
                        PlayerAvatar target = G->IsClient ? PlayerAvatar::instance() : G->AggroTargetPlayer;
                        if (!target || target.isDisabled())
                            break;
                        if (G->IsClient)
                        {
                            EnemyRigidbody body = enemy.Rigidbody();
                            PhysGrabObject targetObject = body ? body.physGrabObject() : nullptr;
                            if (targetObject)
                                SemiFunc::PhysGrabberLocalForceGrab(targetObject);
                        }
                        else
                            enemy.SetChaseTarget(target);
                        break;
                    }
                    }
                }
            }

            if (G->ItemsPool.Empty())
                ParseItems();

            if (G->ValuablesPool.Empty())
                ParseValuables();

            if (G->LevelBans.Empty())
                ParseLevels();

            if (G->EnemyToSpawn)
            {
                EnemySetup enemy = G->EnemyToSpawn;
                G->EnemyToSpawn = null;

                if (LevelGenerator gen = LevelGenerator::Instance())
                {
                    System::List<LevelPoint> points = gen.LevelPathPoints();
                    if (points != null && points.Count() > 0)
                    {
                        LevelPoint point = points[0];
                        if (point)
                            gen.EnemySpawn(enemy, point.GetTransform().GetPosition());
                    }
                }
            }

            if (G->CommandBuffer == null)
            {
                G->CommandBuffer = CommandBuffer::New();
                System::GCHandle::Alloc(G->CommandBuffer, true);
            }

            if (G->Bundle == null)
            {
                auto ptr = Hax::Gui::GetResourceData((Hax::Handle)G->Handle, IDR_BUNDLE1, L"BUNDLE");

                auto bundleRaw = Array<Char>::CreateInstance(ptr.Size());

                memcpy(bundleRaw.begin(), ptr.Data(), ptr.Size());
                G->Bundle = AssetBundle::LoadFromMemory(bundleRaw);
                G->Bundle.SetHideFlags(HideFlags::DontUnloadUnusedAsset);

                Hax::LogDebug(G->Logger, L"Bundle loaded");
            }

            if (!G->ItemsChamsMat && G->Bundle)
            {
                Shader shader = As<Shader>(G->Bundle.LoadAsset(String::New("assets/myshader.shader"), Shader::typeof()));

                G->ItemsChamsMat = Material::New(shader);
                G->ItemsChamsMat.SetHideFlags(HideFlags::DontUnloadUnusedAsset);
                G->ItemsChamsMat.SetColor(Color(1.f, 1.f, 0.9f, 1.f));
                GCHandle::Alloc(G->ItemsChamsMat, true);
                Hax::LogDebug(G->Logger, L"Items chams mat loaded");

                G->PlayerChamsMat = Material::New(shader);
                G->PlayerChamsMat.SetHideFlags(HideFlags::DontUnloadUnusedAsset);
                G->PlayerChamsMat.SetColor(Color::green());
                GCHandle::Alloc(G->PlayerChamsMat, true);
                Hax::LogDebug(G->Logger, L"Player chams mat loaded");

                G->HeadChamsMat = Material::New(shader);
                G->HeadChamsMat.SetHideFlags(HideFlags::DontUnloadUnusedAsset);
                G->HeadChamsMat.SetColor(Color(0x54 / 255.f, 0x54 / 255.f, 0x54 / 255.f, 1.f));
                GCHandle::Alloc(G->HeadChamsMat, true);
                Hax::LogDebug(G->Logger, L"Head chams mat loaded");
            }

            if (G->SetToZero)
            {
                G->SetToZero = false;
                if (ValuableDirector dir = ValuableDirector::instance())
                {
                    for (ValuableObject obj : dir.valuableList())
                    {
                        if (obj)
                            Heal(obj.physGrabObject().impactDetector(), -999999.f);
                    }
                }
            }

            if (G->SetToMax)
            {
                G->SetToMax = false;
                if (ValuableDirector dir = ValuableDirector::instance())
                {
                    for (ValuableObject obj : dir.valuableList())
                    {
                        if (obj)
                            Heal(obj.physGrabObject().impactDetector(), 999999.f);
                    }
                }
            }

            if (G->ItemToSpawn)
            {
                Item item = G->ItemToSpawn;
                G->ItemToSpawn = nullptr;
                SpawnItem(item);
            }

            if (G->ValuableToSpawn != nullptr)
            {
                PrefabRef prefab = G->ValuableToSpawn;
                const int overrideValue = G->SpawnMoneyValue;
                G->ValuableToSpawn = nullptr;
                G->SpawnMoneyValue = 0;
                SpawnValuablePrefab(prefab, overrideValue);
            }

            if (G->ForceNextLevel)
            {
                G->ForceNextLevel = false;
                if (RunManager manager = RunManager::instance())
                    manager.ChangeLevel(true, false);
            }

            if (G->ActivateNextPoint)
            {
                G->ActivateNextPoint = false;
                if (RoundDirector dir = RoundDirector::instance())
                {
                    for (Unity::GameObject go : dir.extractionPointList())
                    {
                        ExtractionPoint point = go.GetComponent<ExtractionPoint>();
                        if (point && point.currentState() == ExtractionPoint_State::Idle() && !point.isLocked())
                        {
                            point.OnClick();
                            break;
                        }
                    }
                }
            }

            if (G->TeleportToTruck)
            {
                G->TeleportToTruck = false;
                if (avatar && !avatar.deadSet())
                    avatar.playerTransform().SetPosition(TruckSafetySpawnPoint::instance().GetTransform().GetPosition());
            }

            if (G->PlayerToTumble)
            {
                PlayerAvatar player = G->PlayerToTumble;
                G->PlayerToTumble = nullptr;
                if (player && !player.isDisabled())
                    player.tumble().TumbleRequest(true, false);
            }

            if (G->PlayerToKill)
            {
                PlayerAvatar player = G->PlayerToKill;
                G->PlayerToKill = nullptr;
                if (player && !player.deadSet() && !G->IsClient)
                    player.PlayerDeath(-1);
            }

            if (G->PlayerToRevive)
            {
                PlayerAvatar player = G->PlayerToRevive;
                G->PlayerToRevive = nullptr;
                if (player && player.deadSet() && !G->IsClient)
                    player.Revive(false);
            }

            PlayerVoiceChat chat = avatar ? avatar.voiceChat() : nullptr;
            if (chat && avatar.voiceChatFetched() && G->ChatPrefered != ChatPref::None)
            {
                bool isDeadPrefered = G->ChatPrefered == ChatPref::Dead;
                if (chat.inLobbyMixer() != isDeadPrefered)
                {
                    chat.ToggleMixer(isDeadPrefered, false);
                    Hax::LogDebug(G->Logger, L"Chat changed");
                }
                G->ChatPrefered = ChatPref::None;
            }

            if (G->UnlockAllCosmetic && MetaManager::instance())
            {
                G->UnlockAllCosmetic = false;
                MetaManager::instance().CosmeticUnlockAll();
            }

            if (G->ResetAllCosmetic && MetaManager::instance())
            {
                G->ResetAllCosmetic = false;
                MetaManager::instance().Reset();
            }

            if (G->RarityToSpawn > 0)
            {
                int rarity = G->RarityToSpawn;
                G->RarityToSpawn = 0;
                SpawnCosmeticBox(rarity - 1);
            }
        }
        catch (Exception& ex)
        {
            String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->EventSystem_Update_Hook.unsafe_call<void, Unity::EventSystem>(__this);
    }

    static void Hooked__PlayerHealth_Hurt(PlayerHealth __this, int damage, bool savingGrace, int enemyIndex)
    {
        if (G->Godmode)
            return;

        G->PlayerHealth_Hurt_Hook.unsafe_call<void, PlayerHealth, int, bool, int>(__this, damage, savingGrace, enemyIndex);
    }

    static void Hooked__PlayerAvatar_PlayerDeath(PlayerAvatar __this, int enemyIndex)
    {
        if (G->Godmode && __this.isLocal())
            return;

        G->PlayerAvatar_PlayerDeath_Hook.unsafe_call<void, PlayerAvatar, int>(__this, enemyIndex);
    }

    static void Hooked__PlayerController_FixedUpdate(PlayerController __this)
    {
        try
        {
            float& walkSpeed = __this.MoveSpeed();
            float& sprintSpeed = __this.SprintSpeed();
            float& crouchSpeed = __this.CrouchSpeed();

            float backup[3] = { walkSpeed, sprintSpeed, crouchSpeed };

            if (G->InfStamina)
                __this.EnergyCurrent() = __this.EnergyStart();

            walkSpeed *= (float)G->Accel.Walking;
            sprintSpeed *= (float)G->Accel.Sprinting;
            crouchSpeed *= (float)G->Accel.Crouching;

            G->PlayerController_FixedUpdate_Hook.unsafe_call<void, PlayerController>(__this);

            walkSpeed = backup[0];
            sprintSpeed = backup[1];
            crouchSpeed = backup[2];
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");

            G->PlayerController_FixedUpdate_Hook.unsafe_call<void, PlayerController>(__this);
        }
    }

    static void Hooked__PlayerController_Update(PlayerController __this)
    {
        int& jumps = __this.JumpExtra();
        int cachedJumps = jumps;
        if (G->InfJumps)
            jumps = 9999;

        G->PlayerController_Update_Hook.unsafe_call<void, PlayerController>(__this);

        jumps = cachedJumps;
    }

    static void Hooked__PlayerTumble_TumbleRequest(PlayerTumble __this, bool isTumbling, bool playerInput)
    {
        if (G->NoTumble && __this.playerAvatar().isLocal() && !playerInput)
            return;

        G->PlayerTumble_TumbleRequest_Hook.unsafe_call<void, PlayerTumble, bool, bool>(__this, isTumbling, playerInput);
    }

    static void Hooked__FlashlightController_Update(FlashlightController __this)
    {
        try
        {
            PlayerAvatar avatar = PlayerAvatar::instance();

            bool wasCrouching = false;
            if (G->Flashlight.InCrouch && avatar)
            {
                wasCrouching = avatar.isCrouching();
                avatar.isCrouching() = false;
            }

            G->FlashlightController_Update_Hook.unsafe_call<void, FlashlightController>(__this);

            if (G->Flashlight.InCrouch && avatar)
                avatar.isCrouching() = wasCrouching;

            Unity::Light spotLight = __this.spotlight();
            if ((float)G->Flashlight.Angle != spotLight.GetSpotAngle())
            {
                spotLight.SetSpotAngle((float)G->Flashlight.Angle);
            }

            spotLight.SetIntensity(spotLight.GetIntensity() * (G->Flashlight.Intensity / 10.f));
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");

            G->FlashlightController_Update_Hook.unsafe_call<void, FlashlightController>(__this);
        }
    }

    static UpgradeData* GetUpgradeInfoFromName(System::String name)
    {
        for (int i = 0; i < (int)UpgradeType::N; ++i)
        {
            if (name.ToHaxView().EndsWith(G->UpgradesData[i].InternalNameEnd))
                return &G->UpgradesData[i];
        }
        return nullptr;
    }

    static void Hooked__ItemUpgrade_Update(ItemUpgrade __this)
    {
        try
        {
            G->UpgradesPool.GetBack().PushBack(__this);

            if (G->AutoUseUpgrs)
            {
                UpgradeData* data = GetUpgradeInfoFromName(__this.itemAttributes().itemAssetName());
                if (data != nullptr && data->AutoUse)
                {
                    ItemToggle toggle = __this.itemToggle();
                    if (toggle)
                    {
                        toggle.photonView() = __this.photonView();
                        toggle.ToggleItem(true, SemiFunc::PhotonViewIDPlayerAvatarLocal());
                    }
                }
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->ItemUpgrade_Update_Hook.unsafe_call<void, ItemUpgrade>(__this);
    }

    static void Hooked__ItemHealthPack_Update(ItemHealthPack __this)
    {
        if (!RunManager::instance().levelIsShop())
            G->AidsPool.GetBack().PushBack(__this);

        G->ItemHealthPack_Update_Hook.unsafe_call<void, ItemHealthPack>(__this);
    }

    static void Hooked__ItemAttributes_Update(ItemAttributes __this)
    {
        if (__this && G->IsInGame)
            G->DroppedItemsPool.GetBack().PushBack(__this);
        G->ItemAttributes_Update_Hook.unsafe_call<void, ItemAttributes>(__this);
    }

    static void Hooked__InventorySpot_Update(InventorySpot __this)
    {
        G->InventorySpot_Update_Hook.unsafe_call<void, InventorySpot>(__this);

        static bool keyWasDown[3]{};
        if (!G->ShopInventory || !G->IsInGame || !G->IsInShop)
        {
            keyWasDown[0] = keyWasDown[1] = keyWasDown[2] = false;
            return;
        }
        if (!__this)
            return;

        const int index = __this.inventorySpotIndex();
        if (index < 0 || index >= 3)
            return;

        // Read the physical number key directly. The game's InputDown helper is
        // consumed by the shop UI on some paths, so InventorySpot never sees it.
        const bool keyDown = (GetAsyncKeyState('1' + index) & 0x8000) != 0;
        const bool justPressed = keyDown && !keyWasDown[index];
        keyWasDown[index] = keyDown;
        if (!justPressed || __this.IsOccupied())
            return;

        PlayerAvatar avatar = PlayerAvatar::instance();
        PhysGrabber grabber = avatar ? avatar.physGrabber() : nullptr;
        PhysGrabObject held = grabber && grabber.grabbed() ? grabber.grabbedPhysGrabObject() : nullptr;
        ItemEquippable equippable = held ? held.GetComponent<ItemEquippable>() : nullptr;
        if (!equippable && held)
            equippable = held.GetComponentInChildren<ItemEquippable>();
        for (Unity::Transform node = held ? held.GetTransform().GetParent() : nullptr;
            !equippable && node; node = node.GetParent())
        {
            equippable = node.GetComponent<ItemEquippable>();
            if (!equippable)
                equippable = node.GetComponentInChildren<ItemEquippable>();
        }

        if (!equippable || equippable.IsEquipped())
        {
            Hax::LogDebug(G->Logger, L"Shop inventory: key %d pressed but no unequipped held item was found", index + 1);
            return;
        }

        equippable.RequestEquip(index, SemiFunc::PhotonViewIDPlayerAvatarLocal());
        Hax::Log(G->Logger, L"Shop inventory: requested normal equip into slot %d", index + 1);
    }

    static void Hooked__SemiUI_Hide(SemiUI __this)
    {
        InventoryUI inventoryUi = InventoryUI::instance();
        if (G->ShopInventory && G->IsInShop && inventoryUi && __this == inventoryUi)
            return;
        G->SemiUI_Hide_Hook.unsafe_call<void, SemiUI>(__this);
    }

    static void Hooked__RunManager_ChangeLevel(RunManager __this, bool completedLevel, bool levelFailed, int changeLevelType)
    {
        G->RunManager_ChangeLevel_Hook.unsafe_call<void, RunManager, bool, bool, int>(
            __this, completedLevel, levelFailed, changeLevelType);
    }

    static void Hooked__UpdateLoop()
    {
        LARGE_INTEGER benchStart{}; const bool bench = IsBenchmarkCaptureEnabled();
        static LONGLONG previousFrameStart = 0;
        if (bench)
        {
            ::QueryPerformanceCounter(&benchStart);
            if (previousFrameStart && benchStart.QuadPart > previousFrameStart)
                BenchmarkRecordFrameInterval((unsigned long long)(benchStart.QuadPart - previousFrameStart));
            previousFrameStart = benchStart.QuadPart;
        }
        else previousFrameStart = 0;
        G->UpgradesPool.GetBack().Clear();
        G->AidsPool.GetBack().Clear();
        G->DroppedItemsPool.GetBack().Clear();

        G->UpdateHook.unsafe_call<void>();

        G->UpgradesPool.RefreshSpare();
        G->AidsPool.RefreshSpare();
        G->DroppedItemsPool.RefreshSpare();
        if (bench)
        {
            LARGE_INTEGER benchEnd{}; ::QueryPerformanceCounter(&benchEnd);
            const unsigned long long ticks = (unsigned long long)(benchEnd.QuadPart - benchStart.QuadPart);
            BenchmarkRecordFrame(ticks);
            BenchmarkRecordSection(0, ticks);
            BenchmarkCaptureUnityCounts();
        }
    }

    static void Hooked__FixedPhysicsLoop()
    {
        LARGE_INTEGER s{}, e{}; const bool b = IsBenchmarkCaptureEnabled(); if (b) ::QueryPerformanceCounter(&s);
        G->FixedPhysicsLoop_Hook.unsafe_call<void>();
        if (b) { ::QueryPerformanceCounter(&e); BenchmarkRecordSection(4, (unsigned long long)(e.QuadPart - s.QuadPart)); }
    }

    static void Hooked__FixedScriptsLoop()
    {
        LARGE_INTEGER s{}, e{}; const bool b = IsBenchmarkCaptureEnabled(); if (b) ::QueryPerformanceCounter(&s);
        G->FixedScriptsLoop_Hook.unsafe_call<void>();
        if (b) { ::QueryPerformanceCounter(&e); BenchmarkRecordSection(5, (unsigned long long)(e.QuadPart - s.QuadPart)); }
    }

    static void Hooked__LateScriptsLoop()
    {
        LARGE_INTEGER s{}, e{}; const bool b = IsBenchmarkCaptureEnabled(); if (b) ::QueryPerformanceCounter(&s);
        G->LateScriptsLoop_Hook.unsafe_call<void>();
        if (b) { ::QueryPerformanceCounter(&e); BenchmarkRecordSection(6, (unsigned long long)(e.QuadPart - s.QuadPart)); }
    }

    static Unity::Transform g_ThirdPersonCameraTransform = nullptr;
    static Unity::Vector3 g_ThirdPersonCameraOriginalPosition{};
    static bool g_ThirdPersonCameraOffsetApplied = false;

    static void RestoreThirdPersonCamera()
    {
        if (g_ThirdPersonCameraOffsetApplied && g_ThirdPersonCameraTransform)
            g_ThirdPersonCameraTransform.SetLocalPosition(g_ThirdPersonCameraOriginalPosition);
        g_ThirdPersonCameraOffsetApplied = false;
    }

    static void SetThirdPerson()
    {
        static String playerTag = null;
        if (playerTag == null)
        {
            playerTag = System::String::New("Player");
            GCHandle::Alloc(playerTag, true);
        }

        Unity::Camera camera = SemiFunc::MainCamera();
        if (!camera)
            return;
        RestoreThirdPersonCamera();
        g_ThirdPersonCameraTransform = camera.GetTransform();
        if (!g_ThirdPersonCameraTransform)
            return;
        g_ThirdPersonCameraOriginalPosition = g_ThirdPersonCameraTransform.GetLocalPosition();

        PlayerAvatar avatar = PlayerAvatar::instance();

        float dist = 3.f;
        Transform normalTransformPivot = avatar.spectatePoint();
        static int layerMask = SemiFunc::LayerMaskGetVisionObstruct();
        auto hits = Unity::Physics::SphereCastAll(normalTransformPivot.GetPosition(), 0.1f, normalTransformPivot.GetForward() * -1.f, 3.f, layerMask);
        bool isMapOpen = avatar.mapToolController() && avatar.mapToolController().Active();
        if (hits != null && !isMapOpen)
        {
            for (RaycastHit& hit : hits)
            {
                Transform transform = hit.GetTransform();
                if (!transform.GetComponent<PlayerHealthGrab>() && !transform.GetGameObject().CompareTag(playerTag) && !transform.GetComponent<PlayerTumble>() && !transform.GetComponent<EnemyRigidbody>())
                    dist = Hax::Min(Hax::Max(1.f, hit.m_Distance), 3.f);
            }
        }
        else if (isMapOpen)
            dist = 1.f;

        g_ThirdPersonCameraTransform.SetLocalPosition(g_ThirdPersonCameraOriginalPosition + Unity::Vector3(0.f, 0.f, -dist));
        g_ThirdPersonCameraOffsetApplied = true;
    }

    static bool ShouldDrawEsp()
    {
        return G->IsInGame && MenuManager::instance() && !MenuManager::instance().currentMenuPage();
    }

    static float prevPlane;
    static float prevFov;
    static bool EspDataUpdateDue(ULONGLONG now)
    {
        static ULONGLONG nextUpdate = 0;
        static unsigned int frameCounter = 0;
        if (G->EspMatchFrameRate)
        {
            const int interval = Hax::Clamp(G->EspFrameInterval, 1, 10);
            return (++frameCounter % (unsigned int)interval) == 0;
        }
        frameCounter = 0;
        if (now < nextUpdate) return false;
        const int rate = Hax::Clamp(G->EspUpdateRate, 10, 60);
        nextUpdate = now + (1000 / rate);
        return true;
    }

    static void Hooked__PostLateUpdateLoop()
    {
        LARGE_INTEGER benchStart{}; const bool bench = IsBenchmarkCaptureEnabled();
        if (bench) ::QueryPerformanceCounter(&benchStart);
        LARGE_INTEGER espStart{};
        bool espCaptureActive = false;
        try
        {
            Unity::Camera mainCam = SemiFunc::MainCamera();
            PlayerAvatar avatar = PlayerAvatar::instance();
            if (mainCam && G->IsInGame && avatar)
            {
                if (G->BetterVision)
                {
                    prevPlane = mainCam.GetFarClipPlane();
                    if (prevPlane < 32.f)
                        mainCam.SetFarClipPlane(32.f);
                }

                prevFov = mainCam.GetFieldOfView();
                if (prevFov < (float)G->Fov)
                    mainCam.SetFieldOfView((float)G->Fov);

                if (!G->ThirdPerson || SpectateCamera::instance())
                    RestoreThirdPersonCamera();
                else
                    SetThirdPerson();

                const ULONGLONG now = GetTickCount64();
                if (ShouldDrawEsp() && EspDataUpdateDue(now))
                {
                    espCaptureActive = true;
                    if (bench) ::QueryPerformanceCounter(&espStart);
                    G->PixelHeight = (float)mainCam.GetPixelHeight();
                    G->PixelWidth = (float)mainCam.GetPixelWidth();
                    G->ScreenHeight = (float)Unity::Screen::GetHeight();
                    G->ScreenWidth = (float)Unity::Screen::GetWidth();

                    if (G->EnemiesEsp)
                    {
                        LARGE_INTEGER categoryStart{}; if (bench) ::QueryPerformanceCounter(&categoryStart);
                        auto& back = G->EnemiesEspBuffer.GetBack();
                        back.Clear();
                        if (EnemyDirector dir = EnemyDirector::instance())
                        {
                            for (EnemyParent parent : dir.enemiesSpawned())
                            {
                                Enemy enemy = parent.enemy();
                                if (enemy && parent.Spawned())
                                {
                                    Hax::Optional<EnemyEspData> data = ParseEnemyEspData(enemy);
                                    if (data.HasValue())
                                        back.PushBack(*data);
                                }
                            }
                        }
                        std::sort(back.begin(), back.end(), [](const EnemyEspData& a, const EnemyEspData& b) { return a.Distance > b.Distance; });
                        G->EnemiesEspBuffer.RefreshSpare();
                        if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(11, (unsigned long long)(e.QuadPart - categoryStart.QuadPart)); }
                    }

                    if (G->ValuablesEsp)
                    {
                        LARGE_INTEGER categoryStart{}; if (bench) ::QueryPerformanceCounter(&categoryStart);

                        auto& back = G->ValuablesEspBuffer.GetBack();
                        back.Clear();
                        if (ValuableDirector dir = ValuableDirector::instance())
                        {
                            for (ValuableObject obj : dir.valuableList())
                            {
                                Hax::Optional<ValuableEspData> data = ParseValuableEspData(obj);
                                if (data.HasValue())
                                    back.PushBack(*data);
                            }
                        }
                        std::sort(back.begin(), back.end(), [](const ValuableEspData& a, const ValuableEspData& b) { return a.Distance > b.Distance; });
                        G->ValuablesEspBuffer.RefreshSpare();
                        if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(12, (unsigned long long)(e.QuadPart - categoryStart.QuadPart)); }
                    }

                    if (G->ExtrPointsEsp)
                    {
                        LARGE_INTEGER categoryStart{}; if (bench) ::QueryPerformanceCounter(&categoryStart);
                        auto& back = G->ExtrPointsEspBuffer.GetBack();
                        back.Clear();
                        if (RoundDirector dir = RoundDirector::instance())
                        {
                            for (Unity::GameObject go : dir.extractionPointList())
                            {
                                Hax::Optional<ExtrPointEspData> data = ParseExtrPointEspData(go);
                                if (data.HasValue())
                                    back.PushBack(*data);
                            }
                        }
                        std::sort(back.begin(), back.end(), [](const ExtrPointEspData& a, const ExtrPointEspData& b) { return a.Distance > b.Distance; });
                        G->ExtrPointsEspBuffer.RefreshSpare();
                        if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(13, (unsigned long long)(e.QuadPart - categoryStart.QuadPart)); }
                    }

                    if (G->TruckEsp)
                    {
                        LARGE_INTEGER categoryStart{}; if (bench) ::QueryPerformanceCounter(&categoryStart);
                        auto& back = G->TruckEspBuffer.GetBack();
                        back.Clear();
                        if (TruckSafetySpawnPoint truck = TruckSafetySpawnPoint::instance())
                        {
                            Hax::Optional<TruckEspData> data = ParseTruckEspData(truck);
                            if (data.HasValue())
                                back.PushBack(*data);
                        }
                        G->TruckEspBuffer.RefreshSpare();
                        if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(14, (unsigned long long)(e.QuadPart - categoryStart.QuadPart)); }
                    }

                    if (G->PlayersEsp)
                    {
                        LARGE_INTEGER categoryStart{}; if (bench) ::QueryPerformanceCounter(&categoryStart);
                        auto& back = G->PlayersEspBuffer.GetBack();
                        back.Clear();
                        if (GameDirector dir = GameDirector::instance())
                        {
                            for (PlayerAvatar avatar : dir.PlayerList())
                            {
                                Hax::Optional<PlayerEspData> data = ParsePlayerEspData(avatar);
                                if (data.HasValue())
                                    back.PushBack(*data);
                            }
                        }
                        std::sort(back.begin(), back.end(), [](const PlayerEspData& a, const PlayerEspData& b) { return a.Distance > b.Distance; });
                        G->PlayersEspBuffer.RefreshSpare();
                        if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(15, (unsigned long long)(e.QuadPart - categoryStart.QuadPart)); }
                    }

                    if (G->CosmeticBoxesEsp)
                    {
                        LARGE_INTEGER categoryStart{}; if (bench) ::QueryPerformanceCounter(&categoryStart);
                        auto& back = G->CosmeticBoxesEspBuffer.GetBack();
                        back.Clear();
                        if (RoundDirector dir = RoundDirector::instance())
                        {
                            for (CosmeticWorldObject box : dir.cosmeticWorldObjects())
                            {
                                Hax::Optional<CosmeticBoxEspData> data = ParseCosmeticBoxEspData(box);
                                if (data.HasValue())
                                    back.PushBack(*data);
                            }
                        }
                        std::sort(back.begin(), back.end(), [](const CosmeticBoxEspData& a, const CosmeticBoxEspData& b) { return a.Distance > b.Distance; });
                        G->CosmeticBoxesEspBuffer.RefreshSpare();
                        if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(16, (unsigned long long)(e.QuadPart - categoryStart.QuadPart)); }
                    }
                    if (espCaptureActive && bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(10, (unsigned long long)(e.QuadPart - espStart.QuadPart)); }
                }
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PostLateUpdateHook.unsafe_call<void>();
        if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(1, (unsigned long long)(e.QuadPart - benchStart.QuadPart)); }
    }

    static void Hooked__PresentAfterDrawLoop()
    {
        LARGE_INTEGER benchStart{}; const bool bench = IsBenchmarkCaptureEnabled();
        if (bench) ::QueryPerformanceCounter(&benchStart);
        try
        {
            RestoreThirdPersonCamera();
            Unity::Camera mainCam = SemiFunc::MainCamera();
            if (mainCam && G->IsInGame)
            {
                if (G->BetterVision)
                    mainCam.SetFarClipPlane(prevPlane);

                mainCam.SetFieldOfView(prevFov);

            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PresentAfterDrawHook.unsafe_call<void>();
        if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(2, (unsigned long long)(e.QuadPart - benchStart.QuadPart)); }
    }

    static void Hooked__PhysGrabObject_PhysicsGrabbingManipulation(PhysGrabObject __this)
    {
        LARGE_INTEGER benchStart{}; const bool bench = IsBenchmarkCaptureEnabled();
        if (bench) ::QueryPerformanceCounter(&benchStart);
        try
        {
            if (PlayerAvatar avatar = PlayerAvatar::instance())
            {
                if (PhysGrabber grabber = avatar.physGrabber())
                {
                    float backup = grabber.grabStrength();
                    if (G->EasyGrab && backup < 20.f)
                        grabber.grabStrength() = 20.f;

                    G->PhysGrabObject_PhysicsGrabbingManipulation_Hook.unsafe_call<void, PhysGrabObject>(__this);

                    grabber.grabStrength() = backup;
                    if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(3, (unsigned long long)(e.QuadPart - benchStart.QuadPart)); }
                    return;
                }
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PhysGrabObject_PhysicsGrabbingManipulation_Hook.unsafe_call<void, PhysGrabObject>(__this);
        if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(3, (unsigned long long)(e.QuadPart - benchStart.QuadPart)); }
    }

    static void Hooked__PostProcessLayer_BuildCommandBuffers(Unity::PostProcessLayer __this)
    {
        try
        {
            if (G->BetterVision && G->IsInGame)
            {
                __this.m_LegacyCmdBufferBeforeReflections().Clear();
                __this.m_LegacyCmdBufferBeforeLighting().Clear();
                __this.m_LegacyCmdBufferOpaque().Clear();
                __this.m_LegacyCmdBuffer().Clear();
                return;
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PostProcessLayer_BuildCommandBuffers_Hook.unsafe_call<void, Unity::PostProcessLayer>(__this);
    }

    void Hooked__PlayerAvatarVisuals_Update(PlayerAvatarVisuals __this)
    {
        try
        {
            if (GameManager gm = GameManager::instance())
            {
                PlayerAvatar avatar = __this.playerAvatar();
                if (G->ThirdPerson && avatar && !__this.isMenuAvatar() && avatar.isLocal())
                {
                    __this.showSelfOverrideTimer() = Hax::Max(0.1f, __this.showSelfOverrideTimer());
                }
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PlayerAvatarVisuals_Update_Hook.unsafe_call<void, PlayerAvatarVisuals>(__this);
    }

    static void Hooked__PhysGrabber_PhysGrabLogic(PhysGrabber __this)
    {
        float& range = __this.grabRange();
        float cached = range;
        PlayerAvatar playerAvatar = PlayerAvatar::instance();

        if (G->UnlimGrabRange && playerAvatar && __this == playerAvatar.physGrabber())
            range = 999999.f;

        G->PhysGrabber_PhysGrabLogic_Hook.unsafe_call<void, PhysGrabber>(__this);

        range = cached;
    }

    static bool g_RaycastMaxDist;
    static bool g_InGrabRayCheck;
    static bool g_GrabAssistScanEnabled;
    static bool g_IgnoreGrabObstructions;
    static PhysGrabObject g_GrabAssistCandidate = nullptr;
    static PhysGrabObject g_CachedAssistCandidate = nullptr;
    static Unity::Vector3 g_CachedAssistPoint{};
    static ULONGLONG g_NextAssistScan = 0;

    static bool IsDuckGrabTransform(Unity::Transform transform)
    {
        for (int depth = 0; transform && depth < 8; ++depth)
        {
            // Duck colliders can be children of the object that owns EnemyDuck.
            if (transform.GetComponent<EnemyDuck>() || transform.GetComponentInChildren<EnemyDuck>())
                return true;
            transform = transform.GetParent();
        }
        return false;
    }

    static bool IsDuckGrabObject(PhysGrabObject object)
    {
        return object && IsDuckGrabTransform(object.GetTransform());
    }

    static void Hooked__PhysGrabber_RayCheck(PhysGrabber __this, bool grab)
    {
        float& range = __this.grabRange();
        float cached = range;
        PlayerAvatar playerAvatar = PlayerAvatar::instance();

        const bool isLocalGrabber = playerAvatar && __this == playerAvatar.physGrabber();
        if (isLocalGrabber && G->AntiDuck && grab && IsDuckGrabObject(__this.currentlyLookingAtPhysGrabObject()))
            grab = false;
        if (isLocalGrabber)
        {
            g_GrabAssistCandidate = nullptr;
            // The assist's nearby-object search is only useful while the player
            // is requesting a grab. Drop its cached target while idle so a later
            // click always performs a fresh search.
            if (!grab)
            {
                g_CachedAssistCandidate = nullptr;
                g_NextAssistScan = 0;
            }
        }
        if (isLocalGrabber && G->UnlimGrabRange)
        {
            range = 999999.f;
            g_RaycastMaxDist = true;
        }

        if (isLocalGrabber && !G->UnlimGrabRange && G->GrabThrough)
            range = cached + 5.f;

        g_InGrabRayCheck = isLocalGrabber;
        g_GrabAssistScanEnabled = isLocalGrabber && grab;
        g_IgnoreGrabObstructions = isLocalGrabber && G->GrabThrough;

        G->PhysGrabber_RayCheck_Hook.unsafe_call<void, PhysGrabber, bool>(__this, grab);

        // Ray redirection can still miss tiny or irregular colliders. If the normal
        // ray check did not start a grab, use the game's own force-grab path for the
        // exact candidate selected by the screen-space assist.
        if (isLocalGrabber && grab && G->GrabAssist && g_GrabAssistCandidate && !__this.grabbed())
            SemiFunc::PhysGrabberLocalForceGrab(g_GrabAssistCandidate);

        range = cached;
        g_RaycastMaxDist = false;
        g_InGrabRayCheck = false;
        g_GrabAssistScanEnabled = false;
        g_IgnoreGrabObstructions = false;
        if (isLocalGrabber)
            g_GrabAssistCandidate = nullptr;
    }

    static bool CallGrabRaycast(void* origin, void* direction, void* hitInfo, float maxDistance, int layerMask, int query)
    {
        const bool hit = G->Physics_Raycast_Hook.unsafe_call<bool, void*, void*, void*, float, int, int>(
            origin, direction, hitInfo, maxDistance, layerMask, query);
        if (hit && g_InGrabRayCheck && G->AntiDuck && hitInfo &&
            IsDuckGrabTransform(static_cast<Unity::RaycastHit*>(hitInfo)->GetTransform()))
            return false;
        return hit;
    }

    static bool Hooked__Physics_Raycast(void* v1, void* v2, void* v3, float maxDistance, int v5, int v6)
    {
        if (g_RaycastMaxDist)
            maxDistance = 999999.f;

        if (g_InGrabRayCheck && g_IgnoreGrabObstructions)
            v5 &= SemiFunc::LayerMaskGetPhysGrabObject();

        if (g_GrabAssistScanEnabled && G->GrabAssist)
        {
            Unity::Camera camera = SemiFunc::MainCamera();
            PlayerAvatar avatar = PlayerAvatar::instance();
            if (camera && avatar)
            {
                const Unity::Vector3 origin = *static_cast<Unity::Vector3*>(v1);
                const float maxGrabRange = avatar.physGrabber().grabRange();
                const float searchRange = G->UnlimGrabRange ? Hax::Min(maxDistance, 250.f) : Hax::Min(maxDistance, maxGrabRange);
                const ULONGLONG now = GetTickCount64();
                if (now >= g_NextAssistScan || !g_CachedAssistCandidate || g_CachedAssistCandidate.grabbedLocal())
                {
                    g_NextAssistScan = now + 50;
                    g_CachedAssistCandidate = nullptr;
                    Unity::LayerMask objectMask{};
                    objectMask.m_Mask = SemiFunc::LayerMaskGetPhysGrabObject();
                    auto nearby = SemiFunc::PhysGrabObjectGetAllWithinRange(searchRange, origin, false, objectMask, nullptr);

                    float bestScreenDistance = (float)G->GrabAssistRadius;
                    const float centerX = camera.GetPixelWidth() * 0.5f;
                    const float centerY = camera.GetPixelHeight() * 0.5f;
                    for (int i = 0; nearby != nullptr && i < nearby.Count(); ++i)
                    {
                        PhysGrabObject candidate = nearby[i];
                        if (!candidate || candidate.grabbedLocal())
                            continue;
                        if (G->AntiDuck && IsDuckGrabObject(candidate))
                            continue;
                        // Small valuables often have an offset centerPoint. Compare the
                        // visible transform, midpoint, and center, then force-grab the
                        // selected object's collider through the game's own grab routine.
                        const Unity::Vector3 points[] = {
                            candidate.GetTransform().GetPosition(), candidate.midPoint(), candidate.centerPoint()
                        };
                        for (const Unity::Vector3& point : points)
                        {
                            const Unity::Vector3 screen = camera.WorldToScreenPoint(point);
                            if (screen.Z <= 0.f) continue;
                            const float dx = screen.X - centerX;
                            const float dy = screen.Y - centerY;
                            const float screenDistance = sqrtf(dx * dx + dy * dy);
                            const float worldDistance = (point - origin).GetMagnitude();
                            if (screenDistance < bestScreenDistance && worldDistance <= searchRange)
                            {
                                g_CachedAssistCandidate = candidate;
                                g_CachedAssistPoint = point;
                                bestScreenDistance = screenDistance;
                            }
                        }
                    }
                }

                if (g_CachedAssistCandidate)
                {
                    g_GrabAssistCandidate = g_CachedAssistCandidate;
                    Unity::Vector3 redirect = g_CachedAssistPoint - origin;
                    const float distance = redirect.GetMagnitude();
                    if (distance > 0.001f)
                    {
                        redirect = redirect / distance;
                        // The assist has already selected a specific grabbable. Keep the
                        // redirected ray on the grabbable layer so scenery in front of its
                        // pivot does not cancel the assisted grab.
                        const int grabMask = SemiFunc::LayerMaskGetPhysGrabObject();
                        return CallGrabRaycast(v1, &redirect, v3, Hax::Max(maxDistance, distance + 0.1f), grabMask, v6);
                    }
                }
            }
        }

        return CallGrabRaycast(v1, v2, v3, maxDistance, v5, v6);
    }

    static void Hooked__MusicBoxTrap_Update(MusicBoxTrap __this)
    {
        G->MusicBoxTrap_Update_Hook.unsafe_call<void, MusicBoxTrap>(__this);
        if (!G->AntiMusicBoxSpin || !__this.MusicBoxPlaying())
            return;

        CameraAim cameraAim = CameraAim::Instance();
        PhysGrabObject box = __this.physgrabObject();
        if (cameraAim && box)
            cameraAim.AimTargetSoftSet(box.GetTransform().GetPosition(), 0.f, 0.f, 0.f, __this.GetGameObject(), 0);
        Unity::Transform rattler = __this.MusicBoxRattler();
        if (rattler)
            rattler.SetLocalRotation(Unity::Quaternion::identity());
    }

    static void Hooked__PhysGrabObjectImpactDetector_Break(PhysGrabObjectImpactDetector __this, float a1, void* a2, int a3, bool a4)
    {
        if (G->Unbreakable && __this.isValuable())
            return;

        G->PhysGrabObjectImpactDetector_Break_Hook.unsafe_call<void, PhysGrabObjectImpactDetector, float, void*, int, bool>(__this, a1, a2, a3, a4);
    }

    static void Hooked__PhysGrabObjectImpactDetector_DestroyObject(PhysGrabObjectImpactDetector __this, bool a1)
    {
        if (G->Unbreakable && __this.isValuable())
            return;

        G->PhysGrabObjectImpactDetector_DestroyObject_Hook.unsafe_call<void, PhysGrabObjectImpactDetector, bool>(__this, a1);
    }

    static void Hooked__ItemBattery_Update(ItemBattery __this)
    {
        try
        {
            if (G->InfBattery && __this.batteryLifeInt() < __this.batteryBars() && __this.physGrabObject().grabbedLocal())
            {
                __this.BatteryFullPercentChange(__this.batteryBars(), false);
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->ItemBattery_Update_Hook.unsafe_call<void, ItemBattery>(__this);
    }

    static void Hooked__Camera_FireOnPreRender(Unity::Camera __this)
    {
        BenchmarkGpuFrameBegin();
        try
        {
            if (IsInGame() && __this == GameDirector::instance().MainCamera() && G->ItemsChamsMat != null && G->CommandBuffer != null)
            {
                G->CommandBuffer.Clear();

                static Unity::Camera s_PrevCam = null;
                if (__this && __this != s_PrevCam)
                {
                    s_PrevCam = __this;
                    __this.AddCommandBuffer(Unity::CameraEvent::AfterEverything, G->CommandBuffer);
                }

                if (G->ValuablesChams && ValuableDirector::instance())
                {
                    for (ValuableObject obj : ValuableDirector::instance().valuableList())
                    {
                        if (obj && obj.GetEnabled())
                        {
                            for (Unity::MeshRenderer renderer : obj.GetTransform().GetChild(0).GetComponentsInChildren<Unity::MeshRenderer>(true))
                            {
                                G->CommandBuffer.DrawRenderer(renderer, G->ItemsChamsMat);
                            }
                        }
                    }
                }

                if (G->PlayersChams && GameDirector::instance())
                {
                    for (PlayerAvatar player : GameDirector::instance().PlayerList())
                    {
                        DrawPlayerChams(player, G->CommandBuffer, G->PlayerChamsMat, G->HeadChamsMat);
                    }
                }
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->Camera_FireOnPreRender_Hook.unsafe_call<void, Unity::Camera>(__this);
    }

    static void Hooked__ItemGun_Update(ItemGun __this)
    {
        try
        {
            Unity::LineRenderer laser = __this.GetComponent<Unity::LineRenderer>();

            if (__this.physGrabObject().grabbedLocal() && G->UseLaser)
            {
                if (!laser)
                {
                    laser = __this.GetGameObject().AddComponent<Unity::LineRenderer>();
                    laser.SetStartWidth(0.02f);
                    laser.SetEndWidth(0.02f);
                    laser.SetSortingOrder(1);
                    Unity::Material mat = Unity::Material::New(Unity::Shader::Find(System::String::New("Sprites/Default")));
                    mat.SetColor(Unity::Color::red());
                    laser.SetMaterial(mat);
                    laser.SetPositionCount(2);
                }

                Unity::Transform transform = __this.gunMuzzle();
                laser.SetEnabled(true);
                laser.SetPosition(0, transform.GetPosition());
                laser.SetPosition(1, transform.GetPosition() + transform.GetForward() * __this.gunRange());
            }
            else if (laser)
            {
                laser.SetEnabled(false);
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->ItemGun_Update_Hook.unsafe_call<void, ItemGun>(__this);
    }

    static void Hooked__RunManager_SetRunLevel(RunManager __this)
    {
        try
        {
            if (G->LevelBans.Size() > 0)
            {
                List<Level> levels = RunManager::instance().levels();

                LevelBan* levelBan = &G->LevelBans[0];
                Level prevLevel = __this.previousRunLevel();

                while (!levelBan->Allowed || ((G->LevelBans.Size() - G->TotalBans) > 1 && prevLevel && levels[levelBan->Index] == prevLevel))
                    levelBan = &G->LevelBans[rand() % G->LevelBans.Size()];

                __this.levelCurrent() = levels[levelBan->Index];
                return;
            }
        }
        catch (Exception& ex)
        {
            String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->RunManager_SetRunLevel_Hook.unsafe_call<void, RunManager>(__this);
    }

    static void Hooked__PhysGrabber_PhysGrabOverCharge(PhysGrabber __this, float _amount, float _multiplier)
    {
        if (G->NoOvercharge)
            return;

        G->PhysGrabber_PhysGrabOverCharge_Hook.unsafe_call<void, PhysGrabber, float, float>(__this, _amount, _multiplier);
    }

    static void Hooked__SpectateCamera_HeadEnergyLogic(SpectateCamera __this)
    {
        if (G->MaxHeadBattery)
        {
            __this.headEnergy() = 9999.f;
            __this.headEnergyEnough() = true;
        }

        G->SpectateCamera_HeadEnergyLogic_Hook.unsafe_call<void, SpectateCamera>(__this);
    }

    static void Hooked__EnemyRigidbody_FixedUpdate(EnemyRigidbody __this)
    {
        LARGE_INTEGER benchStart{}; const bool bench = IsBenchmarkCaptureEnabled();
        if (bench) ::QueryPerformanceCounter(&benchStart);
        if (G->NoGrabMaxTime && SemiFunc::IsMasterClientOrSingleplayer())
        {
            __this.grabShakeReleaseTimer() = 0.f;
            __this.grabTimeCurrent() = 0.f;
        }

        G->EnemyRigidbody_FixedUpdate_Hook.unsafe_call<void, EnemyRigidbody>(__this);
        if (bench) { LARGE_INTEGER e{}; ::QueryPerformanceCounter(&e); BenchmarkRecordSection(9, (unsigned long long)(e.QuadPart - benchStart.QuadPart)); }
    }

    static bool IsInGame()
    {
        RunManager runManager = RunManager::instance();
        GameDirector gameDirector = GameDirector::instance();
        return runManager && PlayerAvatar::instance() && gameDirector
            && runManager.levelCurrent() != runManager.levelLobbyMenu()
            && runManager.levelCurrent() != runManager.levelMainMenu()
            && gameDirector.currentState() == GameDirector_gameState::Main();
    }

    static void* GetPlayerLoopPtr(System::Type type, System::Type subType)
    {
        auto mainLoop = LowLevel::PlayerLoop::GetDefaultPlayerLoop();
        for (PlayerLoopSystem& loop : mainLoop.m_SubSystemList)
        {
            if (loop.m_Type != type)
                continue;

            for (PlayerLoopSystem& subLoop : loop.m_SubSystemList)
                if (subLoop.m_Type == subType && subLoop.m_UpdateFunction)
                    return *subLoop.m_UpdateFunction;
        }
        return nullptr;
    }

    static void Heal(PhysGrabObjectImpactDetector obj, float value)
    {
        if (SemiFunc::IsMultiplayer())
        {
            Array<System::Object> params = { Single(value), Vector3() };
            obj.photonView().RPC(String::New("HealRPC"), Photon::RpcTarget::All, params);
        }
        else
        {
            obj.HealLogic(value, Vector3::zero());
        }
    }

    static void SpawnItem(Item item)
    {
        Unity::Camera camera = SemiFunc::MainCamera();
        if (!camera)
            return;

        if (!item)
            return;

        Unity::Transform transform = camera.GetTransform();
        Unity::Vector3 pos = transform.GetPosition() + transform.GetForward() * 2.f - transform.GetUp();

        if (SemiFunc::IsMasterClient())
            Photon::PhotonNetwork::InstantiateRoomObject(item.prefab().resourcePath(), pos, Unity::Quaternion::identity());
        if (!SemiFunc::IsMultiplayer())
            Unity::Object::Instantiate<Unity::GameObject>(item.prefab().Prefab(), pos, Unity::Quaternion::identity());
    }

    static void SpawnValuablePrefab(PrefabRef prefab, int overrideValue)
    {
        Unity::Camera camera = SemiFunc::MainCamera();
        if (!camera || prefab == nullptr || !SemiFunc::IsMasterClientOrSingleplayer())
            return;

        Unity::Transform transform = camera.GetTransform();
        Unity::Vector3 pos = transform.GetPosition() + transform.GetForward() * 2.f - transform.GetUp();
        Unity::GameObject spawned = nullptr;
        GameManager manager = GameManager::instance();
        if (manager && manager.gameMode() == 0)
            spawned = Unity::Object::Instantiate<Unity::GameObject>(prefab.Prefab(), pos, Unity::Quaternion::identity());
        else
            spawned = Photon::PhotonNetwork::InstantiateRoomObject(prefab.resourcePath(), pos, Unity::Quaternion::identity());

        ValuableObject valuable = spawned ? spawned.GetComponent<ValuableObject>() : nullptr;
        if (valuable && overrideValue > 0)
        {
            // DollarValueSet is delayed by the game; setting the override before the
            // coroutine runs lets the host replicate the chosen amount through its
            // normal DollarValueSetRPC path.
            valuable.dollarValueOverride() = overrideValue;
            valuable.DollarValueSetLogic();
        }
    }

    static void SpawnCosmeticBox(int rarity)
    {
        Unity::Camera camera = SemiFunc::MainCamera();
        if (!camera)
            return;

        ValuableDirector dir = ValuableDirector::instance();
        if (!dir)
            return;

        auto setups = dir.cosmeticWorldObjectSetups();
        if (setups == null || rarity < 0 || rarity >= setups.Count())
            return;

        CosmeticWorldObjectSetup setup = setups[rarity];
        if (setup == null || setup.prefab() == null)
            return;

        Transform transform = camera.GetTransform();
        Vector3 pos = transform.GetPosition() + transform.GetForward() * 2.f - transform.GetUp();

        if (GameManager::instance().gameMode() == 0)
            Unity::Object::Instantiate<Unity::GameObject>(setup.prefab().Prefab(), pos, Quaternion::identity());
        else
            Photon::PhotonNetwork::InstantiateRoomObject(setup.prefab().resourcePath(), pos, Quaternion::identity());
    }

    static void ParseEnemies()
    {
        EnemyDirector dir = EnemyDirector::instance();
        if (!G->EnemiesPool.Empty() || !dir)
            return;

        Hax::LogDebug(G->Logger, L"Parsing enemies");
        //G->LoadRequests.reserve(50);

        List<EnemySetup> setups[3] = {dir.enemiesDifficulty1(), dir.enemiesDifficulty2(), dir.enemiesDifficulty3()};
        for (size_t i = 0; i < 3; ++i)
        {
            for (EnemySetup setup : setups[i])
            {
                if (!setup)
                    continue;

                String name = setup.GetName();
                if (name.Contains(L"Bang") || name.Contains(L"Gnome"))
                    continue;

                for (PrefabRef ref : setup.spawnObjects())
                {
                    auto req = Unity::Resources::LoadAsync(ref.resourcePath());
                    //G->LoadRequests.push_back(new LoadRequest(setup, req));

                    Hax::WStringView enemyName = ref.prefabName().begin() + 8;

                    EnemySetup& entry = G->EnemiesPool.FindOrAdd(enemyName);
                    if (!entry)
                    {
                        entry = setup;
                        GCHandle::Alloc(name, true);
                        Hax::LogDebug(G->Logger, L"Enemy parsed %ls", enemyName.begin());
                    }
                }
            }
        }
    }

    static void ParseItems()
    {
        if (StatsManager manager = StatsManager::instance())
        {
            auto itemDict = manager.itemDictionary();
            int nItems = itemDict.Count();

            if (itemDict.Count() == 0)
                manager.LoadItemsFromFolder();

            for (auto& entry : itemDict)
            {
                if (!entry.Value)
                    continue;

                Hax::WStringView name = entry.Value.itemName().ToHaxView();

                if (!G->ItemsPool.Contains(name))
                {
                    G->ItemsPool.Insert(name, entry.Value);
                    auto req = Resources::LoadAsync(entry.Value.prefab().resourcePath());

                    Hax::LogDebug(G->Logger, L"Item parsed %s", name.begin());
                }
            }
        }
    }

    static void ParseValuables()
    {
        ValuableDirector director = ValuableDirector::instance();
        if (!director) return;

        System::List<PrefabRef> groups[] = {
            director.tinyValuables(), director.smallValuables(), director.mediumValuables(),
            director.bigValuables(), director.wideValuables(), director.tallValuables(),
            director.veryTallValuables()
        };
        for (System::List<PrefabRef> group : groups)
        {
            for (int i = 0; group != nullptr && i < group.Count(); ++i)
            {
                PrefabRef prefab = group[i];
                if (prefab == nullptr) continue;
                System::String managedName = prefab.prefabName();
                const wchar_t* rawName = managedName != nullptr ? managedName.GetRawStringData() : nullptr;
                if (!rawName || !*rawName) continue;
                Hax::WStringView name(rawName);
                if (G->ValuablesPool.Contains(name)) continue;
                G->ValuablesPool.Insert(name, prefab);
                GCHandle::Alloc(managedName, true);
            }
        }
    }

    static void ParseLevels()
    {
        if (RunManager manager = RunManager::instance())
        {
            Hax::LogDebug(G->Logger, L"Parsing levels");

            System::List<Level> levels = manager.levels();
            G->LevelBans.Reserve(levels.Count());

            for (int i = 0; i < levels.Count(); ++i)
            {
                System::String levelName = levels[i].NarrativeName();

                LevelBan ban = {levelName.ToHaxView(), i, true};
                G->LevelBans.PushBack(ban);

                GCHandle::Alloc(levelName, true);
                Hax::LogDebug(G->Logger, L"Parsed level %s", levelName.begin());
            }

            std::sort(G->LevelBans.begin(), G->LevelBans.end(), [](const LevelBan& o1, const LevelBan& o2) { return o1.Name < o2.Name; });
        }
    }

    static void DrawPlayerChams(PlayerAvatar player, Unity::CommandBuffer cb, Unity::Material aliveMat, Unity::Material deadMat)
    {
        if (cb == null || !aliveMat || !deadMat || !player || player.isLocal())
            return;

        if (player.deadSet())
        {
            PlayerDeathHead head = player.playerDeathHead();
            if (head && head.GetEnabled())
            {
                for (Unity::MeshRenderer r : head.meshRenderers())
                    cb.DrawRenderer(r, deadMat);
            }
            return;
        }

        if (!player.isDisabled())
        {
            for (Unity::MeshRenderer renderer : player.playerHealth().renderers())
            {
                if (renderer && renderer.GetEnabled())
                    cb.DrawRenderer(renderer, aliveMat);
            }
        }
    }
}

