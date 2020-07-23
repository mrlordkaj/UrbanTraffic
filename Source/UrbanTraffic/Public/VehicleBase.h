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

#include "VehicleHardwareComponent.h"

#include "VehicleAnimation.h"
#include "UrbanVehicleAI.h"
#include "RoadSegment.h"
#include "CoreMinimal.h"
#include "WheeledVehicle.h"
#include "WheeledVehicleMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "VehicleBase.generated.h"

#define PAINT_PRIMARY	"PrimaryPaint"
#define PAINT_SECONDARY	"SecondaryPaint"

#define BONE_CHASSIS	"chassis"
#define BONE_WHEEL_LF	"wheel_lf"
#define BONE_WHEEL_RF	"wheel_rf"
#define BONE_WHEEL_LB	"wheel_lb"
#define BONE_WHEEL_RB	"wheel_rb"
#define BONE_FRONTSEAT	"ped_frontseat"

class AUrbanTraffic;
class ACharacterBase;
class UAudioComponent;

USTRUCT(BlueprintType)
struct URBANTRAFFIC_API FVehiclePaint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FLinearColor PrimaryColor = FLinearColor::White;

	UPROPERTY(EditAnywhere)
	FLinearColor SecondaryColor = FLinearColor::Gray;
};

USTRUCT(BlueprintType)
struct URBANTRAFFIC_API FVehicleDoor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UCurveFloat* OpenCurve;

	UPROPERTY()
	UVehicleAnimation* DoorAnimation;
};

UENUM(BlueprintType)
enum class SideLightState : uint8
{
	None,
	Left,
	Right
};

UCLASS(Abstract)
class URBANTRAFFIC_API AVehicleBase : public AWheeledVehicle
{
	GENERATED_BODY()

	DECLARE_DELEGATE_OneParam(FHandBrakeDelegate, bool);
	DECLARE_DELEGATE_OneParam(FSideLightDelegate, SideLightState);

public:
	AVehicleBase();
	virtual void PostInitializeComponents() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual void BeginDestroy() override;
	bool Destroy(bool bNetForce = false, bool bShouldModifyLevel = true);

// ================================================================
// ===                     VEHICLE APPEARANCE                   ===
// ================================================================

public:
	/* Randomize vehicle paint colors. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Appearance")
	void RandomizeVehiclePaint();

	/* Applied vehicle paint colors. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Appearance")
	FVehiclePaint VehiclePaint;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
	UAudioComponent* EngineSound;

protected:
	/* Static mesh for generates wheels. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	UStaticMesh* WheelMesh;

	/* Collection of all available paint colors. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance")
	TArray<FVehiclePaint> PaintSets;

private:
	/* Light component containers. */
	USceneComponent *lights, *brakes, *lefts, *rights;
	
	/* All wheel mesh components. */
	TArray<UStaticMeshComponent*> wheelComponents;

// ================================================================
// ===                      VEHICLE ANIMATION                   ===
// ================================================================

public:
	UFUNCTION(BlueprintCallable)
	FVector GetSocketRelativeLocation(FName SocketName, bool bReverseSide = false);

	UPROPERTY(BlueprintReadOnly)
	ACharacterBase* MyDriver;

protected:
	UFUNCTION(BlueprintCallable)
	void AnimateDoor(FName DoorName, bool bReverse, float Duration = 0.2f);

private:
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FVehicleDoor> VehicleDoors;

// ================================================================
// ===                 TRAFFIC SYSTEM MANAGEMENT                ===
// ================================================================

public:
	/* Finds the point on current segment which nearest to vehicle. */
	UFUNCTION(BlueprintCallable)
	FVector FindNearestPointOnSegment();

	/* Gets distance from vehicle to nearest point on segment. */
	UFUNCTION(BlueprintCallable)
	float GetDistanceToSegment();

	/* Gets absolute lane index. */
	UFUNCTION(BlueprintCallable)
	float GetAbsoluteLane();

	/* Called when vehicle is spawned in the traffic system for demostration. */
	void onPlacedInSystem(AUrbanTraffic* manager);

	/* Called when vehicle is spawned in the traffic system. */
	void onSpawnedInSystem(AUrbanTraffic* manager);

	/* TEMPORARY: Checks if a segment can be spawn a new vehicle. */
	bool isSpawnableAt(URoadSegment* segment, bool invert);

	/* Previous travelled node. */
	URoadNode* prevNode;

private:
	/* Unregister this vehicle, called when destroy. */
	void onDestroyInSystem();

	/* Pointer to the traffic manager. */
	AUrbanTraffic* trafficManager;

// ================================================================
// ===                         VEHICLE AI                       ===
// ================================================================

public:
	/* Max speed can reach in AI controlled mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxSpeedLimit = 50;

	/* Bounding for generates obstacle sensors. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Bounding;

	/* Sets autonomous driving mode. Only valid when bind with VehicleControllerInterface. */
	void setAutoMode(bool isAuto);

	/* Checks whether autonomus driving mode. */
	bool isAutoMode();

private:
	/* Vehicle AI controller. */
	IVehicleControllerInterface* autoController;

	//FVector boundingOffset;

// ================================================================
// ===                      CONTROL DELEGATE                    ===
// ================================================================
	
public:
	/* Toggles auto control mode. */
	UFUNCTION(BlueprintCallable)
	void ToggleAutonomous();

	/* Toggles halogen light state. */
	UFUNCTION(BlueprintCallable)
	void ToggleLight();

	/* Sets throttle value. */
	UFUNCTION(BlueprintCallable)
	void SetThrottle(float Throttle);

	/* Sets steering value. */
	UFUNCTION(BlueprintCallable)
	void SetSteering(float Steering);

	/* Sets halogen light state. */
	UFUNCTION(BlueprintCallable)
	void SetLightState(bool bLightState);

	/* Sets side light state. */
	UFUNCTION(BlueprintCallable)
	void SetSideLightState(SideLightState SideState);

	UFUNCTION(BlueprintCallable)
	SideLightState GetSideLightState();

	/* Sets hand brake state. */
	UFUNCTION(BlueprintCallable)
	void SetHandBrake(bool bHandBrake);

	/* Realtime measurement of speed in km/h. */
	UPROPERTY(BlueprintReadOnly)
	float Speed;

	/* Realtime gear level. */
	UPROPERTY(BlueprintReadOnly)
	int Gear;

	/* Realtime throttle value. */
	UPROPERTY(BlueprintReadOnly)
	float Throttle;

	/* Realtime steering value. */
	UPROPERTY(BlueprintReadOnly)
	float Steering;

	/* Realtime angle of front wheels. */
	UPROPERTY(BlueprintReadOnly)
	float WheelAngle;

private:
	/* Current handbrake state, for changes notify. */
	bool handBrakeState;

	/* Current brake/reverse light state, for changes notify. */
	bool brakeLightState = true;

	/* Current light state, for changes notify. */
	bool lightState = true;

	/* Current sidelight state, for changes notify. */
	SideLightState sideLightState;

	/* Countdown for sidelight blink. */
	float sideLightBlink;

// ================================================================
// ===                      EVENT DELEGRATE                     ===
// ================================================================

public:
	void addHandlingListener(UVehicleHardwareComponent* listener);

private:
	TArray<UVehicleHardwareComponent*> handlingListeners;

// ================================================================
// ===                         DEBUGGING                        ===
// ================================================================

public:
	//UFUNCTION(BlueprintCallable)
	//FString GetSensorDebugString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool DrawDebugString;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool DrawSegmentTrace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool DrawTargetHistory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool DrawObstacleSensors;

private:
	void drawDebugInfo();
};
