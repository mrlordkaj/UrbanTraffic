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

#include "VehicleControllerInterface.h"
#include "VehicleBase.h"
#include "WheeledVehicleMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

// ================================================================
// ===                      VEHICLE BINDING                     ===
// ================================================================

void IVehicleControllerInterface::bindVehicle(AVehicleBase* target) {
	// trigger target reached
	currentTarget = target->GetActorLocation();
	
	// initial path data
	//prevNode = trafficManager->findNearestRoadNode(GetActorLocation());
	currentLane = FMath::RoundToInt(target->GetAbsoluteLane()); // this also generate prevNode
	invertPath = FVector::DotProduct(target->prevNode->getHeadVector(true), target->GetActorForwardVector()) > 0;
	roadNodes = target->prevNode->getSegment()->collectNodes(invertPath);
	inPort = (URoadNodePort*)roadNodes[0];
	// skip unused nodes
	while (roadNodes[0] != target->prevNode) {
		roadNodes.RemoveAt(0);
	}
	nextNode = target->prevNode;
	// take the last node as end port
	endPort = (URoadNodePort*)roadNodes.Last();
	roadNodes.Remove(endPort);

	USceneComponent* root = target->GetRootComponent();
	FVector location = target->Bounding->RelativeLocation;
	FVector extend = target->Bounding->GetScaledBoxExtent();
	// create head sensor
	headSensor = NewObject<UObstacleSensorComponent>(target);
	headSensor->RelativeLocation = FVector(location.X + extend.X - 65, 0, 120);
	headSensor->RayQuantity = 3;
	headSensor->TraceAngle = 10;
	headSensor->TraceWidth = 40;
	headSensor->TraceLength = FVector2D(20, 1400);
	headSensor->AttachToComponent(root, FAttachmentTransformRules::KeepRelativeTransform);
	headSensor->RegisterComponent();
	// create back sensor
	backSensor = NewObject<UObstacleSensorComponent>(target);
	backSensor->RelativeLocation = FVector(location.X - extend.X + 25, 0, 120);
	backSensor->RelativeRotation = FRotator(0, 180, 0);
	backSensor->RayQuantity = 3;
	backSensor->TraceWidth = 160;
	backSensor->TraceLength = FVector2D(0, 180);
	backSensor->AttachToComponent(root, FAttachmentTransformRules::KeepRelativeTransform);
	backSensor->RegisterComponent();
	backSensor->SetActive(false);
	// setup right sensor
	rightSensor = NewObject<UObstacleSensorComponent>(target);
	rightSensor->TraceLength = FVector2D(0, 100);
	rightSensor->NormalizeRange = FVector2D(1, 0);
	rightSensor->RayQuantity = 2;
	rightSensor->TraceWidth = 120;
	rightSensor->RelativeLocation = FVector(location.X + extend.X, extend.Y, 120);
	rightSensor->RelativeRotation = FRotator(0, 90, 0);
	rightSensor->AttachToComponent(root, FAttachmentTransformRules::KeepRelativeTransform);
	rightSensor->RegisterComponent();
	// setup left sensor
	leftSensor = NewObject<UObstacleSensorComponent>(target);
	leftSensor->TraceLength = FVector2D(0, 100);
	leftSensor->NormalizeRange = FVector2D(1, 0);
	leftSensor->RayQuantity = 2;
	leftSensor->TraceWidth = 120;
	leftSensor->RelativeLocation = FVector(location.X + extend.X, -extend.Y, 120);
	leftSensor->RelativeRotation = FRotator(0, -90, 0);
	leftSensor->AttachToComponent(root, FAttachmentTransformRules::KeepRelativeTransform);
	leftSensor->RegisterComponent();
	// setup lane switch sensor
	laneSensor = NewObject<UObstacleSensorComponent>(target);
	laneSensor->TraceLength = FVector2D(0, 2000);
	laneSensor->RelativeLocation = headSensor->RelativeLocation;
	laneSensor->AttachToComponent(root, FAttachmentTransformRules::KeepRelativeTransform);
	laneSensor->RegisterComponent();
	laneSensor->SetActive(false);
	
	// draw sensor debug
	if (target->DrawObstacleSensors) {
		headSensor->bVisible = true;
		//backSensor->bVisible = true;
		rightSensor->bVisible = true;
		leftSensor->bVisible = true;
		//laneSensor->bVisible = true;
	}

	// assign vehicle pointer
	vehicle = target;
}

