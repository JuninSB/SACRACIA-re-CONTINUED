#include <Windows.h>

#include "overlay/menu/localization.h"
#include "overlay/esp/esp.h"
#include "game_classes.h"

#include <third_party/safetyhook/safetyhook.hpp>

namespace Cheat
{
    void Initialize(HMODULE hCheatDll);
    void RestoreSettings();
    void SnapshotDiagnostics(const wchar_t* reason);
    void SetBenchmarkCapture(bool enabled);
    bool IsBenchmarkCaptureEnabled();
    void BenchmarkRecordFrame(unsigned long long ticks);
    void BenchmarkRecordFrameInterval(unsigned long long ticks);
    void BenchmarkRecordSection(int section, unsigned long long ticks);
    bool BenchmarkManagedMethodKnown(void* method);
    void BenchmarkRecordManagedMethod(void* method, const char* methodName, const char* className, unsigned long long ticks);
    void BenchmarkRecordGpuTimeUs(unsigned long long microseconds);
    void BenchmarkGpuFrameBegin();
    void BenchmarkGpuFrameEndAndPoll();
    void BenchmarkCaptureUnityCounts();

    enum class UpgradeType : int { Rest, Jump, Range, Speed, Stamina, Strength, Tumble, Wings, Health, Map, Climb, Battery, N };
    enum class AidType { Small, Medium, Large, N };
    enum class ChatPref { None = -1, Alive = 0, Dead = 1 };
    enum class CosmeticRarity { Common, Uncommon, Rare, UltraRare, N };

    struct UpgradeData
    {
        Hax::Gui::TextureHandle Icon;
        Hax::WStringView InternalNameEnd;
        LocKey LocKey;
        bool AutoUse;
    };

    struct AidData
    {
        Hax::Gui::TextureHandle Icon;
        Hax::WStringView InternalNameEnd;
        LocKey LocKey;
    };

    struct LevelBan
    {
        Hax::WStringView Name;
        int Index;
        bool Allowed = true;
    };

    struct Context
    {
        HMODULE                             Handle;
        HWND                                GameWndHandle;

        Hax::IniFile                        Config{L"haxsdk.ini"};
        Hax::LogFile                        Logger;

        bool                                MenuVisible;

        // Settings
        bool                                UseConsole;
        bool                                BenchmarkCapture = false;
        bool                                DarkenBackground;
        bool                                HideHostOnlyOptions = false;
        bool                                EspMatchFrameRate = false;
        int                                 EspUpdateRate = 10;
        int                                 EspFrameInterval = 1;
        int                                 VkOpenClose = VK_OEM_3;
        Lang                                Language;

        // Stats
        bool                                Godmode;
        bool                                InfStamina;
        bool                                InfJumps;
        bool                                NoTumble;
        bool                                EasyGrab;
        bool                                UnlimGrabRange;
        bool                                GrabThrough;
        bool                                GrabAssist;
        bool                                WallGrab;
        bool                                AntiMusicBoxSpin = false;
        bool                                AntiDuck = false;
        int                                 GrabAssistRadius = 64;
        bool                                PerfDisableShadows = true;
        bool                                PerfDisableAntiAliasing = true;
        bool                                PerfDisablePixelLights = true;
        bool                                PerfDisableLodBias = true;
        bool                                PerfDisableFog = false;
        bool                                PerfDisableSoftParticles = true;
        bool                                PerfDisableSoftVegetation = true;
        bool                                PerfDisableReflectionProbes = true;
        bool                                PerfDisablePostProcessing = true;
        bool                                IsInShop = false;
        bool                                FpsCounter = false;
        int                                 FpsCorner = 1;
        int                                 FpsFontSize = 12;
        bool                                NoOvercharge;
        bool                                AutoUseUpgrs;
        bool                                HealToMax;
        struct 
        {
            int                             Walking = 1;
            int                             Sprinting = 1;
            int                             Crouching = 1;
        } Accel;
        ItemUpgrade                         UpgradeToToggle = null;
        ItemHealthPack                      AidToToggle = null;
        ItemAttributes                      ItemToGrab = null;
        bool                                ItemGrabReplacingSlot = false;
        ULONGLONG                           ItemGrabDeadline = 0;
        bool                                ShopInventory = false;
        Hax::TripleBuffer<ItemHealthPack>   AidsPool{24};
        Hax::TripleBuffer<ItemUpgrade>      UpgradesPool{24};
        Hax::TripleBuffer<ItemAttributes>   DroppedItemsPool{64};

