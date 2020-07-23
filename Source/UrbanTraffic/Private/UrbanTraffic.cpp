// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "UrbanTraffic.h"
#include "VehicleBase.h"
#include "RoadNode.h"
#include "RoadNodeNormal.h"
#include "RoadNodePort.h"
#include "RoadNodeCross.h"

#include <string>
#include <fstream>
#include <iostream>

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/Paths.h"
#include "DrawDebugHelpers.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FUrbanTrafficModule"

void FUrbanTrafficModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUrbanTrafficModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUrbanTrafficModule, UrbanTraffic)

AUrbanTraffic::AUrbanTraffic()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 2;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent->SetMobility(EComponentMobility::Static);
}

void AUrbanTraffic::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	// preview road system
	buildRoadSystem();
}

void AUrbanTraffic::PostInitializeComponents() {
	Super::PostInitializeComponents();
	// rebuild road system
	buildRoadSystem();
	// preload levels before play
	ConsumePreloadLevels();
}

void AUrbanTraffic::ConsumePreloadLevels() {
	if (PreloadLevels.Num()) {
		FString funcName = "ConsumePreloadLevels";
		FLatentActionInfo latent(0, 1, *funcName, this);
		FName levelName = PreloadLevels[0];
		PreloadLevels.RemoveAt(0);
		UGameplayStatics::LoadStreamLevel(this, levelName, true, false, latent);
		//UKismetSystemLibrary::PrintString(this, name.ToString());
	}
	else {
		// collect lights from preloaded levels
		RebuildLightSystem();
	}
}

void AUrbanTraffic::BeginPlay()
{
	Super::BeginPlay();
	// register player vehicle
	APawn* playerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	AVehicleBase* playerVehicle = Cast<AVehicleBase>(playerPawn);
	if (playerVehicle) {
		RegisterVehicle(playerVehicle);
		playerVehicle->onPlacedInSystem(this);
		playerVehicle->setAutoMode(PossessPlayerAI);
	}
	// spawn initial vehicles
	updateVehicleSpawnVolume(true);
}

void AUrbanTraffic::Tick(float DeltaSeconds) {
	// update and spawn vehicles
	updateVehicleSpawnVolume(false);
}

void AUrbanTraffic::BeginDestroy() {
	Super::BeginDestroy();
	cleanRoadSystem();
	timedLights.Empty();
}

// ================================================================
// ===                     ENVIRONMENT SYSTEM                   ===
// ================================================================

void AUrbanTraffic::RebuildLightSystem() {
	timedLights.Reset();
	// collect components from street lamps
	TArray<AActor*> lampActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStreetLamp::StaticClass(), lampActors);
	for (AActor* actor : lampActors) {
		AStreetLamp* lamp = Cast<AStreetLamp>(actor);
		lamp->collectLightComponents(timedLights);
	}
	// force update system by start time
	UpdateLightSystem(TimerStart, true);
}

void AUrbanTraffic::UpdateLightSystem(float timerHour, bool forceUpdate) {
	// change light on state by hours
	if (forceUpdate) {
		lightState = TimerStart < 5.5f || TimerStart > 18.5f;
	}
	else {
		if (5.5f < timerHour && timerHour < 18.5f) {
			if (lightState) {
				lightState = false;
				forceUpdate = true;
			}
		}
		else if (!lightState) {
			lightState = true;
			forceUpdate = true;
		}
	}
	// update visibility when needed
	if (forceUpdate) {
		for (USceneComponent* light : timedLights) {
			light->SetVisibility(lightState);
		}
		for (AVehicleBase* vehicle : vehicles) {
			vehicle->SetLightState(lightState);
		}
	}
}

// ================================================================
// ===                       VEHICLE SYSTEM                     ===
// ================================================================

void AUrbanTraffic::RegisterVehicle(AVehicleBase* vehicle) {
	if (!vehicles.Contains(vehicle)) {
		vehicle->SetLightState(lightState);
		vehicles.Add(vehicle);
	}
}

void AUrbanTraffic::UnregisterVehicle(AVehicleBase* vehicle) {
	if (vehicles.Contains(vehicle)) {
		vehicles.Remove(vehicle);
	}
}

