#pragma once

#include <third_party/haxsdk/unity/hax_unity.h>

namespace Unity::Photon
{
    struct RaiseEventOptions : System::Object
    {
        using System::Object::Object;
        META("PhotonRealtime", "Photon.Realtime", "RaiseEventOptions");
        FIELD(Receivers, int);
    };

    struct SendOptions
    {
        META("Photon3Unity3D", "ExitGames.Client.Photon", "SendOptions");
        STATIC_FIELD(SendReliable, SendOptions);
    };
}

namespace Steamworks
{
    struct SteamId { uint64_t Value; };
}

namespace Steamworks::Data
{
    struct Lobby { Steamworks::SteamId Id; };
}

struct PhysGrabObject;
struct Item;
struct SemiColor : Unity::Object
{
    using Unity::Object::Object;
    META("Assembly-CSharp", "", "SemiColor");
};

struct CosmeticAsset : Unity::Object
{
    using Unity::Object::Object;
    META("Assembly-CSharp", "", "CosmeticAsset");
    FIELD(type, int);
};

struct PhysGrabObjectImpactDetector;
struct PhysGrabObject : Unity::Behaviour
{
    using Unity::Behaviour::Behaviour;
    META("Assembly-CSharp", "", "PhysGrabObject");
    FIELD(grabbedLocal, bool);
    FIELD(grabbed, bool);
    FIELD(centerPoint, Unity::Vector3);
    FIELD(boundingBox, Unity::Vector3);
    FIELD(midPoint, Unity::Vector3);
    FIELD(impactDetector, PhysGrabObjectImpactDetector);
    METHOD_WRAPPER(PhysicsGrabbingManipulation);
};

struct PrefabRef : System::Object
{
    using System::Object::Object;

    META("Assembly-CSharp", "", "PrefabRef");

    Unity::GameObject Prefab() { THROW_IF_NULL(); return s_get_Prefab.Call<Unity::GameObject>(*this); }

    FIELD(prefabName, System::String);
    FIELD(resourcePath, System::String);

private:
    METHOD_WRAPPER(get_Prefab);
};

struct MenuCursor : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "MenuCursor");

    FIELD(showTimer, float);

    METHOD_WRAPPER(Update);
};

struct PlayerHealth : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerHealth");

    void HealOther(int healAmount, bool effect)
    {
        THROW_IF_NULL();
        s_HealOther.Call<void>(*this, healAmount, effect);
    }

    FIELD(maxHealth, int);
    FIELD(health, int);
    FIELD(renderers, System::List<Unity::MeshRenderer>);

    METHOD_WRAPPER(Hurt);
    METHOD_WRAPPER(Update);
private:
    METHOD_WRAPPER(HealOther);
};

struct PlayerDeathHead;
struct PhysGrabber;
struct PlayerVoiceChat;
struct PlayerTumble;
struct MapToolController;
struct PlayerAvatarVisuals;
struct PlayerAvatar : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerAvatar");

    void PlayerDeath(int enemyIndex)
    {
        THROW_IF_NULL();
        s_PlayerDeath.Call<void>(*this, enemyIndex);
    }

    void Revive(bool revivedByTruck = false)
    {
        THROW_IF_NULL();
        s_Revive.Call<void>(*this, revivedByTruck);
    }

    STATIC_FIELD(instance, PlayerAvatar);
    FIELD(playerHealth, PlayerHealth);
    FIELD(isLocal, bool);
    FIELD(deadSet, bool);
    FIELD(playerDeathHead, PlayerDeathHead);
    FIELD(physGrabber, PhysGrabber);
    FIELD(playerName, System::String);
    FIELD(voiceChat, PlayerVoiceChat);
    FIELD(spawned, bool);
    FIELD(photonView, Unity::Photon::PhotonView);
    FIELD(tumble, PlayerTumble);
    FIELD(upgradeMapPlayerCount, int);
    FIELD(isCrouching, bool);
    FIELD(isDisabled, bool);
    FIELD(spectatePoint, Unity::Transform);
    FIELD(mapToolController, MapToolController);
    FIELD(playerTransform, Unity::Transform);
    FIELD(playerAvatarVisuals, PlayerAvatarVisuals);
    FIELD(voiceChatFetched, bool);

    METHOD_WRAPPER(OnDestroy);
    METHOD_WRAPPER(PlayerDeath);
    METHOD_WRAPPER(Revive);
};

struct PlayerVoiceChat : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerVoiceChat");

    FIELD(inLobbyMixer, bool);

    void ToggleMixer(bool toggle, bool distorted)
    {
        THROW_IF_NULL();
        s_ToggleMixer.Call<void>(*this, toggle, distorted);
    }

private:
    METHOD_WRAPPER(ToggleMixer);
};

