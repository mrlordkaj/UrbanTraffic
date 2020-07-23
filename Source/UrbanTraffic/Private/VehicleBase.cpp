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

#include "VehicleBase.h"
#include "CharacterBase.h"
#include "UrbanTraffic.h"
#include "RoadNodeGuide.h"
#include "RoadNodePort.h"
#include "RoadNodeNormal.h"
#include "PawnSpectatorComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AVehicleBase::AVehicleBase() {
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	AIControllerClass = AUrbanVehicleAI::StaticClass();
	AutoPossessAI = EAutoPossessAI::Disabled;

	// setup light containers
	lights = CreateDefaultSubobject<USceneComponent>(TEXT("Lights"));
	lights->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	brakes = CreateDefaultSubobject<USceneComponent>(TEXT("Brakes"));
	brakes->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	lefts = CreateDefaultSubobject<USceneComponent>(TEXT("Lefts"));
	lefts->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	rights = CreateDefaultSubobject<USceneComponent>(TEXT("Rights"));
	rights->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// setup bounding for generate sensors
	Bounding = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounding"));
	Bounding->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Bounding->SetCollisionObjectType(ECollisionChannel::ECC_Vehicle);
	Bounding->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Bounding->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Block);
	//Bounding->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Overlap);
	Bounding->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// setup engine sound
	EngineSound = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
	EngineSound->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// setup movement component
	UWheeledVehicleMovementComponent* movement = GetVehicleMovement();
	movement->WheelSetups[0].BoneName = BONE_WHEEL_LF;
	movement->WheelSetups[1].BoneName = BONE_WHEEL_RF;
	movement->WheelSetups[2].BoneName = BONE_WHEEL_LB;
	movement->WheelSetups[3].BoneName = BONE_WHEEL_RB;
	movement->WheelSetups[2].bDisableSteering = true;
	movement->WheelSetups[3].bDisableSteering = true;
}

void AVehicleBase::PostInitializeComponents() {
	Super::PostInitializeComponents();

//	FActorSpawnParameters params;
//	params.Name = FName(*(GetName() + "_Path"));
//	params.Owner = this;
//	followActor = GetWorld()->SpawnActor<AVehicleFollowActor>(params);
//#if WITH_EDITOR
//	followActor->SetActorLabel(GetActorLabel() + "_Path");
//#endif
}

void AVehicleBase::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);

	// apply paint colors
	USkeletalMeshComponent* mesh = GetMesh();
	mesh->SetVectorParameterValueOnMaterials(PAINT_PRIMARY, (FVector)VehiclePaint.PrimaryColor);
	mesh->SetVectorParameterValueOnMaterials(PAINT_SECONDARY, (FVector)VehiclePaint.SecondaryColor);

	// setup wheel meshes
	if (WheelMesh) {
		if (!wheelComponents.Num()) {
			// front left wheel
			UStaticMeshComponent* wheel_lf = NewObject<UStaticMeshComponent>(this);
			wheel_lf->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, BONE_WHEEL_LF);
			wheel_lf->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			wheel_lf->RelativeRotation.Yaw = 180;
			wheel_lf->RegisterComponent();
			wheelComponents.Add(wheel_lf);
			// front right wheel
			UStaticMeshComponent* wheel_rf = NewObject<UStaticMeshComponent>(this);
			wheel_rf->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, BONE_WHEEL_RF);
			wheel_rf->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			wheel_rf->RegisterComponent();
			wheelComponents.Add(wheel_rf);
			// rear left wheel
			UStaticMeshComponent* wheel_lb = NewObject<UStaticMeshComponent>(this);
			wheel_lb->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, BONE_WHEEL_LB);
			wheel_lb->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			wheel_lb->RelativeRotation.Yaw = 180;
			wheel_lb->RegisterComponent();
			wheelComponents.Add(wheel_lb);
			// rear right wheel
			UStaticMeshComponent* wheel_rb = NewObject<UStaticMeshComponent>(this);
			wheel_rb->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, BONE_WHEEL_RB);
			wheel_rb->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			wheel_rb->RegisterComponent();
			wheelComponents.Add(wheel_rb);
		}
		for (UStaticMeshComponent* wheel : wheelComponents) {
			wheel->SetStaticMesh(WheelMesh);
		}
	}
	else {
		while (wheelComponents.Num()) {
			wheelComponents[0]->DestroyComponent();
			wheelComponents.RemoveAt(0);
		}
	}

	// add door animators
	for (TPair<FName, FVehicleDoor> pair : VehicleDoors) {
		FVehicleDoor* door = &VehicleDoors[pair.Key];
		if (!door->DoorAnimation) {
			door->DoorAnimation = NewObject<UVehicleAnimation>(this);
		}
	}
}

