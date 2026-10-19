#pragma once

#include <third_party/haxsdk/hax.h>

#define LOC_LIST(X) \
X(LocKey_Stats, L"Stats", L"Статы") \
X(LocKey_Entities, L"Entities", L"Существа") \
X(LocKey_Vision, L"Vision", L"Зрение") \
X(LocKey_Valuables, L"Valuables", L"Ценности") \
X(LocKey_Items, L"Items", L"Предметы") \
X(LocKey_ShopInventory, L"Shop inventory", L"Инвентарь в магазине") \
X(LocKey_GrabItem, L"Grab item", L"Забрать предмет") \
X(LocKey_NoDroppedItems, L"No storable items on the ground", L"На земле нет предметов для инвентаря") \
X(LocKey_ShopInventoryHint, L"Enable Shop inventory to grab items here.", L"Ative o inventário na loja para guardar itens aqui.") \
X(LocKey_ShopInventoryInstructions, L"Hold an item and press 1, 2, or 3 to equip it in that inventory slot.", L"Segure um item e pressione 1, 2 ou 3 para guardá-lo no espaço correspondente.") \
X(LocKey_Level, L"Level", L"Уровень") \
X(LocKey_Misc, L"Miscellaneous", L"Разное")\
X(LocKey_Settings, L"Settings", L"Настройки")\
X(LocKey_EspUpdateRate, L"ESP update rate", L"Частота обновления ESP")\
X(LocKey_EspMatchFrameRate, L"Match frame rate", L"Обновлять по кадрам")\
X(LocKey_EspFrameInterval, L"Update every N frames", L"Обновлять каждые N кадров")\
X(LocKey_HideHostOnlyOptions, L"Hide host-only options when not host", L"Скрывать настройки хоста")\
X(LocKey_HEALTH, L"HEALTH", L"ЗДОРОВЬЕ") \
X(LocKey_Godmode, L"Godmode", L"Бессмертие") \
X(LocKey_HealToMax, L"Heal to max", L"Исцелиться на максимум") \
X(LocKey_MOVEMENT, L"MOVEMENT", L"ПЕРЕДВИЖЕНИЕ") \
X(LocKey_InfiniteStamina, L"Infinite stamina", L"Стамина не кончается") \
X(LocKey_Default, L"Default", L"Обычная") \
X(LocKey_WalkingSpeed, L"Walking speed", L"Скорость ходьбы") \
X(LocKey_RunningSpeed, L"Running speed", L"Скорость бега") \
X(LocKey_CrouchingSpeed, L"Crouching speed", L"Скорость ползания") \
X(LocKey_InfiniteJumps, L"Infinite jumps", L"Бесконечные прыжки") \
X(LocKey_DontTumble, L"Dont tumble", L"Не падать") \
X(LocKey_DisableTumbling, L"Disable tumbling from external sources", L"Отключает падение от внешних источников") \
X(LocKey_GRAB, L"GRAB", L"ЗАХВАТ") \
X(LocKey_HighStrength, L"High strength", L"Большая сила") \
X(LocKey_AvailableIfHost, L"Available if you are host", L"Доступно если вы хост") \
X(LocKey_UnlimitedRange, L"Unlimited range", L"Неограниченная дальность") \
X(LocKey_NoOvercharge, L"No overcharge", L"Нет перегрузки") \
X(LocKey_POWERUPS, L"POWERUPS", L"УЛУЧШЕНИЯ") \
X(LocKey_AutoApplyUpgrades, L"Automatically apply upgrades", L"Автоматически применять улучшения") \
X(LocKey_ManualApplication, L"Manual application", L"Ручное применение") \
X(LocKey_HEALTHPACKS, L"HEALTH PACKS", L"АПТЕЧКИ") \
X(LocKey_ApplyAidKit, L"Apply an available aid kit to yourself", L"Применить на себя доступную аптечку") \
X(LocKey_VIEW, L"VIEW", L"ОБЗОР") \
X(LocKey_BetterVision, L"Better vision", L"Улучшенное зрение") \
X(LocKey_ThirdPerson, L"Third person", L"Третье лицо") \
X(LocKey_DefaultView, L"Default", L"Обычное") \
X(LocKey_Degrees, L"%d degrees", L"%d градусов") \
X(LocKey_FieldOfView, L"Field of view", L"Поле зрения") \
X(LocKey_FLASHLIGHT, L"FLASHLIGHT", L"ФОНАРИК") \
X(LocKey_LightIntensity, L"Light intensity", L"Яркость света") \
X(LocKey_IlluminationAngle, L"Illumination angle", L"Угол освещения") \
X(LocKey_WorksWhileCrouching, L"Works while crouching", L"Работает в приседе") \
X(LocKey_DEADHEAD, L"DEAD HEAD", L"МЕРТВАЯ ГОЛОВА") \
X(LocKey_MaxBatteryHead, L"Max battery", L"Полный заряд") \
X(LocKey_VISUALS, L"VISUALS", L"ВИЗУАЛ") \
X(LocKey_DisplayThroughWalls, L"Display through walls", L"Отображать через стены") \
X(LocKey_DisplayBoxesThroughWalls, L"Display boxes through walls", L"Отображать ящики через стены") \
X(LocKey_WithinSelectedDistance, L"Within selected distance", L"В пределах выбранной дистанции") \
X(LocKey_DisplayDistance, L"Display distance", L"Дистанция отображения") \
X(LocKey_XrayHighlight, L"X-ray hightlight", L"Рентген подсветка") \
X(LocKey_NotAllHighlighted, L"Some objects are not highlighted", L"Некоторые предметы не подсвечиваются") \
X(LocKey_VALUE, L"VALUE", L"ЦЕНА") \
X(LocKey_NoDamageValuables, L"Valuables dont get damage", L"Ценности не получают повреждений") \
X(LocKey_ChangeValue, L"Change value of all valuables:", L"Изменить цену всех ценностей:") \
X(LocKey_ToZero, L"To zero", L"На ноль") \
X(LocKey_ToMax, L"To max", L"На максимальную") \
X(LocKey_EXTRACTION, L"EXTRACTION POINTS", L"ТОЧКИ СБОРА") \
X(LocKey_ActivateNext, L"Activate next", L"Активировать следующую точку") \
X(LocKey_ENEMIES, L"ENEMIES", L"ВРАГИ") \
X(LocKey_Spawn, L"Spawn", L"Заспавить") \
X(LocKey_SpawnAlt, L"Spawn", L"Заспавнить") \
X(LocKey_CantSeePlayers, L"Cant see players", L"Не видят игроков") \
X(LocKey_NoGrabLimit, L"No grab time limitation", L"Можно держать вечно") \
X(LocKey_PLAYERS, L"PLAYERS", L"ИГРОКИ") \
X(LocKey_Tumble, L"Tumble", L"Уронить") \
X(LocKey_Kill, L"Kill", L"Убить") \
X(LocKey_HostOnly, L"Host only", L"Только хост") \
X(LocKey_Revive, L"Revive", L"Воскресить") \
X(LocKey_VoiceChat, L"Voice chat", L"Голосовой чат") \
X(LocKey_Alive, L"Alive", L"Живых") \
X(LocKey_Dead, L"Dead", L"Мертвых") \
X(LocKey_GENERAL, L"GENERAL", L"ОБЩЕЕ") \
X(LocKey_LevelsCompleted, L"Number of levels completed", L"Количество пройденных уровней") \
X(LocKey_NextLevel, L"Go next level", L"Перейти на следующий уровень") \
X(LocKey_MAPFILTER, L"MAP FILTER", L"ФИЛЬТР КАРТ") \
X(LocKey_MapFilterDesc, L"Select maps that will not be set. Available if you are host", L"Выберите карты которые не будут выпадать. Доступно если вы хост") \
X(LocKey_GUNS, L"GUNS", L"ОРУЖИЕ") \
X(LocKey_MaxBatteryGun, L"Max battery", L"Максимальная батарея") \
X(LocKey_LaserCrosshair, L"Laser crosshair", L"Лазерный прицел") \
X(LocKey_SPAWN, L"SPAWN", L"СПАВН") \
X(LocKey_TRUCK, L"TRUCK", L"ФУРГОН") \
X(LocKey_TeleportTruck, L"Teleport to truck", L"Телепортироваться в фургон") \
X(LocKey_Cosmetic, L"Cosmetic", L"Косметика") \
X(LocKey_Common, L"Common", L"Обычная") \
X(LocKey_Uncommon, L"Uncommon", L"Необычная") \
X(LocKey_Rare, L"Rare", L"Редкая") \
X(LocKey_UltraRare, L"Ultra rare", L"Очень редкая") \
X(LocKey_SETTINGS, L"SETTINGS", L"НАСТРОЙКИ") \
X(LocKey_Language, L"Language", L"Язык") \
X(LocKey_Hotkey, L"Hotkey", L"Горячая клавиша") \
X(LocKey_OpenCloseMenu, L"Open / close menu", L"Открыть / закрыть меню") \
X(LocKey_DarkenBackground, L"Darken the screen behind the menu", L"Затемнять экрана позади меню")\
X(LocKey_CrouchRest, L"Crouch rest", L"Восст. в приседе") \
X(LocKey_ExtraJump, L"Extra jump", L"Доп. прыжок") \
X(LocKey_Range, L"Range", L"Дальность") \
X(LocKey_SprintSpeed, L"Sprint speed", L"Скорость бега") \
X(LocKey_Stamina, L"Stamina", L"Выносливость") \
X(LocKey_Strength, L"Strength", L"Сила") \
X(LocKey_Launch, L"Launch", L"Запуск") \
X(LocKey_Wings, L"Wings", L"Крылья") \
X(LocKey_Health, L"Health", L"Здоровье") \
X(LocKey_PlayersCount, L"Players count", L"Счетчик игроков") \
X(LocKey_Climb, L"Climb", L"Карабканье") \
X(LocKey_HeadBattery, L"Head battery", L"Заряд головы") \
X(LocKey_Small, L"Small", L"Маленькая") \
X(LocKey_Medium, L"Medium", L"Средняя") \
X(LocKey_Large, L"Large", L"Большая") \
X(LocKey_Unknown, L"Unknown", L"Неизвестное") \
X(LocKey_UnlockAll, L"Unlock all", L"Открыть все")\
X(LocKey_ResetAll, L"Reset all", L"Сбросить все") \
X(LocKey_GrabTab, L"Grab", L"Захват") \
X(LocKey_IdentityTab, L"Identity", L"Личность") \
X(LocKey_PerformanceTab, L"Performance", L"Производительность") \
X(LocKey_GrabAssist, L"Grab Assist", L"Помощь в захвате") \
X(LocKey_CenterRadius, L"Center radius", L"Радиус от центра") \
X(LocKey_GrabThrough, L"Grab Through", L"Захват сквозь стены") \
X(LocKey_AntiMusicBoxSpin, L"Anti Music Box Spin", L"Отключить вращение музыкальной шкатулки") \
X(LocKey_AntiMusicBoxSpinHint, L"Stops the Music Box from steering your camera while it plays.", L"Не позволяет музыкальной шкатулке вращать камеру во время игры.") \
X(LocKey_AntiDuck, L"Anti Duck", L"Защита от утки") \
X(LocKey_AntiDuckHint, L"Best effort: tries to block accidental grabs of the Duck; some colliders may not be detected.", L"Пытается блокировать случайный захват утки; некоторые коллайдеры могут не распознаться.") \
X(LocKey_WallGrab, L"WallGrab", L"Захват сквозь стены") \
X(LocKey_IdentityName, L"Name", L"Имя") \
X(LocKey_Apply, L"Apply", L"Применить") \
X(LocKey_RandomName, L"Random Name", L"Случайное имя") \
X(LocKey_RandomAvatar, L"Random Avatar", L"Случайный аватар") \
X(LocKey_ResetIdentity, L"Reset Identity", L"Сбросить личность") \
X(LocKey_AvatarAndLobby, L"AVATAR AND LOBBY", L"АВАТАР И ЛОББИ") \
X(LocKey_Cancel, L"Cancel", L"Отмена") \
X(LocKey_Confirm, L"Confirm", L"Подтвердить") \
X(LocKey_Version, L"Version", L"Версия") \
X(LocKey_Performance, L"POTATO PC OPTIMIZATIONS", L"ОПТИМИЗАЦИЯ POTATO PC") \
X(LocKey_PerfShadows, L"Disable shadows", L"Отключить тени") \
X(LocKey_PerfAntiAliasing, L"Disable anti-aliasing", L"Отключить сглаживание") \
X(LocKey_PerfPixelLights, L"Disable pixel lights", L"Отключить пиксельное освещение") \
X(LocKey_PerfLodBias, L"Disable LOD bias", L"Отключить детализацию по расстоянию") \
X(LocKey_PerfFog, L"Disable fog", L"Отключить туман") \
X(LocKey_PerfSoftParticles, L"Disable soft particles", L"Отключить мягкие частицы") \
X(LocKey_PerfSoftVegetation, L"Disable soft vegetation", L"Отключить мягкую растительность") \
X(LocKey_PerfReflections, L"Disable realtime reflection probes", L"Отключить отражения в реальном времени") \
X(LocKey_PerfPostProcessing, L"Disable post-processing", L"Отключить постобработку") \
X(LocKey_PerfExperimental, L"EXPERIMENTAL", L"ЭКСПЕРИМЕНТАЛЬНО") \
X(LocKey_NameHint, L"Enter a lobby name", L"Введите имя для лобби") \
X(LocKey_RestoreSettings, L"Restore Settings", L"Восстановить настройки") \
X(LocKey_FpsCounter, L"FPS counter", L"Счётчик FPS") \
X(LocKey_FpsPosition, L"Counter position", L"Положение счётчика") \
X(LocKey_FpsSize, L"Text size", L"Размер текста") \
X(LocKey_FpsTopLeft, L"Top left", L"Слева сверху") \
X(LocKey_FpsTopRight, L"Top right", L"Справа сверху") \
X(LocKey_FpsBottomLeft, L"Bottom left", L"Слева снизу") \
X(LocKey_FpsBottomRight, L"Bottom right", L"Справа снизу") \
X(LocKey_Runtime, L"Runtime", L"Состояние игры") \
X(LocKey_ManageEnemies, L"Manage Enemies", L"Управление врагами") \
X(LocKey_AggroMonster, L"Aggro Monster", L"Спровоцировать монстра") \
X(LocKey_AggroTarget, L"Aggro target", L"Цель монстра") \
X(LocKey_AggroSelf, L"Target: you", L"Цель: вы") \
X(LocKey_Bring, L"Bring", L"Переместить") \
X(LocKey_Heal, L"Heal", L"Вылечить") \
X(LocKey_SetOneHp, L"Set 1 HP", L"Установить 1 HP") \
X(LocKey_AiDisable, L"AI Disable", L"Отключить ИИ") \
X(LocKey_AiEnable, L"AI Enable", L"Включить ИИ") \
X(LocKey_Status, L"Status", L"Статус") \
X(LocKey_ValuablesTotal, L"Valuables on map", L"Ценности на карте") \
X(LocKey_ItemsOnGround, L"Items on ground", L"Предметы на земле") \
X(LocKey_CartValue, L"Cart value", L"Стоимость в тележке") \
X(LocKey_ExtractionValue, L"Extraction value", L"Стоимость в точке") \
X(LocKey_MapValue, L"Value remaining on map", L"Осталось ценностей") \
X(LocKey_MonsterTiers, L"Monsters by tier", L"Монстры по уровням") \
X(LocKey_TruckBattery, L"Truck battery", L"Батарея грузовика") \
X(LocKey_HideStored, L"Hide valuables in cart/extraction", L"Скрывать сданные ценности")