struct PlayerController : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerController");

    STATIC_FIELD(instance, PlayerController);
    FIELD(EnergyCurrent, float);
    FIELD(EnergyStart, float);
    FIELD(MoveSpeed, float);
    FIELD(SprintSpeed, float);
    FIELD(CrouchSpeed, float);
    FIELD(JumpExtra, int);
    FIELD(SprintSpeedUpgrades, float);
    FIELD(playerName, System::String);

    void PlayerSetName(System::String name, System::String steamId)
    {
        THROW_IF_NULL();
        s_PlayerSetName.Call<void, PlayerController, System::String, System::String>(*this, name, steamId);
    }

    METHOD_WRAPPER(FixedUpdate);
    METHOD_WRAPPER(Update);
private:
    METHOD_WRAPPER(PlayerSetName);
};

struct PlayerTumble : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerTumble");

    void TumbleRequest(bool _isTumbling, bool _playerInput) 
    { 
        THROW_IF_NULL(); 
        s_TumbleRequest.Call<void, PlayerTumble, bool, bool>(*this, _isTumbling, _playerInput); 
    }

    FIELD(tumbleLaunch, int);
    FIELD(playerAvatar, PlayerAvatar);

    METHOD_WRAPPER(TumbleRequest);
};

struct FlashlightController : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "FlashlightController");

    FIELD(spotlight, Unity::Light);
    FIELD(baseIntensity, float);

    METHOD_WRAPPER(Update);
};

struct ItemToggle : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemToggle");

    void ToggleItem(bool toggle, int player = -1)
    {
        THROW_IF_NULL();
        s_ToggleItem.Call<void, ItemToggle, bool, int>(*this, toggle, player);
    }

    FIELD(toggleState, bool);
    FIELD(photonView, Unity::Photon::PhotonView);

private:
    METHOD_WRAPPER(ToggleItem);
};

struct ItemAttributes : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemAttributes");

    FIELD(item, Item);
    FIELD(itemName, System::String);
    FIELD(itemAssetName, System::String);
    FIELD_EX(physGrabObject, "physGrabObject", PhysGrabObject);
    METHOD_WRAPPER(Update);
};

struct ItemUpgrade : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemUpgrade");

    FIELD(upgradeDone, bool);
    FIELD(itemToggle, ItemToggle);
    FIELD(itemAttributes, ItemAttributes);
    FIELD(isPlayerUpgrade, bool);
    FIELD(photonView, Unity::Photon::PhotonView);

    METHOD_WRAPPER(Update);
};

struct ItemHealthPack : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemHealthPack");

    FIELD(used, bool);
    FIELD(itemToggle, ItemToggle);
    FIELD(photonView, Unity::Photon::PhotonView);
    FIELD(itemAttributes, ItemAttributes);

    METHOD_WRAPPER(Update);
};

struct SemiFunc
{
    META("Assembly-CSharp", "", "SemiFunc");

    static inline Unity::Camera MainCamera() 
    { 
        return s_MainCamera.Call<Unity::Camera>(); 
    }

    static inline bool OnScreen(const Unity::Vector3& pos, float paddWidth, float paddHeight) 
    {
        return s_OnScreen.Call<bool>(System::Box(pos), paddWidth, paddHeight); 
    }

    static inline bool IsMasterClientOrSingleplayer()
    {
        return s_IsMasterClientOrSingleplayer.Call<bool>();
    }

    static inline bool IsMultiplayer()
    {
        return s_IsMultiplayer.Call<bool>();
    }

    static inline bool IsMasterClient()
    {
        return s_IsMasterClient.Call<bool>();
    }

    static inline bool InputDown(int inputKey)
    {
        return s_InputDown.Call<bool, int>(inputKey);
    }

    static inline System::String PlayerGetSteamID(PlayerAvatar player)
    {
        return s_PlayerGetSteamID.Call<System::String>(player);
    }

    static inline int StatSetRunCurrency(int value)
    {
        return s_StatSetRunCurrency.Call<int, int>(value);
    }

    static inline int PhotonViewIDPlayerAvatarLocal()
    {
        return s_PhotonViewIDPlayerAvatarLocal.Call<int>();
    }

    static inline int LayerMaskGetVisionObstruct()//!
    {
        return s_LayerMaskGetVisionObstruct.Call<System::Boxed<Unity::LayerMask>>().Unbox().m_Mask;
    }

    static inline int LayerMaskGetPhysGrabObject()
    {
        return s_LayerMaskGetPhysGrabObject.Call<System::Boxed<Unity::LayerMask>>().Unbox().m_Mask;
    }

    static inline System::String PlayerGetName(PlayerAvatar player)
    {
        return s_PlayerGetName.Call<System::String>(player);
    }