        UpgradeData UpgradesData[(int)UpgradeType::N + 1] =
        {
            { .InternalNameEnd = L"Rest",     .LocKey = LocKey_CrouchRest },
            { .InternalNameEnd = L"Jump",     .LocKey = LocKey_ExtraJump },
            { .InternalNameEnd = L"Range",    .LocKey = LocKey_Range },
            { .InternalNameEnd = L"Speed",    .LocKey = LocKey_SprintSpeed },
            { .InternalNameEnd = L"Energy",   .LocKey = LocKey_Stamina },
            { .InternalNameEnd = L"Strength", .LocKey = LocKey_Strength },
            { .InternalNameEnd = L"Launch",   .LocKey = LocKey_Launch },
            { .InternalNameEnd = L"Wings",    .LocKey = LocKey_Wings },
            { .InternalNameEnd = L"Health",   .LocKey = LocKey_Health },
            { .InternalNameEnd = L"Count",    .LocKey = LocKey_PlayersCount },
            { .InternalNameEnd = L"Climb",    .LocKey = LocKey_Climb },
            { .InternalNameEnd = L"Battery",  .LocKey = LocKey_HeadBattery },
            { .InternalNameEnd = L"",         .LocKey = LocKey_Unknown }
        };

        AidData AidsData[(int)AidType::N + 1] =
        {
            { .InternalNameEnd = L"Small",    .LocKey = LocKey_Small },
            { .InternalNameEnd = L"Medium",   .LocKey = LocKey_Medium },
            { .InternalNameEnd = L"Large",    .LocKey = LocKey_Large },
            { .InternalNameEnd = L"",         .LocKey = LocKey_Unknown }
        };

        // Entities
        bool                                NoGrabMaxTime;
        bool                                EnemiesEsp;
        bool                                PlayersEsp;
        bool                                PlayersChams;
        EnemySetup                          EnemyToSpawn = null;
        PlayerAvatar                        PlayerToTumble = null;
        PlayerAvatar                        PlayerToRevive = null;
        PlayerAvatar                        PlayerToKill = null;
        ChatPref                            ChatPrefered = ChatPref::None;
        size_t SelectedEnemySetup;
        EnemyParent                         SelectedManageEnemy = null;
        PlayerAvatar                        AggroTargetPlayer = null;
        int                                 EnemyAction = 0;

        // Vision
        bool                                BetterVision;
        bool                                ThirdPerson;
        bool                                MaxHeadBattery;
        int                                 Fov = 60;
        struct
        {
            int                             Intensity = 10;
            int                             Angle = 60;
            bool                            InCrouch;
        } Flashlight;
        Unity::CommandBuffer          CommandBuffer = null;
        Unity::AssetBundle            Bundle = null;
        Unity::Material               ItemsChamsMat = null;
        Unity::Material               PlayerChamsMat = null;
        Unity::Material             HeadChamsMat = null;

        // Valuables
        bool                                ValuablesEsp;
        bool                                ValuablesChams;
        bool                                Unbreakable; // TODO: rename
        bool                                ExtrPointsEsp;
        bool                                SetToZero;
        bool                                SetToMax;
        bool                                ActivateNextPoint;
        int                                 ValuablesEspRange = 20;
        bool                                HideStoredValuables = true;

        // Items
        bool                                InfBattery;
        bool                                UseLaser;
        Item                                ItemToSpawn = null;
        PrefabRef                           ValuableToSpawn = null;
        int                                 SpawnMoneyValue = 0;

        // Levels
        Hax::Vector<LevelBan>               LevelBans;
        size_t                              TotalBans;
        bool                                ForceNextLevel;

        // Cosmetics
        bool                                UnlockAllCosmetic;
        bool                                ResetAllCosmetic;
        bool                                CosmeticBoxesEsp;
        int                                 RarityToSpawn;

        // Misc
        bool                                TruckEsp;
        bool                                TeleportToTruck;

        // Grab and identity actions
        wchar_t                             IdentityName[64]{};
        bool                                LoadIdentityName;
        bool                                ApplyIdentityName;
        bool                                RandomizeAvatar;
        // Events
        HANDLE                              UnityLoadedEvent;

        // WinHooks
        bool                                ForceCursorVisible;
        bool                                OrigCursorVisible;
        bool                                ForceCursorUnclip;
        bool                                OrigCursorUnclip;
        bool                                GameInputPrevented;
        bool                                ForceCustomCursorTex;
        Hax::Optional<POINT>                LockedCursorPos;
        Hax::Optional<RECT>                 OrigClipArea;
        HCURSOR                             OrigMouseTexture;

        // Graphics
        void*                               PresentOrig;
        void*                               ResizeBuffersOrig;
        ID3D11RenderTargetView*             RenderTarget;
        ID3D11Device*                       Device;
        ID3D11DeviceContext*                DeviceContext;

        // Overlay
        Hax::Gui::FontHandle                NunitoSans_SemiBold;
        Hax::Gui::FontHandle                NunitoSans_Bold;
        Hax::Gui::FontHandle                NunitoSans_ExtraBold;
        Hax::Gui::FontHandle                Icons_Solid;
        Hax::Gui::TextureHandle             Logo;
        size_t                              KeyListenerId;

        struct
        {
            bool                            ReadyToHook;
            void*                           UpdatePtr;
            void*                           FixedPhysicsPtr;
            void*                           FixedScriptsPtr;
            void*                           LateScriptsPtr;
            void*                           PostLateUpdatePtr;
            void*                           PresentAfterDrawPtr;
        } PlayerLoop;

