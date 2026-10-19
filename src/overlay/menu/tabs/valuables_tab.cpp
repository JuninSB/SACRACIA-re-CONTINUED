#include <src/pch.h>

#include <src/cheat.h>

#include "../localization.h"
#include "../widgets.h"

#define LINE_ID (HAX_LINE * 789)

namespace Cheat
{
    static PrefabRef s_SelectedValuable = nullptr;
    static PrefabRef s_SelectedMoney = nullptr;
    static int s_CustomMoneyValue = 1000;

    void DrawValuablesTab()
    {
        const Hax::Vector2 mainAreaSize = Hax::Gui::GetContentRegionAvail();
        const float spacing = 20_px;
        const Hax::Vector2 columnSize = {(mainAreaSize.X - 3 * spacing) / 2, mainAreaSize.Y};

        // Column 1
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 304, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            if (!ShouldHideHostOnlyOptions())
            {
            BeginPanel(LINE_ID + 10);
            PanelHeader(L"Spawn valuable", G->Loc[LocKey_AvailableIfHost]);
            const float buttonWidth = CalcButtonSize(G->Loc[LocKey_SPAWN]).X;
            const float dropdownWidth = Hax::Gui::GetContentRegionAvail().X - buttonWidth - 5_px;
            const size_t valuableCount = G->ValuablesPool.Size();
            constexpr size_t valuablePopupId = Hax::Hash(L"ValuableSpawnSelect");
            Hax::Gui::BeginHorizontal(5_px);
            Hax::Gui::BeginVertical(3_px);
            Hax::WStringView valuablePreview = s_SelectedValuable != nullptr
                ? s_SelectedValuable.prefabName().ToHaxView() : Hax::WStringView(L"Select valuable");
            if (DropdownBtn(LINE_ID + 10, valuablePreview, dropdownWidth))
                OpenPopup(valuablePopupId, Hax::Gui::GetCursorPos());
            Hax::Gui::EndVertical();
            const float popupHeight = CalcButtonHeight() * (float)Hax::Max(1ULL, Hax::Min(valuableCount, 8ULL)) + 10_px;
            if (BeginDropList(valuablePopupId, {dropdownWidth, popupHeight}))
            {
                for (size_t i = 0; i < valuableCount; ++i)
                {
                    const auto& entry = G->ValuablesPool.begin()[i];
                    if (Selectable(LINE_ID + 11 + i, entry.key, s_SelectedValuable == entry.value, {.MinW = dropdownWidth}))
                    {
                        s_SelectedValuable = entry.value;
                        ClosePopup(valuablePopupId);
                    }
                }
                EndDropList();
            }
            if (Button(LINE_ID + 10, G->Loc[LocKey_SPAWN], {},
                {.Enabled = G->IsInGame && !G->IsClient && s_SelectedValuable != nullptr, .MinW = buttonWidth}))
                G->ValuableToSpawn = s_SelectedValuable;
            Hax::Gui::EndHorizontal();

            HorizontalLine(1_px);
            MainLabel(L"Custom Money value");
            Hax::WStringBuilder<32> moneyLabel;
            moneyLabel.AppendF(L"%d $", s_CustomMoneyValue);
            SliderEx(LINE_ID + 12, L"Value", moneyLabel.View(), &s_CustomMoneyValue, 100, 100000, SliderConvertInt);
            Hax::Gui::BeginHorizontal(5_px);
            const float moneyDropWidth = Hax::Gui::GetContentRegionAvail().X * 0.65f;
            constexpr size_t moneyPopupId = Hax::Hash(L"MoneyValuableSelect");
            Hax::Gui::BeginVertical(3_px);
            Hax::WStringView moneyPreview = s_SelectedMoney != nullptr
                ? s_SelectedMoney.prefabName().ToHaxView() : Hax::WStringView(L"Select Money valuable");
            if (DropdownBtn(LINE_ID + 13, moneyPreview, moneyDropWidth))
                OpenPopup(moneyPopupId, Hax::Gui::GetCursorPos());
            Hax::Gui::EndVertical();
            if (BeginDropList(moneyPopupId, {moneyDropWidth, popupHeight}))
            {
                for (size_t i = 0; i < valuableCount; ++i)
                {
                    const auto& entry = G->ValuablesPool.begin()[i];
                    const wchar_t* name = entry.key.begin();
                    if (!name || !wcsstr(name, L"Money")) continue;
                    if (Selectable(LINE_ID + 14 + i, entry.key, s_SelectedMoney == entry.value, {.MinW = moneyDropWidth}))
                    {
                        s_SelectedMoney = entry.value;
                        ClosePopup(moneyPopupId);
                    }
                }
                EndDropList();
            }
            if (Button(LINE_ID + 15, L"Spawn Money", {},
                {.Enabled = G->IsInGame && !G->IsClient && s_SelectedMoney != nullptr, .MinW = Hax::Gui::GetContentRegionAvail().X}))
            {
                G->ValuableToSpawn = s_SelectedMoney;
                G->SpawnMoneyValue = s_CustomMoneyValue;
            }
            Hax::Gui::EndHorizontal();
            EndPanel();
            }

            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_VISUALS]);
            {
                {
                    ToggleEx(LINE_ID, G->ValuablesEsp, G->Loc[LocKey_DisplayThroughWalls], G->Loc[LocKey_WithinSelectedDistance]);
                    HorizontalLine(1_px);
                    ToggleEx(LINE_ID, G->HideStoredValuables, G->Loc[LocKey_HideStored]);
                }

                HorizontalLine(1_px);

                {
                    Hax::WStringBuilder<16> sb;
                    sb.AppendF(L"%dm", G->ValuablesEspRange);
                    SliderEx(LINE_ID, G->Loc[LocKey_DisplayDistance], sb.View(), &G->ValuablesEspRange, 5, 500, SliderConvertInt);
                }

                HorizontalLine(1_px);

                {
                    ToggleEx(LINE_ID, G->ValuablesChams, G->Loc[LocKey_XrayHighlight], G->Loc[LocKey_NotAllHighlighted]);
                }
            }
            EndPanel();

            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_VALUE]);
            {
                {
                    ToggleEx(LINE_ID, G->Unbreakable, G->Loc[LocKey_NoDamageValuables], G->Loc[LocKey_AvailableIfHost], {.Disabled = G->IsClient});
                }

                HorizontalLine(1_px);

                {
                    MainLabel(G->Loc[LocKey_ChangeValue]);
                    Hax::Gui::BeginHorizontal(5_px);
                    {
                        const float w = CalcWidgetEqWidth(2);
                        if (Button(LINE_ID, G->Loc[LocKey_ToZero], G->Loc[LocKey_HostOnly], {.Enabled = G->IsInGame && !G->IsClient, .MinW = w}))
                            G->SetToZero = true;
                        if (Button(LINE_ID, G->Loc[LocKey_ToMax], G->Loc[LocKey_HostOnly], {.Enabled = G->IsInGame  && !G->IsClient, .MinW = w}))
                            G->SetToMax = true;
                    }
                    Hax::Gui::EndHorizontal();
                }
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();

        // Column 2
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 305, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_EXTRACTION]);
            {
                {
                    ToggleEx(LINE_ID, G->ExtrPointsEsp, G->Loc[LocKey_DisplayThroughWalls]);
                }

                HorizontalLine(1_px);

                RoundDirector dir = RoundDirector::instance();
                bool disabled = !G->IsInGame || !dir || dir.extractionPointActive();
                if (Button(LINE_ID, G->Loc[LocKey_ActivateNext], {}, {.Enabled = !disabled, .MinW = Hax::Gui::GetContentRegionAvail().X}))
                    G->ActivateNextPoint = true;
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }

}
