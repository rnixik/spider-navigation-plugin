// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/SphereComponent.h"
#include "SpiderNavGridTracer.generated.h"

UCLASS()
class ASpiderNavGridTracer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpiderNavGridTracer();

	/** Sphere collision component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpiderNavGridBuilder")
	class USphereComponent* CollisionComp;


	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
