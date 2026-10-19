#include <src/pch.h>

#include <src/cheat.h>

#include "../localization.h"
#include "../widgets.h"

#include <chrono>
#include <mutex>
#include <array>
#include <string>
#include <vector>

#define LINE_ID (HAX_LINE * 981)

namespace Cheat
{
    // Unity objects must be read on Unity's main thread. Spread Runtime's scans
    // over several frames instead of starting all list traversals together.
    static int RuntimeRefreshPhase()
    {
        using Clock = std::chrono::steady_clock;
        static Clock::time_point lastPhase{};
        static int phase = 0;
        const auto now = Clock::now();
        if (now - lastPhase < std::chrono::milliseconds(150)) return -1;
        lastPhase = now;
        const int current = phase;
        phase = (phase + 1) % 6;
        return current;
    }

    using EnemyNameCount = std::pair<std::wstring, int>;
    using EnemyTierNames = std::array<std::vector<EnemyNameCount>, 3>;
    static bool IsLiveEnemy(EnemyParent parent);

    struct RuntimePlayerInfo { std::wstring Name; int Health = 0; int MaxHealth = 0; bool Dead = false; };
    static bool runtimeSnapshotInGame = false;
    static std::mutex runtimeSnapshotMutex;
    static std::wstring runtimeStatus;
    static std::wstring runtimeLevel;
    static int cachedValuables = 0, cachedGroundItems = 0;
    static float cachedRemainingValue = 0.f;
    static EnemyTierNames cachedEnemyTiers;
    static std::vector<RuntimePlayerInfo> cachedPlayerInfo;
    static int cachedPlayers = 0, cachedCartValue = 0, cachedExtractionValue = 0;
    static int cachedBattery = -1;
    static volatile LONG runtimeTabActive = 0;

    void SetRuntimeTabActive(bool active)
    {
        ::InterlockedExchange(&runtimeTabActive, active ? 1 : 0);
    }

    static void AddEnemyName(EnemyTierNames& tiers, EnemyParent parent)
    {
        if (!IsLiveEnemy(parent)) return;
        const int tier = parent.difficulty();
        if (tier < 0 || tier >= (int)tiers.size()) return;
        System::String systemName = parent.enemyName();
        const wchar_t* name = systemName != nullptr ? systemName.GetRawStringData() : nullptr;
        if (!name || !*name) return;
        auto& names = tiers[tier];
        for (auto& entry : names)
            if (entry.first == name) { ++entry.second; return; }
        names.emplace_back(name, 1);
    }

