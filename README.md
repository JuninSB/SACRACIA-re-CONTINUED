# SACRACIA re-CONTINUED

Community continuation of [Sacracia/RepoHax](https://github.com/Sacracia/RepoHax), maintained independently.
<img width="1338" height="691" alt="Screenshot 2026-04-19 021713 (1)" src="https://github.com/user-attachments/assets/aa2fec40-b32f-4616-b4df-26789feb9843" />

### About
**SACRACIA re-CONTINUED** is an independently maintained continuation of RepoHax, an advanced internal C++ mod for the game **REPO**, developed with minimal reliance on third-party libraries. The project features custom graphics library and SDK for interacting with the Unity engine directly from C++ (both will be released as standalone open-source projects in the future).

> [!IMPORTANT]
> This project was created for educational purposes as part of a university project. It aims to provide developers with deeper insights into game hacking / mod development aspects.

---

### Upstream

This project continues development independently from [Sacracia/RepoHax](https://github.com/Sacracia/RepoHax).

---

### O que mudou desde o RepoHax para REPO v0.4.4.3

A referÍncia È o [RepoHax v1.3.3 para REPO v0.4.4.3](https://github.com/Sacracia/RepoHax/releases/tag/v1.3.3). Esta continuaÁ„o acrescenta os recursos abaixo; algumas aÁıes continuam limitadas ao host.

- **Grab:** Grab Assist com raio configur·vel, Grab Through, WallGrab, Anti Music Box Spin e Anti Duck. O Anti Duck tenta bloquear grabs acidentais, mas n„o reconhece todos os colliders.
- **Identity:** aplicar nome personalizado, gerar nome aleatÛrio em portuguÍs, criar avatar aleatÛrio e redefinir a identidade.
- **Manage Enemies:** selecionar inimigos vivos e usar Kill, Bring, Heal, Set 1 HP, AI Disable/Enable e Aggro Monster. No multiplayer, a disponibilidade e o alvo dependem de ser host ou cliente.
- **Items e Valuables:** invent·rio na loja, lista de itens do ch„o que podem ser guardados, spawner de valuables e dinheiro com valor configur·vel, alÈm do filtro para ocultar valuables j· colocados no carrinho ou na extraÁ„o.
- **Runtime:** status e nÌvel/mapa atuais, contagem de valuables e itens no ch„o, inimigos por tier, vida dos jogadores, valores no carrinho/extraÁ„o/mapa e bateria do caminh„o.
- **Performance:** opÁıes para reduzir sombras, antialiasing, luzes de pixel, fog, partÌculas suaves, vegetaÁ„o, reflexos e pÛs-processamento; contador de FPS e captura de benchmark para arquivos CSV.
- **Settings e interface:** portuguÍs, restauraÁ„o das configuraÁıes, opÁ„o para ocultar controles exclusivos do host, taxa do ESP configur·vel e atualizaÁ„o sincronizada por frames; abas e painÈis com rolagem.

A [release 0.4.5.3](https://github.com/JuninSB/SACRACIA-re-CONTINUED/releases/tag/v0.4.5.3) inclui o bin·rio correspondente a esta vers„o.

---
### Features

**Health**
- God Mode
- Heal to Maximum

**Movement**
- Infinite Stamina
- Modify Walk / Crouch / Sprint Speed
- Infinite Jumps
- No tumble (fall only by pressing hotkey)

**Grabbing**
- High Strength (Host / Single-player only)
- Unlimited Grab Range
- No Overload

**Upgrades**
- Apply purchased upgrades (or shop upgrades) to self

**Medkits**
- Apply purchased medkit to self

**Enemies**
- Spawn (Host / Single-player only)
- Blind (Host / Single-player only, enemies can still hear)
- No Hold Time Limit (Host / Single-player only)
- Display through walls (ESP)

**Players**
- Display through walls (ESP)
- Chams (Highlight through walls)
- Tumble selected player
- Kill / Revive selected player (Host / Single-player only)
- Toggle Voice Chat: Alive / Dead

**Vision**
- Enhanced Vision
- Third Person
- Field of View (FOV)
- Infinite Dead Head Charge

**Flashlight**
- Light Intensity
- Lighting Angle
- Works while crawling

**Valuables**
- Display through walls (ESP)
- Chams (Highlight through walls)
- No Damage (Host / Single-player only)
- Set all prices to Zero / Maximum

**Collection Points**
- Display through walls (ESP)
- Activate next point

**Weapons**
- Maximum Battery while holding
- Laser Sight

**Items**
- Spawn (Host / Single-player only)

**Level**
- Modify completed levels count (Host / Single-player only)
- Skip to next level (Host / Single-player only)

**Map Filter**
- Remove maps from rotation (Host / Single-player only)

**Truck**
- Display through walls (ESP)
- Teleport to truck

---

### Configuration
The menu hotkey, interface language, and menu options can be adjusted in the **Settings** tab and are saved to `haxsdk.ini`.

---

### Installation

For convenience and to avoid using external injectors, this mod utilizes **DLL Proxying**:

1. Download the `version.dll` file.
2. Place it in the game's root directory (the folder containing the `.exe` file).
3. Launch the game. The mod will load automatically and the menu will appear.

*If you prefer using a **DLL Injector**, simply rename dll file to something else and proceed with your preferred injector.*

---

## ‚ùó Important Note
Using external mod loaders (such as **BepInEx** or **MelonLoader**) may cause conflicts. RepoHax uses its own hooking system, which may be overwritten by other tools. It is recommended to use this on a clean game installation.
