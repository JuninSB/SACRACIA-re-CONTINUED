#include <src/pch.h>

#include <cstdlib>

#include <src/cheat.h>

#include "../localization.h"
#include "../widgets.h"

#define LINE_ID (HAX_LINE * 456)
#define IDENTITY_LINE (HAX_LINE * 991)

namespace Cheat
{
    static void MakeRandomIdentityName()
    {
        static bool seeded = false;
        if (!seeded)
        {
            std::srand(GetTickCount());
            seeded = true;
        }
        static constexpr const wchar_t* prefixes[] = {
            L"Abóbora",L"Açúcar",L"Alface",L"Algodão",L"Alho",L"Âncora",L"Areia",L"Arco",L"Arroz",L"Bala",
            L"Bambu",L"Banana",L"Barro",L"Batata",L"Biscoito",L"Borracha",L"Brasa",L"Cacto",L"Café",L"Caju",
            L"Canela",L"Capim",L"Caramelo",L"Carvão",L"Castanha",L"Cebola",L"Cenoura",L"Cereja",L"Chumbo",L"Chuva",
            L"Cimento",L"Cobre",L"Coco",L"Concreto",L"Cristal",L"Damasco",L"Doce",L"Ervilha",L"Espuma",L"Estanho",
            L"Farinha",L"Feijão",L"Ferro",L"Figo",L"Fumaça",L"Gengibre",L"Goiaba",L"Granito",L"Graveto",L"Hortelã",
            L"Jabuticaba",L"Jaca",L"Laranja",L"Leite",L"Lentilha",L"Limo",L"Limão",L"Linho",L"Madeira",L"Mamão",
            L"Mandioca",L"Manga",L"Manjericão",L"Mel",L"Melancia",L"Milho",L"Mirtilo",L"Morango",L"Nabo",L"Névoa",
            L"Nuvem",L"Ouro",L"Páprica",L"Papel",L"Pedra",L"Pimenta",L"Pinho",L"Pipoca",L"Platina",L"Prata",
            L"Queijo",L"Quindim",L"Rapadura",L"Repolho",L"Rocha",L"Sabão",L"Sálvia",L"Seda",L"Seixo",L"Tapioca",
            L"Tomate",L"Trigo",L"Turmalina",L"Uva",L"Veludo",L"Vidro",L"Zinco",L"Avelã",L"Paçoca",L"Pêssego"
        };
        static constexpr const wchar_t* suffixes[] = {
            L"Abafado",L"Afiado",L"Agitado",L"Alegre",L"Amassado",L"Amigável",L"Antigo",L"Ardente",L"Astuto",L"Atrevido",
            L"Barulhento",L"Brilhante",L"Calmo",L"Cansado",L"Cintilante",L"Colorido",L"Corajoso",L"Crocante",L"Curioso",L"Delicado",
            L"Desajeitado",L"Dourado",L"Encantado",L"Enrolado",L"Esperto",L"Estranho",L"Famoso",L"Felpudo",L"Feroz",L"Fofinho",
            L"Formidável",L"Forte",L"Fugitivo",L"Gelado",L"Genial",L"Gigante",L"Grudento",L"Honesto",L"Iluminado",L"Imponente",
            L"Inquieto",L"Invencível",L"Irregular",L"Ligeiro",L"Luminoso",L"Mágico",L"Malandro",L"Manso",L"Maroto",L"Misterioso",
            L"Modesto",L"Molhado",L"Musculoso",L"Nervoso",L"Notável",L"Obstinado",L"Ousado",L"Pacífico",L"Poderoso",L"Pompom",
            L"Prestativo",L"Quentinho",L"Radiante",L"Resistente",L"Risonho",L"Robusto",L"Saboroso",L"Saltitante",L"Simpático",L"Sorrateiro",
            L"Suave",L"Teimoso",L"Temido",L"Terrível",L"Tímido",L"Tranquilo",L"Tremeluzente",L"Valente",L"Valioso",L"Veloz",
            L"Vibrante",L"Vitorioso",L"Voador",L"Absurdo",L"Acolhedor",L"Aventureiro",L"Balançante",L"Deslumbrante",L"Enorme",L"Fabuloso",
            L"Lendário",L"Poderosa",L"Radical",L"Sabichão",L"Travesso",L"Único",L"Vermelho",L"Dançarino",L"Brincalhão",L"Incrível"
        };
        const size_t p = (size_t)std::rand() % _countof(prefixes);
        const size_t s = (size_t)std::rand() % _countof(suffixes);
        swprintf_s(G->IdentityName, _countof(G->IdentityName), L"%s %s", prefixes[p], suffixes[s]);
    }

