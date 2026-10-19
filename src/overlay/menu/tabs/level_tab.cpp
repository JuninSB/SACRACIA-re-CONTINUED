#include <src/pch.h>

#include <src/cheat.h>

#include "../localization.h"
#include "../widgets.h"

#define LINE_ID (HAX_LINE * 345)

namespace Cheat
{
    void DrawLevelTab()
    {
        const Hax::Vector2 mainAreaSize = Hax::Gui::GetContentRegionAvail();
        const float spacing = 20_px;
        const Hax::Vector2 columnSize = {(mainAreaSize.X - 3 * spacing) / 2, mainAreaSize.Y};

        // Column 1
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 336, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_GENERAL]);
            {
                {
                    RunManager manager = RunManager::instance();
                    int levelsCompl = manager ? manager.levelsCompleted() : 0;

                    Hax::char16 buf[16] = {};
                    int nChars = swprintf_s(buf, _countof(buf), L"%d", levelsCompl);

                    if (nChars > 0 && !ShouldHideHostOnlyOptions())
                    {
                        Hax::WStringView valTxt = { buf, (size_t)nChars };
                        Hax::Vector2 valTxtSize = CalcMainLabelSize(valTxt);

                        float spacing = 2_px;
                        Hax::Vector2 btnSize = CalcRepeatBtnSize(L"+");

                        Hax::Gui::BeginHorizontal(spacing);
                        {
                            bool disabled = !manager || G->IsClient || !G->IsInGame;

                            MainLabelAlignedByH(G->Loc[LocKey_LevelsCompleted], btnSize.Y);
                            Hax::Gui::Space(Hax::Gui::GetContentRegionAvail().X - valTxtSize.X - spacing * 2.f - btnSize.X * 2.f - 8_px);
                            MainLabelAlignedByH(buf, btnSize.Y);
                            Hax::Gui::Space(8_px);
                            if (RepeatBtn(LINE_ID, L"-", !disabled && levelsCompl > 0)) manager.levelsCompleted()--;
                            if (RepeatBtn(LINE_ID, L"+", !disabled && levelsCompl < 100)) manager.levelsCompleted()++;
                        }
                        Hax::Gui::EndHorizontal();
                    }
                }

                HorizontalLine(1_px);

                bool disabled = G->IsClient || !G->IsInGame;
                if (!ShouldHideHostOnlyOptions() && Button(LINE_ID, G->Loc[LocKey_NextLevel], {}, {.Enabled = !disabled, .MinW = Hax::Gui::GetContentRegionAvail().X}))
                    G->ForceNextLevel = true;
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();

        // Column 2
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 337, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            if (!ShouldHideHostOnlyOptions())
            {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_MAPFILTER], G->Loc[LocKey_AvailableIfHost]);
            {
                Hax::Gui::BeginVertical(5_px);
                {
                    Hax::Gui::BeginHorizontal(5_px);
                    {
                        bool lastMap = G->TotalBans + 1 == G->LevelBans.Size();
                        for (size_t i = 0; i < G->LevelBans.Size(); ++i)
                        {
                            LevelBan& ban = G->LevelBans[i];
                            bool disabled = G->IsClient || lastMap && ban.Allowed;
                            if (Selectable(LINE_ID + i * 10000, ban.Name, !G->IsClient && !ban.Allowed, { .AutoLayout = true, .Disabled = disabled, .R = 5_px }))
                            {
                                ban.Allowed = !ban.Allowed;
                                G->TotalBans += ban.Allowed ? -1 : 1;
                            }
                        }
                    }
                    Hax::Gui::EndHorizontal();
                }
                Hax::Gui::EndVertical();
            }
            EndPanel();
            }
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }
}