#define LOC_PT_LIST(X) \
X(LocKey_Stats, L"Estat\u00edsticas") \
X(LocKey_Entities, L"Entidades") \
X(LocKey_Vision, L"Vis\u00e3o") \
X(LocKey_Valuables, L"Valores") \
X(LocKey_Items, L"Itens") \
X(LocKey_ShopInventory, L"Inventário na loja") \
X(LocKey_GrabItem, L"Pegar item") \
X(LocKey_NoDroppedItems, L"Nenhum item guardável no chão") \
X(LocKey_ShopInventoryHint, L"Ative o inventário na loja para guardar itens aqui.") \
X(LocKey_ShopInventoryInstructions, L"Segure um item e pressione 1, 2 ou 3 para guardá-lo no espaço correspondente.") \
X(LocKey_Level, L"N\u00edvel") \
X(LocKey_Misc, L"Diversos") \
X(LocKey_Settings, L"Configura\u00e7\u00f5es") \
X(LocKey_EspUpdateRate, L"Frequência de atualização do ESP") \
X(LocKey_EspMatchFrameRate, L"Sincronizar com a taxa de quadros") \
X(LocKey_EspFrameInterval, L"Atualizar a cada N quadros") \
X(LocKey_HideHostOnlyOptions, L"Ocultar opções de host quando não for host") \
X(LocKey_HEALTH, L"SA\u00daDE") \
X(LocKey_Godmode, L"Modo Deus") \
X(LocKey_HealToMax, L"Curar at\u00e9 o m\u00e1ximo") \
X(LocKey_MOVEMENT, L"MOVIMENTO") \
X(LocKey_InfiniteStamina, L"Energia infinita") \
X(LocKey_Default, L"Padr\u00e3o") \
X(LocKey_WalkingSpeed, L"Velocidade de caminhada") \
X(LocKey_RunningSpeed, L"Velocidade de corrida") \
X(LocKey_CrouchingSpeed, L"Velocidade agachado") \
X(LocKey_InfiniteJumps, L"Pulos infinitos") \
X(LocKey_DontTumble, L"N\u00e3o cair") \
X(LocKey_DisableTumbling, L"Desativar quedas causadas externamente") \
X(LocKey_GRAB, L"PEGAR") \
X(LocKey_HighStrength, L"For\u00e7a alta") \
X(LocKey_AvailableIfHost, L"Dispon\u00edvel se voc\u00ea for o anfitri\u00e3o") \
X(LocKey_UnlimitedRange, L"Alcance ilimitado") \
X(LocKey_NoOvercharge, L"Sem sobrecarga") \
X(LocKey_POWERUPS, L"MELHORIAS") \
X(LocKey_AutoApplyUpgrades, L"Aplicar melhorias automaticamente") \
X(LocKey_ManualApplication, L"Aplicar manualmente") \
X(LocKey_HEALTHPACKS, L"KITS DE CURA") \
X(LocKey_ApplyAidKit, L"Usar kit de cura dispon\u00edvel em voc\u00ea") \
X(LocKey_VIEW, L"VIS\u00c3O") \
X(LocKey_BetterVision, L"Vis\u00e3o aprimorada") \
X(LocKey_ThirdPerson, L"Terceira pessoa") \
X(LocKey_DefaultView, L"Padr\u00e3o") \
X(LocKey_Degrees, L"%d graus") \
X(LocKey_FieldOfView, L"Campo de vis\u00e3o") \
X(LocKey_FLASHLIGHT, L"LANTERNA") \
X(LocKey_LightIntensity, L"Intensidade da luz") \
X(LocKey_IlluminationAngle, L"\u00c2ngulo de ilumina\u00e7\u00e3o") \
X(LocKey_WorksWhileCrouching, L"Funciona agachado") \
X(LocKey_DEADHEAD, L"CABE\u00c7A MORTA") \
X(LocKey_MaxBatteryHead, L"Bateria m\u00e1xima") \
X(LocKey_VISUALS, L"VISUAL") \
X(LocKey_DisplayThroughWalls, L"Exibir atrav\u00e9s das paredes") \
X(LocKey_DisplayBoxesThroughWalls, L"Exibir caixas atrav\u00e9s das paredes") \
X(LocKey_WithinSelectedDistance, L"Dentro da dist\u00e2ncia selecionada") \
X(LocKey_DisplayDistance, L"Dist\u00e2ncia de exibi\u00e7\u00e3o") \
X(LocKey_XrayHighlight, L"Destaque de raio X") \
X(LocKey_NotAllHighlighted, L"Alguns objetos n\u00e3o recebem destaque") \
X(LocKey_VALUE, L"VALOR") \
X(LocKey_NoDamageValuables, L"Objetos valiosos n\u00e3o sofrem dano") \
X(LocKey_ChangeValue, L"Alterar o valor de todos os objetos:") \
X(LocKey_ToZero, L"Para zero") \
X(LocKey_ToMax, L"Para o m\u00e1ximo") \
X(LocKey_EXTRACTION, L"EXTRA\u00c7\u00c3O") \
X(LocKey_ActivateNext, L"Ativar o pr\u00f3ximo") \
X(LocKey_ENEMIES, L"INIMIGOS") \
X(LocKey_Spawn, L"Gerar") \
X(LocKey_SpawnAlt, L"Gerar") \
X(LocKey_CantSeePlayers, L"N\u00e3o detectam jogadores") \
X(LocKey_NoGrabLimit, L"Sem limite de tempo de agarre") \
X(LocKey_PLAYERS, L"JOGADORES") \
X(LocKey_Tumble, L"Derrubar") \
X(LocKey_Kill, L"Eliminar") \
X(LocKey_HostOnly, L"Apenas anfitri\u00e3o") \
X(LocKey_Revive, L"Reviver") \
X(LocKey_VoiceChat, L"Chat de voz") \
X(LocKey_Alive, L"Vivo") \
X(LocKey_Dead, L"Morto") \
X(LocKey_GENERAL, L"GERAL") \
X(LocKey_LevelsCompleted, L"N\u00famero de fases conclu\u00eddas") \
X(LocKey_NextLevel, L"Ir para a pr\u00f3xima fase") \
X(LocKey_MAPFILTER, L"FILTRO DE MAPAS") \
X(LocKey_MapFilterDesc, L"Selecione os mapas que n\u00e3o ser\u00e3o escolhidos. Dispon\u00edvel se voc\u00ea for o anfitri\u00e3o") \
X(LocKey_GUNS, L"ARMAS") \
X(LocKey_MaxBatteryGun, L"Bateria m\u00e1xima") \
X(LocKey_LaserCrosshair, L"Mira a laser") \
X(LocKey_SPAWN, L"GERAR") \
X(LocKey_TRUCK, L"CAMINH\u00c3O") \
X(LocKey_TeleportTruck, L"Teleportar para o caminh\u00e3o") \
X(LocKey_Cosmetic, L"Cosm\u00e9tico") \
X(LocKey_Common, L"Comum") \
X(LocKey_Uncommon, L"Incomum") \
X(LocKey_Rare, L"Raro") \
X(LocKey_UltraRare, L"Muito raro") \
X(LocKey_SETTINGS, L"CONFIGURA\u00c7\u00d5ES") \
X(LocKey_Language, L"Idioma") \
X(LocKey_Hotkey, L"Tecla de atalho") \
X(LocKey_OpenCloseMenu, L"Abrir / fechar menu") \
X(LocKey_DarkenBackground, L"Escurecer o fundo do menu") \
X(LocKey_CrouchRest, L"Recupera\u00e7\u00e3o agachado") \
X(LocKey_ExtraJump, L"Pulo extra") \
X(LocKey_Range, L"Alcance") \
X(LocKey_SprintSpeed, L"Velocidade de corrida") \
X(LocKey_Stamina, L"Energia") \
X(LocKey_Strength, L"For\u00e7a") \
X(LocKey_Launch, L"Lan\u00e7amento") \
X(LocKey_Wings, L"Asas") \
X(LocKey_Health, L"Sa\u00fade") \
X(LocKey_PlayersCount, L"N\u00famero de jogadores") \
X(LocKey_Climb, L"Escalada") \
X(LocKey_HeadBattery, L"Bateria da cabe\u00e7a") \
X(LocKey_Small, L"Pequeno") \
X(LocKey_Medium, L"M\u00e9dio") \
X(LocKey_Large, L"Grande") \
X(LocKey_Unknown, L"Desconhecido") \
X(LocKey_UnlockAll, L"Desbloquear tudo") \
X(LocKey_ResetAll, L"Redefinir tudo") \
X(LocKey_GrabTab, L"Grab") \
X(LocKey_IdentityTab, L"Identidade") \
X(LocKey_PerformanceTab, L"Desempenho") \
X(LocKey_GrabAssist, L"Assist\u00eancia de grab") \
X(LocKey_CenterRadius, L"Raio do centro") \
X(LocKey_GrabThrough, L"Grab atrav\u00e9s de obst\u00e1culos") \
X(LocKey_AntiMusicBoxSpin, L"Anti Music Box Spin") \
X(LocKey_AntiMusicBoxSpinHint, L"Impede a Music Box de girar sua c\u00e2mera enquanto toca.") \
X(LocKey_AntiDuck, L"Anti Duck") \
X(LocKey_AntiDuckHint, L"Tenta bloquear grabs acidentais no pato; alguns colliders podem n\u00e3o ser detectados.") \
X(LocKey_WallGrab, L"Grab atrav\u00e9s de paredes") \
X(LocKey_IdentityName, L"Nome") \
X(LocKey_Apply, L"Aplicar") \
X(LocKey_RandomName, L"Nome aleat\u00f3rio") \
X(LocKey_RandomAvatar, L"Avatar aleat\u00f3rio") \
X(LocKey_ResetIdentity, L"Redefinir identidade") \
X(LocKey_AvatarAndLobby, L"AVATAR E LOBBY") \
X(LocKey_Cancel, L"Cancelar") \
X(LocKey_Confirm, L"Confirmar") \
X(LocKey_Version, L"Vers\u00e3o") \
X(LocKey_Performance, L"OTIMIZA\u00c7\u00d5ES POTATO PC") \
X(LocKey_PerfShadows, L"Desativar sombras") \
X(LocKey_PerfAntiAliasing, L"Desativar anti-aliasing") \
X(LocKey_PerfPixelLights, L"Desativar luzes por pixel") \
X(LocKey_PerfLodBias, L"Reduzir n\u00edvel de detalhe") \
X(LocKey_PerfFog, L"Desativar neblina") \
X(LocKey_PerfSoftParticles, L"Desativar part\u00edculas suaves") \
X(LocKey_PerfSoftVegetation, L"Desativar vegeta\u00e7\u00e3o suave") \
X(LocKey_PerfReflections, L"Desativar reflexos em tempo real") \
X(LocKey_PerfPostProcessing, L"Desativar p\u00f3s-processamento") \
X(LocKey_PerfExperimental, L"EXPERIMENTAL") \
X(LocKey_NameHint, L"Digite um nome para o lobby") \
X(LocKey_RestoreSettings, L"Restaurar configurações") \
X(LocKey_FpsCounter, L"Contador de FPS") \
X(LocKey_FpsPosition, L"Posição do contador") \
X(LocKey_FpsSize, L"Tamanho do texto") \
X(LocKey_FpsTopLeft, L"Superior esquerdo") \
X(LocKey_FpsTopRight, L"Superior direito") \
X(LocKey_FpsBottomLeft, L"Inferior esquerdo") \
X(LocKey_FpsBottomRight, L"Inferior direito") \
X(LocKey_Runtime, L"Runtime") \
X(LocKey_ManageEnemies, L"Gerenciar inimigos") \
X(LocKey_AggroMonster, L"Aggro Monster") \
X(LocKey_AggroTarget, L"Alvo do aggro") \
X(LocKey_AggroSelf, L"Alvo: voc\u00ea") \
X(LocKey_Bring, L"Trazer") \
X(LocKey_Heal, L"Curar") \
X(LocKey_SetOneHp, L"Definir 1 HP") \
X(LocKey_AiDisable, L"Desativar IA") \
X(LocKey_AiEnable, L"Ativar IA") \
X(LocKey_Status, L"Status") \
X(LocKey_ValuablesTotal, L"Valuables no mapa") \
X(LocKey_ItemsOnGround, L"Itens no chão") \
X(LocKey_CartValue, L"Valor no carrinho") \
X(LocKey_ExtractionValue, L"Valor na extração") \
X(LocKey_MapValue, L"Valor restante no mapa") \
X(LocKey_MonsterTiers, L"Monstros por tier") \
X(LocKey_TruckBattery, L"Bateria do caminhão") \
X(LocKey_HideStored, L"Ocultar valuables no carrinho/extração")

namespace Cheat
{
    enum LocKey
    {
        #define X(key, en, ru) key,
        LOC_LIST(X)
        #undef X
        LocKey_Count
    };

    enum Lang
    {
        Lang_Eng,
        Lang_Ru,
        Lang_Pt,
        Lang_Count
    };

    inline constexpr Hax::WStringView g_LocDict[Lang_Count][LocKey_Count] = 
    {
        {
            #define X(key, en, ru) en,
            LOC_LIST(X)
            #undef X
        },
        {
            #define X(key, en, ru) ru,
            LOC_LIST(X)
            #undef X
        },
        {
            #define X(key, pt) pt,
            LOC_PT_LIST(X)
            #undef X
        }
    };
}
