#include <src/pch.h>

#include <src/cheat.h>

#include "../localization.h"
#include "../widgets.h"

#define LINE_ID (HAX_LINE * 891)

namespace Cheat
{
    void DrawVisionTab()
    {
        const Hax::Vector2 mainAreaSize = Hax::Gui::GetContentRegionAvail();
        const float spacing = 20_px;
        const Hax::Vector2 columnSize = {(mainAreaSize.X - 3 * spacing) / 2, mainAreaSize.Y};

        // Column 1
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 288, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_VIEW]);
            {
                {
                    ToggleEx(LINE_ID, G->BetterVision, G->Loc[LocKey_BetterVision]);
                }

                HorizontalLine(1_px);

                {
                    ToggleEx(LINE_ID, G->ThirdPerson, G->Loc[LocKey_ThirdPerson]);
                }

                HorizontalLine(1_px);

                {
                    Hax::char16 buf[32] = {};
                    swprintf_s(buf, _countof(buf), G->Fov < 61 ? G->Loc[LocKey_Default].Data() : G->Loc[LocKey_Degrees].Data(), G->Fov);

                    SliderEx(LINE_ID, G->Loc[LocKey_FieldOfView], buf, &G->Fov, 60, 140, SliderConvertInt);
                }
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();

        // Column 2
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 289, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_FLASHLIGHT]);
            {
                {
                    int& intensity = G->Flashlight.Intensity;
                    Hax::char16 buf[16] = {};
                    swprintf_s(buf, _countof(buf), intensity < 11 ? G->Loc[LocKey_Default].Data() : L"x%.1f", intensity / 10.f);

                    SliderEx(LINE_ID, G->Loc[LocKey_LightIntensity], buf, &intensity, 10, 20, SliderConvertInt);
                }

                HorizontalLine(1_px);

                {
                    int& angle = G->Flashlight.Angle;
                    Hax::char16 buf[16] = {};
                    swprintf_s(buf, _countof(buf), G->Loc[LocKey_Degrees].Data(), angle);

                    SliderEx(LINE_ID, G->Loc[LocKey_IlluminationAngle], buf, &angle, 60, 120, SliderConvertInt);
                }

                HorizontalLine(1_px);

                {
                    ToggleEx(LINE_ID, G->Flashlight.InCrouch, G->Loc[LocKey_WorksWhileCrouching]);
                }
            }
            EndPanel();

            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_DEADHEAD]);
            {
                ToggleEx(LINE_ID, G->MaxHeadBattery, G->Loc[LocKey_MaxBatteryHead]);
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }
}