void AVehicleBase::BeginPlay() {
	Super::BeginPlay();

	// setup door timelines
	for (TPair<FName, FVehicleDoor> pair : VehicleDoors) {
		FVehicleDoor door = pair.Value;
		door.DoorAnimation->setupTimeline(pair.Key, door.OpenCurve);
	}

	// turn off lights
	SetLightState(false);
	lefts->SetVisibility(false, true);
	rights->SetVisibility(false, true);

	//// remember bounding offset
	//boundingOffset = Bounding->RelativeLocation;
}

void AVehicleBase::BeginDestroy() {
	Super::BeginDestroy();
	onDestroyInSystem();
}

bool AVehicleBase::Destroy(bool bNetForce, bool bShouldModifyLevel) {
	bool rs = Super::Destroy(bNetForce, bShouldModifyLevel);
	onDestroyInSystem();
	return rs;
}

void AVehicleBase::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	UWheeledVehicleMovementComponent* movement = GetVehicleMovement();
	// update front wheel angle
	WheelAngle = movement->Wheels[0]->GetSteerAngle();
	if (WheelAngle > 0) {
		WheelAngle = movement->Wheels[1]->GetSteerAngle();
	}

	//// set bounding to 0.3s of prediction
	//Bounding->RelativeLocation = boundingOffset + FVector(movement->GetForwardSpeed() * 0.3f, 0, 0);

	// convert speed from cm/s to km/h
	Speed = movement->GetForwardSpeed() * 0.036f;

	// notify gear changed
	int curGear = movement->GetCurrentGear();
	if (curGear != Gear) {
		Gear = curGear;
		for (UVehicleHardwareComponent* listener : handlingListeners) {
			listener->OnGearChanged(Gear);
		}
	}

	// update brake/reverse light state
	bool brakeState = handBrakeState || Speed < -4;
	if (brakeState != brakeLightState) {
		brakeLightState = brakeState;
		brakes->SetVisibility(brakeLightState, true);
	}

	// blink side lights
	switch (sideLightState) {
	case SideLightState::Left:
		sideLightBlink -= DeltaSeconds;
		if (sideLightBlink < 0) {
			bool state = !lefts->IsVisible();
			lefts->SetVisibility(state, true);
			sideLightBlink = 0.6f;
			for (UVehicleHardwareComponent* listener : handlingListeners) {
				listener->OnSideLightChanged(false, state);
			}
		}
		break;

	case SideLightState::Right:
		sideLightBlink -= DeltaSeconds;
		if (sideLightBlink < 0) {
			bool state = !rights->IsVisible();
			rights->SetVisibility(state, true);
			sideLightBlink = 0.6f;
			for (UVehicleHardwareComponent* listener : handlingListeners) {
				listener->OnSideLightChanged(state, false);
			}
		}
		break;
	}

	// draw debug string
	drawDebugInfo();
}

// ================================================================
// ===                     VEHICLE APPEARANCE                   ===
// ================================================================

void AVehicleBase::RandomizeVehiclePaint()
{
	if (PaintSets.Num()) {
		USkeletalMeshComponent* mesh = GetMesh();
		VehiclePaint = PaintSets[FMath::RandRange(0, PaintSets.Num() - 1)];
		mesh->SetVectorParameterValueOnMaterials(PAINT_PRIMARY, (FVector)VehiclePaint.PrimaryColor);
		mesh->SetVectorParameterValueOnMaterials(PAINT_SECONDARY, (FVector)VehiclePaint.SecondaryColor);
	}
}

// ================================================================
// ===                      VEHICLE ANIMATION                   ===
// ================================================================

FVector AVehicleBase::GetSocketRelativeLocation(FName SocketName, bool bReverseSide) {
	FTransform transform = GetMesh()->GetSocketTransform(SocketName, ERelativeTransformSpace::RTS_Actor);
	FVector location = transform.GetLocation();
	if (bReverseSide) {
		location.Y = -location.Y;
	}
	return location;
}

void AVehicleBase::AnimateDoor(FName DoorName, bool bReverse, float Duration)
{
	if (VehicleDoors.Contains(DoorName)) {
		FVehicleDoor door = VehicleDoors[DoorName];
		door.DoorAnimation->playTimeline(bReverse, Duration);
	}
}

// ================================================================
// ===                 TRAFFIC SYSTEM MANAGEMENT                ===
// ================================================================

