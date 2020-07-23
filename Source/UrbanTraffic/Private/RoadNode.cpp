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

#include "RoadNode.h"
#include "RoadNodePort.h"
#include "RoadTurn.h"
#include "RoadSegment.h"
#include "DrawDebugHelpers.h"

URoadNode::URoadNode(URoadSegment* segment, int nextIndex, FVector position) {
	this->segment = segment;
	this->nextIndex = nextIndex;
	this->position = position;
}

URoadNode::~URoadNode() {
	adjacentNodes.Reset();
}

RoadTurnType URoadNode::getTurnType() {
	return RoadTurnType::None;
}

void URoadNode::compileData() {
	// create link and precomp direction
	if (nextIndex >= 0) {
		URoadNode* nextNode = segment->getNode(nextIndex);
		// direction
		forward = nextNode->position - position;
		lengthForward = forward.Size();
		forward.Normalize();
		nextNode->backward = -forward;
		nextNode->lengthBackward = lengthForward;
		// setup internal links
		this->adjacentNodes.Add(nextNode, lengthForward);
		nextNode->adjacentNodes.Add(this, lengthForward);
	}
	right = FVector::CrossProduct(FVector::UpVector, forward);
	left = FVector::CrossProduct(FVector::UpVector, backward);
}

void URoadNode::drawDebug(UWorld* world, uint8 debugFlags) {
	if (debugFlags & DEBUG_ROAD_PATH) {
		// draw arrow link
		if (nextIndex >= 0) {
			URoadNode* nextNode = segment->getNode(nextIndex);
			DrawDebugDirectionalArrow(world, position, nextNode->position, 4000, FColor::Black, true);
		}
	}
	if (debugFlags & DEBUG_ROAD_LIMIT) {
		// draw speed limit
		FVector up = position + FVector::UpVector * speedLimit * 500;
		DrawDebugLine(world, position, up, FColor::White, true);
	}
	if (debugFlags & DEBUG_ROAD_VECTOR) {
		// draw direction vectors
		DrawDebugLine(world, position, position + forward * 200, FColor::Green, true);
		DrawDebugLine(world, position, position + backward * 200, FColor::Red, true);
		DrawDebugLine(world, position, position + right * 200, FColor::Cyan, true);
		DrawDebugLine(world, position, position + left * 200, FColor::Orange, true);
	}
}

FVector URoadNode::computeTarget(int lane, bool invert) {
	return position;
}

URoadSegment* URoadNode::getSegment() {
	return segment;
}

FVector URoadNode::getHeadVector(bool invert, float scale) {
	return (invert ? backward : forward) * scale;
}

FVector URoadNode::getHandVector(bool invert, float scale) {
	return (invert ? left : right) * scale;
}

float URoadNode::getNodeLength(bool invert) {
	return invert ? lengthBackward : lengthForward;
}

float URoadNode::getLengthOnSegment(bool invert) {
	return invert ? distanceToEnd : distanceToStart;
}

bool URoadNode::isNearAnyPort(float threshold) {
	return distanceToStart < threshold || distanceToEnd < threshold;
}

FVector URoadNode::findNearestPointOnVectors(FVector worldLocation) {
	FVector nodeVec = worldLocation - position;
	FVector headDir = forward;
	float dot = FVector::DotProduct(nodeVec, headDir);
	if (dot < 0) {
		headDir = backward;
		dot = FVector::DotProduct(nodeVec, headDir);
	}
	return headDir * dot + position;
}

URoadNode* URoadNode::findNearestNode(FVector position, TArray<URoadNode*> &nodes) {
	URoadNode* nearNode = nullptr;
	float minDst2 = TNumericLimits<float>::Max();
	for (URoadNode* node : nodes) {
		if (FMath::Abs(node->position.Z - position.Z) < 500) { // 5m floor check
			float dst2 = FVector::DistSquaredXY(node->position, position);
			if (dst2 < minDst2) {
				nearNode = node;
				minDst2 = dst2;
			}
		}
	}
	if (!nearNode) {
		// re-run without floor check
		for (URoadNode* node : nodes) {
			float dst2 = FVector::DistSquaredXY(node->position, position);
			if (dst2 < minDst2) {
				nearNode = node;
				minDst2 = dst2;
			}
		}
	}
	return nearNode;
}