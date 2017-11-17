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

#include "SpiderNavGridTracer.h"
#include "SpiderNavigationModule.h"

// Sets default values
ASpiderNavGridTracer::ASpiderNavGridTracer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	CollisionComp->SetCollisionObjectType(ECC_WorldStatic);
	CollisionComp->BodyInstance.SetCollisionProfileName("Custom");
	CollisionComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionComp->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics, true);
	CollisionComp->SetSphereRadius(10.f);
	RootComponent = CollisionComp;

}

// Called when the game starts or when spawned
void ASpiderNavGridTracer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASpiderNavGridTracer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