FVector AVehicleBase::FindNearestPointOnSegment() {
	FVector position = GetActorLocation();
	if (trafficManager) {
		// make sure we have correct prevNode
		if (!autoController || !prevNode) {
			prevNode = trafficManager->findNearestRoadNode(position);
		}
		// find point for straight segment only
		if (!prevNode->getSegment()->getCrossNode()) {
			return prevNode->findNearestPointOnVectors(position);
		}
	}
	return position;
}

float AVehicleBase::GetDistanceToSegment() {
	if (trafficManager) {
		FVector position = GetActorLocation();
		// make sure we have correct prevNode
		if (!autoController || !prevNode) {
			prevNode = trafficManager->findNearestRoadNode(position);
		}
		// calculate distance for straight segment only
		if (!prevNode->getSegment()->getCrossNode()) {
			FVector segmentPoint = prevNode->findNearestPointOnVectors(position);
			// check lane direction
			FVector laneVec = position - segmentPoint;
			bool corrent = FVector::DotProduct(GetActorRightVector(), laneVec) >= 0;
			float dst = laneVec.Size();
			return corrent ? dst : -dst;
		}
	}
	return NAN;
}

float AVehicleBase::GetAbsoluteLane() {
	float dst = GetDistanceToSegment();
	if (!FMath::IsNaN(dst)) {
		// FIXME: find correct entry port
		URoadNodePort* port = (URoadNodePort*)prevNode->getSegment()->getNode(0);
		return (dst / port->laneWidth) - 0.5f;
	}
	return NAN;
}

void AVehicleBase::onPlacedInSystem(AUrbanTraffic* manager) {
	if (manager) {
		trafficManager = manager;
		APlayerController* pc = UGameplayStatics::GetPlayerController(this, 0);
		pc->Possess(this);
	}
}

void AVehicleBase::onSpawnedInSystem(AUrbanTraffic* manager) {
	if (manager) {
		trafficManager = manager;
		SpawnDefaultController();
		setAutoMode(true);
	}
}

void AVehicleBase::onDestroyInSystem() {
	if (autoController) {
		autoController->unbindVehicle();
		autoController = nullptr;
	}
	if (trafficManager) {
		trafficManager->UnregisterVehicle(this);
		trafficManager = nullptr;
	}
}

bool AVehicleBase::isSpawnableAt(URoadSegment* segment, bool invert) {
	if (autoController && prevNode->getSegment() == segment) {
		if (autoController->invertPath == invert) {
			return false;
		}
	}
	return true;
}

// ================================================================
// ===                         VEHICLE AI                       ===
// ================================================================

//bool compareInputActionBindings(FInputActionBinding lhs, FInputActionBinding rhs)
//{
//	return lhs.ActionDelegate.GetDelegateForManualSet().GetHandle() == rhs.ActionDelegate.GetDelegateForManualSet().GetHandle() &&
//		lhs.GetActionName() == rhs.GetActionName();
//}

void AVehicleBase::setAutoMode(bool isAuto) {
	if (isAuto) {
		autoController = Cast<IVehicleControllerInterface>(GetController());
		if (autoController) {
			autoController->bindVehicle(this);
		}
	}
	else if (autoController) {
		autoController->unbindVehicle();
		autoController = nullptr;
	}
}

bool AVehicleBase::isAutoMode() {
	return autoController != nullptr;
}

// ================================================================
// ===                      CONTROL DELEGATE                    ===
// ================================================================

void AVehicleBase::SetupPlayerInputComponent(UInputComponent* input) {
	Super::SetupPlayerInputComponent(input);
	input->BindAction(IA_INTERACTIVE, IE_Pressed, MyDriver, &ACharacterBase::BeginGetOutVehicle);
	input->BindAction(IA_AUTONOMOUS, IE_Pressed, this, &AVehicleBase::ToggleAutonomous);
	input->BindAction(IA_FLASHLIGHT, IE_Pressed, this, &AVehicleBase::ToggleLight);
	input->BindAction<FHandBrakeDelegate>(IA_MOVE_BRAKE, IE_Pressed, this, &AVehicleBase::SetHandBrake, true);
	input->BindAction<FHandBrakeDelegate>(IA_MOVE_BRAKE, IE_Released, this, &AVehicleBase::SetHandBrake, false);
	input->BindAction<FSideLightDelegate>(IA_SIDELIGHT_L, IE_Pressed, this, &AVehicleBase::SetSideLightState, SideLightState::Left);
	input->BindAction<FSideLightDelegate>(IA_SIDELIGHT_R, IE_Pressed, this, &AVehicleBase::SetSideLightState, SideLightState::Right);
	input->BindAction<FSideLightDelegate>(IA_SIDELIGHT_O, IE_Pressed, this, &AVehicleBase::SetSideLightState, SideLightState::None);
	input->BindAxis(IA_MOVE_FORWARD, this, &AVehicleBase::SetThrottle);
	input->BindAxis(IA_MOVE_RIGHT, this, &AVehicleBase::SetSteering);
	// setup input component for spectator
	UActorComponent* spec = GetComponentByClass(UPawnSpectatorComponent::StaticClass());
	if (spec) {
		input->BindAction(IA_CAMERA_SWITCH, IE_Pressed, (UPawnSpectatorComponent*)spec, &UPawnSpectatorComponent::ToggleSpectatorCamera);
		input->BindAxis(IA_CAMERA_UP);
		input->BindAxis(IA_CAMERA_RIGHT);
		input->BindAxis(IA_CAMERA_ZOOM);
	}
}