    void UpdateRuntimeSnapshot()
    {
        if (::InterlockedCompareExchange(&runtimeTabActive, 0, 0) == 0)
            return;

        if (!G->IsInGame)
        {
            std::lock_guard<std::mutex> lock(runtimeSnapshotMutex);
            runtimeSnapshotInGame = false;
            runtimeStatus = L"Menu";
            runtimeLevel.clear();
            cachedValuables = cachedGroundItems = cachedCartValue = cachedExtractionValue = cachedPlayers = 0;
            cachedRemainingValue = 0.f;
            cachedBattery = -1;
            for (auto& tier : cachedEnemyTiers) tier.clear();
            cachedPlayerInfo.clear();
            return;
        }
        const int phase = RuntimeRefreshPhase();
        if (phase < 0) return;
        std::lock_guard<std::mutex> lock(runtimeSnapshotMutex);
        runtimeSnapshotInGame = true;
        if (phase == 0)
        {
            const bool pt = G->Language == Lang_Pt;
            RunManager run = RunManager::instance();
            runtimeStatus = pt ? L"Hospedando" : L"Hosting";
            if (run && run.levelCurrent())
            {
                System::String narrative = run.levelCurrent().NarrativeName();
                const wchar_t* narrativeName = narrative != nullptr ? narrative.GetRawStringData() : nullptr;
                if (narrativeName && *narrativeName)
                {
                    const int levelNumber = run.levelsCompleted() + 1;
                    wchar_t levelLabel[256]{};
                    _snwprintf_s(levelLabel, _countof(levelLabel), _TRUNCATE,
                        pt ? L"Nível %d | %ls" : L"Level %d | %ls", levelNumber, narrativeName);
                    runtimeLevel = levelLabel;
                }
                else runtimeLevel.clear();
                const wchar_t* name = run.levelCurrent().GetName().GetRawStringData();
                if (name && wcsstr(name, L"Tutorial")) runtimeStatus = L"Tutorial";
                else if (G->IsClient) runtimeStatus = pt ? L"Multijogador" : L"Multiplayer";
                else
                {
                    GameManager manager = GameManager::instance();
                    if (manager && manager.gameMode() == 0) runtimeStatus = pt ? L"Um jogador" : L"Singleplayer";
                }
            }

            ValuableDirector valuables = ValuableDirector::instance();
            System::List<ValuableObject> valuableList = valuables ? valuables.valuableList() : nullptr;
            cachedValuables = 0;
            cachedRemainingValue = 0.f;
            for (int i = 0; valuableList != nullptr && i < valuableList.Count(); ++i)
            {
                ValuableObject value = valuableList[i];
                if (!value) continue;
                ++cachedValuables;
                cachedRemainingValue += value.dollarValueCurrent();
            }
        }
        else if (phase == 1)
        {
            ItemManager items = ItemManager::instance();
            System::List<ItemAttributes> spawned = items ? items.spawnedItems() : nullptr;
            cachedGroundItems = 0;
            for (int i = 0; spawned != nullptr && i < spawned.Count(); ++i)
            {
                ItemAttributes item = spawned[i];
                if (!item || !item.GetEnabled()) continue;
                PhysGrabObject body = item.physGrabObject();
                if (body && !body.grabbed()) ++cachedGroundItems;
            }
        }
        else if (phase == 2)
        {
            EnemyDirector enemies = EnemyDirector::instance();
            System::List<EnemyParent> live = enemies ? enemies.enemiesSpawned() : nullptr;
            for (auto& tier : cachedEnemyTiers) tier.clear();
            for (int i = 0; live != nullptr && i < live.Count(); ++i)
                AddEnemyName(cachedEnemyTiers, live[i]);
        }
        else if (phase == 3)
        {
            GameDirector director = GameDirector::instance();
            System::List<PlayerAvatar> players = director ? director.PlayerList() : nullptr;
            cachedPlayerInfo.clear();
            for (int i = 0; players != nullptr && i < players.Count(); ++i)
            {
                PlayerAvatar player = players[i];
                if (!player) continue;
                RuntimePlayerInfo info;
                System::String name = player.playerName();
                if (name != nullptr) info.Name.assign(name.GetRawStringData());
                info.Dead = player.deadSet();
                if (!info.Dead)
                {
                    PlayerHealth hp = player.playerHealth();
                    if (hp) { info.Health = hp.health(); info.MaxHealth = hp.maxHealth(); }
                }
                cachedPlayerInfo.emplace_back(std::move(info));
            }
            cachedPlayers = (int)cachedPlayerInfo.size();
        }
        else if (phase == 4)
        {
            cachedCartValue = 0;
            auto carts = Unity::Object::FindObjectsOfType<PhysGrabCart>();
            for (int i = 0; carts != nullptr && i < carts.Length(); ++i)
                if (carts[i]) cachedCartValue += carts[i].haulCurrent();
            RoundDirector round = RoundDirector::instance();
            cachedExtractionValue = round ? round.currentHaul() : 0;
        }
        else
        {
            BatteryUI batteryUi = BatteryUI::instance();
            BatteryVisualLogic battery = batteryUi ? batteryUi.batteryVisualLogic() : nullptr;
            ItemBattery batteryItem = battery ? battery.itemBattery() : nullptr;
            const float batteryPercent = batteryItem ? batteryItem.batteryLife() : -1.f;
            cachedBattery = battery && batteryPercent >= 0.f && batteryPercent <= 100.f
                ? (int)std::lround(batteryPercent) : -1;
        }
    }

    static bool IsLiveEnemy(EnemyParent parent)
    {
        return parent && parent.Spawned() && parent.enemy() && !parent.enemy().Health().dead();
    }

    static void DrawTierNames(const std::vector<EnemyNameCount>& names)
    {
        if (names.empty()) { DescLabel(L"None"); return; }
        for (const auto& entry : names)
        {
            Hax::WStringBuilder<128> label;
            if (entry.second > 1) label.AppendF(L"%ls [%d]", entry.first.c_str(), entry.second);
            else label.Append(entry.first.c_str());
            DescLabel(label.View());
        }
    }

