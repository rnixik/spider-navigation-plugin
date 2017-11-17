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

#include "SpiderNavGridBuilder.h"
#include "SpiderNavigationModule.h"

DEFINE_LOG_CATEGORY(SpiderNAVGRID_LOG);

// Sets default values
ASpiderNavGridBuilder::ASpiderNavGridBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	VolumeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("VolumeBox"));
	VolumeBox->BodyInstance.SetCollisionProfileName("Custom");
	VolumeBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	VolumeBox->BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VolumeBox->SetWorldScale3D(FVector(50.f, 50.f, 50.f));
	RootComponent = VolumeBox;

	GridStepSize = 100.0f;
	bUseActorWhiteList = false;
	bUseActorBlackList = false;
	bAutoRemoveTracers = true;
	bAutoSaveGrid = true;

	TracerActorBP = ASpiderNavGridTracer::StaticClass();
	NavPointActorBP = ASpiderNavPoint::StaticClass();
	NavPointEdgeActorBP = ASpiderNavPointEdge::StaticClass();

	BounceNavDistance = 3.0f;
	TraceDistanceModificator = 1.5f;
	ClosePointsFilterModificator = 0.1f;
	ConnectionSphereRadiusModificator = 1.5f;
	TraceDistanceForEdgesModificator = 1.9f;
	EgdeDeviationModificator = 0.15f;
	TracersInVolumesCheckDistance = 100000.0f;
	bShouldTryToRemoveTracersEnclosedInVolumes = false;
}

// Called when the game starts or when spawned
void ASpiderNavGridBuilder::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASpiderNavGridBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int32 ASpiderNavGridBuilder::BuildGrid()
{
	UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("Empty All"));
	EmptyAll();
	UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("Spawn tracers"));
	SpawnTracers();

	if (bShouldTryToRemoveTracersEnclosedInVolumes) {
		UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("RemoveTracersClosedInVolumes"));
		RemoveTracersClosedInVolumes();
	}

	UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("Trace from all tracers"));
	TraceFromAllTracers();
	
	if (bAutoRemoveTracers) {
		UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("Remove all tracers"));
		RemoveAllTracers();
	}
	

	UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("Nav Points Locations = %d"), NavPointsLocations.Num());

	UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("Spawn nav points"));
	SpawnNavPoints();
	UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("Build Relations"));
	BuildRelations();
	UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("Build edge relations"));
	BuildPossibleEdgeRelations();
	UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("End of build edge relations"));

	UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("RemoveNoConnected"));
	RemoveNoConnected();

	if (bAutoSaveGrid) {
		UE_LOG(SpiderNAVGRID_LOG, Log, TEXT("Saving grid"));
		SaveGrid();
	}
	

	//DrawDebugRelations();
	UE_LOG(SpiderNAVGRID_LOG, Warning, TEXT("Grid has been build. Nav Points = %d"), NavPoints.Num());

	return NavPoints.Num();
}

void ASpiderNavGridBuilder::SpawnTracers()
{
	FVector Origin;
	FVector BoxExtent;
	GetActorBounds(false, Origin, BoxExtent);
	FVector GridStart = Origin - BoxExtent;
	FVector GridEnd = Origin + BoxExtent;
	

	//Create a Spawn Parameters struct
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = false;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
	FRotator DefaultRotator = FRotator(0.0f, 0.0f, 0.0f);
	
	FVector TracerLocation;
	
	ASpiderNavGridTracer* Tracer = NULL;
	
	for (float x = GridStart.X; x < GridEnd.X; x = x + GridStepSize) {
		TracerLocation.X = x;
		for (float y = GridStart.Y; y < GridEnd.Y; y = y + GridStepSize) {
			TracerLocation.Y = y;
			for (float z = GridStart.Z; z < GridEnd.Z; z = z + GridStepSize) {
				TracerLocation.Z = z;
				Tracer = GetWorld()->SpawnActor<ASpiderNavGridTracer>(TracerActorBP, TracerLocation, DefaultRotator, SpawnParams);
				if (Tracer) {
					Tracers.Add(Tracer);
				}
			}
		}
		
	}
}

