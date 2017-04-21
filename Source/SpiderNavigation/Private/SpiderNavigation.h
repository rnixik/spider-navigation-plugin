// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <algorithm>
#include <vector>         // std::vector
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "SpiderNavGridSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "SpiderNavigation.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SpiderNAV_LOG, Log, All);

USTRUCT()
struct FSpiderNavNode
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FVector Normal;


	UPROPERTY()
	int32 Index;

	TArray <FSpiderNavNode*> Neighbors;

	float F;
	float G;
	float H;
	bool Opened;
	bool Closed;
	int32 ParentIndex;


	FSpiderNavNode()
	{
		Location = FVector(0.0f, 0.0f, 0.0f);
		Index = -1;
		F = 0.0f;
		G = 0.0f;
		H = 0.0f;
		Opened = false;
		Closed = false;
		ParentIndex = -1;

		Neighbors.Empty();
	}

	void ResetMetrics()
	{
		F = 0.0f;
		G = 0.0f;
		H = 0.0f;
		Opened = false;
		Closed = false;
		ParentIndex = -1;
	}
};

struct LessThanByNodeF {
	bool operator()(const FSpiderNavNode* lhs, const FSpiderNavNode* rhs) const {
		return lhs->F > rhs->F;
	}
};

struct LessThanByNodeH {
	bool operator()(const FSpiderNavNode* lhs, const FSpiderNavNode* rhs) const {
		return lhs->H > rhs->H;
	}
};


UCLASS()
class ASpiderNavigation : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpiderNavigation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<FSpiderNavNode> NavNodes;
	// SavedIndex -> LocalIndex
	TMap<int32, int32> NodesSavedIndexes;

	

	void ResetGridMetrics();
	TArray<FVector> BuildPathFromEndNode(FSpiderNavNode* EndNode);

    FSpiderNavNode* FindClosestNode(FVector Location);

	void AddGridNode(int32 SavedIndex, FVector Location, FVector Normal);
	void SetGridNodeNeighbors(int32 SavedIndex, TArray<int32> NeighborsSavedIndexes);
	void EmptyGrid();

	TArray<FSpiderNavNode*> FindNodesPath(FSpiderNavNode* StartNode, FSpiderNavNode* EndNode, bool& bFoundCompletePath);
	TArray<FSpiderNavNode*> BuildNodesPathFromEndNode(FSpiderNavNode* EndNode);

	TArray<FSpiderNavNode*> OpenList;
	FSpiderNavNode* GetFromOpenList();

public:	

	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavigation")
	bool bAutoLoadGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	float DebugLinesThickness;
	
	UFUNCTION(BlueprintCallable, Category = "SpiderNavigation")
	int32 GetNavNodesCount();

	

	UFUNCTION(BlueprintCallable, Category = "SpiderNavigation")
	TArray<FVector> FindPath(FVector Start, FVector End, bool& bFoundCompletePath);

	UFUNCTION(BlueprintCallable, Category = "SpiderNavigation")
    bool LoadGrid();

	UFUNCTION(BlueprintCallable, Category = "SpiderNavigation")
	void DrawDebugRelations();

	UFUNCTION(BlueprintCallable, Category = "SpiderNavigation")
	FVector FindClosestNodeLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "SpiderNavigation")
	FVector FindClosestNodeNormal(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "SpiderNavigation")
	bool FindNextLocationAndNormal(FVector CurrentLocation, FVector TargetLocation, FVector& NextLocation, FVector& Normal);
};
