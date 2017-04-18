// Fill out your copyright notice in the Description page of Project Settings.

#include "SpiderNavigationModule.h"
#include "SpiderNavPoint.h"


// Sets default values
ASpiderNavPoint::ASpiderNavPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->BodyInstance.SetCollisionProfileName("Custom");
	CollisionComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionComp->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetSphereRadius(10.f);

	RootComponent = CollisionComp;
}

// Called when the game starts or when spawned
void ASpiderNavPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpiderNavPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