    static inline void PhysGrabberLocalForceGrab(PhysGrabObject target)
    {
        s_PhysGrabberLocalForceGrab.Call<void, PhysGrabObject>(target);
    }

    static inline System::List<PhysGrabObject> PhysGrabObjectGetAllWithinRange(float range, const Unity::Vector3& pos, bool includeGrabbed, Unity::LayerMask mask, PhysGrabObject exclude)
    {
        return s_PhysGrabObjectGetAllWithinRange.Call<System::List<PhysGrabObject>>(range, System::Box(pos), includeGrabbed, System::Box(mask), exclude);
    }

private:
    METHOD_WRAPPER(OnScreen);
    METHOD_WRAPPER(MainCamera);
    METHOD_WRAPPER(IsMasterClientOrSingleplayer);
    METHOD_WRAPPER(IsMultiplayer);
    METHOD_WRAPPER(IsMasterClient);
    METHOD_WRAPPER(InputDown);
    METHOD_WRAPPER(PlayerGetSteamID);
    METHOD_WRAPPER(StatSetRunCurrency);
    METHOD_WRAPPER(PhotonViewIDPlayerAvatarLocal);
    METHOD_WRAPPER(LayerMaskGetVisionObstruct);
    METHOD_WRAPPER(LayerMaskGetPhysGrabObject);
    METHOD_WRAPPER(PlayerGetName);
    METHOD_WRAPPER(PhysGrabberLocalForceGrab);
    METHOD_WRAPPER(PhysGrabObjectGetAllWithinRange);
};

struct PhysGrabber : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PhysGrabber");

    void PhysGrabOverChargeImpact()
    {
        THROW_IF_NULL();
        s_PhysGrabOverChargeImpact.Call<void, PhysGrabber>(*this);
    }

    FIELD(grabRange, float);
    FIELD(grabStrength, float);
    FIELD(throwStrength, float);
    FIELD(currentlyLookingAtPhysGrabObject, PhysGrabObject);
    STATIC_FIELD(instance, PhysGrabber);
    FIELD(grabbed, bool);
    FIELD_EX(grabbedPhysGrabObject, "grabbedPhysGrabObject", PhysGrabObject);

    METHOD_WRAPPER(PhysGrabLogic);
    METHOD_WRAPPER(RayCheck);
    METHOD_WRAPPER(PhysGrabOverChargeImpact);
    METHOD_WRAPPER(PhysGrabOverCharge);
};

struct MusicBoxTrap : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "MusicBoxTrap");

    FIELD(physgrabObject, PhysGrabObject);
    FIELD(MusicBoxRattler, Unity::Transform);
    FIELD(MusicBoxPlaying, bool);
    METHOD_WRAPPER(Update);
};

struct CameraAim : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "CameraAim");

    STATIC_FIELD(Instance, CameraAim);
    void AimTargetSoftSet(const Unity::Vector3& position, float time, float strength, float strengthNoAim, Unity::GameObject obj, int priority)
    {
        THROW_IF_NULL();
        s_AimTargetSoftSet.Call<void>(*this, System::Box(position), time, strength, strengthNoAim, obj, priority);
    }
private:
    METHOD_WRAPPER_EX(AimTargetSoftSet, "System.Void(UnityEngine.Vector3,System.Single,System.Single,System.Single,UnityEngine.GameObject,System.Int32)");
};

struct EnemyDuck : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "EnemyDuck");
};

struct Level : Unity::Object
{
    using Unity::Object::Object;

    META("Assembly-CSharp", "", "Level");

    FIELD(NarrativeName, System::String);
};

struct RunManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "RunManager");

    void ChangeLevel(bool completedLevel, bool levelFailed, int changeLevelType = 0)
    {
        THROW_IF_NULL();
        s_ChangeLevel.Call<void, RunManager, bool, bool, int>(*this, completedLevel, levelFailed, changeLevelType);
    }

    void BeginLeaveToMainMenu()
    {
        THROW_IF_NULL();
        static auto leaveMethod = typeof().GetMethod("LeaveToMainMenu", "System.Collections.IEnumerator()", true).Wrap();
        System::Object routine = leaveMethod.Call<System::Object>(*this);
        static auto startMethod = Unity::MonoBehaviour::typeof().GetMethod("StartCoroutine", "UnityEngine.Coroutine(System.Collections.IEnumerator)", true).Wrap();
        startMethod.Call<System::Object>(*this, routine);
    }

    STATIC_FIELD(instance, RunManager);
    FIELD(levels, System::List<Level>);
    FIELD(levelCurrent, Level);
    FIELD(levelMainMenu, Level);
    FIELD(levelLobbyMenu, Level);
    FIELD(levelShop, Level);
    FIELD(previousRunLevel, Level);
    FIELD(levelsCompleted, int);
    FIELD(levelIsShop, bool);

    METHOD_WRAPPER(ChangeLevel);
    METHOD_WRAPPER(SetRunLevel);
};

