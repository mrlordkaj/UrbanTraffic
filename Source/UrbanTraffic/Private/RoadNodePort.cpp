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

#include "RoadNodePort.h"
#include "RoadNodeCross.h"
#include "RoadSegment.h"
#include "RoadTurn.h"
#include "DrawDebugHelpers.h"

URoadNodePort::URoadNodePort(URoadSegment* segment, int nextIndex, FVector position, float laneWidth, int numRights, int numLefts) :
	URoadNode(segment, nextIndex, position) {
	this->laneWidth = laneWidth;
	this->numRights = numRights;
	this->numLefts = numLefts;
	this->speedLimit = 0.8f;
}

void URoadNodePort::compileData() {
	URoadNode::compileData();
	// first port have no backward, last port have no forward
	// so we need generate pseudo data for them
	if (nextIndex < 0) {
		// last port
		forward = -backward;
		right = -left;
	}
	else {
		// first port
		backward = -forward;
		left = -right;
	}
	// precomp min/max lanes
	minRight = numLefts ? 0 : -(numRights >> 1);
	maxRight = minRight + numRights - 1;
	minLeft = numRights ? 0 : -(numLefts >> 1);
	maxLeft = minLeft + numLefts - 1;
}

void URoadNodePort::drawDebug(UWorld* world, uint8 debugFlags) {
	URoadNode::drawDebug(world, debugFlags);
	if (debugFlags & DEBUG_ROAD_PATH) {
		DrawDebugPoint(world, position, 6, FColor::Red, true);
		// draw lanes
		bool invert = nextIndex < 0;
		FVector head = invert ? backward : forward;
		FVector arm = invert ? left : right;
		// right lanes are placed in the right,
		// same direction with origin
		for (int i = minRight; i <= maxRight; i++) {
			float laneFactor = (i + 0.5f);
			FVector start = arm * laneFactor * laneWidth + position;
			FVector end = start + head * 400;
			DrawDebugDirectionalArrow(world, start, end, 4000, FColor::Green, true);
		}
		// left lanes are placed int the left,
		// reversed direction with origin
		for (int i = minLeft; i <= maxLeft; i++) {
			float laneFactor = (i + 0.5f);
			FVector start = arm * -laneFactor * laneWidth + position;
			FVector end = start - head * 400;
			start.Z += 10;
			end.Z += 10;
			DrawDebugDirectionalArrow(world, start, end, 4000, FColor::Red, true);
		}
	}
}

FVector URoadNodePort::computeTarget(int lane, bool invert) {
	FVector armVector = invert ? left : right;
	float laneFactor = (lane + 0.5f) * laneWidth;
	return armVector * laneFactor + position;
}

RoadTurnType URoadNodePort::appendRoadNodes(TArray<URoadNode*> &roadNodes) {
	URoadNodeCross* crossNode = segment->getCrossNode();
	if (crossNode) {
		TArray<URoadTurn*> turns = crossNode->getPortTurns(this);
		int rand = FMath::RandRange(0, turns.Num() - 1);
		URoadTurn* turnData = turns[rand];
		roadNodes.Append(turnData->collectNodes());
		return turnData->getTurnType();
	}
	else {
		bool invert = nextIndex < 0;
		roadNodes.Append(segment->collectNodes(invert));
		return RoadTurnType::None;
	}
}

int URoadNodePort::randomRightLane(int vehicleLane) {
	if (vehicleLane < minRight || vehicleLane > maxRight) {
		// try maintain current lane
		// or random new lane when enter narrower segment
		vehicleLane = FMath::RandRange(minRight, maxRight);
	}
	return vehicleLane;
}

int URoadNodePort::getMinLeft() {
	return minLeft;
}

int URoadNodePort::getMaxLeft() {
	return maxLeft;
}

int URoadNodePort::getMinRight() {
	return minRight;
}

int URoadNodePort::getMaxRight() {
	return maxRight;
}

URoadNodePort* URoadNodePort::getConnectedPort() {
	return connectedPort;
}

void URoadNodePort::connectPort(URoadNodePort* otherPort) {
	connectedPort = otherPort;
	otherPort->connectedPort = this;
	float limit = FMath::Min(speedLimit, otherPort->speedLimit);
	speedLimit = otherPort->speedLimit = limit;
	fixDirectionVectors(otherPort);
	otherPort->fixDirectionVectors(this);
}

void URoadNodePort::fixDirectionVectors(URoadNodePort* otherPort) {
	bool otherInvert = otherPort->nextIndex < 0;
	if (nextIndex < 0) {
		forward = (otherInvert ? otherPort->backward : otherPort->forward);
		right = (otherInvert ? otherPort->left : otherPort->right);
	}
	else {
		backward = (otherInvert ? otherPort->backward : otherPort->forward);
		left = (otherInvert ? otherPort->left : otherPort->right);
	}
}