void ASpiderNavGridBuilder::RemoveTracersClosedInVolumes()
{
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), false, this);
	RV_TraceParams.bTraceComplex = false;
	RV_TraceParams.bTraceAsyncScene = true;
	RV_TraceParams.bReturnPhysicalMaterial = false;

	//ignore all tracers
	TArray<AActor*> ActorsToIgnore;
	for (int32 i = 0; i < Tracers.Num(); ++i) {
		AActor* Actor = Cast<AActor>(Tracers[i]);
		ActorsToIgnore.Add(Actor);
	}
	RV_TraceParams.AddIgnoredActors(ActorsToIgnore);

	float TraceDistance = TracersInVolumesCheckDistance;

	int32 RemovedNum = 0;

	for (int32 i = 0; i < Tracers.Num(); ++i) {
		ASpiderNavGridTracer* Tracer = Tracers[i];

		FVector StartLocation = Tracer->GetActorLocation();
		FVector EndLocation;

		TArray<AActor*> ActorsBumpedIn;

		for (int32 x = -1; x <= 1; x++) {
			for (int32 y = -1; y <= 1; y++) {
				for (int32 z = -1; z <= 1; z++) {
					FVector Direction = FVector(x, y, z);
					if (Direction.Size() == 1) {

						EndLocation = StartLocation + Direction * TraceDistance;
						FHitResult RV_Hit(ForceInit);

						GetWorld()->LineTraceSingleByChannel(
							RV_Hit,
							StartLocation,
							EndLocation,
							ECC_WorldStatic,
							RV_TraceParams
						);

						if (RV_Hit.bBlockingHit) {
							AActor* BlockingActor = RV_Hit.GetActor();
							ActorsBumpedIn.Add(BlockingActor);
						}
						
					}
				}
			}
		}

		// remove iterated tracer if there is the same actor around the tracer in 6 directions
		if (ActorsBumpedIn.Num() == 6) {
			AActor* FirstActor = ActorsBumpedIn[0];
			bool bFoundDifferent = false;
			for (AActor* ActorBumpedIn : ActorsBumpedIn)
			{
				if (ActorBumpedIn != FirstActor) {
					bFoundDifferent = true;
				}
			}
			if (!bFoundDifferent) {
				Tracers.Remove(Tracer);
				RemovedNum++;
			}
		}

	}

	UE_LOG(SpiderNAVGRID_LOG, Warning, TEXT("Removed enclosed tracers: %d"), RemovedNum);
}

void ASpiderNavGridBuilder::TraceFromAllTracers()
{
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), false, this);
	RV_TraceParams.bTraceComplex = false;
	RV_TraceParams.bTraceAsyncScene = true;
	RV_TraceParams.bReturnPhysicalMaterial = false;

	TArray<AActor*> ActorsToIgnore;
	for (int32 i = 0; i < Tracers.Num(); ++i) {
		AActor* Actor = Cast<AActor>(Tracers[i]);
		ActorsToIgnore.Add(Actor);
	}
	RV_TraceParams.AddIgnoredActors(ActorsToIgnore);

	float TraceDistance = GridStepSize * TraceDistanceModificator;

	for (int32 i = 0; i < Tracers.Num(); ++i) {
		ASpiderNavGridTracer* Tracer = Tracers[i];

		FVector StartLocation = Tracer->GetActorLocation();
		FVector EndLocation;

		for (int32 x = -1; x <= 1; x++) {
			for (int32 y = -1; y <= 1; y++) {
				for (int32 z = -1; z <= 1; z++) {
					FVector Direction = FVector(x, y, z);
					if (Direction.Size() == 1) {
						
						EndLocation = StartLocation + Direction * TraceDistance;

						//Re-initialize hit info
						FHitResult RV_Hit(ForceInit);

						//call GetWorld() from within an actor extending class
						GetWorld()->LineTraceSingleByChannel(
							RV_Hit,        //result
							StartLocation,    //start
							EndLocation, //end
							ECC_WorldStatic, //collision channel
							RV_TraceParams
						);

						AddNavPointByHitResult(RV_Hit);
					}
				}
			}
		}

		
	}
}

