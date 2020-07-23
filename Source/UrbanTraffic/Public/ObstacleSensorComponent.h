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

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ObstacleSensorComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (UrbanTraffic), meta = (BlueprintSpawnableComponent))
class URBANTRAFFIC_API UObstacleSensorComponent : public USceneComponent
{
	GENERATED_BODY()
	
public:
	UObstacleSensorComponent();
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void SetActive(bool bNewActive, bool bReset = false) override;
	
	/* Checks if any obstacle detected. */
	UFUNCTION(BlueprintCallable)
	bool IsObstacleDetected();

	/* Gets distance to the nearest obstacle if found. */
	UFUNCTION(BlueprintCallable)
	float GetNearestDistance();

	/* Gets normalized factor value. Range from 0 to 1. */
	UFUNCTION(BlueprintCallable)
	float GetNormalizedValue();

	/* Force perform a collision test. */
	UFUNCTION(BlueprintCallable)
	bool DoCollisionTest();

	/* Force perform a collision test with custom length. */
	UFUNCTION(BlueprintCallable)
	bool DoCustomCollisionTest(float CustomTraceLength, float &ObstacleDistance);

	/* Number of testing rays. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = "1", UIMax = "11", ClampMin = "1", ClampMax = "11"))
	int RayQuantity = 1;

	/* Total width of all rays. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = "1", UIMax = "1000"))
	float TraceWidth;

	/* Total angle of all rays. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = "0", UIMax = "180"))
	float TraceAngle;

	/* Base trace length for collision testing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = "0", UIMax = "2000"))
	FVector2D TraceLength = FVector2D(0, 1000);

	/* Base range for calculate normalized value from obstacle distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D NormalizeRange = FVector2D(0, 1);

	/* The collision channel of testing. */
	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECollisionChannel::ECC_PhysicsBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SensorDataOrder;

private:
	bool obstacleDetected;

	float nearestDistance;

	TArray<FHitResult> hitResults;
};
