#include <src/pch.h>

#include <src/cheat.h>

#include "menu/menu.h"
#include "esp/esp.h"

namespace Cheat
{
    static bool IsKeyJustDown(size_t id, Hax::uint8 vk)
    {
        return (G->KeyListenerId == 0 || G->KeyListenerId == id) && Hax::Gui::IsKeyJustDown(vk);
    }

    void RenderOverlay()
    {
        if (IsKeyJustDown(HAX_LINE, VK_INSERT))
        {
            if (!G->MenuVisible)
                ToggleMenuVisibility();
            else
                CenterMenu();
        }
        else if (IsKeyJustDown(HAX_LINE, G->VkOpenClose))
            ToggleMenuVisibility();

        Hax::Gui::SwitchLayer(L"Background");
        RenderEsp();
        if (G->MenuVisible && G->DarkenBackground)
            Hax::Gui::DrawRect(Hax::Vector2(), Hax::Gui::GetViewportSize(), {.FillColor = 0x000000E6});
        Hax::Gui::RestoreLayer();

        if (G->MenuVisible)
        {
            Hax::Gui::SetMouseIcon(Hax::Gui::MouseIcon_Arrow);
            RenderMenu();
        }
    }
}