void ASpiderNavGridBuilder::AddNavPointByHitResult(FHitResult RV_Hit)
{
	if (RV_Hit.bBlockingHit) {

		AActor* BlockingActor = RV_Hit.GetActor();

		if (bUseActorWhiteList) {
			bool bFoundFromWhiteList = false;
			for (int32 i = 0; i < ActorsWhiteList.Num(); ++i) {
				if (ActorsWhiteList[i] == BlockingActor) {
					bFoundFromWhiteList = true;
					break;
				}
			}
			if (!bFoundFromWhiteList) {
				return;
			}
		}

		if (bUseActorBlackList) {
			for (AActor* BlackListedActor : ActorsBlackList) {
				if (BlockingActor == BlackListedActor) {
					return;
				}
			}
		}

		FVector NavPointLocation = RV_Hit.Location + RV_Hit.Normal * BounceNavDistance;
		bool bIsTooClose = false;

		for (int32 i = 0; i < NavPointsLocations.Num(); i++) {
			float Distance = (NavPointsLocations[i] - NavPointLocation).Size();
			if (Distance < GridStepSize * ClosePointsFilterModificator) {
				bIsTooClose = true;
				break;
			}
		}

		if (!bIsTooClose) {
			int32 PointIndex = NavPointsLocations.Add(NavPointLocation);
			NavPointsNormals.Add(PointIndex, RV_Hit.Normal);
		}
	}
}

void ASpiderNavGridBuilder::SpawnNavPoints()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	FRotator DefaultRotator = FRotator(0.0f, 0.0f, 0.0f);
	ASpiderNavPoint* NavPoint = NULL;

	for (int32 i = 0; i < NavPointsLocations.Num(); i++) {
		NavPoint = GetWorld()->SpawnActor<ASpiderNavPoint>(NavPointActorBP, NavPointsLocations[i], DefaultRotator, SpawnParams);
		if (NavPoint) {
			NavPoints.Add(NavPoint);
			FVector* Normal = NavPointsNormals.Find(i);
			if (Normal) {
				NavPoint->Normal = *Normal;
			}
		}
	}
}

void ASpiderNavGridBuilder::BuildRelations()
{
	FCollisionObjectQueryParams FCOQP = FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_WorldDynamic));
	
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(GridStepSize * ConnectionSphereRadiusModificator);
	

	ASpiderNavPoint* NavPoint = NULL;
	for (int32 i = 0; i < NavPoints.Num(); ++i) {
		NavPoint = NavPoints[i];
		FCollisionQueryParams FCQP = FCollisionQueryParams(FName(TEXT("RV_Trace")), false, this);
		FCQP.AddIgnoredActor(NavPoint);
		TArray < FOverlapResult > OutOverlaps;
		bool bIsOverlap = GetWorld()->OverlapMultiByObjectType(
			OutOverlaps,
			NavPoint->GetActorLocation(),
			FQuat::Identity, 
			FCOQP,
			Sphere,
			FCQP
		);

		if (bIsOverlap) {
			for (int32 j = 0; j < OutOverlaps.Num(); ++j) {
				FOverlapResult OverlapResult = OutOverlaps[j];
				AActor* OverlappedActor = OverlapResult.GetActor();
				if (OverlappedActor->IsA(ASpiderNavPoint::StaticClass())) {
					ASpiderNavPoint* OverlappedNavPoint = Cast<ASpiderNavPoint>(OverlappedActor);
					bool bIsVisible = CheckNavPointsVisibility(NavPoint, OverlappedNavPoint);
					if (bIsVisible) {
						NavPoint->Neighbors.AddUnique(OverlappedNavPoint);
						OverlappedNavPoint->Neighbors.AddUnique(NavPoint);
					} else {
						NavPoint->PossibleEdgeNeighbors.AddUnique(OverlappedNavPoint);
					}
					
				}
			}
		}
	}
}

