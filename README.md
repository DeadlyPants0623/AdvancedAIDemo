# AdvancedAIDemo

A modular Unreal Engine 5 C++ gameplay sandbox that demonstrates multiple playable variants (combat, platforming, side-scrolling) plus AI behaviors built with StateTree.

This project appears to have started from the Third Person template and evolved into a larger teaching/prototyping environment focused on movement systems, combat interactions, and AI-driven enemies/NPCs.

## Table of Contents

- [Project Goals](#project-goals)
- [Current Gameplay Variants](#current-gameplay-variants)
- [Tech Stack](#tech-stack)
- [Project Layout](#project-layout)
- [Requirements](#requirements)
- [Getting Started](#getting-started)
- [Build and Run (Optional CLI)](#build-and-run-optional-cli)
- [Input System](#input-system)
- [AI Architecture](#ai-architecture)
- [Variant Deep Dive](#variant-deep-dive)
- [Maps and Default Startup](#maps-and-default-startup)
- [Configuration Highlights](#configuration-highlights)
- [Respawn and Progression Patterns](#respawn-and-progression-patterns)
- [Troubleshooting](#troubleshooting)
- [Known Gaps and Next Improvements](#known-gaps-and-next-improvements)

## Project Goals

- Demonstrate reusable Unreal gameplay architecture across multiple genres/modes in one codebase.
- Showcase `Enhanced Input` workflows for desktop and touch-friendly controls.
- Showcase `StateTree` AI workflows in practical gameplay scenarios.
- Keep systems modular enough to be remixed for learning and rapid prototyping.

## Current Gameplay Variants

1. **Combat (Third-person melee)**
   - Combo and charged attacks.
   - Enemy danger notifications and reactions.
   - Damage, knockback, life bars, death/respawn.
   - Enemy spawning waves via activatable spawners.

2. **Platforming (Third-person traversal)**
   - Hold jump, coyote time, double jump, wall jump.
   - Directional dash with cooldown/land reset behavior.

3. **Side-scrolling (2.5D platformer)**
   - Plane-constrained movement.
   - Wall jump, coyote time, soft-platform drop logic.
   - Interactions, pickups, NPC interactions, simple UI counters.

4. **Stealth Prototype (content-level only in current source view)**
   - Default startup map currently points to a stealth prototype level.

## Tech Stack

- **Engine:** Unreal Engine `5.7` (from `AdvancedAIDemo.uproject`)
- **Language:** C++ (with Blueprint extension points)
- **Core Modules:**
  - `EnhancedInput`
  - `AIModule`
  - `StateTreeModule`
  - `GameplayStateTreeModule`
  - `UMG` / `Slate`
  - `GameplayDebugger`
- **Project Module:** `AdvancedAIDemo` (runtime)

## Project Layout

```text
AdvancedAIDemo/
  Config/
  Content/
    Variant_Combat/
    Variant_Platforming/
    Variant_SideScrolling/
    Stealth/
    ThirdPerson/
  Source/
    AdvancedAIDemo/
      Variant_Combat/
        AI/
        Animation/
        Gameplay/
        Interfaces/
        UI/
      Variant_Platforming/
        Animation/
      Variant_SideScrolling/
        AI/
        Gameplay/
        Interfaces/
        UI/
  Plugins/
    VisualStudioTools/
```

### Key C++ Entry Areas

- `Source/AdvancedAIDemo/AdvancedAIDemo.Build.cs` - module dependencies/include paths.
- `Source/AdvancedAIDemo/Variant_Combat` - combat character, enemy AI, spawners, combat interfaces.
- `Source/AdvancedAIDemo/Variant_Platforming` - traversal-heavy movement character/controller.
- `Source/AdvancedAIDemo/Variant_SideScrolling` - side-scrolling movement, interactions, pickups, NPC AI.

## Requirements

- Unreal Engine `5.7` installed and associated with this project.
- Windows 64-bit target for the current workspace setup.
- C++ build toolchain compatible with your Unreal installation (typically Visual Studio + MSVC workloads used by UE).

## Getting Started

### 1) Open in Unreal Editor

- Open `AdvancedAIDemo.uproject`.
- If prompted, generate project files and build the C++ module.
- Launch in-editor and use map selection to switch between variants.

### 2) Recommended first maps

- Combat: `Content/Variant_Combat/Lvl_Combat.umap`
- Platforming: `Content/Variant_Platforming/Lvl_Platforming.umap`
- Side-scrolling: `Content/Variant_SideScrolling/Lvl_SideScrolling.umap`

### 3) Default startup behavior

From `Config/DefaultEngine.ini`:

- `GameDefaultMap=/Game/Stealth/Levels/Lvl_Prototype`
- `EditorStartupMap=/Game/Stealth/Levels/Lvl_Prototype`

So the editor/game will currently boot to the stealth prototype level unless changed in project settings.

## Build and Run (Optional CLI)

If you prefer command-line builds, use Unreal Build Tool from your UE installation. Paths vary by machine, so update paths to match your setup.

```powershell
# Example only - update Unreal installation path/version as needed
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" AdvancedAIDemoEditor Win64 Development "C:\Users\odin\Documents\Unreal Projects\AdvancedAIDemo\AdvancedAIDemo.uproject"
```

To launch from editor binaries (example path may differ):

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\odin\Documents\Unreal Projects\AdvancedAIDemo\AdvancedAIDemo.uproject"
```

## Input System

All three playable variants use `Enhanced Input` through their player controllers and character classes.

### Shared Input Pattern

- Player controllers add one or more `UInputMappingContext` assets at runtime.
- Variants support mobile/touch UI overlays when touch controls should be active.
- Characters expose `Do*` methods (`DoMove`, `DoJumpStart`, etc.) so both hardware input and UI controls can route through the same gameplay logic.

### Action Names by Variant (from C++)

**Combat character actions**

- `MoveAction`
- `LookAction`
- `MouseLookAction`
- `ComboAttackAction`
- `ChargedAttackAction`
- `ToggleCameraAction`

**Platforming character actions**

- `MoveAction`
- `LookAction`
- `MouseLookAction`
- `JumpAction`
- `DashAction`

**Side-scrolling character actions**

- `MoveAction`
- `JumpAction`
- `DropAction`
- `InteractAction`

> Note: concrete keyboard/gamepad key bindings are stored in Enhanced Input assets (Content), not directly in the C++ shown here.

## AI Architecture

The project uses Unreal `StateTree` for AI decision/control layers.

### Combat AI

Core classes:

- `ACombatAIController` - AIController with `UStateTreeAIComponent`.
- `ACombatEnemy` - AI-controlled melee enemy implementing attacker/damageable interfaces.
- `CombatStateTreeUtility.*` - custom StateTree tasks and conditions.

Custom StateTree logic includes:

- Grounded checks.
- Incoming danger checks with reaction windows/cone tests.
- Attack tasks for combo and charged attacks.
- Utility tasks for facing actors/locations, speed changes, and player info gathering.
- Landing wait tasks and delegates to sync animation/state transitions.

Combat ecosystem helpers:

- `ACombatEnemySpawner` spawns enemies sequentially and can activate linked actors when depleted.
- EQS context helpers (`EnvQueryContext_Player`, `EnvQueryContext_Danger`) support AI queries.

### Side-scrolling AI

Core classes:

- `ASideScrollingAIController` - AIController with `UStateTreeAIComponent`.
- `ASideScrollingNPC` - interactable AI character with temporary deactivation behavior.
- `SideScrollingStateTreeUtility.*` - task to locate/evaluate player target distance.

## Variant Deep Dive

### Combat Variant

Key systems in `Source/AdvancedAIDemo/Variant_Combat`:

- **Melee combo pipeline** using montage sections and cached input windows.
- **Charged attack pipeline** with hold/release behavior and loop section checks.
- **Damage model** with HP, life bar widgets, knockback impulses, partial ragdoll reactions.
- **Danger broadcast** so nearby enemies can react before attacks land.
- **Respawn flow** where character destruction triggers controller-driven respawn at checkpoint transform.

Important interfaces:

- `ICombatAttacker`
- `ICombatDamageable`
- `ICombatActivatable`

### Platforming Variant

Key systems in `Source/AdvancedAIDemo/Variant_Platforming`:

- **Multi-jump behavior**: regular jump, coyote-time jump, double jump, wall jump.
- **Wall jump detection** via forward sphere sweep.
- **Dash behavior** with gravity disable/restore and montage-driven dash end.
- **Movement-state tracking** using compact flags and timers.
- **Respawn flow** through controller that respawns at `PlayerStart`.

### Side-scrolling Variant

Key systems in `Source/AdvancedAIDemo/Variant_SideScrolling`:

- **2.5D movement** constrained to a movement plane.
- **Advanced jump behavior**: coyote-time, double jump, wall jump lockout.
- **Drop-through platform logic** with soft collision checks.
- **Interaction traces** for NPCs/world interactables.
- **Pickups + UI counter** via `ASideScrollingGameMode` and `USideScrollingUI`.
- **Respawn flow** through controller that respawns at `PlayerStart`.

## Maps and Default Startup

Detected map assets:

- `Content/Variant_Combat/Lvl_Combat.umap`
- `Content/Variant_Platforming/Lvl_Platforming.umap`
- `Content/Variant_SideScrolling/Lvl_SideScrolling.umap`
- `Content/Stealth/Levels/Lvl_Prototype.umap`
- `Content/ThirdPerson/Lvl_ThirdPerson.umap`

Current default startup map (from config):

- `Stealth/Levels/Lvl_Prototype`

## Configuration Highlights

From `Config/DefaultEngine.ini` and related config:

- Desktop-targeted project with DX12 default RHI.
- Ray tracing and Lumen-related settings enabled in project config.
- `GameplayDebugger` category keys customized.
- Redirects included for renamed classes/enums from older template or prior naming.
- `EnhancedInput` classes configured as defaults in `DefaultInput.ini`.

## Respawn and Progression Patterns

- **Combat:** character stores checkpoint-like transform in `ACombatPlayerController`; on death the pawn is destroyed and controller respawns `CharacterClass` at saved transform.
- **Platforming/Side-scrolling:** controllers respawn pawn at first `APlayerStart` actor.
- **Side-scrolling pickups:** collected pickups increment game mode counter and update/show UI.

## Troubleshooting

- If input appears inactive:
  - Verify the variant's mapping contexts are assigned in the active player controller Blueprint/class defaults.
  - Confirm you are possessing the expected variant character class.
- If touch UI appears unexpectedly:
  - Check `bForceTouchControls` in the active player controller defaults/config.
- If respawn does not happen:
  - Confirm `CharacterClass` is assigned on the player controller.
  - Verify pawn destruction delegates are wired (happens in `OnPossess`).
- If AI seems idle:
  - Check controller possession and assigned StateTree assets in Blueprint defaults.

## Known Gaps and Next Improvements

- Add explicit setup docs for required Blueprint asset assignments (mapping contexts, montages, UI widget classes, AI StateTrees).
- Add per-variant control tables with concrete key/gamepad bindings from Enhanced Input assets.
- Add automated gameplay tests or functional test maps for jump/combat/AI regressions.
- Add packaged build instructions (Development/Shipping) and CI pipeline notes.
- Add architecture diagrams for variant boundaries and shared systems.

