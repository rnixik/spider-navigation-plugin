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

#include "SpiderNavigation.h"
#include "SpiderNavigationModule.h"

DEFINE_LOG_CATEGORY(SpiderNAV_LOG);

// Sets default values
ASpiderNavigation::ASpiderNavigation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bAutoLoadGrid = true;
	DebugLinesThickness = 0.0f;
}

// Called when the game starts or when spawned
void ASpiderNavigation::BeginPlay()
{
	Super::BeginPlay();
	if (bAutoLoadGrid) {
		LoadGrid();
	}
}

// Called every frame
void ASpiderNavigation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASpiderNavigation::AddGridNode(int32 SavedIndex, FVector Location, FVector Normal)
{
	FSpiderNavNode NavNode;
	NavNode.Location = Location;
	NavNode.Normal = Normal;

	int32 Index = NavNodes.Add(NavNode);
	NavNodes[Index].Index = Index;

	NodesSavedIndexes.Add(SavedIndex, Index);
}

void ASpiderNavigation::SetGridNodeNeighbors(int32 SavedIndex, TArray<int32> NeighborsSavedIndexes)
{
	int32* Index = NodesSavedIndexes.Find(SavedIndex);
	if (Index) {
		FSpiderNavNode* NavNode =  &(NavNodes[*Index]);
		for (int32 i = 0; i != NeighborsSavedIndexes.Num(); ++i) {
			 int32* NeighborIndex = NodesSavedIndexes.Find(NeighborsSavedIndexes[i]);
			 if (NeighborIndex) {
				 FSpiderNavNode* NeighborNode = &(NavNodes[*NeighborIndex]);
				 NavNode->Neighbors.Add(NeighborNode);
			 }
		}
	}
}

int32 ASpiderNavigation::GetNavNodesCount()
{
	return NavNodes.Num();
}

TArray<FVector> ASpiderNavigation::FindPath(FVector Start, FVector End, bool& bFoundCompletePath)
{
	TArray<FVector> Path;

	FSpiderNavNode* StartNode = FindClosestNode(Start);
	FSpiderNavNode* EndNode = FindClosestNode(End);
	TArray<FSpiderNavNode*> NodesPath = FindNodesPath(StartNode, EndNode, bFoundCompletePath);

	for (int32 i = 0; i < NodesPath.Num(); i++) {
		FSpiderNavNode* Node = NodesPath[i];
		Path.Add(Node->Location);
	}

	return Path;
}

TArray<FSpiderNavNode*> ASpiderNavigation::FindNodesPath(FSpiderNavNode* StartNode, FSpiderNavNode* EndNode, bool& bFoundCompletePath)
{
	TArray<FSpiderNavNode*> Path;
	FSpiderNavNode* Node = NULL;
	TArray <FSpiderNavNode*> Neighbors;

	if (!StartNode || !EndNode) {
		//GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Yellow, TEXT("Not found closest nodes"));
		UE_LOG(SpiderNAV_LOG, Warning, TEXT("Not found closest nodes"));
		return Path;
	}


	ResetGridMetrics();
	//OpenList.Empty();
	std::vector<FSpiderNavNode*> openList;
	TArray<FSpiderNavNode*> ClosedList;

	//OpenList.Add(StartNode);
	openList.push_back(StartNode);
	std::make_heap(openList.begin(), openList.end(), LessThanByNodeF());
	StartNode->Opened = true;


	while (openList.size()) {
		//Node = GetFromOpenList();
		std::pop_heap(openList.begin(), openList.end(), LessThanByNodeF());
		Node = openList.back();
		openList.pop_back();

		Node->Closed = true;
		if (Node != StartNode) {
			ClosedList.Add(Node);
		}

		if (Node->Index == EndNode->Index) {
			bFoundCompletePath = true;
			return BuildNodesPathFromEndNode(Node);
		}

		for (FSpiderNavNode* Neighbor : Node->Neighbors) {

			if (Neighbor->Closed) {
				continue;
			}

			//DrawDebugString(GetWorld(), Neighbor->Location, *FString::Printf(TEXT("[%d]"), Neighbor->Neighbors.Num()), NULL, FLinearColor(0.0f, 1.0f, 0.0f, 1.0f).ToFColor(true), 20.0f, false);

			// get the distance between current node and the neighbor
			// and calculate the next g score
			float NewG = Node->G + (Neighbor->Location - Node->Location).Size();

			// check if the neighbor has not been inspected yet, or
			// can be reached with smaller cost from the current node
			if (!Neighbor->Opened || NewG < Neighbor->G) {
				Neighbor->G = NewG;
				Neighbor->H = (Neighbor->Location - EndNode->Location).Size();
				Neighbor->F = Neighbor->G + Neighbor->H;
				Neighbor->ParentIndex = Node->Index;

				if (!Neighbor->Opened) {
					//OpenList.Add(Neighbor);
					openList.push_back(Neighbor);
					std::push_heap(openList.begin(), openList.end(), LessThanByNodeF());
					Neighbor->Opened = true;
				} else {
					// the neighbor can be reached with smaller cost.
					// Since its f value has been updated, we have to
					// update its position in the open list
					std::make_heap(openList.begin(), openList.end(), LessThanByNodeF());
				}
			}
		}
	}

	UE_LOG(SpiderNAV_LOG, Warning, TEXT("Not found complete path"));


	//Finding closest to end
	float IterMin = 99999999999.0f;
	for (FSpiderNavNode* IterNode : ClosedList) {
		if (IterNode->F < IterMin) {
			IterMin = IterNode->F;
			Node = IterNode;
		}
		//DrawDebugString(GetWorld(), closedList[i]->Location, *FString::Printf(TEXT("[%f, %f]"), closedList[i]->F, closedList[i]->H), NULL, FLinearColor(0.0f, 1.0f, 0.0f, 1.0f).ToFColor(true), DebugLinesThickness, false);
	}

	if (Node) {
		UE_LOG(SpiderNAV_LOG, Log, TEXT("Min F = %f"), Node->F);
		bFoundCompletePath = false;
		//DrawDebugString(GetWorld(), Node->Location, *FString::Printf(TEXT("MINH[%f]"), Node->H), NULL, FLinearColor(1.0f, 0.0f, 0.0f, 1.0f).ToFColor(true), DebugLinesThickness, false);

		return BuildNodesPathFromEndNode(Node);
	}


	return Path;
}