void ASpiderNavGridBuilder::DrawDebugRelations()
{
	ASpiderNavPoint* NavPoint = NULL;
	ASpiderNavPoint* NeighborNavPoint = NULL;

	for (int32 i = 0; i < NavPoints.Num(); ++i) {
		NavPoint = NavPoints[i];

		FColor DrawColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f).ToFColor(true);
		FColor DrawColorNormal = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f).ToFColor(true);
		float DrawDuration = 10.0f;
		bool DrawShadow = false;
		//DrawDebugString(GEngine->GetWorldFromContextObject(this), NavPoint->GetActorLocation(), *FString::Printf(TEXT("[%d] - [%d]"), NavPoint->Neighbors.Num(), NavPoint->PossibleEdgeNeighbors.Num()), NULL, DrawColor, DrawDuration, DrawShadow);

		DrawDebugLine(
			GetWorld(),
			NavPoint->GetActorLocation(),
			NavPoint->GetActorLocation() + NavPoint->Normal * 70.0f,
			DrawColorNormal,
			false,
			DrawDuration,
			0,
			DebugThickness
		);

		for (int32 j = 0; j < NavPoint->Neighbors.Num(); ++j) {
			NeighborNavPoint = NavPoint->Neighbors[j];
			DrawDebugLine(
				GetWorld(),
				NavPoint->GetActorLocation(),
				NeighborNavPoint->GetActorLocation(),
				DrawColor,
				false,
				DrawDuration,
				0,
				DebugThickness
			);
		}

	}
}

/** 
* Returns true if navpoints could see each other 
*/
bool ASpiderNavGridBuilder::CheckNavPointsVisibility(ASpiderNavPoint* NavPoint1, ASpiderNavPoint* NavPoint2)
{
	FHitResult OutHit;
	ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility;

	FCollisionQueryParams TraceQueryParams = FCollisionQueryParams(FName(TEXT("RV_Trace_NavPoints")), false, this);
	TraceQueryParams.bTraceComplex = false;
	TraceQueryParams.bTraceAsyncScene = true;
	TraceQueryParams.bReturnPhysicalMaterial = false;
	TraceQueryParams.AddIgnoredActor(NavPoint1);

	bool bBlockingFound = GetWorld()->LineTraceSingleByChannel
	(
		OutHit,
		NavPoint1->GetActorLocation(),
		NavPoint2->GetActorLocation(),
		TraceChannel,
		TraceQueryParams
	);


	if (!bBlockingFound) {
		return false;
	}

	AActor* BlockedActor = OutHit.GetActor();
	if (!BlockedActor) {
		return false;
	}

	if (BlockedActor == NavPoint2) {
		return true;
	}

	return false;
}

/**
* Returns true if locations could see each other
*/
bool ASpiderNavGridBuilder::CheckNavPointCanSeeLocation(ASpiderNavPoint* NavPoint, FVector Location)
{
	FHitResult OutHit;
	ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility;

	FCollisionQueryParams TraceQueryParams = FCollisionQueryParams(FName(TEXT("RV_Trace_Locations")), false, this);
	TraceQueryParams.bTraceComplex = false;
	TraceQueryParams.bTraceAsyncScene = true;
	TraceQueryParams.bReturnPhysicalMaterial = false;
	TraceQueryParams.AddIgnoredActor(NavPoint);

	bool bBlockingFound = GetWorld()->LineTraceSingleByChannel
	(
		OutHit,
		NavPoint->GetActorLocation(),
		Location,
		TraceChannel,
		TraceQueryParams
	);

	return !bBlockingFound;
}

