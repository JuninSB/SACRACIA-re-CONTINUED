#include <src/pch.h>

#include <src/cheat.h>

#include "../localization.h"
#include "../widgets.h"

#define LINE_ID (HAX_LINE * 911)

namespace Cheat
{
    void DrawPerformanceTab()
    {
        const float spacing = 20_px;
        const Hax::Vector2 area = Hax::Gui::GetContentRegionAvail();
        const float width = (area.X - spacing * 3.f) / 2.f;

        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 400, {.W = width, .H = area.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        BeginPanel(LINE_ID);
        PanelHeader(G->Loc[LocKey_Performance]);
        ToggleEx(LINE_ID, G->PerfDisableShadows, G->Loc[LocKey_PerfShadows]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID, G->PerfDisableAntiAliasing, G->Loc[LocKey_PerfAntiAliasing]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID, G->PerfDisablePixelLights, G->Loc[LocKey_PerfPixelLights]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID, G->PerfDisableLodBias, G->Loc[LocKey_PerfLodBias]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID, G->PerfDisableFog, G->Loc[LocKey_PerfFog]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID, G->PerfDisableSoftParticles, G->Loc[LocKey_PerfSoftParticles]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID, G->PerfDisableSoftVegetation, G->Loc[LocKey_PerfSoftVegetation]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID, G->PerfDisableReflectionProbes, G->Loc[LocKey_PerfReflections]);
        HorizontalLine(1_px);
        ToggleEx(LINE_ID, G->PerfDisablePostProcessing, G->Loc[LocKey_PerfPostProcessing]);
        HorizontalLine(1_px);
        EndPanel();
        Hax::Gui::Dummy({0.f, 0.f});
        BeginPanel(LINE_ID + 2);
        PanelHeader(G->Loc[LocKey_FpsCounter]);
        ToggleEx(LINE_ID + 2, G->FpsCounter, G->Loc[LocKey_FpsCounter]);
        if (G->FpsCounter)
        {
            HorizontalLine(1_px);
            static constexpr LocKey corners[] = {LocKey_FpsTopLeft, LocKey_FpsTopRight, LocKey_FpsBottomLeft, LocKey_FpsBottomRight};
            if (Button(LINE_ID + 2, G->Loc[LocKey_FpsPosition], G->Loc[corners[G->FpsCorner]], {.MinW = Hax::Gui::GetContentRegionAvail().X}))
                G->FpsCorner = (G->FpsCorner + 1) % 4;
            HorizontalLine(1_px);
            Hax::char16 fontSize[20]{};
            swprintf_s(fontSize, _countof(fontSize), L"%d px", G->FpsFontSize);
            SliderEx(LINE_ID + 2, G->Loc[LocKey_FpsSize], fontSize, &G->FpsFontSize, 8, 20, SliderConvertInt);
        }
        EndPanel();
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();

    }
}