    void DrawMiscTab()
    {
        const Hax::Vector2 mainAreaSize = Hax::Gui::GetContentRegionAvail();
        const float spacing = 20_px;
        const Hax::Vector2 columnSize = {(mainAreaSize.X - 3 * spacing) / 2, mainAreaSize.Y};

        // Column 1
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 368, {.W = columnSize.X, .H = columnSize.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_TRUCK]);
            {
                {
                    ToggleEx(LINE_ID, G->TruckEsp, G->Loc[LocKey_DisplayThroughWalls]);
                }

                HorizontalLine(1_px);

                {
                    TruckSafetySpawnPoint truck = TruckSafetySpawnPoint::instance();
                    PlayerAvatar avatar = PlayerAvatar::instance();
                    bool disabled = !truck || !G->IsInGame || !avatar || avatar.deadSet();
                    if (Button(LINE_ID, G->Loc[LocKey_TeleportTruck], {}, {.Enabled = !disabled, .MinW = Hax::Gui::GetContentRegionAvail().X}))
                        G->TeleportToTruck = true;
                }
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }

    void DrawIdentityTab()
    {
        const float spacing = 20_px;
        const Hax::Vector2 area = Hax::Gui::GetContentRegionAvail();
        const float width = (area.X - spacing * 3.f) / 2.f;
        static bool initialized = false;
        if (!initialized)
        {
            initialized = true;
            G->LoadIdentityName = true;
            if (!G->IdentityName[0])
                wcscpy_s(G->IdentityName, _countof(G->IdentityName), L"Player");
        }

        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 369, {.W = width, .H = area.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        BeginPanel(IDENTITY_LINE);
        PanelHeader(G->Loc[LocKey_IdentityTab]);
        MainLabel(G->Loc[LocKey_IdentityName]);
        {
            Hax::Rect bounds;
            bounds.Min = Hax::Gui::GetCursorPos();
            bounds.Max = bounds.Min + Hax::Vector2(Hax::Gui::GetContentRegionAvail().X, CalcButtonHeight());
            Hax::Gui::StringEdit(IDENTITY_LINE, G->NunitoSans_Bold, G->IdentityName, _countof(G->IdentityName), 13_px,
                bounds, {.Hint = G->Loc[LocKey_NameHint], .TextColor = Hax::Gui::Color::White});
            Hax::Gui::PlaceItem(bounds.GetSize());
        }
        HorizontalLine(1_px);
        if (Button(IDENTITY_LINE, G->Loc[LocKey_Apply], {}, {.MinW = Hax::Gui::GetContentRegionAvail().X}))
            G->ApplyIdentityName = true;
        HorizontalLine(1_px);
        if (Button(IDENTITY_LINE, G->Loc[LocKey_RandomName], {}, {.MinW = Hax::Gui::GetContentRegionAvail().X}))
        {
            MakeRandomIdentityName();
            G->ApplyIdentityName = true;
        }
        EndPanel();
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();

        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(LINE_ID + 370, {.W = width, .H = area.Y, .Clip = true, .ScrollY = true});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        BeginPanel(IDENTITY_LINE + 1);
        PanelHeader(G->Loc[LocKey_AvatarAndLobby]);
        if (Button(IDENTITY_LINE + 1, G->Loc[LocKey_RandomAvatar], {}, {.MinW = Hax::Gui::GetContentRegionAvail().X}))
            G->RandomizeAvatar = true;
        HorizontalLine(1_px);
        if (Button(IDENTITY_LINE + 1, G->Loc[LocKey_ResetIdentity], {}, {.MinW = Hax::Gui::GetContentRegionAvail().X}))
        {
            MakeRandomIdentityName();
            G->ApplyIdentityName = true;
            G->RandomizeAvatar = true;
        }
        HorizontalLine(1_px);
        EndPanel();
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }
}
