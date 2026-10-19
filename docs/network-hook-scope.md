# Hook scope after connection patches were removed

## Photon connection callbacks

RepoHax no longer detours `NetworkConnect.OnConnectedToMaster`, `OnJoinedRoom`, or `OnJoinRoomFailed`. It no longer changes Photon `DisconnectTimeout` or `SentCountAllowance`, retries rooms that return `GameClosed` or `GameFull`, or overrides the lobby player limit. Those callbacks and settings now follow the game's behavior.

## Gameplay hooks

Gameplay hooks detour individual game methods and call the original method through a trampoline. They support unrelated RepoHax features such as grab behavior, player cheats, item handling, and ESP state updates. They remain enabled. Photon API calls made by the game or by gameplay features (for RPCs, item actions, and spawning) are not the connection callback hooks listed above and remain available.

The `PhotonNetwork.IsMasterClient` detour is not active; its registration was already commented out. The `mono_runtime_invoke` and Unity PlayerLoop hooks remain for managed exception logging, benchmark capture, and frame processing.

## LateJoin

The LateJoin tab, config integration, and lobby player-limit override were removed from the active build. Its former source is kept in `backup` only and is not compiled.