URoadNode* AUrbanTraffic::findNearestRoadNode(FVector position) {
	return URoadNode::findNearestNode(position, roadNodes);
}

void AUrbanTraffic::cleanRoadSystem() {
	vehicles.Empty();
	spawnVolumeNodes.Reset();
	roadNodes.Reset();
	roadPorts.Reset();
	roadSegments.Empty();
	FlushPersistentDebugLines(GetWorld());
}

void AUrbanTraffic::buildRoadSystem() {
	// clear previous data
	cleanRoadSystem();

	// read and compile road data from file
	for (FString Path : RoadFiles) {
		std::ifstream is(*(FPaths::ProjectDir() + Path));
		URoadSegment* segment = new URoadSegment(this);
		TArray<URoadNode*> nodes; // temporary nodes for current segment
		std::string line;
		while (std::getline(is, line)) {
			if (line.compare("break")) {
				// definition command
				TArray<FString> params;
				FString(line.c_str()).ParseIntoArray(params, _T(","));
				int nodeType = FCString::Atoi(*params[0]);
				int nextIndex = FCString::Atoi(*params[1]);
				FVector position(
					FCString::Atof(*params[2]),
					FCString::Atof(*params[3]),
					FCString::Atof(*params[4]) + 10 // move up for easier debugging
				);
				switch (nodeType) {
				case 1:
					nodes.Add(new URoadNodePort(segment, nextIndex, position,
						FCString::Atof(*params[5]), // laneWidth
						FCString::Atoi(*params[6]), // numRights
						FCString::Atoi(*params[7])  // numLefts
					));
					break;
				case 2:
					nodes.Add(new URoadNodeNormal(segment, nextIndex, position));
					break;
				case 3:
					nodes.Add(new URoadNodeCross(segment, nextIndex, position));
					break;
				}
				roadNodes.Add(nodes.Last()); // cached
			}
			else {
				// break command
				roadSegments.Add(segment);
				segment->compileData(nodes);
				// collect ports and all spawn nodes
				for (URoadNode* node : nodes) {
					switch (node->getNodeType()) {
					case RoadNodeType::Port:
						roadPorts.Add((URoadNodePort*)node);
						break;
					}
				}
				segment = new URoadSegment(this);
				nodes.Reset(); // reset for new segment
			}
		}
		// free left-over stuffs
		delete segment;
		is.close();
	}

	// precomp connected ports
	TArray<URoadNodePort*> ports(roadPorts);
	while (ports.Num()) {
		URoadNodePort* a = ports[0];
		ports.RemoveAt(0);
		[&] {
			for (int i = 0; i < ports.Num(); i++) {
				URoadNodePort* b = ports[i];
				if (FVector::Dist(a->position, b->position) < 100) {
					a->connectPort(b);
					ports.RemoveAt(i);
					return; // break inner loop (for)
				}
			}
		} ();
	}

	// compile debug flags and draw
	uint8 debugFlags = 0;
	if (DrawRoadPaths) {
		debugFlags |= DEBUG_ROAD_PATH;
	}
	if (DrawRoadCrosses) {
		debugFlags |= DEBUG_ROAD_CROSS;
	}
	if (DrawRoadLimits) {
		debugFlags |= DEBUG_ROAD_LIMIT;
	}
	if (DrawRoadVectors) {
		debugFlags |= DEBUG_ROAD_VECTOR;
	}
	if (debugFlags) {
		UWorld* world = GetWorld();
		for (URoadSegment* segment : roadSegments) {
			segment->drawDebug(world, debugFlags);
		}
	}
}