    void DrawRuntimeTab()
    {
        // Never make the render thread wait for Unity object enumeration. Runtime
        // refresh runs on Unity's main thread; this UI consumes the last completed
        // snapshot whenever the short lock is available.
        static bool drawInGame = false;
        static std::wstring drawStatus = L"Menu";
        static std::wstring drawLevel;
        static int drawValuables = 0, drawGroundItems = 0;
        static float drawRemainingValue = 0.f;
        static EnemyTierNames drawEnemyTiers;
        static std::vector<RuntimePlayerInfo> drawPlayerInfo;
        static int drawPlayers = 0, drawCartValue = 0, drawExtractionValue = 0;
        static int drawBattery = -1;
        {
            std::unique_lock<std::mutex> lock(runtimeSnapshotMutex, std::try_to_lock);
            if (lock.owns_lock())
            {
                drawInGame = runtimeSnapshotInGame;
                drawStatus = runtimeStatus;
                drawLevel = runtimeLevel;
                drawValuables = cachedValuables;
                drawGroundItems = cachedGroundItems;
                drawRemainingValue = cachedRemainingValue;
                drawEnemyTiers = cachedEnemyTiers;
                drawPlayerInfo = cachedPlayerInfo;
                drawPlayers = cachedPlayers;
                drawCartValue = cachedCartValue;
                drawExtractionValue = cachedExtractionValue;
                drawBattery = cachedBattery;
            }
        }
        static bool wasOpen = false;
        if (!wasOpen)
            Hax::Log(G->Logger, L"Runtime tab entered; inGame=%d client=%d", G->IsInGame, G->IsClient);
        wasOpen = true;
        // Poll only once per second; the overlay can redraw many times between
        // polls. Reuse the last safe snapshot for those frames.
        const float spacing = 20_px;
        const Hax::Vector2 area = Hax::Gui::GetContentRegionAvail();
        const float width = (area.X - spacing * 3.f) / 2.f;

        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 512, {.W = width, .H = area.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        BeginPanel(LINE_ID);
        PanelHeader(G->Loc[LocKey_Runtime]);
        MainLabel(G->Loc[LocKey_Status]);
        if (drawStatus.empty()) DescLabel(L"Menu");
        else DescLabel(Hax::WStringView(drawStatus.c_str()));
        if (!drawLevel.empty()) DescLabel(Hax::WStringView(drawLevel.c_str()));
        if (!drawInGame)
        {
            EndPanel();
            Hax::Gui::EndVertical();
            Hax::Gui::EndContainer();
            return;
        }

        Hax::WStringBuilder<48> summary;
        summary.AppendF(L"%ls: %d", G->Loc[LocKey_ValuablesTotal].begin(), drawValuables);
        HorizontalLine(1_px);
        DescLabel(summary.View());

        Hax::WStringBuilder<48> itemSummary;
        itemSummary.AppendF(L"%ls: %d", G->Loc[LocKey_ItemsOnGround].begin(), drawGroundItems);
        DescLabel(itemSummary.View());
        EndPanel();

        Hax::Gui::Dummy({0.f, 0.f});
        BeginPanel(LINE_ID + 1);
        PanelHeader(G->Loc[LocKey_MonsterTiers]);
        for (int tier = 0; tier < 3; ++tier)
        {
            Hax::WStringBuilder<32> header;
            header.AppendF(L"Tier %d (%d)", tier + 1, (int)drawEnemyTiers[tier].size());
            MainLabel(header.View(), Hax::Gui::Color::White);
            DrawTierNames(drawEnemyTiers[tier]);
        }
        EndPanel();
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();

        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 513, {.W = width, .H = area.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        BeginPanel(LINE_ID + 2);
        PanelHeader(G->Loc[LocKey_PLAYERS]);
        for (const RuntimePlayerInfo& player : drawPlayerInfo)
        {
            Hax::WStringBuilder<96> line;
            line.Append(player.Name.c_str());
            if (player.Dead)
            {
                line.Append(L"  DEAD");
                MainLabel(line.View(), Hax::Gui::Color::Red);
            }
            else
            {
                line.AppendF(L"  [%d/%d]", player.Health, player.MaxHealth);
                MainLabel(line.View());
            }
        }
        EndPanel();

        Hax::Gui::Dummy({0.f, 0.f});
        BeginPanel(LINE_ID + 3);
        PanelHeader(G->Loc[LocKey_CartValue]);
        const int cartValue = drawCartValue;
        const int extractionValue = drawExtractionValue;
        const float mapValue = std::max(0.f, drawRemainingValue - (float)cartValue - (float)extractionValue);
        Hax::WStringBuilder<64> cart;
        cart.AppendF(L"%d $", cartValue);
        DescLabel(cart.View());
        MainLabel(G->Loc[LocKey_ExtractionValue]);
        Hax::WStringBuilder<64> extracted;
        extracted.AppendF(L"%d $", extractionValue);
        DescLabel(extracted.View());
        MainLabel(G->Loc[LocKey_MapValue]);
        Hax::WStringBuilder<64> map;
        map.AppendF(L"%.0f $", mapValue);
        DescLabel(map.View());
        MainLabel(G->Loc[LocKey_TruckBattery]);
        Hax::WStringBuilder<32> batteryText;
        if (drawBattery >= 0) batteryText.AppendF(L"%d%%", drawBattery);
        else batteryText.Append(L"--");
        DescLabel(batteryText.View());
        EndPanel();
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }
}
