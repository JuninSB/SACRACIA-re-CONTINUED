#include <src/pch.h>

#include "esp.h"

#include <src/cheat.h>

namespace Cheat
{
    enum VerticalAlignment : int 
    {
        VerticalAlignment_Bottom,
        VerticalAlignment_Top,
        VerticalAlignment_Center
    };

    enum HorizontalAlignment : int
    {
        HorizontalAlignment_Right,
        HorizontalAlignment_Left,
        HorizontalAlignment_Center
    };

    static void DrawEnemyEsp(EnemyEspData& data);
    static void DrawValuableEsp(ValuableEspData& data);
    static void DrawExtrPointEsp(ExtrPointEspData& data);
    static void DrawTruckEsp(TruckEspData& data);
    static void DrawPlayerEsp(PlayerEspData& data);
    static void DrawCosmeticBoxEsp(CosmeticBoxEspData& data);
    static void Box(const Hax::Rect& rect, const Hax::Gui::Color& col);
    static void Text(Hax::Gui::FontHandle hFont,
                     Hax::WStringView text,
                     Hax::Vector2 pos,
                     Hax::Gui::Color col,
                     float fontSize,
                     VerticalAlignment vAlign = VerticalAlignment_Bottom,
                     HorizontalAlignment hAlign = HorizontalAlignment_Right);
    static float EspFontSize() { return 9_px; }
    static float IconFontSize() { return 14_px; }
    static void VertHealthBar(float x, float y, float width, float height, float cur, float max);
    static void VertHealthBar(float x, float y, float width, float height, int cur, int max);
    static Unity::Rect CalcBoundsInScreenSpace(const Unity::Bounds& bigBounds, Unity::Camera cam);