FSpiderNavNode* ASpiderNavigation::GetFromOpenList()
{
	float MinF = 9999999999.0f;
	FSpiderNavNode* MinNode = NULL;
	for (FSpiderNavNode* Node : OpenList) {
		if (Node->F < MinF) {
			MinF = Node->F;
			MinNode = Node;
		}
	}
	if (MinNode) {
		OpenList.Remove(MinNode);
	}

	return MinNode;
}

FSpiderNavNode* ASpiderNavigation::FindClosestNode(FVector Location)
{
	FSpiderNavNode* ClosestNode = nullptr;
	float MinDistance = 999999999;
	for (int32 i = 0; i != NavNodes.Num(); i++) {
		float Distance = (NavNodes[i].Location - Location).Size();
		if (Distance < MinDistance) {
			MinDistance = Distance;
			ClosestNode = &(NavNodes[i]);
		}
	}

	return ClosestNode;
}

void ASpiderNavigation::ResetGridMetrics()
{
	for (int32 i = 0; i != NavNodes.Num(); ++i) {
		FSpiderNavNode Node = NavNodes[i];
		Node.ResetMetrics();
		NavNodes[i] = Node;
	}
}

TArray<FVector> ASpiderNavigation::BuildPathFromEndNode(FSpiderNavNode* EndNode)
{
	TArray<FVector> Path;
	TArray<int32> ReversedPathIndexes;

	ReversedPathIndexes.Add(EndNode->Index);

	FSpiderNavNode* IterNode = EndNode;
	while (IterNode->ParentIndex > -1) {
		ReversedPathIndexes.Add(IterNode->ParentIndex);
		IterNode = &NavNodes[IterNode->ParentIndex];
	}

	for (int32 i = ReversedPathIndexes.Num() - 1; i > -1; i--) {
		int32 Index = ReversedPathIndexes[i];
		Path.Add(NavNodes[Index].Location);
	}

	return Path;
}

TArray<FSpiderNavNode*> ASpiderNavigation::BuildNodesPathFromEndNode(FSpiderNavNode* EndNode)
{
	TArray<FSpiderNavNode*> Path;
	TArray<int32> ReversedPathIndexes;

	ReversedPathIndexes.Add(EndNode->Index);

	FSpiderNavNode* IterNode = EndNode;
	while (IterNode->ParentIndex > -1) {
		ReversedPathIndexes.Add(IterNode->ParentIndex);
		IterNode = &NavNodes[IterNode->ParentIndex];
	}

	for (int32 i = ReversedPathIndexes.Num() - 1; i > -1; i--) {
		int32 Index = ReversedPathIndexes[i];
		Path.Add(&NavNodes[Index]);
	}

	return Path;
}

