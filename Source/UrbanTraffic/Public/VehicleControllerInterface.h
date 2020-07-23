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

#include "ObstacleSensorComponent.h"
#include "RoadNode.h"
#include "CoreMinimal.h"
#include "Interface.h"
#include "VehicleControllerInterface.generated.h"

class AUrbanTraffic;
class AVehicleBase;

UINTERFACE(BlueprintType)
class URBANTRAFFIC_API UVehicleControllerInterface : public UInterface
{
	GENERATED_BODY()
};

class URBANTRAFFIC_API IVehicleControllerInterface
{
	GENERATED_BODY()

// ================================================================
// ===                      VEHICLE BINDING                     ===
// ================================================================

public:
	/* Binds autonomous vehicle. */
	void bindVehicle(AVehicleBase* vehicle);

	/* Unbinds autonomous vehicle. */
	void unbindVehicle();

private:
	/* Pointer to autonomous vehicle. */
	AVehicleBase* vehicle;

// ================================================================
// ===                      AUTO CONTROLLER                     ===
// ================================================================

protected:
	/* Simulates player input every frame. */
	void computeDrivingInput(float deltaSeconds);

private:
	/* Sets new destination where our vehicle need to move to. */
	void setNewTarget(FVector newTarget);

	/* The destination in world space where our vehicle need to move to. */
	FVector currentTarget;

	/* The obstacle sensor when move forward. */
	UObstacleSensorComponent* headSensor;

	/* The obstacle sensor when move backward. */
	UObstacleSensorComponent* backSensor;

	/* The obstacle sensor when turn right. */
	UObstacleSensorComponent* rightSensor;

	/* The obstacle sensor when turn left. */
	UObstacleSensorComponent* leftSensor;

	UObstacleSensorComponent* laneSensor;

	/* Vehicle control parameters. */
	float maxSpeed, speedLimit;

	/* When the car begin move backward, keep that direction for at least 1 second.
	This behaviour makes the AI looks like human, and prevents dead position. */
	float forceBackward = -1;

// ================================================================
// ===                      PATH MANAGEMENT                     ===
// ================================================================

public:
	/* Updates target whenever the current is reached. */
	bool updatePathTarget();

	/* Is the vehicle came from reversed direction. */
	bool invertPath;

private:
	/* Switchs to new lane. Called when next target is not equals current target. */
	void switchLane(int targetLane, float distance = 3000);

	/* Internal check for switch to left lane. */
	bool canSwitchLeft();

	/* Internal check for switch to right lane. */
	bool canSwitchRight();

	/* Scheduled path nodes to be followed. */
	TArray<URoadNode*> roadNodes;

	URoadNodePort* inPort;

	/* The current exiting port. */
	URoadNodePort* endPort;

	/* Next node to be travelled. */
	URoadNode* nextNode;

	/* Remember current selected lane. */
	int currentLane;

	bool switchingLane;
	
// ================================================================
// ===                         DEBUGGING                        ===
// ================================================================

public:
	/* Appends auto controller info to debug string. */
	void appendDebugString(FString &string);
};