    void RenderEsp()
    {
        if (G->FpsCounter && G->ScreenWidth > 0.f && G->ScreenHeight > 0.f)
        {
            static ULONGLONG sampleStart = 0;
            static unsigned frameCount = 0;
            static int fpsAverage = 0;
            const ULONGLONG now = GetTickCount64();
            if (!sampleStart) sampleStart = now;
            ++frameCount;
            const ULONGLONG elapsed = now - sampleStart;
            if (elapsed >= 1000)
            {
                fpsAverage = (int)(frameCount * 1000ULL / elapsed);
                frameCount = 0;
                sampleStart = now;
            }
            Hax::char16 value[24]{};
            swprintf_s(value, _countof(value), L"%d FPS", fpsAverage);
            const bool right = G->FpsCorner == 1 || G->FpsCorner == 3;
            const bool bottom = G->FpsCorner >= 2;
            const Hax::Vector2 pos(right ? G->ScreenWidth - 12_px : 12_px,
                                   bottom ? G->ScreenHeight - 12_px : 12_px);
            Text(G->NunitoSans_Bold, value, pos, 0xFFFFFFFF, (float)G->FpsFontSize,
                 bottom ? VerticalAlignment_Top : VerticalAlignment_Bottom,
                 right ? HorizontalAlignment_Left : HorizontalAlignment_Right);
        }

        bool shouldDrawEsp = G->IsInGame && MenuManager::instance() && !MenuManager::instance().currentMenuPage();
        if (!shouldDrawEsp)
            return;

        size_t i1 = 9999;
        size_t i2 = 9999;
        size_t i3 = 9999;
        size_t i4 = 9999;
        size_t i5 = 9999;
        size_t i6 = 9999;

        Hax::Vector<EnemyEspData>& buf1 = G->EnemiesEspBuffer.GetFront();
        Hax::Vector<ValuableEspData>& buf2 = G->ValuablesEspBuffer.GetFront();
        Hax::Vector<ExtrPointEspData>& buf3 = G->ExtrPointsEspBuffer.GetFront();
        Hax::Vector<TruckEspData>& buf4 = G->TruckEspBuffer.GetFront();
        Hax::Vector<PlayerEspData>& buf5 = G->PlayersEspBuffer.GetFront();
        Hax::Vector<CosmeticBoxEspData>& buf6 = G->CosmeticBoxesEspBuffer.GetFront();

        if (G->EnemiesEsp)
        {
            i1 = 0;
        }
        if (G->ValuablesEsp)
        {
            i2 = 0;
        }
        if (G->ExtrPointsEsp)
        {
            i3 = 0;
        }
        if (G->PlayersEsp)
        {
            i5 = 0;
        }
        if (G->TruckEsp)
        {
            i4 = 0;
        }
        if (G->CosmeticBoxesEsp)
        {
            i6 = 0;
        }

        while (i1 < buf1.Size() || i2 < buf2.Size() || i3 < buf3.Size() || i4 < buf4.Size() || i5 < buf5.Size() || i6 < buf6.Size())
        {
            float d1 = i1 < buf1.Size() ? buf1[i1].Distance : FLT_MIN;
            float d2 = i2 < buf2.Size() ? buf2[i2].Distance : FLT_MIN;
            float d3 = i3 < buf3.Size() ? buf3[i3].Distance : FLT_MIN;
            float d4 = i4 < buf4.Size() ? buf4[i4].Distance : FLT_MIN;
            float d5 = i5 < buf5.Size() ? buf5[i5].Distance : FLT_MIN;
            float d6 = i6 < buf6.Size() ? buf6[i6].Distance : FLT_MIN;

            if (d1 >= d2 && d1 >= d3 && d1 >= d4 && d1 >= d5 && d1 >= d6)       { DrawEnemyEsp(buf1[i1++]);     }
            else if (d2 >= d1 && d2 >= d3 && d2 >= d4 && d2 >= d5 && d2 >= d6)  { DrawValuableEsp(buf2[i2++]);  }
            else if (d3 >= d1 && d3 >= d2 && d3 >= d4 && d3 >= d5 && d3 >= d6)  { DrawExtrPointEsp(buf3[i3++]); }
            else if (d4 >= d1 && d4 >= d2 && d4 >= d3 && d4 >= d5 && d4 >= d6)  { DrawTruckEsp(buf4[i4++]);     }
            else if (d5 >= d1 && d5 >= d2 && d5 >= d3 && d5 >= d4 && d5 >= d6)  { DrawPlayerEsp(buf5[i5++]);    }
            else                                                                { DrawCosmeticBoxEsp(buf6[i6++]); }
        }
    }

    Hax::Optional<EnemyEspData> ParseEnemyEspData(Enemy enemy)
    {
        Unity::Camera mainCam = GameDirector::instance().MainCamera();
        if (!mainCam || !enemy)
            return {};

        float dist = mainCam.GetTransform().GetPosition().Distance(enemy.GetTransform().GetPosition());
        if (dist < 2.f)
            return {};

        EnemyParent parent = enemy.enemyParent();
        if (parent.enemyName() == L"Spewer")
        {
            EnemySlowMouth comp = enemy.GetComponent<EnemySlowMouth>();
            if (comp && comp.attachedTimer() > 0.f)
                return {};
        }

        Unity::Bounds enemyBounds{enemy.CenterTransform().GetPosition(), Unity::Vector3()};
        if (EnemyRigidbody rb = enemy.Rigidbody())
        {
            PhysGrabObject grabObj = rb.physGrabObject();
            enemyBounds = Unity::Bounds(grabObj.centerPoint(), grabObj.boundingBox());
        }
        else
        {
            enemyBounds = enemy.GetTransform().GetChild(1).GetChild(0).GetComponent<Unity::Collider>().GetBounds();
        }

        Unity::Rect bounds = CalcBoundsInScreenSpace(enemyBounds, mainCam);
        Unity::Rect screenRect{0.f, 0.f, G->ScreenWidth, G->ScreenHeight};

        if (!bounds.Overlaps(screenRect))
            return {};

        EnemyHealth health = enemy.Health();

        return EnemyEspData
        {
            .Box = bounds,
            .Name = enemy.enemyParent().enemyName().ToHaxView(),
            .Distance = dist,
            .CurHp = health.healthCurrent(),
            .MaxHp = health.health(),
        };
    }

