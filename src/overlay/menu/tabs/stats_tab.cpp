#include <src/pch.h>

#include <src/cheat.h>

#include "../localization.h"
#include "../widgets.h"

#define LINE_ID (HAX_LINE * 678)

namespace Cheat
{
    static bool IsPlayerAlive()
    {
        PlayerAvatar avatar = PlayerAvatar::instance();
        if (!avatar)
            return false;

        return !avatar.deadSet() && !avatar.isDisabled();
    }

    static bool IsPlayerFullHp()
    {
        if (!IsPlayerAlive())
            return true;

        PlayerHealth health = PlayerAvatar::instance().playerHealth();
        return health.health() >= health.maxHealth();
    }

    void DrawStatsTab()
    {
        const Hax::Vector2 mainAreaSize = Hax::Gui::GetContentRegionAvail();
        const float spacing = 20_px;
        const Hax::Vector2 columnSize = {(mainAreaSize.X - 3 * spacing) / 2, mainAreaSize.Y};

        PlayerAvatar avatar = PlayerAvatar::instance();
        PlayerHealth health = avatar ? avatar.playerHealth() : nullptr;

        // Column 1
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 256, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_HEALTH]);
            {
                ToggleEx(LINE_ID, G->Godmode, G->Loc[LocKey_Godmode]);

                HorizontalLine(1_px);

                bool disabled = !G->IsInGame || !avatar || !health || health.health() >= health.maxHealth();
                if (Button(LINE_ID, G->Loc[LocKey_HealToMax], {}, {.Enabled = !disabled, .MinW = Hax::Gui::GetContentRegionAvail().X}))
                    G->HealToMax = true;
            }
            EndPanel();

            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_MOVEMENT]);
            {
                ToggleEx(LINE_ID, G->InfStamina, G->Loc[LocKey_InfiniteStamina]);

                HorizontalLine(1_px);

                {
                    Hax::char16 buf[16]{};
                    swprintf_s(buf, _countof(buf), G->Accel.Walking == 1 ? G->Loc[LocKey_Default].Data() : L"%dx", G->Accel.Walking);

                    SliderEx(LINE_ID, G->Loc[LocKey_WalkingSpeed], buf, &G->Accel.Walking, 1, 5, SliderConvertInt);
                }

                HorizontalLine(1_px);

                {
                    Hax::char16 buf[16]{};
                    swprintf_s(buf, _countof(buf), G->Accel.Sprinting == 1 ? G->Loc[LocKey_Default].Data() : L"%dx", G->Accel.Sprinting);

                    SliderEx(LINE_ID, G->Loc[LocKey_RunningSpeed], buf, &G->Accel.Sprinting, 1, 5, SliderConvertInt);
                }

                HorizontalLine(1_px);

                {
                    Hax::char16 buf[16]{};
                    swprintf_s(buf, _countof(buf), G->Accel.Crouching == 1 ? G->Loc[LocKey_Default].Data() : L"%dx", G->Accel.Crouching);

                    SliderEx(LINE_ID, G->Loc[LocKey_CrouchingSpeed], buf, &G->Accel.Crouching, 1, 5, SliderConvertInt);
                }

                HorizontalLine(1_px);

                ToggleEx(LINE_ID, G->InfJumps, G->Loc[LocKey_InfiniteJumps]);

                HorizontalLine(1_px);

                ToggleEx(LINE_ID, G->NoTumble, G->Loc[LocKey_DontTumble], G->Loc[LocKey_DisableTumbling]);
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();

        // Column 2
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 257, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_POWERUPS]);
            {
                size_t id = LINE_ID;
                constexpr size_t settingsPopupId = Hax::Hash("PowerupPopup");
                ToggleEx(id, G->AutoUseUpgrs, G->Loc[LocKey_AutoApplyUpgrades], {}, {.SettingsId = settingsPopupId});

                if (BeginModal(settingsPopupId))
                {
                    const float px10 = 10_px;
                    Hax::Gui::BeginHorizontal(15_px);
                    {
                        size_t half = ((size_t)UpgradeType::N + 1) / 2;

                        Hax::Gui::Dummy(Hax::Vector2(px10, 0.f));
                        Hax::Gui::BeginVertical(5_px);
                        {
                            {
                                Hax::Gui::Dummy(Hax::Vector2(0.f, px10));
                                for (size_t i = 0; i < half; ++i)
                                {
                                    auto& upgrInfo = G->UpgradesData[i];
                                    CheckboxEx((size_t)&upgrInfo, upgrInfo.AutoUse, G->Loc[upgrInfo.LocKey]);
                                }
                                Hax::Gui::Dummy(Hax::Vector2(0.f, px10));
                            }
                        }
                        Hax::Gui::EndVertical();

                        Hax::Gui::BeginVertical(5_px);
                        {
                            Hax::Gui::Dummy(Hax::Vector2(0.f, px10));
                            for (size_t i = half; i < (size_t)UpgradeType::N; ++i)
                            {
                                auto& upgrInfo = G->UpgradesData[i];
                                CheckboxEx((size_t)&upgrInfo, upgrInfo.AutoUse, G->Loc[upgrInfo.LocKey]);
                            }
                            Hax::Gui::Dummy(Hax::Vector2(0.f, px10));
                        }

                        Hax::Gui::EndVertical();
                        Hax::Gui::Dummy(Hax::Vector2(px10, 0.f));
                    }
                    Hax::Gui::EndHorizontal();

                    EndModal();
                }

                HorizontalLine(1_px);

                {
                    MainLabel(G->Loc[LocKey_ManualApplication]);

                    Hax::Gui::BeginVertical(5_px);
                    Hax::Gui::BeginHorizontal(5_px);
                    {
                        const Hax::Vector<ItemUpgrade>& upgrades = G->UpgradesPool.GetFront();
                        for (size_t i = 0; i < upgrades.Size(); ++i)
                        {
                            ItemUpgrade item = upgrades[i];
                            if (!item || !item.itemAttributes() || !item.itemToggle())
                                continue;

                            UpgradeData& data = G->UpgradesData[(int)UpgradeType::N];
                            Hax::WStringView name = item.itemAttributes().itemAssetName().ToHaxView();
                            for (size_t j = 0; j < (size_t)UpgradeType::N; ++j)
                            {
                                if (name.EndsWith(G->UpgradesData[j].InternalNameEnd))
                                {
                                    data = G->UpgradesData[j];
                                    break;
                                }
                            }

                            bool disabled = item.itemToggle().toggleState() == true;
                            if (AutoLayoutButton(LINE_ID + i * 5542, G->Loc[data.LocKey], {.Disabled = disabled, .Icon = data.Icon}))
                                G->UpgradeToToggle = item;
                        }
                    }
                    Hax::Gui::EndHorizontal();
                    Hax::Gui::EndVertical();
                }
            }
            EndPanel();

            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_HEALTHPACKS], G->Loc[LocKey_ApplyAidKit]);
            {
                Hax::Gui::BeginVertical(5_px);
                Hax::Gui::BeginHorizontal(5_px);
                {
                    const Hax::Vector<ItemHealthPack>& aids = G->AidsPool.GetFront();
                    for (size_t i = 0; i < aids.Size(); ++i)
                    {
                        ItemHealthPack aid = aids[i];
                        if (!aid || !aid.itemAttributes() || !aid.itemToggle())
                            continue;

                        AidData& data = G->AidsData[(int)AidType::N];
                        Hax::WStringView name = aid.itemAttributes().itemAssetName().ToHaxView();
                        for (size_t j = 0; j < (size_t)AidType::N; ++j)
                        {
                            if (name.EndsWith(G->AidsData[j].InternalNameEnd))
                            {
                                data = G->AidsData[j];
                                break;
                            }
                        }

                        bool disabled = aid.itemToggle().toggleState() == true || IsPlayerFullHp();
                        if (AutoLayoutButton(LINE_ID + i * 5542, G->Loc[data.LocKey], {.Disabled = disabled, .Icon = data.Icon}))
                            G->AidToToggle = aid;
                    }
                }
                Hax::Gui::EndHorizontal();
                Hax::Gui::EndVertical();
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }

    void DrawGrabTab()
    {
        const float spacing = 20_px;
        const Hax::Vector2 area = Hax::Gui::GetContentRegionAvail();
        const float width = (area.X - spacing * 3.f) / 2.f;

        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 258, {.W = width, .H = area.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        BeginPanel(LINE_ID + 7000);
        PanelHeader(G->Loc[LocKey_GrabAssist]);
        ToggleEx(LINE_ID + 7000, G->GrabAssist, G->Loc[LocKey_GrabAssist]);
        HorizontalLine(1_px);
        {
            Hax::char16 radius[32]{};
            swprintf_s(radius, _countof(radius), L"%d px", G->GrabAssistRadius);
            SliderEx(LINE_ID + 7000, G->Loc[LocKey_CenterRadius], radius, &G->GrabAssistRadius, 25, 600, SliderConvertInt);
        }
        HorizontalLine(1_px);
        ToggleEx(LINE_ID + 7002, G->AntiMusicBoxSpin, G->Loc[LocKey_AntiMusicBoxSpin], G->Loc[LocKey_AntiMusicBoxSpinHint]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID + 7003, G->AntiDuck, G->Loc[LocKey_AntiDuck], G->Loc[LocKey_AntiDuckHint]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID + 7000, G->EasyGrab, G->Loc[LocKey_HighStrength], G->Loc[LocKey_AvailableIfHost], {.Disabled = G->IsClient});
        EndPanel();
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();

        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 259, {.W = width, .H = area.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        BeginPanel(LINE_ID + 7001);
        PanelHeader(G->Loc[LocKey_WallGrab]);
        ToggleEx(LINE_ID + 7001, G->GrabThrough, G->Loc[LocKey_GrabThrough]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID + 7001, G->WallGrab, G->Loc[LocKey_WallGrab]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID + 7001, G->UnlimGrabRange, G->Loc[LocKey_UnlimitedRange]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID + 7001, G->NoOvercharge, G->Loc[LocKey_NoOvercharge]);
        EndPanel();
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }
}
