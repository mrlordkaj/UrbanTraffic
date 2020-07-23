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
#include "RoadNodeNormal.h"
#include "RoadNodePort.h"
#include "RoadNodeCross.h"
#include "CoreMinimal.h"

class AUrbanTraffic;

/**
 * 
 */
class URBANTRAFFIC_API URoadSegment
{
public:
	URoadSegment(AUrbanTraffic* manager);

	/* Reset precomp data and free their memory. */
	~URoadSegment();

	/* Compiles runtime data from raw data. */
	void compileData(TArray<URoadNode*> newNodes);

	void drawDebug(UWorld* world, uint8 debugFlags);

	/* Gets cross type node if this is a crossing segment. */
	URoadNodeCross* getCrossNode();

	/* Gets node by index. */
	URoadNode* getNode(int index);

	/* Collects all available nodes in this segment. */
	TArray<URoadNode*> collectNodes(bool invert);

	/* Collects all available ports in this segment. */
	TArray<URoadNodePort*> collectPorts(bool invert);

	/* Collects all entry (can enter from outside) ports in this segment. */
	TArray<URoadNodePort*> collectEntryPorts();

	/* Gets total length of this segment. */
	float getSegmentLength();

	/* Gets length of a point on segment. */
	float getLengthOnSegment(FVector position, bool invert);

	/* Gets max vehicles from density. */
	int computeMaxVehicles(int laneDensity);

private:
	/* UrbanTraffic manager actor. */
	AUrbanTraffic* manager;

	/* Cached data, only set on crossing segment. */
	URoadNodeCross* crossNode = nullptr;

	/* Stores all nodes in this segment. */
	TArray<URoadNode*> nodes;

	/* Cached data, for collect all ports. */
	TArray<URoadNodePort*> ports;

	/* Precomp, total length of segment. */
	float segmentLength = 0;
};