    Hax::Optional<ValuableEspData> ParseValuableEspData(ValuableObject obj)
    {
        if (!obj || (G->HideStoredValuables && !obj.GetEnabled()))
            return {};

        Unity::Camera cam = GameDirector::instance().MainCamera();
        float dist = cam.GetTransform().GetPosition().Distance(obj.GetTransform().GetPosition());
        if (dist < 2.f || dist > (float)G->ValuablesEspRange)
            return {};

        PhysGrabObject physObject = obj.physGrabObject();
        if (!physObject)
            return {};
        if (G->HideStoredValuables)
        {
            PhysGrabObjectImpactDetector impact = physObject.impactDetector();
            if (impact && impact.inCart()) return {};

            RoundDirector round = RoundDirector::instance();
            if (round)
            {
                System::List<Unity::GameObject> extracted = round.dollarHaulList();
                Unity::GameObject valuableObject = obj.GetGameObject();
                for (int i = 0; extracted != nullptr && i < extracted.Count(); ++i)
                    if (extracted[i] == valuableObject) return {};

                System::List<ItemValuableBox> boxes = round.valuableBoxHaulList();
                for (Unity::Transform node = obj.GetTransform(); node; node = node.GetParent())
                {
                    Unity::GameObject parentObject = node.GetGameObject();
                    for (int i = 0; boxes != nullptr && i < boxes.Count(); ++i)
                        if (boxes[i] && boxes[i].GetGameObject() == parentObject) return {};
                }
            }
        }

        Unity::Vector3 worldPos = physObject.midPoint();
        Unity::Vector3 screenPos = cam.WorldToScreenPoint(worldPos);
        Unity::Rect screenRect{0.f, 0.f, G->ScreenWidth, G->ScreenHeight};
        if (screenPos.Z <= 0 || !screenRect.Contains(screenPos))
            return {};

        float scaleX = G->ScreenWidth / G->PixelWidth;
        float scaleY = G->ScreenHeight / G->PixelHeight;

        screenPos.X *= scaleX;
        screenPos.Y = G->ScreenHeight - screenPos.Y * scaleY;

        return ValuableEspData
        {
            .Name = obj.GetName().ToHaxView(),
            .Pos = Unity::Vector2(screenPos),
            .Distance = dist,
            .Value = obj.dollarValueCurrent()
        };
    }

    Hax::Optional<ExtrPointEspData> ParseExtrPointEspData(Unity::GameObject obj)
    {
        Unity::Camera cam = GameDirector::instance().MainCamera();

        if (!obj || !cam)
            return {};

        ExtractionPoint point = obj.GetComponent<ExtractionPoint>();
        if (!point)
            return {};

        float dist = cam.GetTransform().GetPosition().Distance(obj.GetTransform().GetPosition());
        if (dist < 1.f)
            return {};

        Unity::Vector3 screenPos = cam.WorldToScreenPoint(obj.GetTransform().GetPosition());
        Unity::Rect screenRect{0.f, 0.f, G->ScreenWidth, G->ScreenHeight};
        if (screenPos.Z <= 0 || !screenRect.Contains(screenPos))
            return {};

        float scaleX = G->ScreenWidth / G->PixelWidth;
        float scaleY = G->ScreenHeight / G->PixelHeight;

        screenPos.X *= scaleX;
        screenPos.Y = G->ScreenHeight - screenPos.Y * scaleY;

        return ExtrPointEspData 
        {
            .Pos = Unity::Vector2(screenPos),
            .Distance = dist,
            .Completed = (point.currentState() == ExtractionPoint_State::Complete()),
            .Active = (point == RoundDirector::instance().extractionPointCurrent())
        };
    }

