// Fill out your copyright notice in the Description page of Project Settings.

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

UCLASS()
class ASpiderNavGridBuilder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpiderNavGridBuilder();

	UPROPERTY(VisibleAnywhere, Category="SpiderNavGridBuilder")
		UBoxComponent* VolumeBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
		TSubclassOf<ASpiderNavGridTracer> TracerActorBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
		TSubclassOf<ASpiderNavPoint> NavPointActorBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
		TSubclassOf<ASpiderNavPointEdge> NavPointEdgeActorBP;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	//	ASpiderGridTracer* TracerActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float GridStepSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	TArray<AActor*> ActorsWhiteList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	bool bUseActorWhiteList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	bool bAutoRemoveTracers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	bool bAutoSaveGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float BounceNavDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float TraceDistanceModificator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float ClosePointsFilterModificator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float ConnectionSphereRadiusModificator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float TraceDistanceForEdgesModificator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float EgdeDeviationModificator;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<ASpiderNavGridTracer*> Tracers;
	TArray<ASpiderNavPoint*> NavPoints;

	TArray<FVector> NavPointsLocations;
	TMap<int32, FVector> NavPointsNormals;

	void TraceFromAllTracers();

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

	

	UFUNCTION(BlueprintCallable, Category = "SpiderNavGridBuilder")
	int32 BuildGrid();

	UFUNCTION(BlueprintCallable, Category = "SpiderNavGridBuilder")
	void DrawDebugRelations();

	UFUNCTION(BlueprintCallable, Category = "SpiderNavGridBuilder")
	void SaveGrid();
};
