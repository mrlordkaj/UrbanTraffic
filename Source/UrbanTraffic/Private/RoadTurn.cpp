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

#include "RoadTurn.h"
#include "RoadNodePort.h"
#include "RoadNodeCross.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

URoadTurn::URoadTurn(URoadNodePort* startPort, URoadNodeCross* crossNode, URoadNodePort* endPort) {
	this->segment = startPort->getSegment();
	this->startPort = startPort;
	this->crossNode = crossNode;
	this->endPort = endPort;

	// compile data immediately
	FRotator inRot = UKismetMathLibrary::FindLookAtRotation(startPort->position, crossNode->position);
	FRotator outRot = UKismetMathLibrary::FindLookAtRotation(crossNode->position, endPort->position);
	float angle = UKismetMathLibrary::NormalizedDeltaRotator(outRot, inRot).Yaw;
	if (FMath::Abs(angle) > 30) {
		bool turnRight = angle > 30; // turn left else
		turnType = turnRight ? RoadTurnType::Right : RoadTurnType::Left;
		float speedLimit = turnRight ? 0.3f : 0.5f;
		int maxCurveLength = turnRight ? 500 : 1000;
		int aLane = turnRight ? startPort->getMaxRight() : startPort->getMinRight();
		int bLane = turnRight ? endPort->getMaxLeft() : endPort->getMinLeft();
		FVector startPoint = startPort->computeTarget(aLane, false);
		FVector endPoint = endPort->computeTarget(bLane, true);
		// find cross point
		FVector crossPoint = crossNode->position +
			endPort->getHeadVector(turnRight, (aLane + 0.5f) * startPort->laneWidth) +
			startPort->getHeadVector(turnRight, (bLane + 0.5f) * endPort->laneWidth);
		// create guide nodes
		if (FVector::Dist(startPoint, endPoint) > 1000) {
			// add start curve point (if needed)
			float aLen = FVector::Dist(startPoint, crossPoint);
			if (aLen > maxCurveLength) {
				FVector position = startPoint + startPort->getHeadVector(false, aLen - maxCurveLength);
				guideNodes.Add(new URoadNodeGuide(segment, position, this, speedLimit));
			}
			// add end curve point (if needed)
			float bLen = FVector::Dist(crossPoint, endPoint);
			if (bLen > maxCurveLength) {
				FVector position = endPoint + endPort->getHeadVector(false, bLen - maxCurveLength);
				guideNodes.Add(new URoadNodeGuide(segment, position, this, speedLimit));
			}
		}
		//else {
		//	FVector position = (startPoint + endPoint) / 2;
		//	guideNodes.Add(new URoadNodeGuide(segment, position, this, speedLimit));
		//}

		// zero offset for guide nodes
		FVector posOffset = startPort->getHandVector(false, -aLane * startPort->laneWidth);
		for (URoadNodeGuide* node : guideNodes) {
			node->position += posOffset;
		}
	}
	else {
		// no turn
		turnType = RoadTurnType::None;
	}
}

URoadTurn::~URoadTurn() {
	guideNodes.Empty();
}

void URoadTurn::drawDebug(UWorld* world, uint8 debugFlags) {
	if (turnType != RoadTurnType::None) {
		bool turnRight = (turnType == RoadTurnType::Right);
		FColor colors[] = { FColor::Black, FColor::Red, FColor::Green };
		FColor color = colors[(int)turnType];
		int aLane = turnRight ? startPort->getMaxRight() : startPort->getMinRight();
		int bLane = turnRight ? endPort->getMaxLeft() : endPort->getMinLeft();
		FVector startPoint = startPort->computeTarget(aLane, false);
		FVector endPoint = endPort->computeTarget(bLane, true);
		if (guideNodes.Num()) {
			DrawDebugLine(world, startPoint, guideNodes[0]->computeTarget(aLane, false), color, true);
			for (int i = 0; i < guideNodes.Num() - 1; i++) {
				FVector from = guideNodes[i]->computeTarget(aLane, false);
				FVector to = guideNodes[i + 1]->computeTarget(aLane, false);
				DrawDebugLine(world, from, to, color, true);
			}
			DrawDebugLine(world, guideNodes.Last()->computeTarget(aLane, false), endPoint, color, true);
		}
		else {
			DrawDebugLine(world, startPoint, endPoint, color, true);
		}
	}
}

TArray<URoadNode*> URoadTurn::collectNodes() {
	TArray<URoadNode*> nodes;
	nodes.Reserve(guideNodes.Num() + 2);
	nodes.Add(startPort);
	nodes.Append(guideNodes);
	nodes.Add(endPort);
	return nodes;
}

RoadTurnType URoadTurn::getTurnType() {
	return turnType;
}

URoadNodePort* URoadTurn::getStartPort() {
	return startPort;
}

URoadNodePort* URoadTurn::getEndPort() {
	return endPort;
}