    Hax::Optional<TruckEspData> ParseTruckEspData(TruckSafetySpawnPoint truck)
    {
        Unity::Camera cam = GameDirector::instance().MainCamera();

        if (!truck || !cam)
            return {};

        Unity::Vector3 worldPos = truck.GetTransform().GetPosition();

        float dist = cam.GetTransform().GetPosition().Distance(worldPos);
        if (dist < 1.f)
            return {};

        Unity::Vector3 screenPos = cam.WorldToScreenPoint(worldPos);
        Unity::Rect screenRect{0.f, 0.f, G->ScreenWidth, G->ScreenHeight};
        if (screenPos.Z <= 0 || !screenRect.Contains(screenPos))
            return {};

        float scaleX = G->ScreenWidth / G->PixelWidth;
        float scaleY = G->ScreenHeight / G->PixelHeight;

        screenPos.X *= scaleX;
        screenPos.Y = G->ScreenHeight - screenPos.Y * scaleY;

        return TruckEspData
        {
            .Pos = Unity::Vector2(screenPos),
            .Distance = dist
        };
    }

    Hax::Optional<PlayerEspData> ParsePlayerEspData(PlayerAvatar avatar)
    {
        Unity::Camera mainCam = GameDirector::instance().MainCamera();
        if (!mainCam || !avatar || avatar.isLocal())
            return {};

        float dist = mainCam.GetTransform().GetPosition().Distance(avatar.GetTransform().GetPosition());
        if (dist < 1.f)
            return {};

        if (avatar.deadSet())
        {
            PlayerDeathHead head = avatar.playerDeathHead();
            if (head && head.GetEnabled())
            {
                System::Array<Unity::Collider> colliders = head.colliders();
                Unity::Bounds bounds = colliders[0].GetBounds();
                for (auto col : colliders)
                    bounds.Encapsulate(col.GetBounds());

                if (bounds.Extents == Unity::Vector3::zero())
                    return {};

                Unity::Rect rect = CalcBoundsInScreenSpace(bounds, mainCam);
                Unity::Rect screenRect{0.f, 0.f, G->ScreenWidth, G->ScreenHeight};

                if (!rect.Overlaps(screenRect))
                    return {};

                PlayerEspData espData{};
                espData.Box = rect;
                espData.Distance = dist;
                espData.Name = SemiFunc::PlayerGetName(avatar).ToHaxView();
                espData.CurHp = 0;
                espData.MaxHp = 0;
                espData.Dead = true;

                return espData;
            }

            return {};
        }

        if (avatar.isDisabled())
            return {};

        PlayerAvatarVisuals visuals = avatar.playerAvatarVisuals();
        Unity::Bounds bounds2 = visuals.headSideTransform().GetComponent<Unity::Collider>().GetBounds();
        Unity::Vector3 ext = bounds2.Extents;
        for (Unity::Collider col2 : visuals.legTwistTransform().GetComponentsInChildren<Unity::Collider>())
        {
            if (col2)
                bounds2.Encapsulate(col2.GetBounds());
        }
        if (bounds2.Extents == Unity::Vector3::zero())
        {
            return {};
        }

        ext.Y = bounds2.Extents.Y * 1.2f;
        ext.Z *= 1.5f;
        ext.X *= 1.5f;
        bounds2.Extents = ext;
        Unity::Rect rect = CalcBoundsInScreenSpace(bounds2, mainCam);

        Unity::Rect screenRect{0.f, 0.f, G->ScreenWidth, G->ScreenHeight};
        if (!rect.Overlaps(screenRect))
            return {};

        PlayerHealth health = avatar.playerHealth();

        return PlayerEspData
        {
            .Box = rect,
            .Name = SemiFunc::PlayerGetName(avatar).ToHaxView(),
            .Distance = dist,
            .CurHp = health.health(),
            .MaxHp = health.maxHealth(),
            .Dead = false,
        };
    }

