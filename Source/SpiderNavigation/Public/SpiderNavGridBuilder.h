//The MIT License
//
//Copyright(C) 2017 Roman Nix
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.

#pragma once

#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "SpiderNavGridTracer.h"
#include "SpiderNavPoint.h"
#include "SpiderNavPointEdge.h"
#include "SpiderNavGridSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "SpiderNavGridBuilder.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(SpiderNAVGRID_LOG, Log, All);

/** Provides settings to configure navigation builder. Builds navigation grid with relations, which can be saved to file */
UCLASS()
class ASpiderNavGridBuilder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpiderNavGridBuilder();

    /** Volume where to build navigation */
	UPROPERTY(VisibleAnywhere, Category="SpiderNavGridBuilder")
		UBoxComponent* VolumeBox;

	/** For debug. Blueprint class which will be used to spawn actors on scene in specified volume */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
		TSubclassOf<ASpiderNavGridTracer> TracerActorBP;

	/** For debug. Blueprint class which will be used to spawn Navigation Points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
		TSubclassOf<ASpiderNavPoint> NavPointActorBP;

	/** For debug. Blueprint class which will be used to spawn Navigation Points on egdes when checking possible neightbors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
		TSubclassOf<ASpiderNavPointEdge> NavPointEdgeActorBP;

	/** The minimum distance between tracers to fill up scene */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float GridStepSize;

	/** The list of actors which could have navigation points on them */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	TArray<AActor*> ActorsWhiteList;

	/** Whether to use ActorsWhiteList */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	bool bUseActorWhiteList;

	/** The list of actors which COULD NOT have navigation points on them */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	TArray<AActor*> ActorsBlackList;

	/** Whether to use ActorsBlackList */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	bool bUseActorBlackList;

	/** For debug. If false then all tracers remain on the scene after grid rebuild */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	bool bAutoRemoveTracers;

	/** Whether to save the navigation grid after rebuild */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	bool bAutoSaveGrid;

	/** How far put navigation point from a WorldStatic face */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float BounceNavDistance;

	/** How far to trace from tracers. Multiplier of GridStepSize */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float TraceDistanceModificator;

	/** How close navigation points can be to each other. Multiplier of GridStepSize */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float ClosePointsFilterModificator;

	/** The radius of a sphere to find neighbors of each NavPoint. Multiplier of GridStepSize */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float ConnectionSphereRadiusModificator;

	/** How far to trace from each NavPoint to find intersection through egdes of possible neightbors. Multiplier of GridStepSize */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float TraceDistanceForEdgesModificator;

	/** How far can be one trace line from other trace line near the point of intersection when checking possible neightbors. Multiplier of GridStepSize */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float EgdeDeviationModificator;

    /** Whether should try to remove tracers enclosed in volumes */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
    bool bShouldTryToRemoveTracersEnclosedInVolumes;

    /** Distance threshold to remove tracers enclosed in volumes  */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
    float TracersInVolumesCheckDistance;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<ASpiderNavGridTracer*> Tracers;
	TArray<ASpiderNavPoint*> NavPoints;

	TArray<FVector> NavPointsLocations;
	TMap<int32, FVector> NavPointsNormals;

	void TraceFromAllTracers();

	void RemoveTracersClosedInVolumes();

	void AddNavPointByHitResult(FHitResult RV_Hit);

	void SpawnNavPoints();

	void BuildRelations();

	bool CheckNavPointsVisibility(ASpiderNavPoint* NavPoint1, ASpiderNavPoint* NavPoint2);

	bool CheckNavPointCanSeeLocation(ASpiderNavPoint* NavPoint, FVector Location);

	void BuildPossibleEdgeRelations();

	void RemoveNoConnected();

	void RemoveAllNavPoints();

	void EmptyAll();

	bool GetLineLineIntersection(FVector Start0, FVector End0, FVector Start1, FVector End1, FVector& Intersection);

	void CheckAndAddIntersectionNavPointEdge(FVector Intersection, ASpiderNavPoint* NavPoint1, ASpiderNavPoint* NavPoint2);

	float Dmnop(const TMap<int32, FVector> *v, int32 m, int32 n, int32 o, int32 p);

	void RemoveAllTracers();

	void SpawnTracers();

	int32 GetNavPointIndex(ASpiderNavPoint* NavPoint);

	float DebugThickness;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
    /** Builds navigation grid. Returs number of navigations points */
	UFUNCTION(BlueprintCallable, Category = "SpiderNavGridBuilder")
	int32 BuildGrid();

    /** Draws debug lines between connected navigation points */
	UFUNCTION(BlueprintCallable, Category = "SpiderNavGridBuilder")
	void DrawDebugRelations();

    /** Saves navigation grid to save file */
	UFUNCTION(BlueprintCallable, Category = "SpiderNavGridBuilder")
	void SaveGrid();
};