void AVehicleBase::ToggleAutonomous() {
	bool isAutoMode = !autoController;
	setAutoMode(isAutoMode);
}

void AVehicleBase::ToggleLight() {
	SetLightState(!lightState);
}

void AVehicleBase::SetThrottle(float value) {
	GetVehicleMovement()->SetThrottleInput(value);
	Throttle = value;
}

void AVehicleBase::SetSteering(float value) {
	GetVehicleMovement()->SetSteeringInput(value);
	Steering = value;
}

void AVehicleBase::SetLightState(bool state) {
	if (state != lightState) {
		lightState = state;
		lights->SetVisibility(state, true);
		for (UVehicleHardwareComponent* listener : handlingListeners) {
			listener->OnLightChanged(state);
		}
	}
}

void AVehicleBase::SetSideLightState(SideLightState state) {
	if (state != sideLightState) {
		sideLightState = state;
		lefts->SetVisibility(false, true);
		rights->SetVisibility(false, true);
		if (state != SideLightState::None) {
			sideLightBlink = 0;
		}
		else {
			for (UVehicleHardwareComponent* listener : handlingListeners) {
				listener->OnSideLightChanged(false, false);
			}
		}
	}
}

SideLightState AVehicleBase::GetSideLightState() {
	return sideLightState;
}

void AVehicleBase::SetHandBrake(bool state) {
	if (handBrakeState != state) {
		handBrakeState = state;
		GetVehicleMovement()->SetHandbrakeInput(state);
		for (UVehicleHardwareComponent* listener : handlingListeners) {
			listener->OnBrakeChanged(state);
		}
	}
}

// ================================================================
// ===                      EVENT DELEGRATE                     ===
// ================================================================

void AVehicleBase::addHandlingListener(UVehicleHardwareComponent* listener) {
	if (!handlingListeners.Contains(listener)) {
		handlingListeners.Add(listener);
	}
}

// ================================================================
// ===                         DEBUGGING                        ===
// ================================================================

void AVehicleBase::drawDebugInfo() {
	FVector position = GetActorLocation();
	if (DrawDebugString) {
		float lane = GetAbsoluteLane();
		FString strInfo
			= FString::Printf(L"Gear: %d\n", Gear)
			+ FString::Printf(L"Speed: %.1f\n", Speed)
			//+ FString::Printf(L"Throttle: %.2f\n", Throttle)
			//+ FString::Printf(L"Steering: %.2f\n", Steering)
			//+ FString::Printf(L"Wheel: %.2f\n", GetWheelAngle())
			//+ FString::Printf(L"Lane: %.2f\n", GetAbsoluteLane())
			;
		//if (autoController) {
		//	autoController->appendDebugString(strInfo);
		//}
		UKismetSystemLibrary::DrawDebugString(this, position, strInfo);
	}
	if (DrawSegmentTrace) {
		FVector center = FindNearestPointOnSegment();
		center.Z += 10;
		UKismetSystemLibrary::DrawDebugLine(this, center, position, FLinearColor(1, 0, 1));
	}
}

//FString AVehicleBase::GetSensorDebugString() {
//	FString str;
//	TArray<UObstacleSensorComponent*> sensors;
//	GetComponents(sensors);
//	for (UObstacleSensorComponent* sensor : sensors) {
//		FString rs = sensor->IsObstacleDetected() ?
//			FString::Printf(L"%d cm", (int)sensor->GetNearestDistance()) :
//			"undetected";
//		str += FString::Printf(L"%s: %s\n", *sensor->GetName(), *rs);
//	}
//	return str;
//}