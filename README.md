# Stealth Variant AI System

A comprehensive stealth gameplay AI system for Unreal Engine 5, featuring intelligent guard behavior with dynamic suspicion mechanics, state-driven AI control, and inter-agent communication. This system provides a complete foundation for stealth-action games with realistic guard patrol, investigation, and pursuit mechanics.

---

## Table of Contents

1. [Overview](#overview)
2. [Core Architecture](#core-architecture)
3. [Guard States](#guard-states)
4. [AI Components](#ai-components)
5. [Character Systems](#character-systems)
6. [Behavior Tree Integration](#behavior-tree-integration)
7. [Suspicion System](#suspicion-system)
8. [Communication System](#communication-system)
9. [Configuration & Tuning](#configuration--tuning)
10. [Debug Tools](#debug-tools)
11. [Build & Deployment](#build--deployment)
12. [API Reference](#api-reference)

---

## Overview

The Stealth Variant system implements a multi-layered guard AI system with the following key features:

- **Dynamic Suspicion Mechanics**: Guards accumulate suspicion based on visibility, distance, and line-of-sight to targets
- **Five-State Guard FSM**: Patrol → Investigate → Alert → Chase, with external alert integration
- **AI Perception**: Sight-based perception with configurable ranges and peripheral vision
- **Inter-Agent Communication**: Guards broadcast alerts to nearby allies through the AI Communication Component
- **Behavior Tree-Driven**: Data-driven decision-making through Unreal's Behavior Tree system
- **Blueprint-Friendly**: Most parameters are editable in editor with real-time updates
- **Gameplay Debugger Integration**: In-game visualization of guard state, suspicion, and targets

### Key Systems

| System | Purpose |
|--------|---------|
| **AStealthAIController** | Brain of the guard; manages perception, state transitions, and blackboard |
| **AStealthGuardCharacter** | Physical representation; handles movement speeds and equipment (headlamp) |
| **AMyCharacter** | Player character with perception stimuli source for stealth gameplay |
| **APatrolRoute** | Visual editor tool for designing guard patrol paths |
| **UAICommunicationComponent** | Enables guards to broadcast and receive alerts |
| **Behavior Tree Services** | Continuous logic for suspicion updates, target confirmation, and state management |
| **Gameplay Debugger** | Real-time visualization of AI decision-making |

---

## Core Architecture

### Controller Hierarchy

```
AAIController
    └── AAStealthAIController (IAIAlertReceiver)
            ├── AIPerceptionComponent (Sight sense)
            ├── Blackboard (AI state/decisions)
            └── BehaviorTree (Logic flow)
```

### Component Relationships

```
AAStealthGuardCharacter
    ├── AAStealthAIController (possesses)
    ├── USpotLightComponent (HeadLamp - visual equipment)
    └── UAICommunicationComponent (broadcast/receive alerts)

AAMyCharacter (Player)
    └── UAIPerceptionStimuliSourceComponent (registers as hearing source)
```

### Data Flow

```
1. Perception Events (Sight)
   ↓
2. Blackboard Update (TargetActor, SuspectedActor, HasLos, InvestigateLocation)
   ↓
3. Behavior Tree Services (Update Suspicion, Confirm Target, Change State)
   ↓
4. State Transition (UpdateGuardState method)
   ↓
5. Character Movement Speed & AI Behavior Adjustment
   ↓
6. Communication Broadcasts (if target confirmed)
```

---

## Guard States

Guards operate in a five-state finite state machine. States are controlled by `UpdateGuardState()` and stored in the `"State"` blackboard key as an `EStealthGuardState` enum.

### EStealthGuardState Enumeration

```cpp
enum class EStealthGuardState : uint8
{
    Null          // Invalid/uninitialized
    Patrol        // Following patrol route passively
    Investigate   // Actively investigating suspicion location
    Alert         // High alert searching for target (lost LOS recently)
    Chase         // Pursuing confirmed target with active line-of-sight
    Debug         // Debug/testing state
};
```

### State Behavior Breakdown

#### **Patrol** (Suspicion < 0.2)
- **Trigger**: Default state when calm
- **Guard Movement**: `PatrolSpeed` (200 UU/s)
- **Behavior**: Follows `PatrolRoute` waypoints in sequence
- **Perception**: Active sight sense, scanning surroundings
- **Communication**: Inactive
- **Transition Conditions**:
  - → **Investigate**: If suspicion >= 0.2
  - → **Alert**: If external alert received
  - → **Chase**: If target acquired with LOS

#### **Investigate** (Suspicion ≥ 0.2 AND no confirmed target)
- **Trigger**: Guard becomes suspicious of something
- **Guard Movement**: `InvestigateSpeed` (300 UU/s)
- **Behavior**: 
  - Moves toward `InvestigateLocation` (last known suspect position)
  - Performs "Look Around" actions (scanning head/body)
  - Suspicion decays slowly while in this state
- **Perception**: Active sight sense with focus on investigation area
- **Communication**: May broadcast if highly suspicious
- **Transition Conditions**:
  - → **Patrol**: If suspicion decays below 0.2
  - → **Chase**: If target confirmed (distance < ConfirmDistance or Suspicion >= ConfirmSuspicion)
  - → **Alert**: If external alert interrupts

#### **Alert** (Recently chasing OR external alert active)
- **Trigger**: 
  - Lost target recently (within 5 seconds of losing LOS)
  - External alert received and still active
- **Guard Movement**: `AlertSpeed` (400 UU/s)
- **Behavior**: 
  - Searches last known location
  - May look around intensively
  - High vigilance posture
- **Perception**: Active heightened sight sense
- **Communication**: Broadcasting to allies
- **Transition Conditions**:
  - → **Chase**: If target re-acquired
  - → **Investigate**: If alert window expires and suspicion remains
  - → **Patrol**: If alert window expires and suspicion too low

#### **Chase** (bHasTarget == true AND bHasLOS == true)
- **Trigger**: Target acquired with visual confirmation
- **Guard Movement**: `ChaseSpeed` (600 UU/s)
- **Behavior**: 
  - Direct pursuit of target
  - Maintains line-of-sight focus
  - Updates "LastSeenTime" continuously
- **Perception**: Active sight sense tracking confirmed target
- **Communication**: Broadcasts to allies
- **Combat**: (Optional - framework supports combat integration)
- **Transition Conditions**:
  - → **Alert**: If line-of-sight lost
  - → **Patrol**: If alert window expires and target not re-found

#### **Debug** (Utility state)
- **Trigger**: Developer/testing mode
- **Guard Movement**: `DebugSpeed` (2000 UU/s - allows rapid testing)
- **Behavior**: Bypasses normal state logic for testing

---

## AI Components

### AAStealthAIController

The brain of the guard AI system. Extends `AAIController` and implements `IAIAlertReceiver` for inter-agent communication.

#### Key Properties

```cpp
// Core Assets
UBehaviorTree* BehaviorTreeAsset           // BT to execute
UBlackboardComponent* Blackboard           // AI decision data
UAIPerceptionComponent* AiPerceptionComponent
UAISenseConfig_Sight* AiSightConfig

// Perception Configuration
float SightRadius                = 3500.f  // How far guards can see
float LoseSightRadius            = 4000.f  // How far before losing target
float PeripheralVisionAngleDeg   = 90.f    // Cone of vision (±45°)
float MaxAgeSeconds              = 5.0f    // How long to remember stale stimuli
```

#### Suspicion System Tuning

```cpp
// Suspicion Gain Parameters
float SuspicionMaxGainDistance   = 2500.f  // Distance at which suspicion gain is max
float SuspicionMinDot            = 0.4f    // Dot product threshold for suspicion gain
float SuspicionBaseGainPerSecond = 0.6f    // Max suspicion gained per second (when close/centered)
float SuspicionDecayPerSecond    = 0.25f   // How fast suspicion decays when not suspicious

// Target Confirmation Parameters
float ConfirmDistance            = 600.f   // Distance to confirm target without high suspicion
float ConfirmSuspicion           = 0.95f   // Suspicion threshold to auto-confirm target

// Patrol Configuration
int8 StartingPatrolIndex         = 0       // Which patrol point to start at
```

#### External Alert System

```cpp
float ExternalAlertHoldSeconds   = 4.0f    // How long to maintain alert state from external broadcasts
```

#### Core Methods

**`ReceiveAIAlert(const FAIAlertData& AlertData)`**
- Interface implementation for receiving alerts from other guards
- Updates blackboard with alert information
- Raises suspicion based on alert strength
- Maintains alert state for duration specified

**`OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)`**
- Callback from perception system
- Distinguishes sight vs. hearing events
- Updates `HasLos`, `TargetActor`, `SuspectedActor`, `InvestigateLocation` blackboard keys

**`UpdateGuardState()`**
- Called by `BTService_ChangeState` every 0.15 seconds
- Implements state machine logic based on blackboard values
- Transitions between Patrol → Investigate → Alert → Chase
- Handles external alert timing
- Stores result in `"State"` blackboard key

#### Blackboard Keys

| Key | Type | Purpose |
|-----|------|---------|
| `TargetActor` | Object | Currently confirmed target (non-null = Chase state) |
| `SuspectedActor` | Object | Suspected but unconfirmed target |
| `InvestigateLocation` | Vector | Location to investigate (last known suspect position) |
| `HasLos` | Bool | Does guard have line-of-sight to suspected actor |
| `Suspicion` | Float | Current suspicion level (0.0 - 1.0) |
| `InvestigateActive` | Bool | Is investigation currently active |
| `PatrolActor` | Object | Current patrol route (reference) |
| `PatrolIndex` | Int | Current waypoint in patrol route |
| `State` | Enum | Current guard state (EStealthGuardState) |
| `LastSeenTime` | Float | World time when target was last confirmed |
| `ExternalAlertUntil` | Float | World time until external alert expires |

---

### AAStealthGuardCharacter

Physical character representation for guards. Extends `ACharacter` and provides movement speed management.

#### Components

```cpp
// Equipment
USpotLightComponent* HeadLamp
    └── Attached to "headLampSocket" on skeleton
    ├── Intensity: 20000
    ├── AttenuationRadius: 1500 UU
    └── Visual feedback for guard presence

// Communication
UAICommunicationComponent* AIComms
    └── Broadcasts alerts to nearby guards
    └── Radius: 2000 UU (configurable)
```

#### Movement Speeds

| State | Speed | Usage |
|-------|-------|-------|
| `PatrolSpeed` | 200 UU/s | Casual patrolling |
| `InvestigateSpeed` | 300 UU/s | Active searching |
| `AlertSpeed` | 400 UU/s | High alert searching |
| `ChaseSpeed` | 600 UU/s | Direct pursuit |
| `DebugSpeed` | 2000 UU/s | Developer testing |

#### Speed-Setting Methods

```cpp
void SetPatrolSpeed();       // CharacterMovement->MaxWalkSpeed = 200
void SetInvestigateSpeed();  // CharacterMovement->MaxWalkSpeed = 300
void SetAlertSpeed();        // CharacterMovement->MaxWalkSpeed = 400
void SetChaseSpeed();        // CharacterMovement->MaxWalkSpeed = 600
void SetDebugSpeed();        // CharacterMovement->MaxWalkSpeed = 2000
```

These are called from behavior tree tasks or services to transition movement speeds during state changes.

#### Blueprint Integration

The character is designed to be blueprinted with:
- Custom skeletal mesh
- Animation blueprints (idling, walking, running, investigating)
- Socket definitions (headLampSocket, gun socket, etc.)
- Material instances for visual feedback

---

### AAMyCharacter

Player character implementation with perception stimuli capabilities for stealth gameplay.

#### Key Features

```cpp
// Perception Registration
UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent
    └── Automatically registers for UAISense_Hearing
    └── Makes player "audible" to guards

// Camera System
USpringArmComponent* CameraBoom        // Third-person camera arm
UCameraComponent* FollowCamera         // Third-person follow camera

// Movement
MaxWalkSpeed: 500.f
JumpZVelocity: 500.f
AirControl: 0.35f
```

#### Gameplay Mechanics

- **Enhanced Input System**: Supports modern input actions (Jump, Move, Look, MouseLook)
- **Team Affiliation**: Set to team ID 2 (vs. guards on team 1)
- **Stimulus Broadcasting**: Player movement generates hearing stimuli for guards (framework ready)
- **Third-Person Camera**: Classic follow camera with spring arm collision

---

## Behavior Tree Integration

The system uses Unreal's Behavior Tree system for guard logic orchestration. The BT architecture follows this pattern:

```
BehaviorTree Root
├── Services (Continuous execution)
│   ├── BTService_Suspicion (Update suspicion based on perception)
│   ├── BTService_ChangeState (Update FSM state)
│   └── BTService_ConfirmTarget (Confirm suspected actors as targets)
└── Task Selection (State-dependent execution)
    ├── Patrol Tasks (Follow waypoints, idle)
    ├── Investigate Tasks (Move to location, look around)
    ├── Alert Tasks (Search, look around, broadcast)
    └── Chase Tasks (Direct pursuit, optional combat)
```

### Service-Based Architecture

Services are "passive" nodes that continuously update AI state without blocking execution. They run every `Interval` seconds (typically 0.15s).

#### BTService_ChangeState

- **Interval**: 0.15 seconds
- **Purpose**: Call `UpdateGuardState()` to manage FSM transitions
- **Result**: Updates `"State"` blackboard key
- **Implementation**: Simple wrapper around controller's `UpdateGuardState()`

#### BTService_ConfirmTarget

- **Interval**: 0.15 seconds
- **Purpose**: Confirm suspected actors as actual targets
- **Logic**:
  ```cpp
  if (HasLos && SuspectedActor != null)
  {
      float Distance = Distance(Guard, Suspect);
      float Suspicion = BB->GetValue("Suspicion");
      
      if (Distance <= ConfirmDistance || Suspicion >= ConfirmSuspicion)
      {
          BB->SetValue("TargetActor", Suspect);
          BB->SetValue("Suspicion", 1.0f);
          
          // Broadcast to allies!
          Guard->AIComms->BroadcastAlert(Confirmed, Location, 1.0f);
      }
  }
  ```
- **Communication**: Broadcasts confirmed targets to nearby guards

#### BTService_Suspicion

- **Interval**: 0.15 seconds
- **Purpose**: Update suspicion based on visual perception
- **Gain Conditions**:
  - Guard has line-of-sight to suspected actor
  - Actor is within `SuspicionMaxGainDistance`
  - Actor is within peripheral vision (adjusted by dot product)
- **Gain Formula**:
  ```cpp
  SuspicionGain = BaseGainPerSecond * DistanceFactor * AngleFactor * DeltaTime
  
  DistanceFactor = clamp(1.0 - (Distance / MaxGainDistance), 0, 1)
  AngleFactor = clamp((Dot - MinDot) / (1.0 - MinDot), 0, 1)
  ```
- **Decay**:
  - Without LOS in non-Investigate state: `DecayPerSecond`
  - In Investigate state without LOS: `DecayPerSecond * 0.5` (slower decay)
  - Final value clamped to [0, 1]

---

## Suspicion System

The suspicion system is the **core decision-making driver** of the stealth gameplay. It creates the nuanced guard behavior that makes stealth satisfying.

### Suspicion Mechanics

Suspicion is a float value in the range **[0.0, 1.0]** that represents how suspicious a guard is of the player.

### Suspicion Accumulation

**Conditions for Suspicion Gain:**
1. Guard has **line-of-sight** to player
2. Player is within **SuspicionMaxGainDistance** (2500 UU)
3. Player is within **peripheral vision cone** (modified by MinDot threshold)

**Gain Calculation Per Frame:**

```cpp
// Distance factor: max gain when close, zero at max distance
DistanceFactor = clamp(1.0 - (Distance / SuspicionMaxGainDistance), 0, 1)

// Angle factor: max gain when directly ahead, less at edges
Dot = ForwardVector · DirectionToPlayer
AngleFactor = clamp((Dot - SuspicionMinDot) / (1 - SuspicionMinDot), 0, 1)

// Combined gain
SuspicionGain = SuspicionBaseGainPerSecond * DistanceFactor * AngleFactor * DeltaTime
```

### Default Suspicion Thresholds

| Level | Value | Behavior |
|-------|-------|----------|
| Calm | 0.0 - 0.2 | Patrol state, no suspicion |
| Suspicious | 0.2 - 0.8 | Investigate state, searching |
| Highly Suspicious | 0.8 - 0.95 | Alert state, active search |
| Confirmed | ≥ 0.95 | Chase state, or by proximity |

### Suspicion Decay

**Without Line-of-Sight:**
```cpp
SuspicionDecay = SuspicionDecayPerSecond * DeltaTime

// In Investigate state, decay is halved (more persistent investigation)
SuspicionDecay = (SuspicionDecayPerSecond * 0.5) * DeltaTime
```

### Gameplay Implications

- **Short Distance**: Guards accumulate suspicion quickly (high DistanceFactor)
- **Peripheral Vision**: Guards miss targets outside their viewing cone (AngleFactor)
- **Investigate State**: Suspicion decays slower, encouraging guards to search thoroughly
- **Recent Chase**: Guards remain in Alert state for 5 seconds even without suspicion (AlertAfterChaseWindow)

### Player Stealth Strategies

Given these mechanics, stealth players can:

1. **Stay Behind**: Remain outside peripheral vision (low AngleFactor)
2. **Keep Distance**: Maintain distance > SuspicionMaxGainDistance (low DistanceFactor)
3. **Crouch/Hide**: Break line-of-sight (no LOS = no gain)
4. **Time Patrols**: Wait for guards to turn away before moving
5. **Create Distractions**: Make guards investigate elsewhere
6. **Use Elevation**: Get above guard's peripheral vision

---

## Communication System

Guards can broadcast alerts to nearby allies through the **AI Communication System**. This creates emergent gameplay where confirmed targets trigger chain reactions.

### UAICommunicationComponent

**Broadcast Range**: `AlertRadius` (2000 UU default)
- Guards within this radius who implement `IAIAlertReceiver` will receive broadcasts

**Broadcast Cooldown**: `BroadcastCooldownSeconds` (1.0s default)
- Prevents spam of multiple broadcasts in quick succession
- Guards broadcast again only if cooldown elapsed

### Alert Types

```cpp
enum class EAIAlertType : uint8
{
    Suspicious      // "Something might be here"
    Confirmed       // "I see the player!"
    LostTarget      // "They got away"
    BodyFound       // "There's a body here" (framework extensible)
};
```

### FAIAlertData Structure

```cpp
struct FAIAlertData
{
    EAIAlertType Type;              // Alert category
    FVector WorldLocation;          // Where the alert occurred
    TWeakObjectPtr<AActor> Sender;  // Which guard sent it
    float Strength;                 // Alert strength (0.0-1.0)
    float TimeStampSeconds;         // When it occurred
    TWeakObjectPtr<AActor> TargetActor;  // (Optional) confirmed target
};
```

### Broadcast Flow

1. **Guard Confirms Target** (in `BTService_ConfirmTarget`)
   ```cpp
   Guard->AIComms->BroadcastAlert(
       EAIAlertType::Confirmed, 
       TargetLocation, 
       1.0f  // Full strength
   );
   ```

2. **Communication Component Broadcasts**
   - Performs overlap query with `AlertRadius`
   - Filters results (can require LOS, skip self, etc.)
   - For each pawn found:
     - If pawn has AIController, forward to controller
     - Call `ReceiveAIAlert()` on controller if implements interface

3. **Receiving Guard Processes Alert**
   ```cpp
   bool AAStealthAIController::ReceiveAIAlert(const FAIAlertData& AlertData)
   {
       // Ignore if already chasing with LOS
       if (bHasTarget && bHasLOS && AlertData.Type != Confirmed)
           return false;
       
       // Update investigation location
       BB->SetValue("InvestigateLocation", AlertData.WorldLocation);
       
       // Raise suspicion
       BB->SetValue("Suspicion", Max(CurrentSuspicion, AlertData.Strength));
       
       // Hold alert state for duration
       if (AlertData.Type == Confirmed)
           BB->SetValue("ExternalAlertUntil", Now + ExternalAlertHoldSeconds);
       
       return true;
   }
   ```

4. **Receiving Guard Transitions**
   - Receiving alert raises suspicion/investigation location
   - Guard naturally transitions through Investigate → Alert → Chase as they search
   - Creates coordinated multi-guard responses to player

### Configuration

In `AICommunicationComponent`:

```cpp
UPROPERTY(EditAnywhere, Category="AI|Communication")
float AlertRadius = 2000.f;  // How far alerts broadcast

UPROPERTY(EditAnywhere, Category="AI|Communication")
float BroadcastCooldownSeconds = 1.0f;  // Cooldown between broadcasts

UPROPERTY(EditAnywhere, Category="AI|Communication")
bool bSkipSelf;  // Don't alert sender

UPROPERTY(EditAnywhere, Category="AI|Communication")
bool bRequireLineOfSightToReceiver = false;  // LOS requirement (not used by default)
```

---

## Patrol Route System

### APatrolRoute

A visual editor-friendly patrol system for designing guard paths. Guards follow waypoints in sequence, looping when complete.

#### Design Approach

Instead of hard-coded patrol points, **guards are assigned a `PatrolRoute` actor** in the world. This enables:
- **Visual editing**: Drag points in viewport
- **Per-instance variation**: Different guards follow different routes
- **Easy iteration**: Adjust routes without recompiling
- **Flexible sizing**: Any number of points

#### Key Features

```cpp
// Configuration
UPROPERTY(EditAnywhere, BlueprintReadOnly)
int32 PointCount = 5;  // How many points in this route

// Auto-Generated Points
UPROPERTY(VisibleAnywhere, Instanced)
TArray<TObjectPtr<USceneComponent>> PatrolPoints;  // 0..PointCount-1

// Utility
TArray<FVector> GetPatrolPointsLocations() const;
```

#### Usage in AI

1. **Assign Route in Editor**
   ```
   AStealthGuardCharacter -> PatrolRoute = (drag in APatrolRoute actor)
   ```

2. **Controller Initializes Patrol**
   ```cpp
   // In OnPossess()
   int8 MaxPatrolIndex = Guard->PatrolRoute->PointCount;
   BB->SetValueAsInt("PatrolIndex", FMath::RandRange(0, MaxPatrolIndex - 1));
   ```

3. **Behavior Tree Executes Patrol**
   ```
   BT Task: Move To PatrolRoute[PatrolIndex]
   BT Task: Increment PatrolIndex (wrap to 0 at end)
   Repeat
   ```

#### Editor Features

- **OnConstruction()**: Rebuilds points when `PointCount` property changes
- **PostEditChangeProperty()**: Responds to editor changes in real-time
- **Debug Visualization**: Console variable `patrolpoints.debug.show` displays patrol route with green spheres and connecting lines

#### Debug Visualization

Enable patrol route visualization:
```
patrolpoints.debug.show 1  // In console or launch config
```

This draws:
- Green spheres at each patrol point
- Yellow lines connecting sequential points
- Updates in real-time as game runs

---

## Configuration & Tuning

All guard parameters are **configurable per-instance** in the editor for rapid iteration. Key tuning locations:

### Guard-Level Tuning (AAStealthAIController)

Set these values on each guard instance:

```cpp
// === Perception ===
AiSightConfig->SightRadius = 3500.f;              // Detection range
AiSightConfig->LoseSightRadius = 4000.f;          // Range where guard forgets target
AiSightConfig->PeripheralVisionAngleDegrees = 90.f;  // ±45° vision cone

// === Suspicion Gain ===
SuspicionMaxGainDistance = 2500.f;     // Distance for max suspicion gain
SuspicionMinDot = 0.4f;                // Cos(angle) threshold for peripheral vision
SuspicionBaseGainPerSecond = 0.6f;     // Max gain rate when close & centered

// === Suspicion Decay ===
SuspicionDecayPerSecond = 0.25f;       // Decay rate without LOS

// === Target Confirmation ===
ConfirmDistance = 600.f;               // Auto-confirm distance
ConfirmSuspicion = 0.95f;              // Auto-confirm suspicion threshold

// === Patrol ===
StartingPatrolIndex = 0;               // Which point to start on

// === Communication ===
ExternalAlertHoldSeconds = 4.0f;       // Duration of external alert state
```

### Character-Level Tuning (AAStealthGuardCharacter)

```cpp
// === Movement Speeds ===
PatrolSpeed = 200.f;         // Casual patrolling
InvestigateSpeed = 300.f;    // Active searching
AlertSpeed = 400.f;          // High alert
ChaseSpeed = 600.f;          // Full pursuit

// === Equipment ===
HeadLamp->Intensity = 20000.f;
HeadLamp->AttenuationRadius = 1500.f;
```

### Communication Tuning (UAICommunicationComponent)

```cpp
AlertRadius = 2000.f;                  // Broadcast range
BroadcastCooldownSeconds = 1.0f;       // Throttle rate
bSkipSelf = true;                      // Don't alert broadcaster
bRequireLineOfSightToReceiver = false;  // LOS not required
```

### Behavior Tree Services

Modify service intervals for different update rates:

```cpp
// BTService_ChangeState, BTService_ConfirmTarget, BTService_Suspicion
Interval = 0.15f;  // Updates 6-7 times per second (good balance)
```

### Typical Tuning Scenarios

**"Paranoid Guards"** - Increase gain, decrease decay:
```cpp
SuspicionMaxGainDistance = 3500.f;     // See farther
SuspicionBaseGainPerSecond = 1.0f;     // Gain faster
SuspicionDecayPerSecond = 0.1f;        // Remember longer
```

**"Oblivious Guards"** - Opposite settings:
```cpp
SuspicionMaxGainDistance = 1500.f;
SuspicionBaseGainPerSecond = 0.2f;
SuspicionDecayPerSecond = 0.5f;
```

**"Coordinated Squad"** - Fast communication:
```cpp
AlertRadius = 3000.f;                  // Broadcast farther
ExternalAlertHoldSeconds = 6.0f;       // Longer alert state
```

---

## Debug Tools

### Gameplay Debugger Integration

The system includes a custom Gameplay Debugger category for in-game monitoring.

#### FGameplayDebuggerCategory_Suspicion

Displays real-time guard AI information when using the Gameplay Debugger (`Apostrophe key to toggle, then select guard`).

**Displayed Information:**

```
State: {current guard state}
HasLOS: {true/false}    HasTarget: {true/false}
Target: {target actor name}    Suspect: {suspected actor name}
Suspicion: {0.00 - 1.00}
Suspicion Range: {max distance}
Confirm Range: {distance}  (ConfirmSuspicion: {threshold})
ExternalAlert: ACTIVE  ({remaining time})
Comms Range: {radius} Cooldown: {seconds until ready}
```

#### Enabling Gameplay Debugger

```cpp
// Compile with:
#if WITH_GAMEPLAY_DEBUGGER
// Debugger features enabled
#endif

// In-game:
- Press Apostrophe key to open debugger
- Click on guard to focus
- Debugger category appears automatically
```

#### What to Look For

1. **State Transitions**: Watch states change as player moves
2. **Suspicion Growth**: Confirm suspicion increases with proximity/visibility
3. **External Alerts**: Verify broadcasts from other guards arrive
4. **Target Locking**: Confirm target actor updates when guard spots player
5. **LOS Tracking**: HasLOS updates when guards gain/lose sight

### Console Variables

```cpp
// Patrol Route Visualization
patrolpoints.debug.show 0/1      // Toggle patrol point visualization
```

When enabled, draws green spheres at patrol points and yellow lines between them.

### Logging

The system uses Unreal's logging system extensively. Check `Output Log` for:

```cpp
UE_LOG(LogTemp, Display, TEXT("AI Info"));  // Informational
UE_LOG(LogTemp, Warning, TEXT("AI Issue"));  // Potential problems
```

Key log messages:
- "Behavior tree started successfully"
- "Saw Actor: {name}"
- "Failed to initialize blackboard"
- "Found PatrolRoute: {name}"

---

## Performance Considerations

- **Perception**: Sight checks run every 0.1s by default (Unreal standard)
- **BT Services**: Set to 0.15s (6-7 updates/second) - balance between responsiveness and performance
- **Communication**: Limited by `BroadcastCooldownSeconds` to prevent spam
- **Blackboard Access**: Cheap operations; BT reads/writes happen frequently
- **Patrol Route**: Pre-computed patrol points; minimal overhead

For large numbers of guards (10+), consider:
- Reducing service `Interval` if guards feel unresponsive
- Increasing `Interval` if frame rate drops
- Disabling communication broadcasts for distant guards

---

## Extension Points

The system is designed for easy extension:

1. **Custom Alert Types**: Add to `EAIAlertType` enum
2. **Additional Services**: Create new `UBTService` subclasses
3. **State-Specific Behaviors**: Add cases to `UpdateGuardState()`
4. **Combat System**: Hook into Chase state for combat tasks
5. **Noise System**: Integrate with hearing sense (scaffolding exists)
6. **Memory System**: Track player sighting history in blackboard
7. **Coordinated Tactics**: Create group decision-making services

---

## Build & Deployment

### GitHub Actions Workflow

The project includes automated build, cook, and packaging via GitHub Actions. The workflow is defined in `.github/workflows/build-shipping.yml`.

#### Workflow Configuration

```yaml
name: Build UE Development (Win64)

on:
  push:
    branches: [ "master", "main" ]
  workflow_dispatch:

jobs:
  build:
    runs-on: [self-hosted, windows]

    steps:
      - name: Checkout
        uses: actions/checkout@v4
        with:
          lfs: true

      - name: Build/Cook/Package (Development)
        shell: powershell
        env:
          # CHANGE THIS to your Unreal Engine install path on YOUR PC.
          UE_PATH: C:\Program Files\Epic Games\UE_5.7
        run: |
          $ErrorActionPreference = "Stop"

          # Find the .uproject (or hardcode it for reliability)
          $uproject = Get-ChildItem -Recurse -Filter *.uproject | Select-Object -First 1
          if (-not $uproject) { throw "No .uproject found in repo." }

          $projectPath = $uproject.FullName
          Write-Host "Project: $projectPath"

          $uat = Join-Path $env:UE_PATH "Engine\Build\BatchFiles\RunUAT.bat"
          if (-not (Test-Path $uat)) { throw "RunUAT not found at: $uat. Check UE_PATH." }

          $outDir = Join-Path $env:GITHUB_WORKSPACE "BuildOutput"
          New-Item -ItemType Directory -Force -Path $outDir | Out-Null

          & $uat BuildCookRun `
            -project="$projectPath" `
            -noP4 `
            -platform=Win64 `
            -clientconfig=Development `
            -build -cook -stage -pak `
            -archive -archivedirectory="$outDir"

      - name: Upload artifact
        uses: actions/upload-artifact@v4
        with:
          name: DevelopmentBuild-Win64
          path: BuildOutput
```

#### Setup Instructions

1. **Configure Self-Hosted Runner**: The workflow uses `[self-hosted, windows]` runners, so you need a Windows machine registered as a GitHub Actions self-hosted runner.

2. **Set Unreal Engine Path**: Update `UE_PATH` environment variable to match your Unreal Engine 5.7 installation:
   ```powershell
   UE_PATH: C:\Program Files\Epic Games\UE_5.7
   ```

3. **Enable Triggers**: The workflow automatically triggers on pushes to `master` or `main` branches. You can also manually trigger via GitHub's "Actions" tab using `workflow_dispatch`.

4. **Build Output**: Packaged builds are available as artifacts named `DevelopmentBuild-Win64` in the GitHub Actions run summary.

#### What the Workflow Does

1. **Checkout**: Clones the repository with Git LFS support
2. **Build**: Compiles C++ source code
3. **Cook**: Processes assets into engine-native formats
4. **Stage**: Prepares the distribution package
5. **Package**: Creates the final executable package
6. **Archive**: Stores output in BuildOutput directory
7. **Upload Artifact**: Makes build available for download from GitHub

#### Customization

- **Change Platforms**: Modify `-platform=Win64` to support other platforms (e.g., `Linux`, `Mac`)
- **Change Configuration**: Replace `clientconfig=Development` with `Shipping` for final releases
- **Add More Branches**: Add branches to the `on.push.branches` list to trigger on additional branches

---

## Summary

The Stealth Variant AI system provides a **complete, extensible framework** for stealth gameplay. The layered architecture (Perception → Suspicion → State Machine → Behavior Tree) creates emergent guard behavior that's both challenging and fair to players. Parameters are tunable for different difficulty levels, and the communication system enables coordinated multi-guard responses for engaging encounters.

Key strengths:
- ✅ Data-driven behavior (Behavior Tree + Blackboard)
- ✅ Intuitive suspicion mechanics that encourage stealth gameplay
- ✅ Inter-agent communication for emergent squad behavior
- ✅ Extensive in-game debugging tools
- ✅ Fully configurable for rapid iteration
- ✅ Blueprint-friendly for designers
- ✅ Production-ready foundation for stealth games


