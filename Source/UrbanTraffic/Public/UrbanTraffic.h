/*
 * The MIT License
 *
 * Copyright 2019 Thinh Pham.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include "StreetLamp.h"
#include "RoadSegment.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "Modules/ModuleManager.h"
#include "UrbanTraffic.generated.h"

class FUrbanTrafficModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

class AVehicleBase;

UCLASS()
class URBANTRAFFIC_API AUrbanTraffic : public AActor
{
	GENERATED_BODY()

public:
	AUrbanTraffic();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginDestroy() override;

// ================================================================
// ===                     ENVIRONMENT SYSTEM                   ===
// ================================================================

public:
	/* Collects lighting objects such as street lights. */
	UFUNCTION(BlueprintCallable)
	void RebuildLightSystem();

protected:
	/* Updates lighting system based on day/night shifting. */
	UFUNCTION(BlueprintCallable)
	void UpdateLightSystem(float TimerHour, bool ForceUpdate = false);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (UIMin = "0", UIMax = "24", ClampMin = "0", ClampMax = "24"))
	float TimerStart = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (UIMin = "1", UIMax = "2000", ClampMin = "1", ClampMax = "2000"))
	float TimerSpeed = 100;

private:
	/* Collection of light components. */
	TArray<USceneComponent*> timedLights;

	/* Lighting system state: true at night, false at day. */
	bool lightState;

// ================================================================
// ===                       VEHICLE SYSTEM                     ===
// ================================================================

public:
	/* Register managed vehicle. */
	UFUNCTION(BlueprintCallable)
	void RegisterVehicle(AVehicleBase* Vehicle);

	/* Unregister managed vehicle. */
	UFUNCTION(BlueprintCallable)
	void UnregisterVehicle(AVehicleBase* Vehicle);

	/* Finds nearest road node to a position. */
	URoadNode* findNearestRoadNode(FVector position);

private:
	/* Vehicle road data files. */
	UPROPERTY(EditAnywhere, Category = "Vehicle")
	TArray<FString> RoadFiles;

	/* List of all AI Vehicle classes spawned randomly. */
	UPROPERTY(EditAnywhere, Category = "Vehicle")
	TArray<TSubclassOf<AVehicleBase>> VehicleTypes;

	/* Maximum vehicles can run on the map. */
	UPROPERTY(EditAnywhere, Category = "Vehicle", meta = (UIMin = "0", UIMax = "10", ClampMin = "0", ClampMax = "10"))
	int VehicleDensity = 5;

	/* Cleans previous compiled road path system. */
	void cleanRoadSystem();

	/* Builds road path system from data file. */
	void buildRoadSystem();

	/* Update spawn volume at specified node. */
	void updateVehicleSpawnVolume(bool isBegin);

	/* Stores all computer controlled vehicles. */
	TArray<AVehicleBase*> vehicles;

	/* Stores all road segments. */
	TArray<URoadSegment*> roadSegments;

	/* Cached collection vehicle ports. */
	TArray<URoadNodePort*> roadPorts;

	/* Cached collection of vehicle nodes. */
	TArray<URoadNode*> roadNodes;

	/* Only update spawn nodes when origin changed. */
	URoadNode* spawnOrigin;

	/* Cached data for collecting spawn volume nodes and distance from them to origin node. */
	TMap<URoadNode*, float> spawnVolumeNodes;

// ================================================================
// ===                        DEMONSTATION                      ===
// ================================================================

//public:
//	UPROPERTY(EditAnywhere, BlueprintReadOnly)
//	APawn* PlayerPawn;
//
private:
	/* Recursively call before all preload levels is loaded. */
	UFUNCTION()
	void ConsumePreloadLevels();

	UPROPERTY(EditAnywhere)
	TArray<FName> PreloadLevels;

	UPROPERTY(EditAnywhere, Category = "Demonstration")
	bool PossessPlayerAI;
	
// ================================================================
// ===                         DEBUGGING                        ===
// ================================================================

private:
	/* Display vehicle road paths. */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Debug")
	bool DrawRoadPaths;

	/* Display guide nodes of vehicle crossroad. */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Debug")
	bool DrawRoadCrosses;

	/* Display speed limit of vehicle road nodes. */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Debug")
	bool DrawRoadLimits;

	/* Display speed limit of vehicle road nodes. */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Debug")
	bool DrawRoadVectors;

	//UFUNCTION(BlueprintCallable, CallInEditor)
	//void recaptureNavigation();
};