    Hax::Optional<CosmeticBoxEspData> ParseCosmeticBoxEspData(CosmeticWorldObject box)
    {
        Unity::Camera cam = GameDirector::instance().MainCamera();

        if (!box || !cam)
            return {};

        Unity::Vector3 worldPos = box.GetTransform().GetPosition();

        float dist = cam.GetTransform().GetPosition().Distance(worldPos);
        if (dist < 1.f)
            return {};

        Unity::Vector3 screenPos = cam.WorldToScreenPoint(worldPos);
        Unity::Rect screenRect{0.f, 0.f, G->ScreenWidth, G->ScreenHeight};
        if (screenPos.Z <= 0 || !screenRect.Contains(screenPos))
            return {};

        float scaleX = G->ScreenWidth / G->PixelWidth;
        float scaleY = G->ScreenHeight / G->PixelHeight;

        screenPos.X *= scaleX;
        screenPos.Y = G->ScreenHeight - screenPos.Y * scaleY;

        return CosmeticBoxEspData
        {
            .Pos = Unity::Vector2(screenPos),
            .Distance = dist,
            .Rarity = box.rarity()
        };
    }

    static void DrawEnemyEsp(EnemyEspData& data)
    {
        float fontH = 9_px;

        Box(data.Box, 0xFF0000FF);
        Text(G->NunitoSans_Bold, data.Name, Hax::Vector2(data.Box.GetCenter().X, data.Box.Min.Y - 2.f), 0xFFFFFFFF, fontH, VerticalAlignment_Top, HorizontalAlignment_Center);
        if (!G->IsClient)
        {
            VertHealthBar(data.Box.Min.X - 10.f, data.Box.Min.Y, 3.f, data.Box.GetSize().Y, data.CurHp, data.MaxHp);
            Hax::Gui::Color lerped = Hax::Lerp(0xFF0000FF, Hax::Gui::Color(0x00FF00FF), (float)data.CurHp / data.MaxHp);
            Hax::WStringBuilder<16> sb;
            sb.AppendF(L"%d", data.CurHp);
            Text(G->NunitoSans_Bold, sb.View(), Hax::Vector2(data.Box.Min.X - 15.f, data.Box.Min.Y), lerped, fontH, VerticalAlignment_Bottom, HorizontalAlignment_Left);
        }
    }

    static void DrawValuableEsp(ValuableEspData& data)
    {
        Hax::WStringView s = data.Name;
        if (s.EndsWith(L"(Clone)"))
            s.RemoveSuffix(7);
        if (data.Name.StartsWith(L"Valuable "))
            s.RemovePrefix(9);

        wchar_t buf[64] = {0};
        if (swprintf_s(buf, L"%.*ls | %.0f$", (int)s.Length(), s.begin(), data.Value) > 0)
        {
            Text(G->NunitoSans_Bold, buf, data.Pos, 0xFFFF00FF, EspFontSize(), VerticalAlignment_Bottom, HorizontalAlignment_Center);
            return;
        }

        Text(G->NunitoSans_Bold, s, data.Pos, 0xFF00FFFF, EspFontSize(), VerticalAlignment_Bottom, HorizontalAlignment_Center);
    }

    static void DrawExtrPointEsp(ExtrPointEspData& data)
    {
        Hax::Gui::Color color = data.Active ? 0x00FF00FF : data.Completed ? 0x545454FF : 0xFF0000FF;
        Text(G->Icons_Solid, L"\uF468", data.Pos, color, IconFontSize());
    }

    static void DrawTruckEsp(TruckEspData& data)
    {
        Hax::Gui::Color color = 0x00FF00FF;
        Text(G->Icons_Solid, L"\uF0D1", data.Pos, color, IconFontSize());
    }