void IVehicleControllerInterface::unbindVehicle() {
	if (vehicle) {
		vehicle = nullptr;
		headSensor->DestroyComponent();
		backSensor->DestroyComponent();
		rightSensor->DestroyComponent();
		leftSensor->DestroyComponent();
		laneSensor->DestroyComponent();
	}
}

// ================================================================
// ===                      AUTO CONTROLLER                     ===
// ================================================================

void IVehicleControllerInterface::computeDrivingInput(float deltaSeconds) {
	if (vehicle) {
		bool targetReached = FVector::Dist(vehicle->GetActorLocation(), currentTarget) < 300;
		if (targetReached) {
			// update target when current target reached
			if (updatePathTarget()) {
				vehicle->SetThrottle(0);
				vehicle->SetSteering(0);
				vehicle->SetHandBrake(false);
			}
		}
		else {
			if (!switchingLane && !nextNode->getSegment()->getCrossNode()
				&& vehicle->GetSideLightState() == SideLightState::None
				&& headSensor->IsObstacleDetected()) {
				float remainLength = nextNode->getLengthOnSegment(!invertPath);
				if (remainLength > 1400) {
					if (canSwitchRight()) {
						float targetDst = FMath::Min(remainLength, laneSensor->GetNearestDistance());
						switchLane(currentLane + 1, targetDst);
						return;
					}
					else if (canSwitchLeft()) {
						float targetDst = FMath::Min(remainLength, laneSensor->GetNearestDistance());
						switchLane(currentLane - 1, targetDst);
						return;
					}
				}
			}

			float throttle, steering;

			// find direction to target
			float targetLen;
			FVector targetDir;
			FVector targetVec = currentTarget - vehicle->GetActorLocation();
			targetVec.ToDirectionAndLength(targetDir, targetLen);
			float cosForward = FVector::DotProduct(vehicle->GetActorForwardVector(), targetDir);
			float cosRight = FVector::DotProduct(vehicle->GetActorRightVector(), targetDir);
			
			// speed limit by collision detectors or nodes
			speedLimit = headSensor->GetNormalizedValue();
			if (nextNode && nextNode->speedLimit < speedLimit) {
				speedLimit = nextNode->speedLimit;
			}
			maxSpeed = vehicle->MaxSpeedLimit * speedLimit * cosForward;

			// hand brake
			bool needBrake = (vehicle->Speed > 0.2f && speedLimit < 0.24f) ||
				(vehicle->Speed < -2 && backSensor->DoCollisionTest());

			// steering
			if (rightSensor->IsObstacleDetected()) {
				float sensorWeight = rightSensor->GetNormalizedValue();
				steering = sensorWeight * -0.2f;
				maxSpeed *= (1 - sensorWeight);
			}
			else if (leftSensor->IsObstacleDetected()) {
				float sensorWeight = leftSensor->GetNormalizedValue();
				steering = sensorWeight * 0.2f;
				maxSpeed *= (1 - sensorWeight);
			}
			else if (vehicle->Speed < -1) {
				// steering when backward
				steering = (cosRight < 0) ? 1 : -1;
			}
			else if (vehicle->Speed > 1) {
				// steering when forward: (90 - angle) / 90;
				steering = 1 - (FMath::Acos(cosRight) / 1.571f);
				float traceAngleScale = 1 - FMath::Abs(steering);
				headSensor->TraceWidth = traceAngleScale * 40;
				headSensor->TraceAngle = traceAngleScale * 10;
				headSensor->RelativeLocation.Y = steering * 80;
				headSensor->RelativeRotation.Yaw = steering * 90;
			}
			else {
				// no move, no steering
				steering = 0;
			}

			// throttle
			float deltaSpeed = maxSpeed - vehicle->Speed;
			if (forceBackward >= 0) {
				// force backward
				throttle = -0.5f;
				forceBackward -= deltaSeconds;
			}
			else if (cosForward < 0.342f) {
				// bad direction > 70deg
				throttle = -0.5f;
				if (forceBackward < 0) {
					forceBackward = 1.2f; // secs at least
				}
			}
			else if (deltaSpeed > 0.2f && speedLimit > 0.24f && !needBrake) {
				throttle = FMath::Min(deltaSpeed, 1.0f) * 0.68f;
			}
			else if (deltaSpeed < -0.2f && !needBrake) {
				throttle = FMath::Max(-1.0f, deltaSpeed) * 0.68f;
			}
			else {
				throttle = 0;
			}

			vehicle->SetThrottle(throttle);
			vehicle->SetSteering(steering);
			vehicle->SetHandBrake(needBrake);
		}
	}
}