struct SemiUI : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "SemiUI");
    void Hide() { THROW_IF_NULL(); s_Hide.Call<void>(*this); }
    METHOD_WRAPPER(Hide);
};

struct InventoryUI : SemiUI
{
    using SemiUI::SemiUI;
    META("Assembly-CSharp", "", "InventoryUI");
    STATIC_FIELD(instance, InventoryUI);
};

struct GameDirector : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "GameDirector");

    STATIC_FIELD(instance, GameDirector);
    FIELD(MainCamera, Unity::Camera);
    FIELD(currentState, int);
    FIELD(PlayerList, System::List<PlayerAvatar>);
};

struct DataDirector : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "DataDirector");

    STATIC_FIELD(instance, DataDirector);
    FIELD(networkJoinServerName, System::String);
};

struct GameDirector_gameState
{
    META("Assembly-CSharp", "", "GameDirector/gameState");

    ENUM_VALUE(Main);
};

struct SpectateCamera : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "SpectateCamera");

    STATIC_FIELD(instance, SpectateCamera);
    FIELD(headEnergy, float);
    FIELD(headEnergyEnough, bool);    

    METHOD_WRAPPER(HeadEnergyLogic);
};

struct PlayerHealthGrab : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerHealthGrab");
};

struct PlayerAvatarVisuals : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerAvatarVisuals");

    FIELD(playerAvatar, PlayerAvatar);
    FIELD(isMenuAvatar, bool);
    FIELD(meshParent, Unity::GameObject);
    FIELD(animator, Unity::Behaviour);
    FIELD(headSideTransform, Unity::Transform);
    FIELD(legTwistTransform, Unity::Transform);
    FIELD(showSelfOverrideTimer, float);

    METHOD_WRAPPER(Update);
};

struct GameManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "GameManager");

    STATIC_FIELD(instance, GameManager);
    PROPERTY(gameMode, int);
};

struct MapToolController : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "MapToolController");

    FIELD(Active, bool);
};

struct EnemyRigidbody : Unity::Behaviour
{
    using Unity::Behaviour::Behaviour;

    META("Assembly-CSharp", "", "EnemyRigidbody");

    FIELD(physGrabObject, PhysGrabObject);
    FIELD(grabShakeReleaseTimer, float);
    FIELD(grabStrengthTimer, float);
    FIELD(grabStrengthTime, float);
    FIELD(grabTimeCurrent, float);
    FIELD(followTarget, Unity::Transform);
    FIELD(yOffset, float);

    METHOD_WRAPPER(FixedUpdate);
    void Teleport() { THROW_IF_NULL(); s_Teleport.Call<void, EnemyRigidbody>(*this); }
    METHOD_WRAPPER(Teleport);
};

struct EnemyHealth : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "EnemyHealth");

    void Hurt(int damage, const Unity::Vector3& hurtDirection)
    {
        THROW_IF_NULL();
        s_Hurt.Call<void>(*this, damage, System::Box(hurtDirection));
    }

    FIELD(healthCurrent, int);
    FIELD(health, int);
    FIELD(dead, bool);

    METHOD_WRAPPER(Hurt);
};

struct EnemyState
{
    META("Assembly-CSharp", "", "EnemyState");

    ENUM_VALUE(None);
    ENUM_VALUE(Spawn);
    ENUM_VALUE(Despawn);
};

struct EnemyParent;
struct Enemy : Unity::Behaviour
{
    META("Assembly-CSharp", "", "Enemy");

    FIELD(Rigidbody, EnemyRigidbody);
    FIELD(CenterTransform, Unity::Transform);
    FIELD(Health, EnemyHealth);
    FIELD(CurrentState, int);
    FIELD_EX(enemyParent, "EnemyParent", EnemyParent);

    void SetChaseTarget(PlayerAvatar target)
    {
        THROW_IF_NULL();
        s_SetChaseTarget.Call<void>(*this, target);
    }

    METHOD_WRAPPER(Update);
private:
    METHOD_WRAPPER(SetChaseTarget);
};

struct EnemyParent : Unity::Behaviour
{
    using Unity::Behaviour::Behaviour;
    META("Assembly-CSharp", "", "EnemyParent");

    FIELD_EX(enemy, "Enemy", Enemy);
    FIELD(Spawned, bool);
    FIELD(enemyName, System::String);
    FIELD(difficulty, int);
};

struct EnemySetup : Unity::Object
{
    using Unity::Object::Object;

    META("Assembly-CSharp", "", "EnemySetup");

