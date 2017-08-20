# Spider Navigation Plugin For Unreal Engine 4

It implements custom navigation system suitable for spiders.
Navigation grid builds on floors, walls, ceilings.

![A demonstration of spider navigation](http://s.getid.org/github/spider-navigation-plugin.gif)

## Now available on [Marketplace](https://www.unrealengine.com/marketplace/spider-navigation)

## How it works

### To build grid

1. Scene filled up with special actors `Tracer` in specified volume with specified step.
2. From each tracer plugin traces to the nearest WorldStatic.
3. At point of the hit it spawns actor `NavPoint` with some offset.
4. From each `NavPoint` plugin check sphere collision with the othe nearest `NavPoint`.
5. Checks visibility between these two actors. If they are visible to each other - add connection. If not - add to the list of pissible neighbors.
6. Iterates the list of pissible neighbors and traces in 6 directions from each of two points for possible connection through an edge. 
Checks visibility between points of intersection. If a point of intersection is visible to each of two points - add a new point `NavPointEdge` and connections between them.

### To find path
* Plugin implements A* to find path. Can return a normal to each navigation point.

Plugin contains auxiliary blueprints for movement on this grid:

* SpiderAIController - containts the main code of movement on the spider grid
* SpiderPawn - Simple Pawn without any code
* SpiderBB - Demo Blackboard
* SpiderBT - Demo Behaviour Tree
* SpiderAgroCheck - Behaviour Task
* SpiderCloseEnough - Behaviour Decorator
* SpiderIsAtLocation - Behaviour Decorator
* SpiderIsTargetMoved - Behaviour Decorator
* SpiderMoveToLocation - Behaviour Task
* SpiderRapidMoveTo - Behaviour Task

## Setup

1. Close your project in UE4
2. Download plugin sources
3. Copy it into your project in `Project` folder. Make sure the path `./Plugins/SpiderNavigation/SpiderNavigation.uplugin` is correct.
4. Rebuild your project. If you have non-c++ project, then add in the editor one empty c++ class and then rebuild project in VS.
5. Run project. Be sure that SpiderNavigation plugin is enabled in Editor's Plugins menu.
6. Switch on the view option in the `Content` panel `View Options` (bottom right corner) to show plugin content.
7. Click on `Choose a path` - a small button to left of Content path with icon of open folder. Choose `SpiderNavigation Content`.
8. Add `DebugSpiderNavigationBP`, `DebugSpiderNavGridBuilderBP`, `SpiderPawn` on the scene.
9. Adjust the size of the `DebugSpiderNavGridBuilderBP` to match size of your level.
10. For `DebugSpiderNavigationBP` scene instance choose `Input`->`Auto Receive Input`: `Player 0` to enable hotkeys of navaigation.
11. Click on `Play`. Press `R` button to rebuild navigation grid. Press `Q` button to show navigation grid.

Plugin autosaves navigation grid. Try to click `Stop` and `Play` again. `Spider` pawn should follow you.

Here is a video guide:

[![Video guide](https://img.youtube.com/vi/ayobvtejDKg/0.jpg)](https://www.youtube.com/watch?v=ayobvtejDKg)


## Configuration

### SpiderAIController

* `MoveSpeed` - How fast spiders move
* `RotationSpeed` - How fast spiders rotate for each next navigation point
* `MustCheckTargetVisibility` - Whether a spider must trace the target by the visibility channel to follow it

### SpiderNavGridBuilder

* `GridStepSize` - The minimum distance between tracers. Should not be less then 40 (calculates too long)
* `ActorsWhiteList` - The list of actors which could have navigation points on them
* `bUseActorWhiteList` - Whether to use `ActorsWhiteList`
* `ActorsBlackList` - The list of actors which COULD NOT have navigation points on them
* `bUseActorBlackList` - Whether to use `ActorsBlackList`
* `bAutoRemoveTracers` - For debug. If false then all tracers remain on the scene after grid rebuild
* `bAutoSaveGrid` - Whether to save the navigation grid after rebuild
* `BounceNavDistance` - How far put navigation point from a WorldStatic face
* `TraceDistanceModificator` - How far to trace from tracers. Multiplier of `GridStepSize`
* `ClosePointsFilterModificator` - How close navigation points can be to each other. Multiplier of `GridStepSize`
* `ConnectionSphereRadiusModificator` - The radius of a sphere to find neighbors of each `NavPoint`. Multiplier of `GridStepSize`
* `TraceDistanceForEdgesModificator` - How far to trace from each `NavPoint` to find intersection through egdes of possible neightbors. Multiplier of `GridStepSize`
* `EgdeDeviationModificator` - How far can be one trace line from other trace line near the point of intersection when checking possible neightbors. Multiplier of `GridStepSize`
* `Tracer Actor BP` - For debug. Blueprint class which will be used to spawn actors on scene in specified volume
* `NavPointActorBP` - For debug. Blueprint class which will be used to spawn Navigation Points
* `NavPointEgdeActorBP` - For debug. Blueprint class which will be used to spawn Navigation Points on egdes when checking possible neightbors

### SpiderNavigation

* `bAutoLoadGrid` - Whether to load the navigation grid on BeginPlay

## Blueprint functions from the plugin

* `SpiderNavGridBuilder::BuildGrid`
* `SpiderNavGridBuilder::DrawDebugRelations`
* `SpiderNavGridBuilder::SaveGrid`

* `SpiderNavigation::FindPath`
* `SpiderNavigation::LoadGrid`
* `SpiderNavigation::DrawDebugRelations`
* `SpiderNavigation::FindClosestNodeLocation`
* `SpiderNavigation::FindClosestNodeNormal`
* `SpiderNavigation::FindNextLocationAndNormal`

## License

The MIT License

Copyright (C) 2017 Roman Nix

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
