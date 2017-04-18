// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "SpiderNavGridSaveGame.generated.h"

USTRUCT()
struct FSpiderNavRelations
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<int32> Neighbors;
};

/**
 * 
 */
UCLASS()
class USpiderNavGridSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<int32, FVector> NavLocations;

	UPROPERTY()
	TMap<int32, FVector> NavNormals;

	UPROPERTY()
	TMap<int32, FSpiderNavRelations> NavRelations;
	
	UPROPERTY()
	FString SaveSlotName;

	UPROPERTY()
	uint32 UserIndex;

	USpiderNavGridSaveGame();
};