    FIELD(spawnObjects, System::List<PrefabRef>);
};

struct EnemyDirector : Unity::Behaviour
{
    using Unity::Behaviour::Behaviour;

    META("Assembly-CSharp", "", "EnemyDirector");

    STATIC_FIELD(instance, EnemyDirector);
    FIELD(enemiesSpawned, System::List<EnemyParent>);
    FIELD(enemiesDifficulty1, System::List<EnemySetup>);
    FIELD(enemiesDifficulty2, System::List<EnemySetup>);
    FIELD(enemiesDifficulty3, System::List<EnemySetup>);
    FIELD(debugNoVision, bool);
    FIELD(debugNoGrabMaxTime, bool);
    FIELD(debugEasyGrab, bool);
};

struct RoomVolume : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "RoomVolume");
        
    void SetExplored()
    {
        THROW_IF_NULL();
        s_SetExplored.Call<void, RoomVolume>(*this);
    }

private:
    METHOD_WRAPPER(SetExplored);
};

struct LevelPoint : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "LevelPoint");

    FIELD(Room, RoomVolume);
};

struct LevelGenerator : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "LevelGenerator");

    void EnemySpawn(EnemySetup enemySetup, Unity::Vector3 position)
    {
        THROW_IF_NULL();
        s_EnemySpawn.Call<void>(*this, enemySetup, System::Box(position));
    }

    STATIC_FIELD(Instance, LevelGenerator);
    FIELD(LevelPathPoints, System::List<LevelPoint>);
    FIELD(Generated, bool);

private:
    METHOD_WRAPPER(EnemySpawn);
};

struct EnemySlowMouth : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "EnemySlowMouth");

    FIELD(attachedTimer, float);
};

struct MenuManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "MenuManager");

    STATIC_FIELD(instance, MenuManager);
    FIELD(currentMenuPage, Component);
};

struct PhysGrabObjectImpactDetector : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PhysGrabObjectImpactDetector");

    void HealLogic(float healAmount, const Unity::Vector3& healingPoint)
    {
        s_HealLogic.Call<void>(*this, healAmount, System::Box(healingPoint));
    }

    FIELD(isValuable, bool);
    FIELD(inCart, bool);
    FIELD(impactDisabledTimer, float);
    FIELD(photonView, Unity::Photon::PhotonView);

    METHOD_WRAPPER(FixedUpdate);
    METHOD_WRAPPER(Break);
    METHOD_WRAPPER(DestroyObject);
private:
    METHOD_WRAPPER(HealLogic);
};

struct ItemBattery : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemBattery");

    void SetBatteryLife(int batteryLife)
    {
        THROW_IF_NULL();
        s_SetBatteryLife.Call<void, ItemBattery, int>(*this, batteryLife);
    }

    void BatteryFullPercentChange(int batteryLifeInt, bool charge = false)
    {
        THROW_IF_NULL();
        s_BatteryFullPercentChange.Call<void, ItemBattery, int, bool>(*this, batteryLifeInt, charge);
    }

    FIELD(batteryLife, float);
    FIELD(batteryLifeInt, int);
    FIELD(batteryBars, int);
    FIELD(physGrabObject, PhysGrabObject);

    METHOD_WRAPPER(Update);
    METHOD_WRAPPER(SetBatteryLife);
    METHOD_WRAPPER(BatteryFullPercentChange);
};

struct BatteryVisualLogic : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "BatteryVisualLogic");
    FIELD(batteryBars, int);
    FIELD(currentBars, int);
    FIELD(batteryPercent, float);
    FIELD(itemBattery, ItemBattery);
};

struct BatteryUI : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "BatteryUI");
    STATIC_FIELD(instance, BatteryUI);
    FIELD(batteryVisualLogic, BatteryVisualLogic);
};

struct ValuableObject : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ValuableObject");

    void Discover(int state)
    {
        THROW_IF_NULL();
        s_Discover.Call<void, ValuableObject, int>(*this, state);
    }

    FIELD(physGrabObject, PhysGrabObject);
    FIELD(dollarValueCurrent, float);
    FIELD(dollarValueOverride, int);
    FIELD(dollarValueSet, bool);
    void DollarValueSetLogic() { THROW_IF_NULL(); s_DollarValueSetLogic.Call<void>(*this); }
    Unity::Photon::PhotonView photonView() { return GetComponent<Unity::Photon::PhotonView>(); }

private:
    METHOD_WRAPPER(Discover);
    METHOD_WRAPPER(DollarValueSetLogic);
};

struct CosmeticWorldObjectSetup : System::Object
{
    using System::Object::Object;

    META("Assembly-CSharp", "", "ValuableDirector/CosmeticWorldObjectSetup");

    FIELD(prefab, PrefabRef);
};