// ================================================================
// ===                      PATH MANAGEMENT                     ===
// ================================================================

bool IVehicleControllerInterface::updatePathTarget() {
	int nextLane = currentLane;
	URoadNodePort* startPort = endPort->getConnectedPort();
	if (startPort) {
		// append nodes when the remaining lengthOnSegment going too short (always straight segment)
		URoadSegment* curSegment = nextNode->getSegment();
		if (!curSegment->getCrossNode()) {
			float lengthToEnd = nextNode->getLengthOnSegment(!invertPath);
			if (lengthToEnd < 6000 && endPort->getSegment() == curSegment) {
				//DrawDebugPoint(GetWorld(), GetActorLocation() + FVector(0, 0, 20), 6, FColor::Red, true);
				switch (startPort->appendRoadNodes(roadNodes)) {
				case RoadTurnType::Left:
					nextLane = startPort->getMinRight();
					vehicle->SetSideLightState(SideLightState::Left);
					break;
				case RoadTurnType::Right:
					nextLane = startPort->getMaxRight();
					vehicle->SetSideLightState(SideLightState::Right);
					break;
				}
				endPort = (URoadNodePort*)roadNodes.Last();
				roadNodes.Remove(endPort);
			}
		}
		// append nodes when current nodes running out (usually crossing segment)
		if (!roadNodes.Num()) {
			startPort->appendRoadNodes(roadNodes);
			switch (nextNode->getTurnType()) {
			case RoadTurnType::Left:
				nextLane = startPort->getMinRight();
				break;
			case RoadTurnType::Right:
				nextLane = FMath::Max(endPort->getMaxLeft(), startPort->getMaxRight());
				break;
			case RoadTurnType::None:
				nextLane = startPort->randomRightLane(currentLane);
				break;
			}
			vehicle->SetSideLightState(SideLightState::None);
			currentLane = nextLane;
			endPort = (URoadNodePort*)roadNodes.Last();
			roadNodes.Remove(endPort);
			//DrawDebugPoint(GetWorld(), GetActorLocation() + FVector(0, 0, 40), 6, FColor::Blue, true);
		}
	}
	if (roadNodes.Num()) {
		// remember previous node
		vehicle->prevNode = nextNode;
		// when changed segment, it need to re-check invertPath flag
		if (roadNodes[0]->getNodeType() == RoadNodeType::Port) {
			invertPath = roadNodes[0]->nextIndex < 0;
		}
		// if next lane is the left or right, try switch to that lane
		if (nextLane < currentLane && canSwitchLeft()) {
			switchLane(nextLane);
		}
		// if next lane is the right, try switch to that lane
		else if (nextLane > currentLane && canSwitchRight()) {
			switchLane(nextLane);
		}
		// if next lane is the same, or can't switch, consume next node as usual
		else {
			nextNode = roadNodes[0];
			roadNodes.RemoveAt(0);
			inPort = nextNode->getSegment()->collectPorts(invertPath)[0];
			setNewTarget(nextNode->computeTarget(currentLane, invertPath));
			switchingLane = false;
		}
		return true;
	}
	else {
		vehicle->Destroy();
		return false;
	}
}

