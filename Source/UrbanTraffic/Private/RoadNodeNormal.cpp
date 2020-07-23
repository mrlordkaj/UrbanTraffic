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

#include "RoadNodeNormal.h"
#include "RoadSegment.h"
#include "DrawDebugHelpers.h"

URoadNodeNormal::URoadNodeNormal(URoadSegment* segment, int nextIndex, FVector position) :
	URoadNode(segment, nextIndex, position) {
}

void URoadNodeNormal::compileData() {
	URoadNode::compileData();
	// set speed limit for node with angle < 135 and 25% of speed limit at minimum
	float dot = FVector::DotProduct(forward, backward); // from 1 to -1
	speedLimit = FMath::GetMappedRangeValueClamped(FVector2D(-0.70710678118f, -1), FVector2D(0.25f, 1), dot);
}

void URoadNodeNormal::drawDebug(UWorld* world, uint8 debugFlags) {
	URoadNode::drawDebug(world, debugFlags);
	if (debugFlags & DEBUG_ROAD_PATH) {
		DrawDebugPoint(world, position, 6, FColor::Yellow, true);
	}
}

FVector URoadNodeNormal::computeTarget(int lane, bool invert) {
	FVector armVector = invert ? left : right;
	float laneWidth = segment->collectEntryPorts()[0]->laneWidth;
	float laneFactor = (lane + 0.5f) * laneWidth;
	return armVector * laneFactor + position;
}

//FTransform URoadNodeNormal::createSpawnData() {
//	FTransform transform;
//	// random port and lane
//	TArray<URoadNodePort*> inPorts = segment->collectEntryPorts();
//	URoadNodePort* port = inPorts[FMath::RandRange(0, inPorts.Num() - 1)];
//	int lane = port->randomRightLane(100);
//	// compute transform
//	bool invert = port->nextIndex < 0;
//	transform.SetLocation(computeTarget(lane, invert));
//	transform.SetRotation(getHeadVector(invert).ToOrientationQuat());
//	return transform;
//}