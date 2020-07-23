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

#include "ObstacleSensorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"

UObstacleSensorComponent::UObstacleSensorComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bTickInEditor = true;
	bAutoActivate = true;
	bVisible = false;
}

void UObstacleSensorComponent::OnRegister() {
	Super::OnRegister();
	// create hit result array
	hitResults.Empty();
	for (int i = 0; i < RayQuantity; i++) {
		hitResults.Add(FHitResult());
	}
}

void UObstacleSensorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) {
	if (IsActive()) {
		DoCollisionTest();
	}
	if (bVisible) {
		UWorld* world = GetWorld();
		for (FHitResult hit : hitResults) {
			FVector end = hit.bBlockingHit ? hit.Location : hit.TraceEnd;
			FLinearColor color = hit.bBlockingHit ? FLinearColor::Red : FLinearColor::Green;
			UKismetSystemLibrary::DrawDebugLine(world, hit.TraceStart, end, color);
		}
	}
}

bool UObstacleSensorComponent::DoCollisionTest() {
	obstacleDetected = DoCustomCollisionTest(TraceLength.Y, nearestDistance);
	return obstacleDetected;
}

bool UObstacleSensorComponent::DoCustomCollisionTest(float CustomTraceLength, float &ObstacleDistance) {
	float hitOffset, hitStep, angleOffset, angleStep;
	if (hitResults.Num() > 1) {
		int numSpaces = hitResults.Num() - 1;
		hitOffset = TraceWidth * -0.5f;
		hitStep = (TraceWidth / numSpaces);
		angleOffset = TraceAngle * -0.5f;
		angleStep = (TraceAngle / numSpaces);
	}
	else {
		hitOffset = hitStep = 0;
		angleOffset = angleStep = 0;
	}
	bool detected = false;
	ObstacleDistance = CustomTraceLength;
	FCollisionQueryParams query(TEXT("Sensor_Trace"), false, GetOwner());
	//FCollisionQueryParams query;
	//query.AddIgnoredActor(GetOwner());
	UWorld* world = GetWorld();
	for (int i = 0; i < hitResults.Num(); i++) {
		FHitResult* hit = &hitResults[i];
		FVector start = GetComponentLocation() + (GetRightVector() * hitOffset);
		FVector end = GetForwardVector().RotateAngleAxis(angleOffset, FVector::UpVector) * CustomTraceLength + start;
		hitOffset += hitStep;
		angleOffset += angleStep;
		if (world->LineTraceSingleByChannel(*hit, start, end, CollisionChannel, query)) {
			detected = true;
			if (hit->Distance < ObstacleDistance) {
				ObstacleDistance = hit->Distance;
			}
		}
	}
	return detected;
}

void UObstacleSensorComponent::SetActive(bool bNewActive, bool bReset) {
	/*if (bNewActive && !IsActive()) {
		DoCollisionTest();
	}
	else */if (!bNewActive) {
		obstacleDetected = false;
		nearestDistance = TraceLength.Y;
	}
	Super::SetActive(bNewActive, bReset);
}

bool UObstacleSensorComponent::IsObstacleDetected() {
	return obstacleDetected;
}

float UObstacleSensorComponent::GetNearestDistance() {
	return nearestDistance;
}

float UObstacleSensorComponent::GetNormalizedValue() {
	return FMath::GetMappedRangeValueClamped(TraceLength, NormalizeRange, nearestDistance);
}