void IVehicleControllerInterface::switchLane(int targetLane, float forwardDistance) {
	URoadSegment* segment = nextNode->getSegment();
	float currentLength = segment->getLengthOnSegment(vehicle->GetActorLocation(), invertPath);
	float targetLength = FMath::Min(currentLength + forwardDistance, segment->getSegmentLength());

	//while (roadNodes.Num()) {
	//	vehicle->prevNode = nextNode;
	//	nextNode = roadNodes[0];
	//	bool segmentChanged = nextNode->getSegment() != segment;
	//	if (segmentChanged) {
	//		// when segment changed, the next node must be port
	//		nextNode = ((URoadNodePort*)nextNode)->getConnectedPort();
	//	}
	//	roadNodes.RemoveAt(0);
	//	float nextNodeLength = nextNode->getLengthOnSegment(invertPath);
	//	if (nextNodeLength > targetLength || segmentChanged) {
	//		float nodeOffset = targetLength - nextNodeLength;
	//		FVector target = nextNode->computeTarget(targetLane, invertPath) +
	//			vehicle->prevNode->getHeadVector(invertPath, nodeOffset);
	//		setNewTarget(target);
	//		break;
	//	}
	//}
	//inPort = (URoadNodePort*)nextNode->getSegment()->collectNodes(invertPath)[0];

	TArray<URoadNode*> nodes = segment->collectNodes(invertPath);
	inPort = (URoadNodePort*)nodes[0];
	while (nodes.Num()) {
		vehicle->prevNode = nextNode;
		nextNode = nodes[0];
		//bool segmentChanged = nextNode->getSegment() != segment;
		//if (segmentChanged) {
		//	// when segment changed, the next node must be port
		//	nextNode = ((URoadNodePort*)nextNode)->getConnectedPort();
		//}
		nodes.RemoveAt(0);
		roadNodes.Remove(nextNode);
		float nextNodeLength = nextNode->getLengthOnSegment(invertPath);
		if (nextNodeLength > targetLength/* || segmentChanged*/) {
			float nodeOffset = targetLength - nextNodeLength;
			FVector target = nextNode->computeTarget(targetLane, invertPath) +
				vehicle->prevNode->getHeadVector(invertPath, nodeOffset);
			setNewTarget(target);
			break;
		}
	}

	currentLane = targetLane;
	switchingLane = true;
}

bool IVehicleControllerInterface::canSwitchLeft() {
	if (currentLane > inPort->getMinRight()) {
		//laneSensor->RelativeRotation.Yaw = headSensor->RelativeRotation.Yaw;
		laneSensor->RelativeLocation.Y = -500;
		laneSensor->DoCollisionTest();
		return laneSensor->GetNearestDistance() > headSensor->GetNearestDistance() + 400;
	}
	return false;
}

bool IVehicleControllerInterface::canSwitchRight() {
	if (currentLane < inPort->getMaxRight()) {
		//laneSensor->RelativeRotation.Yaw = headSensor->RelativeRotation.Yaw;
		laneSensor->RelativeLocation.Y = 500;
		laneSensor->DoCollisionTest();
		return laneSensor->GetNearestDistance() > headSensor->GetNearestDistance() + 400;
	}
	return false;
}

void IVehicleControllerInterface::setNewTarget(FVector newTarget) {
	if (vehicle->DrawTargetHistory) {
		DrawDebugLine(vehicle->GetWorld(), currentTarget, newTarget, FColor::Cyan, true);
	}
	currentTarget = newTarget;
}

// ================================================================
// ===                         DEBUGGING                        ===
// ================================================================

void IVehicleControllerInterface::appendDebugString(FString &string) {
	string += FString::Printf(L"Limit: %.2f\n", speedLimit)
		+ FString::Printf(L"Lane: %.2f / %d\n", vehicle->GetAbsoluteLane(), currentLane)
		//+ FString::Printf(L"Right: %s\n", canSwitchRight() ? L"true" : L"false")
		//+ FString::Printf(L"Left: %s\n", canSwitchLeft() ? L"true" : L"false")
		//+ FString::Printf(L"Remain: %.0f\n", nextNode->getLengthOnSegment(!invertPath))
		//+ FString::Printf(L"Switch: %s\n", switchingLane ? L"true" : L"false")
		;
}