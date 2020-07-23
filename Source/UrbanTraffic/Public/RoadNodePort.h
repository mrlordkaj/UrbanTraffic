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

#include "RoadNode.h"
#include "CoreMinimal.h"

/**
 * 
 */
class URBANTRAFFIC_API URoadNodePort : public URoadNode
{
public:
	URoadNodePort(URoadSegment* segment, int nextIndex, FVector position, float laneWidth, int numRights, int numLefts);
	virtual RoadNodeType getNodeType() { return RoadNodeType::Port; }
	virtual void compileData() override;
	virtual void drawDebug(UWorld* world, uint8 debugFlags) override;
	virtual FVector computeTarget(int lane, bool invert) override;

	/* Appends collection of nodes which start from this port. */
	RoadTurnType appendRoadNodes(TArray<URoadNode*> &roadNodes);

	/* Maintains current vehicle lane, or selects new valid right lane. */
	int randomRightLane(int vehicleLane);

	/* Gets min index of left lanes. */
	int getMinLeft();

	/* Gets max index of left lanes. */
	int getMaxLeft();

	/* Gets min index of right lanes. */
	int getMinRight();

	/* Gets max index of right lanes. */
	int getMaxRight();

// ================================================================
// ===                          RAW DATA                        ===
// ================================================================

public:
	/* Absolute lane width, in centimeters. */
	float laneWidth = 500;

	/* Number of left lanes of this node. */
	int numLefts = 0;

	/* Number of right lanes of this node. */
	int numRights = 0;

// ================================================================
// ===                      PRECOMPUTED DATA                    ===
// ================================================================

public:
	/* Connects this port to it's adjacent port. */
	void connectPort(URoadNodePort* otherPort);

	/* Gets connected port of this port. */
	URoadNodePort* getConnectedPort();

private:
	/* Fix direction vectors when connect to another port. */
	void fixDirectionVectors(URoadNodePort* otherPort);

	/* Precomp, for lane selection. */
	int minLeft, maxLeft, minRight, maxRight;

	/* Precomp, connected port comes from other segment. */
	URoadNodePort* connectedPort = nullptr;
};
