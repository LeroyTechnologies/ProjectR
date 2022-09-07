# ProjectM

## Latest Demo

1,000 vs 1,000 solider city battle:

[![1,000 vs 1,000 solider city battle](https://img.youtube.com/vi/3JT4XytxOiQ/0.jpg)](https://www.youtube.com/watch?v=3JT4XytxOiQ)

https://youtu.be/3JT4XytxOiQ

## What is this?

ProjectM is a proof of concept [MilSim](https://en.wikipedia.org/wiki/MilSim) FPS game developed with [Unreal Engine 5](https://www.unrealengine.com/en-US/unreal-engine-5), leveraging the new [MassEntity](https://docs.unrealengine.com/5.0/en-US/overview-of-mass-entity-in-unreal-engine/) ([ECS](https://en.wikipedia.org/wiki/Entity_component_system)) framework to have a very large number of entities not seen in traditional FPS games (tens or hundreds of thousands). Think [Squad](https://joinsquad.com/) meets [Foxhole](https://www.foxholegame.com/) meets [PlanetSide 2](https://www.planetside2.com/home) mixed in with massive number of AI controlled soldiers.

The project is open sourced to make it easy for others to contribute and to help others learn the MassEntity system which is not very well-documented/supported yet.

## Motivation

In today's "MilSim" games players frequently end up doing things or seeing this that would not happen in an actual MilSim. Examples:
- Soldiers magically spawn onto the battlefield
- Areas of interest are captured by standing near them
- Areas that should be defended have little or no one defending them (because it's boring)
- Soldiers being revived who should be dead
- Soliders being healed much faster than they should be able to

ProjectM aims to eliminate these unrealistic occurrences while keeping the game fun. It mainly does this by leveraging AI to do the not-so-fun stuff. Examples:
- Moving soldiers to the frontlines
- Defending all areas of interest, even those with little activity
- Logistics
- Vehicle repairs
- Medical evacuation (medevac)

Human players will never spawn soliders onto the battlefield. Instead they will take control of an AI-controller soldier already on the battlefield. Each match will last days/weeks, until all soldiers of one team are dead or main objectives have been accomplished.

# Phases

Currently this is a hobby project, so it's unclear how far it'll go. Therefore, the project is broken into multiple phases. For current status, see [Project Tracker](https://github.com/users/LeroyTechnologies/projects/1).

## Phase 1: Single-player

Same as multiplayer below, except only one human-controller soldier.

## Phase 2: Multiplayer MMO

All players in the game will be in a single battle at a time. Because of the scale being attempted, a single server will likely not be able to handle hosting this game. Instead it will require multiple servers communicating with each other.

The multiplayer phase of the game is split into two sub-phases below.

### Phase 2A: Server switching between zones

The map will be split up into zones. When a player reaches the border between zones, they load into a different server that manages that area of the map.

### Phase 2B: Server meshing across zones

Seamless switching between servers and players at the borders of servers will talk to all nearby servers to have a seamless experience across zones.

## Possible Additional Features

These ideas below would be explored in one of the phases above.

1. Railroad system for logistics. Railroads can be damaged and repaired.
1. Pontoon bridges which can be built for river crossings. Bridges can be destroyed.
1. Building destruction

# FAQ

- Where can I ask questions or get involved in the project? Email leroytech231+gh@gmail.com.


# Development Environment Setup

1. Install [Git for Windows](https://gitforwindows.org/). Use the default options.
1. Install Unreal Engine 5.0.3 from [Epic Games Launcher](https://store.epicgames.com/en-US/download).
1. Create a new project using the [City Sample project](https://www.unrealengine.com/marketplace/en-US/product/city-sample) from UE Marketplace.
1. In PowerShell:
    1. cd to City Sample project folder
    1. `cd Plugins`
    1. `git clone https://github.com/LeroyTechnologies/ProjectM.git`
1. Install Visual Studio 2022.
1. Generate VS project from right clicking CitySample.uproject in City Sample project folder.
1. Open CitySample.sln.
1. In order to get Mass ParallelForEachEntityChunk to actually parallelize, it requires passing argument to editor on launch:
    1. In VS Solution Explorer, right click CitySample project > Properties.
    1. Debugging > Command Arguments > Add "-ParallelMassQueries=1" to the end.
1. Run project from VS.
1. Add First Person Content Pack from Content Drawer > Add > Add Feature or Content Pack > Blueprint > First Person > Add to project.
1. Add Military Weapons Silver to project: https://www.unrealengine.com/marketplace/en-US/product/military-weapons-silver
    1. Then open M_WeaponMaster_01 > Used with Instanced Static Meshes > Apply > Save
1. Edit > Project Settings
    1. Engine - Input > Bindings > Action Mappings
        1. Add "MoveToCommand", 1 key
        1. Add "Spectate", F key
        1. Add "Respawn", R key
        1. Add "Map", M key
    1. (Unclear if this is actually needed, but if soldiers or projectiles don't render, do this) Engine - Mass > Mass > Module Settings > Mass Entity > Processor CDOs > "MassSimpleUpdateISMProcessor'/Script/ProjectM.Default__MassSimpleUpdateISMProcessor'" > Auto Register with Processing Phases > check
1. Now open L_Small level and use PIE to test out the project.
1. For a larger amount of soldiers see L_Large.

## To add soldiers to City Sample Level
1. Duplicate Small_City_LVL and call it Small_City_ProjectM_LVL
1. Open Small_City_ProjectM_LVL
1. Delete Actors
    1. Mass Spawners: BP_MassTraffic* and BP_MassCrowdSpawner
    1. BP_CitySampleWorldInfo
    1. BP_Nightmode
    1. SmartObjectCollection
1. Open L_Template
1. Copy MilitaryUnitMassSpawner*, BP_MassRifle, NavMeshBoundsVolume, and ProjectMWorldInfo actors
1. Paste in Small_City_ProjectM_LVL
1. Ensure NavMeshBoundsVolume is at location (0,0,90).
1. Select each MilitaryUnitMassSpawner* actor > Details > World Partition > Is Spatially Loaded > uncheck
1. In ProjectMWorldInfo actor, select WorldMapBoundingBox component and under Shape > Box Extent set the X and Y values so the box surrounds the map. If actor is at (0,0,0), a good extent is (100000.0, 100000.0).
1. Move BP_MassRifle to (X=-2190.125000,Y=18088.185547,Z=68.000000)
1. World Settings > GameMode Override > BP_FirstPersonGameModeCommander
1. We need to make collisions loaded all the time because AI soldiers rely on them to determine if they have line of sight to enemies:
    1. Load all cells in World Partition
    1. While we've got all cells loaded, build NavMesh from menu: Build > Build Paths
    1. In Outliner search for "coll"
    1. Click on any actor in the Outliner and select all (Ctrl+A)
    1. Details > World Partition > Is Spatially Loaded > uncheck
    1. World Partition: Unload all cells and load any if desired