struct ValuableDirector : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ValuableDirector");

    STATIC_FIELD(instance, ValuableDirector);
    FIELD(valuableList, System::List<ValuableObject>);
    FIELD(cosmeticWorldObjectSetups, System::List<CosmeticWorldObjectSetup>);
    FIELD(tinyValuables, System::List<PrefabRef>);
    FIELD(smallValuables, System::List<PrefabRef>);
    FIELD(mediumValuables, System::List<PrefabRef>);
    FIELD(bigValuables, System::List<PrefabRef>);
    FIELD(wideValuables, System::List<PrefabRef>);
    FIELD(tallValuables, System::List<PrefabRef>);
    FIELD(veryTallValuables, System::List<PrefabRef>);
};

struct ItemManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "ItemManager");
    STATIC_FIELD(instance, ItemManager);
    FIELD(spawnedItems, System::List<ItemAttributes>);
};

struct ItemGun : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemGun");

    FIELD(gunRandomSpread, float);
    FIELD(physGrabObject, PhysGrabObject);
    FIELD(gunMuzzle, Unity::Transform);
    FIELD(gunRange, float);

    METHOD_WRAPPER(Shoot);
    METHOD_WRAPPER(Update);
};

struct Item : Unity::Object
{
    using Unity::Object::Object;

    META("Assembly-CSharp", "", "Item");

    FIELD(prefab, PrefabRef);
    FIELD(itemName, System::String);
    FIELD(itemType, int);
};

struct ItemEquippable : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "ItemEquippable");
    bool IsEquipped() { THROW_IF_NULL(); return s_IsEquipped.Call<bool>(*this); }
    void RequestEquip(int spot, int requestingPlayerId) { THROW_IF_NULL(); s_RequestEquip.Call<void>(*this, spot, requestingPlayerId); }
    void RequestUnequip() { THROW_IF_NULL(); s_RequestUnequip.Call<void>(*this); }
private:
    METHOD_WRAPPER(IsEquipped);
    METHOD_WRAPPER(RequestEquip);
    METHOD_WRAPPER(RequestUnequip);
};

struct InventorySpot : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "InventorySpot");
    FIELD(inventorySpotIndex, int);
    bool IsOccupied() { THROW_IF_NULL(); return s_IsOccupied.Call<bool>(*this); }
    ItemEquippable CurrentItem() { THROW_IF_NULL(); return s_get_CurrentItem.Call<ItemEquippable>(*this); }
    METHOD_WRAPPER(Update);
private:
    METHOD_WRAPPER(IsOccupied);
    METHOD_WRAPPER(get_CurrentItem);
};

struct Inventory : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "Inventory");
    STATIC_FIELD(instance, Inventory);
    FIELD(spotsFeched, bool);
    int GetFirstFreeInventorySpotIndex() { THROW_IF_NULL(); return s_GetFirstFreeInventorySpotIndex.Call<int>(*this); }
    InventorySpot GetSpotByIndex(int index) { THROW_IF_NULL(); return s_GetSpotByIndex.Call<InventorySpot>(*this, index); }
private:
    METHOD_WRAPPER(GetFirstFreeInventorySpotIndex);
    METHOD_WRAPPER(GetSpotByIndex);
};

struct StatsManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "StatsManager");

    void LoadItemsFromFolder()
    {
        THROW_IF_NULL();
        s_LoadItemsFromFolder.Call<void, StatsManager>(*this);
    }

    STATIC_FIELD(instance, StatsManager);
    FIELD(itemDictionary, System::Dictionary<System::String COMMA Item>);
    void ItemPurchase(System::String itemName) { THROW_IF_NULL(); s_ItemPurchase.Call<void>(*this, itemName); }
    METHOD_WRAPPER(LoadItemsFromFolder);
    METHOD_WRAPPER(ItemPurchase);
};

struct PhotonEventData : System::Object
{
    using System::Object::Object;
    META("Photon3Unity3D", "ExitGames.Client.Photon", "EventData");
    FIELD(Code, System::Byte);
    System::Object CustomData() { THROW_IF_NULL(); return s_get_CustomData.Call<System::Object>(*this); }
    METHOD_WRAPPER(get_CustomData);
};

struct LoadBalancingClient : System::Object
{
    using System::Object::Object;
    META("PhotonRealtime", "Photon.Realtime", "LoadBalancingClient");
    void OnEvent(PhotonEventData eventData) { THROW_IF_NULL(); s_OnEvent.Call<void>(*this, eventData); }
    METHOD_WRAPPER(OnEvent);
};