    static void DrawPlayerEsp(PlayerEspData& data)
    {
        Box(data.Box, data.Dead ? 0x545454FF : 0x00FF00FF);
        Text(G->NunitoSans_Bold, data.Name, Hax::Vector2(data.Box.GetCenter().X, data.Box.Min.Y - 2.f), 0xFFFFFFFF, EspFontSize(), VerticalAlignment_Top, HorizontalAlignment_Center);
        if (!data.Dead)
        {
            VertHealthBar(data.Box.Min.X - 10_px, data.Box.Min.Y, 3_px, data.Box.GetSize().Y, data.CurHp, data.MaxHp);

            Hax::Gui::Color lerped = Hax::Lerp(Hax::Gui::Color(0xFF0000FF), Hax::Gui::Color(0x00FF00FF), (float)data.CurHp / data.MaxHp);
            Hax::WStringBuilder<16> sb;
            sb.AppendF(L"%d", data.CurHp);
            Text(G->NunitoSans_Bold, sb.View(), Hax::Vector2(data.Box.Min.X - 15_px, data.Box.Min.Y), lerped, EspFontSize(), VerticalAlignment_Bottom, HorizontalAlignment_Left);
        }
    }

    static void DrawCosmeticBoxEsp(CosmeticBoxEspData& data)
    {
        static Hax::Array<Hax::Gui::Color, 4> s_RarityToColor = {0x7FFF00FF, 0x57A0D3FF, 0xF81894FF, 0xE6CC00FF};

        Text(G->Icons_Solid, L"\uF553", data.Pos, s_RarityToColor[data.Rarity], IconFontSize());
    }

    static void Box(const Hax::Rect& rect, const Hax::Gui::Color& col)
    {
        float px1 = 1_px;
        float px3 = 3_px;

        Hax::Vector2 tl = rect.GetTL();
        Hax::Vector2 tr = rect.GetTR();
        Hax::Vector2 br = rect.GetBR();
        Hax::Vector2 bl = rect.GetBL();

        Hax::Gui::DrawLine(tl, tr, {.FillColor = 0x000000FF, .Th = px3});
        Hax::Gui::DrawLine(tr, br, {.FillColor = 0x000000FF, .Th = px3});
        Hax::Gui::DrawLine(bl, br, {.FillColor = 0x000000FF, .Th = px3});
        Hax::Gui::DrawLine(tl, bl, {.FillColor = 0x000000FF, .Th = px3});

        Hax::Gui::DrawLine(tl, tr, {.FillColor = col, .Th = px1});
        Hax::Gui::DrawLine(tr, br, {.FillColor = col, .Th = px1});
        Hax::Gui::DrawLine(bl, br, {.FillColor = col, .Th = px1});
        Hax::Gui::DrawLine(tl, bl, {.FillColor = col, .Th = px1});
    }

    static void Text(Hax::Gui::FontHandle hFont,
                     Hax::WStringView text,
                     Hax::Vector2 pos,
                     Hax::Gui::Color col,
                     float fontSize,
                     VerticalAlignment vAlign,
                     HorizontalAlignment hAlign)
    {
        Hax::Vector2 textSize = Hax::Gui::CalcTextSize(hFont, text, fontSize);
        float xShift = hAlign == HorizontalAlignment_Right ? 0.f : textSize.X / (float)hAlign;
        float yShift = vAlign == VerticalAlignment_Bottom ? 0.f : textSize.Y / (float)vAlign;

        Hax::Gui::DrawString(hFont, text, Hax::Vector2(pos.X - xShift + 1.F, pos.Y - yShift - 1.F), fontSize, {.Color = 0x000000FF});
        Hax::Gui::DrawString(hFont, text, Hax::Vector2(pos.X - xShift + 1.F, pos.Y - yShift + 1.F), fontSize, {.Color = 0x000000FF});
        Hax::Gui::DrawString(hFont, text, Hax::Vector2(pos.X - xShift - 1.F, pos.Y - yShift + 1.F), fontSize, {.Color = 0x000000FF});
        Hax::Gui::DrawString(hFont, text, Hax::Vector2(pos.X - xShift - 1.F, pos.Y - yShift - 1.F), fontSize, {.Color = 0x000000FF});
        Hax::Gui::DrawString(hFont, text, Hax::Vector2(pos.X - xShift, pos.Y - yShift), fontSize, {.Color = col});
    }