bool ASpiderNavigation::LoadGrid()
{
	UE_LOG(SpiderNAV_LOG, Log, TEXT("Start loading Spider nav data"));
	EmptyGrid();
	UE_LOG(SpiderNAV_LOG, Log, TEXT("After empty grid"));

	FVector* NormalRef = NULL;
	FVector Normal;

	USpiderNavGridSaveGame* LoadGameInstance = Cast<USpiderNavGridSaveGame>(UGameplayStatics::CreateSaveGameObject(USpiderNavGridSaveGame::StaticClass()));
	LoadGameInstance = Cast<USpiderNavGridSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->SaveSlotName, LoadGameInstance->UserIndex));
	if (LoadGameInstance) {
		UE_LOG(SpiderNAV_LOG, Log, TEXT("After getting load game instance"));
		for (auto It = LoadGameInstance->NavLocations.CreateConstIterator(); It; ++It) {
			NormalRef = LoadGameInstance->NavNormals.Find(It.Key());
			if (NormalRef) {
				Normal = *NormalRef;
			} else {
				Normal = FVector(0.0f, 0.0f, 1.0f);
			}
			AddGridNode(It.Key(), It.Value(), Normal);
		}
		UE_LOG(SpiderNAV_LOG, Log, TEXT("After setting locations"));

		for (auto It = LoadGameInstance->NavRelations.CreateConstIterator(); It; ++It) {
			SetGridNodeNeighbors(It.Key(), It.Value().Neighbors);
		}
		UE_LOG(SpiderNAV_LOG, Log, TEXT("After setting relations"));

		UE_LOG(SpiderNAV_LOG, Log, TEXT("Nav Nodes Loaded: %d"), GetNavNodesCount());

		return true;
	}
	return false;
}

void ASpiderNavigation::EmptyGrid()
{
	NodesSavedIndexes.Empty();
	NavNodes.Empty();
}


void ASpiderNavigation::DrawDebugRelations()
{
	FColor DrawColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f).ToFColor(true);
	FColor DrawColorNormal = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f).ToFColor(true);
	float DrawDuration = 20.0f;
	bool DrawShadow = false;

	for (int32 i = 0; i != NavNodes.Num(); ++i) {
		FSpiderNavNode Nav = NavNodes[i];

		
		//DrawDebugString(GetWorld(), Nav.Location, *FString::Printf(TEXT("[%d]"), Nav.Neighbors.Num()), NULL, DrawColor, DrawDuration, DrawShadow);


		for (int32 j = 0; j != Nav.Neighbors.Num(); ++j) {
			FSpiderNavNode* NeighborNav = Nav.Neighbors[j];
			DrawDebugLine(
				GetWorld(),
				Nav.Location,
				NeighborNav->Location,
				DrawColor,
				false,
				DrawDuration,
				0,
				DebugLinesThickness
			);
		}

		DrawDebugLine(
			GetWorld(),
			Nav.Location,
			Nav.Location + Nav.Normal * 100.0f,
			DrawColorNormal,
			false,
			DrawDuration,
			0,
			DebugLinesThickness
		);


	}
}

FVector ASpiderNavigation::FindClosestNodeLocation(FVector Location)
{
	FVector NodeLocation;
	FSpiderNavNode* Node = FindClosestNode(Location);
	if (Node) {
		NodeLocation = Node->Location;
	}
	return NodeLocation;
}

FVector ASpiderNavigation::FindClosestNodeNormal(FVector Location)
{
	FVector NodeNormal;
	FSpiderNavNode* Node = FindClosestNode(Location);
	if (Node) {
		NodeNormal = Node->Normal;
	}
	return NodeNormal;
}

bool ASpiderNavigation::FindNextLocationAndNormal(FVector CurrentLocation, FVector TargetLocation, FVector& NextLocation, FVector& Normal)
{
	FSpiderNavNode* StartNode = FindClosestNode(CurrentLocation);
	FSpiderNavNode* EndNode = FindClosestNode(TargetLocation);
	bool bFoundPartialPath;

	FSpiderNavNode* NextNode = NULL;

	TArray<FSpiderNavNode*> NodesPath = FindNodesPath(StartNode, EndNode, bFoundPartialPath);
	
	if (NodesPath.Num() > 1) {
		NextNode = NodesPath[1];
	}

	if (!NextNode) {
		return false;
	}

	NextLocation = NextNode->Location;
	Normal = NextNode->Normal;

	return true;
}