/** 
* Finds nav points on egdes of boxes (i.e. from side to top) finding intersection of pack of ortogonal vectors from both points.
*/
void ASpiderNavGridBuilder::BuildPossibleEdgeRelations()
{
	ASpiderNavPoint* NavPoint = NULL;
	ASpiderNavPoint* PossibleNavPoint = NULL;
	for (int32 i = 0; i < NavPoints.Num(); ++i) {
		NavPoint = NavPoints[i];
		for (int32 j = 0; j < NavPoint->PossibleEdgeNeighbors.Num(); ++j) {
			PossibleNavPoint = NavPoint->PossibleEdgeNeighbors[j];
			for (int32 x1 = -1; x1 <= 1; x1++) {
				for (int32 y1 = -1; y1 <= 1; y1++) {
					for (int32 z1 = -1; z1 <= 1; z1++) {
						for (int32 x2 = -1; x2 <= 1; x2++) {
							for (int32 y2 = -1; y2 <= 1; y2++) {
								for (int32 z2 = -1; z2 <= 1; z2++) {
									FVector Direction1 = FVector(x1, y1, z1);
									FVector Direction2 = FVector(x2, y2, z2);
									bool bOnlyOneNonZero = (Direction1.Size() == 1 && Direction2.Size() == 1);
									bool bCorrespingValuesAreNotEqual = ((Direction1.GetAbs() - Direction2.GetAbs()).Size() != 0);
									if (bOnlyOneNonZero && bCorrespingValuesAreNotEqual) {
										FVector Start0 = NavPoint->GetActorLocation();
										FVector End0 = NavPoint->GetActorLocation() + Direction1 * GridStepSize * TraceDistanceForEdgesModificator;
										FVector Start1 = PossibleNavPoint->GetActorLocation();
										FVector End1 = PossibleNavPoint->GetActorLocation() + Direction2 * GridStepSize * TraceDistanceForEdgesModificator;
										FVector Intersection;
										bool bIntersect = GetLineLineIntersection(Start0, End0, Start1, End1, Intersection);
										if (bIntersect) {
											CheckAndAddIntersectionNavPointEdge(Intersection, NavPoint, PossibleNavPoint);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		NavPoint->PossibleEdgeNeighbors.Empty();
	}
}

bool ASpiderNavGridBuilder::GetLineLineIntersection(FVector Start0, FVector End0, FVector Start1, FVector End1, FVector& Intersection)
{
	TMap<int32, FVector> v;
	v.Add(0, Start0);
	v.Add(1, End0);
	v.Add(2, Start1);
	v.Add(3, End1);

	float d0232 = Dmnop(&v, 0, 2, 3, 2);
	float d3210 = Dmnop(&v, 3, 2, 1, 0);
	float d3232 = Dmnop(&v, 3, 2, 3, 2);
	float mu = (d0232 * d3210 - Dmnop(&v, 0, 2, 1, 0)*d3232) / (Dmnop(&v, 1, 0, 1, 0) * Dmnop(&v, 3, 2, 3, 2) - Dmnop(&v, 3, 2, 1, 0) * Dmnop(&v, 3, 2, 1, 0));
    float NormalisedDistanceLine1 = (d0232 + mu * d3210) / d3232;
		 
	
	FVector ClosestPointLine0 = Start0 + mu * (End0 - Start0);
	FVector ClosestPointLine1 = Start1 + NormalisedDistanceLine1 * (End1 - Start1);

	float Distance = (ClosestPointLine0 - ClosestPointLine1).Size();

	bool bItersectionOnSegment1 = (NormalisedDistanceLine1 > 0);

	//@TODO: check if 3 points are collinear?
	
	if (Distance < GridStepSize * EgdeDeviationModificator && bItersectionOnSegment1) {
		Intersection = ClosestPointLine0;
		return true;
	}

	return false;
}

float ASpiderNavGridBuilder::Dmnop(const TMap<int32, FVector>* v, int32 m, int32 n, int32 o, int32 p)
{
	const FVector* vm = v->Find(m);
	const FVector* vn = v->Find(n);
	const FVector* vo = v->Find(o);
	const FVector* vp = v->Find(p);
	return (vm->X - vn->X) * (vo->X - vp->X) + (vm->Y -vn->Y) * (vo->Y - vp->Y) + (vm->Z - vn->Z) * (vo->Z - vp->Z);
}

void ASpiderNavGridBuilder::CheckAndAddIntersectionNavPointEdge(FVector Intersection, ASpiderNavPoint* NavPoint1, ASpiderNavPoint* NavPoint2)
{

	bool bFirstSee = CheckNavPointCanSeeLocation(NavPoint1, Intersection);
	if (!bFirstSee) {
		return;
	}
	bool bSecondSee = CheckNavPointCanSeeLocation(NavPoint2, Intersection);
	if (!bSecondSee) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = false;
	FRotator DefaultRotator = FRotator(0.0f, 0.0f, 0.0f);

	ASpiderNavPointEdge* NavPointEdge = GetWorld()->SpawnActor<ASpiderNavPointEdge>(NavPointEdgeActorBP, Intersection, DefaultRotator, SpawnParams);
	if (NavPointEdge) {
		NavPoints.Add(NavPointEdge);
		NavPoint1->Neighbors.Add(NavPointEdge);
		NavPoint2->Neighbors.Add(NavPointEdge);
		NavPointEdge->Neighbors.Add(NavPoint1);
		NavPointEdge->Neighbors.Add(NavPoint2);

		FVector Normal = NavPoint1->Normal + NavPoint2->Normal;
		Normal.Normalize();
		NavPointEdge->Normal = Normal;
	}
}

void ASpiderNavGridBuilder::RemoveAllTracers()
{
	for (int32 i = 0; i < Tracers.Num(); ++i) {
		Tracers[i]->Destroy();
	}
	Tracers.Empty();
}

void ASpiderNavGridBuilder::SaveGrid()
{
	TMap<int32, FVector> NavLocations;
	TMap<int32, FVector> NavNormals;
	TMap<int32, FSpiderNavRelations> NavRelations;

	ASpiderNavPoint* NavPoint = NULL;
	ASpiderNavPoint* NeighborNavPoint = NULL;

	for (int32 i = 0; i < NavPoints.Num(); ++i) {
		NavPoint = NavPoints[i];
		NavLocations.Add(i, NavPoint->GetActorLocation());
		NavNormals.Add(i, NavPoint->Normal);
		FSpiderNavRelations SpiderNavRelations;
		for (int32 j = 0; j < NavPoint->Neighbors.Num(); ++j) {
			int32 NeighborIndex = GetNavPointIndex(NavPoint->Neighbors[j]);
			SpiderNavRelations.Neighbors.Add(NeighborIndex);
		}
		NavRelations.Add(i, SpiderNavRelations);
	}

	USpiderNavGridSaveGame* SaveGameInstance = Cast<USpiderNavGridSaveGame>(UGameplayStatics::CreateSaveGameObject(USpiderNavGridSaveGame::StaticClass()));
	SaveGameInstance->NavLocations = NavLocations;
	SaveGameInstance->NavNormals = NavNormals;
	SaveGameInstance->NavRelations = NavRelations;
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex);
}

int32 ASpiderNavGridBuilder::GetNavPointIndex(ASpiderNavPoint* NavPoint)
{
	for (int32 i = 0; i < NavPoints.Num(); ++i) {
		if (NavPoints[i] == NavPoint) {
			return i;
		}
	}
	return -1;
}

void ASpiderNavGridBuilder::RemoveNoConnected()
{
	TArray<ASpiderNavPoint*> FilteredNavPoints;
	ASpiderNavPoint* NavPoint = NULL;

	for (int32 i = 0; i < NavPoints.Num(); ++i) {
		NavPoint = NavPoints[i];

		if (NavPoint->Neighbors.Num() > 1) {
			FilteredNavPoints.Add(NavPoint);
		} else {
			NavPoint->Destroy();
		}
	}

	NavPoints = FilteredNavPoints;
}

void ASpiderNavGridBuilder::RemoveAllNavPoints()
{
	ASpiderNavPoint* NavPoint = NULL;
	for (int32 i = 0; i < NavPoints.Num(); ++i) {
		NavPoint = NavPoints[i];
		NavPoint->Destroy();
	}
	NavPoints.Empty();
}

void ASpiderNavGridBuilder::EmptyAll()
{
	RemoveAllTracers();
	RemoveAllNavPoints();
	NavPointsNormals.Empty();
	NavPointsLocations.Empty();
}