    static void VertHealthBar(float x, float y, float width, float height, float cur, float max)
    {
        float coef = Hax::Clamp(cur / max, 0.f, 1.f);
        Hax::Gui::Color lerped = Hax::Lerp(Hax::Gui::Color(0xFF0000FF), Hax::Gui::Color(0x00FF00FF), coef);
        Hax::Gui::DrawLine({x, y}, {x, y + height}, {.FillColor = 0x000000FF, .Th = width + 2_px});
        Hax::Gui::DrawLine({x, y + height - height * coef}, {x, y + height}, {.FillColor = lerped, .Th = width});
    }

    static void VertHealthBar(float x, float y, float width, float height, int cur, int max)
    {
        VertHealthBar(x, y, width, height, (float)cur, (float)max);
    }

    static Unity::Rect CalcBoundsInScreenSpace(const Unity::Bounds& bigBounds, Unity::Camera cam)
    {
        float scaleX = G->ScreenWidth / G->PixelWidth;
        float scaleY = G->ScreenHeight / G->PixelHeight;

        Unity::Vector3 screenSpaceCorners[8] =
        {
            {bigBounds.Center.X + bigBounds.Extents.X, bigBounds.Center.Y + bigBounds.Extents.Y, bigBounds.Center.Z + bigBounds.Extents.Z},
            {bigBounds.Center.X + bigBounds.Extents.X, bigBounds.Center.Y + bigBounds.Extents.Y, bigBounds.Center.Z - bigBounds.Extents.Z},
            {bigBounds.Center.X + bigBounds.Extents.X, bigBounds.Center.Y - bigBounds.Extents.Y, bigBounds.Center.Z + bigBounds.Extents.Z},
            {bigBounds.Center.X + bigBounds.Extents.X, bigBounds.Center.Y - bigBounds.Extents.Y, bigBounds.Center.Z - bigBounds.Extents.Z},
            {bigBounds.Center.X - bigBounds.Extents.X, bigBounds.Center.Y + bigBounds.Extents.Y, bigBounds.Center.Z + bigBounds.Extents.Z},
            {bigBounds.Center.X - bigBounds.Extents.X, bigBounds.Center.Y + bigBounds.Extents.Y, bigBounds.Center.Z - bigBounds.Extents.Z},
            {bigBounds.Center.X - bigBounds.Extents.X, bigBounds.Center.Y - bigBounds.Extents.Y, bigBounds.Center.Z + bigBounds.Extents.Z},
            {bigBounds.Center.X - bigBounds.Extents.X, bigBounds.Center.Y - bigBounds.Extents.Y, bigBounds.Center.Z - bigBounds.Extents.Z}
        };

        for (int i = 0; i < 8; ++i)
        {
            screenSpaceCorners[i] = cam.WorldToScreenPoint(screenSpaceCorners[i]);
            screenSpaceCorners[i].X *= scaleX;
            screenSpaceCorners[i].Y = G->ScreenHeight - screenSpaceCorners[i].Y * scaleY;
        }

        float x = FLT_MAX;
        float y = FLT_MAX;
        float x2 = FLT_MIN;
        float y2 = FLT_MIN;

        for (const Unity::Vector3& v : screenSpaceCorners)
        {
            if (v.Z <= 0.f)
                continue;

            x = Hax::Min(v.X, x);
            y = Hax::Min(v.Y, y);
            x2 = Hax::Max(v.X, x2);
            y2 = Hax::Max(v.Y, y2);
        }

        if (y2 - y < 15.f)
            y2 = y + 15.f;

        return Unity::Rect::MinMaxRect(x, y, x2, y2);
    }
}
