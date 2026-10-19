#include <src/pch.h>

#include <src/cheat.h>

#include "../localization.h"
#include "../widgets.h"

#define LINE_ID (HAX_LINE * 567)

namespace Cheat
{
    void DrawSettingsTab()
    {
        const Hax::Vector2 mainAreaSize = Hax::Gui::GetContentRegionAvail();
        const float spacing = 20_px;
        const Hax::Vector2 columnSize = {(mainAreaSize.X - 3 * spacing) / 2, mainAreaSize.Y};

        // Column 1
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 384, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(HAX_LINE);
            PanelHeader(G->Loc[LocKey_SETTINGS]);
            {
                {
                    MainLabel(G->Loc[LocKey_Language]);

                    constexpr size_t dropListId = Hax::Hash(L"LanguageSelect");
                    static Hax::WStringView s_Languages[] = { L"English", L"Ð ÑƒÑÑÐºÐ¸Ð¹", L"PortuguÃªs" };
                    const float w = Hax::Gui::GetContentRegionAvail().X;
                    Hax::Gui::BeginVertical(3_px);
                    if (DropdownBtn(HAX_LINE, s_Languages[G->Language], w))
                        OpenPopup(dropListId, Hax::Gui::GetCursorPos());
                    Hax::Gui::EndVertical();

                    const float selectableH = CalcButtonHeight();
                    const float dropListH = selectableH * Hax::Max(1ULL, Hax::Min(_countof(s_Languages), 5ULL)) + 5_px * 2.f + 1.f;
                    if (BeginDropList(dropListId, {w, dropListH}))
                    {
                        if (Selectable(HAX_LINE, s_Languages[0], G->Language == Lang_Eng, {.MinW = w})) { G->Loc = g_LocDict[Lang_Eng]; G->Language = Lang_Eng; }
                        if (Selectable(HAX_LINE, s_Languages[1], G->Language == Lang_Ru, {.MinW = w})) { G->Loc = g_LocDict[Lang_Ru]; G->Language = Lang_Ru; }
                        if (Selectable(HAX_LINE, s_Languages[2], G->Language == Lang_Pt, {.MinW = w})) { G->Loc = g_LocDict[Lang_Pt]; G->Language = Lang_Pt; }
                        EndDropList();
                    }
                }

                HorizontalLine(1_px);

                {
                    HotkeyEx(HAX_LINE, G->VkOpenClose, G->Loc[LocKey_Hotkey], G->Loc[LocKey_OpenCloseMenu]);
                }

                HorizontalLine(1_px);

                {
                    ToggleEx(HAX_LINE, G->DarkenBackground, G->Loc[LocKey_DarkenBackground]);

                HorizontalLine(1_px);
                ToggleEx(HAX_LINE + 1, G->HideHostOnlyOptions, G->Loc[LocKey_HideHostOnlyOptions]);

                HorizontalLine(1_px);
                Hax::WStringBuilder<16> espRateLabel;
                espRateLabel.AppendF(L"%d Hz", G->EspUpdateRate);
                SliderEx(HAX_LINE + 2, G->Loc[LocKey_EspUpdateRate], espRateLabel.View(), &G->EspUpdateRate, 10, 60, SliderConvertInt);
                HorizontalLine(1_px);
                ToggleEx(HAX_LINE + 3, G->EspMatchFrameRate, G->Loc[LocKey_EspMatchFrameRate]);
                Hax::WStringBuilder<16> frameIntervalLabel;
                frameIntervalLabel.AppendF(L"%d", G->EspFrameInterval);
                SliderEx(HAX_LINE + 4, G->Loc[LocKey_EspFrameInterval], frameIntervalLabel.View(), &G->EspFrameInterval, 1, 10, SliderConvertInt);

                }

                HorizontalLine(1_px);
                const wchar_t* benchmarkLabel = G->BenchmarkCapture ? L"Stop benchmark capture" : L"Start benchmark capture";
                if (Button(HAX_LINE + 20, benchmarkLabel, {}, {.MinW = Hax::Gui::GetContentRegionAvail().X}))
                {
                    G->BenchmarkCapture = !G->BenchmarkCapture;
                    SetBenchmarkCapture(G->BenchmarkCapture);
                }
                DescLabel(IsBenchmarkCaptureEnabled() ? L"Recording frame phases and managed Update methods. See Benchmark_Methods_*.csv beside the main CSV. Scene counts are sampled every 5 seconds." : L"Captures frame phases, managed Update methods, CPU/memory and low-rate scene counts. See docs/update-performance.md for limits.");
                HorizontalLine(1_px);
                MainLabel(G->Loc[LocKey_Version]);
                DescLabel(L"RepoHax 0.4.5.4 [PATCH]");
                HorizontalLine(1_px);
                if (Button(HAX_LINE, G->Loc[LocKey_RestoreSettings], {}, {.MinW = Hax::Gui::GetContentRegionAvail().X}))
                    RestoreSettings();
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }
}