struct ExtractionPoint : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ExtractionPoint");

    FIELD(isLocked, bool);
    FIELD(currentState, int);
    FIELD(haulCurrent, int);

    void HaulGoalSet(int value)
    {
        THROW_IF_NULL();
        s_HaulGoalSet.Call<void, ExtractionPoint, int>(*this, value);
    }

    void OnClick()
    {
        THROW_IF_NULL();
        s_OnClick.Call<void, ExtractionPoint>(*this);
    }

private:
    METHOD_WRAPPER(HaulGoalSet);
    METHOD_WRAPPER(OnClick);
};

struct ItemValuableBox : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "ItemValuableBox");
    FIELD(currentValue, float);
    FIELD(extractionValue, float);
    FIELD(inExtractionPoint, bool);
};

struct PhysGrabCart : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "PhysGrabCart");
    FIELD(itemsInCart, System::List<PhysGrabObject>);
    FIELD(haulCurrent, int);
};

struct CosmeticWorldObject : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "CosmeticWorldObject");

    FIELD(rarity, int);
};

struct RoundDirector : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "RoundDirector");

    void ExtractionPointsUnlock()
    {
        THROW_IF_NULL();
        s_ExtractionPointsUnlock.Call<void>(*this);
    }

    STATIC_FIELD(instance, RoundDirector);
    FIELD(extractionPointActive, bool);
    FIELD(extractionPointCurrent, ExtractionPoint);
    FIELD(extractionPointList, System::List<Unity::GameObject>);
    FIELD(currentHaul, int);
    FIELD(totalHaul, int);
    FIELD(dollarHaulList, System::List<Unity::GameObject>);
    FIELD(valuableBoxHaulList, System::List<ItemValuableBox>);
    FIELD(cosmeticWorldObjects, System::List<CosmeticWorldObject>);

private:
    METHOD_WRAPPER(ExtractionPointsUnlock);
};

struct ExtractionPoint_State
{
    META("Assembly-CSharp", "", "ExtractionPoint/State");

    ENUM_VALUE(Complete);
    ENUM_VALUE(Active);
    ENUM_VALUE(Idle);
};

struct TruckSafetySpawnPoint : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "TruckSafetySpawnPoint");

    STATIC_FIELD(instance, TruckSafetySpawnPoint);
};

struct PlayerDeathHead : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerDeathHead");

    FIELD(colliders, System::Array<Unity::Collider>);
    FIELD(meshRenderers, System::List<Unity::MeshRenderer>);
    FIELD(eyeRenderers, System::Array<Unity::MeshRenderer>);
};

struct MetaManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "MetaManager");

    bool CosmeticUnlockAll()
    {
        THROW_IF_NULL();
        return s_CosmeticUnlockAll.Call<bool, MetaManager>(*this);
    }

    void Reset()
    {
        THROW_IF_NULL();
        s_Reset.Call<void, MetaManager>(*this);
    }

    STATIC_FIELD(instance, MetaManager);
    FIELD(cosmeticUnlocks, System::List<System::Int32>);
    FIELD(cosmeticEquipped, System::List<System::Int32>);
    FIELD(cosmeticAssets, System::List<CosmeticAsset>);
    FIELD(colors, System::List<SemiColor>);
    FIELD(colorsEquipped, System::Array<System::Int32>);

    bool CosmeticEquip(CosmeticAsset asset, bool save)
    {
        THROW_IF_NULL();
        return s_CosmeticEquip.Call<bool, MetaManager, CosmeticAsset, bool>(*this, asset, save);
    }

    bool CosmeticUnequip(CosmeticAsset asset, bool save, bool update)
    {
        THROW_IF_NULL();
        return s_CosmeticUnequip.Call<bool, MetaManager, CosmeticAsset, bool, bool>(*this, asset, save, update);
    }

    void CosmeticColorSet(int colorType, int colorIndex)
    {
        THROW_IF_NULL();
        s_CosmeticColorSet.Call<void, MetaManager, int, int>(*this, colorType, colorIndex);
    }

    void CosmeticPreviewSet(bool enabled)
    {
        THROW_IF_NULL();
        s_CosmeticPreviewSet.Call<void, MetaManager, bool>(*this, enabled);
    }

    bool CosmeticPlayerUpdateLocal(bool save, bool update)
    {
        THROW_IF_NULL();
        return s_CosmeticPlayerUpdateLocal.Call<bool, MetaManager, bool, bool>(*this, save, update);
    }
private:
    METHOD_WRAPPER(CosmeticUnlockAll);
    METHOD_WRAPPER(Reset);
    METHOD_WRAPPER(CosmeticEquip);
    METHOD_WRAPPER(CosmeticUnequip);
    METHOD_WRAPPER(CosmeticColorSet);
    METHOD_WRAPPER(CosmeticPreviewSet);
    METHOD_WRAPPER(CosmeticPlayerUpdateLocal);
};

