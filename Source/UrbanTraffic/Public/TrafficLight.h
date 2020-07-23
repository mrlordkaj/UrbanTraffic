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
#include "Engine/StaticMeshActor.h"
#include "Components/BoxComponent.h"
#include "TrafficLight.generated.h"

UENUM(BlueprintType)
enum class TrafficLightState : uint8 {
	Red, Yellow, Green, Blink
};

/**
 * 
 */
UCLASS(Abstract)
class URBANTRAFFIC_API ATrafficLight : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	ATrafficLight();
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

protected:
	/* When the master is set, this will follow its signal, standalone mode will be disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SynchronousSignal")
	ATrafficLight* Master = nullptr;

	/* Is this will counter the master light signal, or follow it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SynchronousSignal")
	bool Reversed = false;

	/* Current light state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StandaloneSignal")
	TrafficLightState LightState = TrafficLightState::Red;

	/* The time period which the red lights on. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StandaloneSignal")
	int RedPeriod = 20;

	/* The time period which the yellow lights on. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StandaloneSignal")
	int YellowPeriod = 3;

	/* The time period which the green lights on. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StandaloneSignal")
	int GreenPeriod = 17;

	/* The time period which the yellow lights flash. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StandaloneSignal")
	int BlinkPeriod = 1;

	/* Countdown timer determines when the lights change their state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StandaloneSignal")
	int TickCountdown = 1;

	UPROPERTY(VisibleDefaultsOnly, SimpleDisplay, BlueprintReadOnly)
	UBoxComponent* VehicleBlocker;

private:
	/* Collection of red lights. */
	USceneComponent* red;

	/* Collection of yellow lights. */
	USceneComponent* yellow;

	/* Collection of green lights. */
	USceneComponent* green;

	/* Yellow light on/off state in blink mode. */
	bool blinkState = false;

	/* Precomputed reversed light state. */
	TrafficLightState reversedState;

	/* Sets new light state and adjust relevant properties. */
	void setLightState(TrafficLightState newState, bool setCountdown = true);

	/* Cached slaves which follow this light signal. */
	TArray<ATrafficLight*> slaves;

	/* Update light state of slaves. */
	void updateSlaves();
};