        // Esp
        Hax::TripleBuffer<EnemyEspData>     EnemiesEspBuffer{20};
        Hax::TripleBuffer<ValuableEspData>  ValuablesEspBuffer{50};
        Hax::TripleBuffer<ExtrPointEspData> ExtrPointsEspBuffer{10};
        Hax::TripleBuffer<TruckEspData>     TruckEspBuffer{2};
        Hax::TripleBuffer<PlayerEspData>    PlayersEspBuffer{6};
        Hax::TripleBuffer<CosmeticBoxEspData> CosmeticBoxesEspBuffer{20};


        // Misc
        Hax::Handle                         UvmThread;
        const Hax::WStringView*             Loc = g_LocDict[0];
        bool                                IsClient;
        bool                                IsInGame;
        Hax::Map<Hax::WStringView, EnemySetup> EnemiesPool;
        Hax::Map<Hax::WStringView, Item>     ItemsPool;
        Hax::Map<Hax::WStringView, PrefabRef> ValuablesPool;
        float PixelHeight;
        float PixelWidth;
        float ScreenHeight;
        float ScreenWidth;


        // Hooks
        SafetyHookInline                    MonoRuntimeInvokeHook;

        SafetyHookInline                    SetCursorHook;
        SafetyHookInline                    GetRawInputDataHook;
        SafetyHookInline                    GetCursorPosHook;
        SafetyHookInline                    ShowCursorHook;
        SafetyHookInline                    ClipCursorHook;
        SafetyHookInline                    SetCursorPosHook;
        SafetyHookInline                    GetAsyncKeyStateHook;
        SafetyHookInline                    GetKeyStateHook;
        SafetyHookInline                    GetKeyboardStateHook;
        SafetyHookInline                    PeekMessageAHook;
        SafetyHookInline                    PeekMessageWHook;
        SafetyHookInline                    GetMessageAHook;
        SafetyHookInline                    GetMessageWHook;
        SafetyHookInline                    TerminateProcessHook;
        SafetyHookInline                    ExitProcessHook;

        SafetyHookInline                    Application_Quit_Hook;
        SafetyHookInline                    EventSystem_Update_Hook;
        SafetyHookInline                    PlayerHealth_Hurt_Hook;
        SafetyHookInline                    PlayerAvatar_PlayerDeath_Hook;
        SafetyHookInline                    PlayerController_FixedUpdate_Hook;
        SafetyHookInline                    PlayerController_Update_Hook;
        SafetyHookInline                    PlayerTumble_TumbleRequest_Hook;
        SafetyHookInline                    FlashlightController_Update_Hook;
        SafetyHookInline                    ItemUpgrade_Update_Hook;
        SafetyHookInline                    ItemHealthPack_Update_Hook;
        SafetyHookInline                    ItemAttributes_Update_Hook;
        SafetyHookInline                    InventorySpot_Update_Hook;
        SafetyHookInline                    SemiUI_Hide_Hook;
        SafetyHookInline                    RunManager_ChangeLevel_Hook;
        SafetyHookInline                    UpdateLoop_Hook;
        SafetyHookInline                    PostLateUpdateLoop_Hook;
        SafetyHookInline                    PresentAfterDrawLoop_Hook;
        SafetyHookInline                    FixedPhysicsLoop_Hook;
        SafetyHookInline                    FixedScriptsLoop_Hook;
        SafetyHookInline                    LateScriptsLoop_Hook;
        SafetyHookInline                    PhysGrabObject_PhysicsGrabbingManipulation_Hook;
        SafetyHookInline                    PostProcessLayer_BuildCommandBuffers_Hook;
        SafetyHookInline                    PlayerAvatarVisuals_Update_Hook;
        SafetyHookInline                    PhysGrabber_PhysGrabLogic_Hook;
        SafetyHookInline                    PhysGrabber_RayCheck_Hook;
        SafetyHookInline                    MusicBoxTrap_Update_Hook;
        SafetyHookInline                    Physics_Raycast_Hook;
        SafetyHookInline                    PhysGrabObjectImpactDetector_Break_Hook;
        SafetyHookInline                    PhysGrabObjectImpactDetector_DestroyObject_Hook;
        SafetyHookInline                    ItemBattery_Update_Hook;
        SafetyHookInline                    Camera_FireOnPreRender_Hook;
        SafetyHookInline                    ItemGun_Update_Hook;
        SafetyHookInline                    RunManager_SetRunLevel_Hook;
        SafetyHookInline                    PhysGrabber_PhysGrabOverCharge_Hook;
        SafetyHookInline                    SpectateCamera_HeadEnergyLogic_Hook;
        SafetyHookInline                    EnemyRigidbody_FixedUpdate_Hook;

        SafetyHookInline                    UpdateHook;
        SafetyHookInline                    PostLateUpdateHook;
        SafetyHookInline                    PresentAfterDrawHook;
    };

    extern Context* G;
}