struct SteamManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "SteamManager");
    STATIC_FIELD(instance, SteamManager);
    FIELD(currentLobby, Steamworks::Data::Lobby);
    FIELD(joinLobby, bool);
    void LeaveLobby() { THROW_IF_NULL(); s_LeaveLobby.Call<void, SteamManager>(*this); }
    void JoinLobby(Steamworks::SteamId id) { THROW_IF_NULL(); s_JoinLobby.Call<void, SteamManager, Steamworks::SteamId>(*this, id); }
private:
    METHOD_WRAPPER(LeaveLobby);
    METHOD_WRAPPER(JoinLobby);
};

struct NetworkManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Assembly-CSharp", "", "NetworkManager");
    STATIC_FIELD(instance, NetworkManager);
    FIELD(leavePhotonRoom, bool);
};

struct EventSystem : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("UnityEngine.UI", "UnityEngine.EventSystems", "EventSystem");

    METHOD_WRAPPER(Update);
};

struct Cursor
{
    META("UnityEngine.CoreModule", "UnityEngine", "Cursor");
    METHOD_WRAPPER(set_visible);
};

struct PostProcessLayer : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;
    META("Unity.Postprocessing.Runtime", "UnityEngine.Rendering.PostProcessing", "PostProcessLayer");
    METHOD_WRAPPER(BuildCommandBuffers);
};

struct QualitySettings
{
    META("UnityEngine.CoreModule", "UnityEngine", "QualitySettings");
    static int Shadows() { return s_get_shadows.Call<int>(); }
    static void Shadows(int value) { s_set_shadows.Call<void, int>(value); }
    static float ShadowDistance() { return s_get_shadowDistance.Call<float>(); }
    static void ShadowDistance(float value) { s_set_shadowDistance.Call<void, float>(value); }
    static int AntiAliasing() { return s_get_antiAliasing.Call<int>(); }
    static void AntiAliasing(int value) { s_set_antiAliasing.Call<void, int>(value); }
    static int PixelLightCount() { return s_get_pixelLightCount.Call<int>(); }
    static void PixelLightCount(int value) { s_set_pixelLightCount.Call<void, int>(value); }
    static float LodBias() { return s_get_lodBias.Call<float>(); }
    static void LodBias(float value) { s_set_lodBias.Call<void, float>(value); }
    static bool SoftParticles() { return s_get_softParticles.Call<bool>(); }
    static void SoftParticles(bool value) { s_set_softParticles.Call<void, bool>(value); }
    static bool SoftVegetation() { return s_get_softVegetation.Call<bool>(); }
    static void SoftVegetation(bool value) { s_set_softVegetation.Call<void, bool>(value); }
    static bool RealtimeReflectionProbes() { return s_get_realtimeReflectionProbes.Call<bool>(); }
    static void RealtimeReflectionProbes(bool value) { s_set_realtimeReflectionProbes.Call<void, bool>(value); }
private:
    METHOD_WRAPPER(get_shadows);
    METHOD_WRAPPER(set_shadows);
    METHOD_WRAPPER(get_shadowDistance);
    METHOD_WRAPPER(set_shadowDistance);
    METHOD_WRAPPER(get_antiAliasing);
    METHOD_WRAPPER(set_antiAliasing);
    METHOD_WRAPPER(get_pixelLightCount);
    METHOD_WRAPPER(set_pixelLightCount);
    METHOD_WRAPPER(get_lodBias);
    METHOD_WRAPPER(set_lodBias);
    METHOD_WRAPPER(get_softParticles);
    METHOD_WRAPPER(set_softParticles);
    METHOD_WRAPPER(get_softVegetation);
    METHOD_WRAPPER(set_softVegetation);
    METHOD_WRAPPER(get_realtimeReflectionProbes);
    METHOD_WRAPPER(set_realtimeReflectionProbes);
};

struct RenderSettings
{
    META("UnityEngine.CoreModule", "UnityEngine", "RenderSettings");
    static bool Fog() { return s_get_fog.Call<bool>(); }
    static void Fog(bool value) { s_set_fog.Call<void, bool>(value); }
private:
    METHOD_WRAPPER(get_fog);
    METHOD_WRAPPER(set_fog);
};

struct Physics
{
    META("UnityEngine.PhysicsModule", "UnityEngine", "Physics");
    METHOD_WRAPPER_EX(Raycast, "System.Boolean(UnityEngine.Vector3,UnityEngine.Vector3,UnityEngine.RaycastHit&,System.Single,System.Int32,UnityEngine.QueryTriggerInteraction)");
};

struct Camera
{
    META("UnityEngine.CoreModule", "UnityEngine", "Camera");
    METHOD_WRAPPER(FireOnPreRender);
};