void AUrbanTraffic::updateVehicleSpawnVolume(bool isBegin) {
	// find node from origin
	URoadNode* newSpawnNode = nullptr;
	APawn* playerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	AVehicleBase* playerVehicle = Cast<AVehicleBase>(playerPawn);
	if (playerVehicle) {
		if (!playerVehicle->isAutoMode() || playerVehicle->prevNode) {
			playerVehicle->prevNode = findNearestRoadNode(playerVehicle->GetActorLocation());
		}
		if (playerVehicle->prevNode != spawnOrigin) {
			spawnOrigin = playerVehicle->prevNode;
			newSpawnNode = spawnOrigin;
		}
	}

	// update spawn volume from new spawn origin
	if (newSpawnNode) {
		TArray<URoadNode*> nextNodes;
		spawnVolumeNodes.Reset();
		spawnVolumeNodes.Add(newSpawnNode, 0);
		nextNodes.Add(newSpawnNode);
		while (nextNodes.Num()) {
			URoadNode* node = nextNodes[0];
			nextNodes.RemoveAt(0);
			float baseDst = spawnVolumeNodes[node];
			URoadNode* crossNode = node->getSegment()->getCrossNode();
			if (crossNode) {
				// cross nodes
				for (TPair<URoadNode*, float> pair : crossNode->adjacentNodes) {
					URoadNode* connNode = ((URoadNodePort*)pair.Key)->getConnectedPort();
					if (connNode && !spawnVolumeNodes.Contains(connNode)) {
						spawnVolumeNodes.Add(connNode, baseDst);
						nextNodes.Add(connNode);
					}
				}
			}
			else {
				// straight nodes
				for (TPair<URoadNode*, float> pair : node->adjacentNodes) {
					URoadNode* connNode = pair.Key;
					float connDst = pair.Value;
					if (connNode->getNodeType() == RoadNodeType::Port) {
						connNode = ((URoadNodePort*)connNode)->getConnectedPort();
					}
					if (connNode && !spawnVolumeNodes.Contains(connNode)) {
						float dst = baseDst + connDst;
						if (dst < 15000) { // distance threshold
							spawnVolumeNodes.Add(connNode, dst);
							nextNodes.Add(connNode);
						}
					}
				}
			}
		}
	}

	// destroy vehicles outside volume
	if (!isBegin) {
		for (int i = vehicles.Num() - 1; i >= 0; i--) {
			AVehicleBase* vehicle = vehicles[i];
			if (vehicle->prevNode && vehicle->prevNode->getNodeType() == RoadNodeType::Normal) {
				if (!spawnVolumeNodes.Contains(vehicle->prevNode)) {
					//UKismetSystemLibrary::PrintString(GetWorld(), L"Vehicle Destroyed", true, false);
					vehicle->Destroy();
				}
			}
		}
	}

	// collects spawnable nodes (must straight node, and not near any port)
	TArray<URoadNodeNormal*> spawnableNodes;
	for (TPair<URoadNode*, float> pair : spawnVolumeNodes) {
		URoadNode* node = pair.Key;
		if (node->getNodeType() == RoadNodeType::Normal && !node->isNearAnyPort(1000)) {
			if (isBegin || pair.Value > 4000) { // 40m near check
				spawnableNodes.Add((URoadNodeNormal*)node);
			}
		}
	}

	// spawn vehicles inside volume
	if (spawnableNodes.Num()) {
		int maxVehicles = spawnOrigin->getSegment()->computeMaxVehicles(VehicleDensity);
		int numPawns = maxVehicles - vehicles.Num();
		// spawn random vehicles by numPawns
		UWorld* world = GetWorld();
		int i = 0;
		while (i < numPawns) {
			// random spawn node
			URoadNodeNormal* node = spawnableNodes[FMath::RandRange(0, spawnableNodes.Num() - 1)];
			URoadSegment* segment = node->getSegment();
			// random port and lane
			TArray<URoadNodePort*> inPorts = segment->collectEntryPorts();
			URoadNodePort* inPort = inPorts[FMath::RandRange(0, inPorts.Num() - 1)];
			bool invert = inPort->nextIndex < 0;
			if (isBegin || playerVehicle->isSpawnableAt(segment, invert)) {
				int lane = inPort->randomRightLane(100);
				FVector location = node->computeTarget(lane, invert);
				FRotator rotation = node->getHeadVector(invert).ToOrientationRotator();
				TSubclassOf<AVehicleBase> type = VehicleTypes[FMath::RandRange(0, VehicleTypes.Num() - 1)];
				AVehicleBase* vehicle = world->SpawnActor<AVehicleBase>(type, location, rotation);
				if (vehicle) {
					vehicle->RandomizeVehiclePaint();
					RegisterVehicle(vehicle);
					vehicle->onSpawnedInSystem(this);
				}
				i++;
			}
		}
	}
}

//void AUrbanTraffic::recaptureNavigation() {
//
//}