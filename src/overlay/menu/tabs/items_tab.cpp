#include <src/pch.h>

#include <src/cheat.h>

#include "../localization.h"
#include "../widgets.h"

#define LINE_ID (HAX_LINE * 234)

namespace Cheat
{
    static bool IsStorableDroppedItem(ItemAttributes attributes)
    {
        if (!attributes) return false;
        Item item = attributes.item();
        if (!item) return false;
        const int type = item.itemType();
        // Valuable containers, upgrade drops, crystals, medkits, carts and
        // vehicles do not belong in the player's three inventory slots.
        if (type == 2 || type == 3 || type == 4 || type == 5 || type == 8 || type == 14)
            return false;
        if (attributes.GetComponent<ItemUpgrade>() || attributes.GetComponent<ItemHealthPack>() ||
            attributes.GetComponent<PhysGrabCart>() || !attributes.GetComponent<ItemEquippable>())
            return false;
        System::String assetName = attributes.itemAssetName();
        const wchar_t* rawName = assetName != nullptr ? assetName.GetRawStringData() : nullptr;
        if (rawName && (wcsstr(rawName, L"Cart") || wcsstr(rawName, L"cart") ||
                        wcsstr(rawName, L"Upgrade") || wcsstr(rawName, L"upgrade")))
            return false;
        PhysGrabObject body = attributes.physGrabObject();
        ItemEquippable equippable = attributes.GetComponent<ItemEquippable>();
        return body && !body.grabbed() && equippable && !equippable.IsEquipped();
    }

    void DrawItemsTab()
    {
        const Hax::Vector2 mainAreaSize = Hax::Gui::GetContentRegionAvail();
        const float spacing = 20_px;
        const Hax::Vector2 columnSize = {(mainAreaSize.X - 3 * spacing) / 2, mainAreaSize.Y};

        // Column 1
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 320, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_GUNS]);
            {
                {
                    bool enabled = !G->IsClient;
                    ToggleEx(LINE_ID, G->InfBattery, G->Loc[LocKey_MaxBatteryGun], G->Loc[LocKey_HostOnly], {.Disabled = !enabled});
                }

                HorizontalLine(1_px);

                {
                    ToggleEx(LINE_ID, G->UseLaser, G->Loc[LocKey_LaserCrosshair]);
                }
            }
            EndPanel();

            BeginPanel(LINE_ID + 4);
            PanelHeader(G->Loc[LocKey_GrabItem]);
            ToggleEx(LINE_ID + 4, G->ShopInventory, G->Loc[LocKey_ShopInventory]);
            HorizontalLine(1_px);
            if (G->IsInShop && !G->ShopInventory)
                DescLabel(G->Loc[LocKey_ShopInventoryHint]);
            else if (!G->IsInGame)
                DescLabel(G->Loc[LocKey_NoDroppedItems]);
            else
            {
                if (G->IsInShop)
                    DescLabel(G->Loc[LocKey_ShopInventoryInstructions]);
                const Hax::Vector<ItemAttributes>& dropped = G->DroppedItemsPool.GetFront();
                size_t shown = 0;
                for (size_t i = 0; i < dropped.Size(); ++i)
                {
                    ItemAttributes attributes = dropped[i];
                    if (!IsStorableDroppedItem(attributes)) continue;
                    System::String displayName = attributes.itemName();
                    Item metadata = attributes.item();
                    Hax::WStringView label = displayName != nullptr
                        ? displayName.ToHaxView()
                        : (metadata ? metadata.itemName().ToHaxView() : Hax::WStringView(L"Item"));
                    if (Button(LINE_ID + 5000 + i, label, {}, {.MinW = Hax::Gui::GetContentRegionAvail().X}))
                    {
                        G->ItemToGrab = attributes;
                        G->ItemGrabReplacingSlot = false;
                        G->ItemGrabDeadline = GetTickCount64() + 10000;
                    }
                    ++shown;
                }
                if (shown == 0) DescLabel(G->Loc[LocKey_NoDroppedItems]);
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();

        // Column 2
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 321, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            if (!ShouldHideHostOnlyOptions())
            {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_SPAWN], G->Loc[LocKey_AvailableIfHost]);
            {
                static Item s_SelectedItem;

                const float spacing2 = 5_px;
                Hax::Vector2 sz = CalcButtonSize(G->Loc[LocKey_Spawn]);
                Hax::Gui::BeginHorizontal(spacing2);
                {
                    size_t nItems = G->ItemsPool.Size();
                    constexpr size_t dropListId = Hax::Hash(L"ItemSelect");
                    const float dropListW = Hax::Gui::GetContentRegionAvail().X - spacing2 - sz.X;

                    Hax::Gui::BeginVertical(3_px);
                    {
                        Hax::WStringView preview = L"-";
                        if (s_SelectedItem != null)
                            preview = s_SelectedItem.itemName().ToHaxView();

                        if (DropdownBtn(LINE_ID, preview, dropListW))
                            OpenPopup(dropListId, Hax::Gui::GetCursorPos());
                    }
                    Hax::Gui::EndVertical();

                    const float selectableH = CalcButtonHeight();
                    const float dropListH = selectableH * Hax::Max(1ULL, Hax::Min(nItems, 8ULL)) + 5_px * 2.f + 1.f;
                    if (BeginDropList(dropListId, {dropListW, dropListH}))
                    {
                        for (size_t i = 0; i < nItems; ++i)
                        {
                            const auto& p = G->ItemsPool.begin()[i];
                            if (Selectable(LINE_ID + i * 10000, p.key, s_SelectedItem == p.value, {.MinW = dropListW}))
                            {
                                s_SelectedItem = p.value;
                                ClosePopup(dropListId);
                            }
                        }
                        EndDropList();
                    }

                    bool enabled = !G->IsClient && s_SelectedItem < G->ItemsPool.Size() && G->IsInGame;
                    if (Button(LINE_ID, G->Loc[LocKey_Spawn], {}, {.Enabled = enabled}))
                        G->ItemToSpawn = s_SelectedItem;
                }
                Hax::Gui::EndHorizontal();
            }
            EndPanel();
            }
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }
}
