# Investigating and reducing Unity Update cost

## What the existing benchmark tells us

The main benchmark CSV times the Unity PlayerLoop `Update` phase as a whole. It cannot identify which managed method consumed that time. A high `update_cpu_avg_ms` is therefore evidence about the phase, not proof that a particular `MonoBehaviour.Update` is responsible.

The benchmark now writes a sibling `Benchmark_Methods_*.csv` file. While capture is enabled and the game is in a level, RepoHax measures managed methods named `Update`, `FixedUpdate`, and `LateUpdate` that pass through Mono's `mono_runtime_invoke`. Each one-second row contains method/class, call count, accumulated inclusive time, average time, and maximum single call time. Rows are sorted by accumulated time and capped at the top 24 per second; up to 256 distinct methods are tracked.

These are inclusive wall-clock durations around the managed invocation. Nested methods are included in their caller's duration, so summing rows double-counts nested work. This is targeted instrumentation, not Unity's full CPU Profiler: native engine work, callbacks dispatched by another route, jobs, GPU time, and wait attribution may be absent. An absent method does not establish that it is cheap. The instrumentation itself adds overhead while capture runs, especially from resolving method names; compare runs with the same capture settings and use Unity Profiler Timeline/Hierarchy where available.

## How to collect a useful capture

1. Start the game and load the same scene/state used for the comparison.
2. In Settings, start Benchmark capture and let the scene settle for at least 10 seconds.
3. Reproduce the slowdown for 30–60 seconds, then stop capture.
4. Open the newest main `Benchmark_*.csv` and matching `Benchmark_Methods_*.csv` in the diagnostics folder shown by the mod.
5. In the main CSV, compare frame time and `update_cpu_avg_ms`. In the methods CSV, group rows by class/method and compare `total_ms`, `avg_ms`, calls, and spikes. A costly method should be repeatably high across several rows, not just one transient sample.
6. Compare single-player and multiplayer at the same location, camera direction, and duration. Keep host/client role explicit; network and server-side work differ.

For Unity's detailed workflow, inspect CPU Usage Profiler **Timeline** to see activity across threads and **Hierarchy** to compare self time, total time, calls, and GC allocations. Timeline/Hierarchy and the CPU Profiler are documented by Unity: [CPU Usage Profiler (Unity 2022.3)](https://docs.unity3d.com/2022.3/Documentation/Manual/ProfilerCPU.html). Deep Profiling instruments every managed call and can substantially alter runtime cost; prefer the targeted capture first.

## Safe optimization applied

Mono's `mono_runtime_invoke` hook previously resolved a managed method name on every managed invocation, even when no benchmark was running. The normal path now skips that lookup. It resolves names during benchmark capture, when logging a managed exception, or while waiting for the Unity-loaded signal. This removes avoidable per-call metadata work from ordinary play without changing game script scheduling or ordering.

The benchmark records target method names and resolves the declaring class only when it first encounters a method. It performs no allocations in the per-invocation measurement path. The method table uses fixed storage and atomics; its output thread writes CSV once per second.

The next build also gates Runtime snapshot collection on the Runtime tab being open. Previously, the main-thread `EventSystem.Update` hook collected these diagnostics in the background during ordinary play, and repeatedly cleared the snapshot while outside a level. Post-processing component discovery is now limited to once every two seconds while its disable option is active; toggling it off still restores tracked components on the next settings pass. These changes target work owned by RepoHax visible on the measured `EventSystem.Update` path, while leaving the game's physics and network callbacks untouched.

The multiplayer crash capture from 2026-10-17 ended in the ESP's cart-content snapshot (`PhysGrabCart.itemsInCart` -> `List<PhysGrabObject>::TrySnapshot`) while collecting valuables. That code path has been removed. The valuables ESP now uses each object's `PhysGrabObjectImpactDetector.inCart` state to hide cart contents, avoiding traversal of the cart's live mutable managed list. The same capture also contains Photon `TimeoutDisconnect` and game `NullReferenceException` logs during lobby connection; those occur separately from the native crash stack and are not treated as its cause.

## Why not replace every Update with a custom manager

Unity notes that native-to-managed callback overhead can matter when a project has hundreds or thousands of `Update` callbacks, and recommends a custom Update Manager in that situation. Scripts must register and unregister when enabled/disabled, and the manager must preserve execution order and lifecycle behavior: [Unity per-frame event optimization](https://docs.unity3d.com/Manual/events-per-frame-optimization.html).

REPO's gameplay assemblies are closed source in this workspace. Globally redirecting their callbacks would require identifying every relevant script and preserving Unity's ordering, enable/disable behavior, destruction semantics, and interactions with plugins. Component counts alone do not reveal which components implement `Update`. A global rewrite is therefore not a safe optimization to apply from a mod without method-level evidence and compatibility testing.

## Next optimization decision

Use the method capture to identify repeatable hotspots first. If one mod-owned method dominates, optimize its work cadence, allocations, or cache invalidation locally. If a game-owned method dominates, inspect its callers and data with Unity Profiler or instrument a narrow, safe path. Avoid broad callback suppression or physics/render changes based only on aggregate `Update` time. Verify each change with matching captures and check gameplay behavior in the same multiplayer role and scene.
