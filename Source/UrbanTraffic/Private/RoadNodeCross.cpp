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

#include "RoadNodeCross.h"
#include "RoadSegment.h"
#include "RoadTurn.h"
#include "DrawDebugHelpers.h"

URoadNodeCross::URoadNodeCross(URoadSegment* segment, int nextIndex, FVector position) :
	URoadNode(segment, nextIndex, position) {
}

URoadNodeCross::~URoadNodeCross() {
	URoadNode::~URoadNode();
	portTurns.Empty();
}

void URoadNodeCross::compileData() {
	URoadNode::compileData();
	// delete previous turn data
	portTurns.Empty();
	// collect all ports
	TArray<URoadNodePort*> ports;
	for (URoadNode* node : segment->collectNodes(false)) {
		if (node->getNodeType() == RoadNodeType::Port) {
			ports.Add((URoadNodePort*)node);
		}
	}
	// build node turns
	for (URoadNodePort* a : ports) {
		TArray<URoadTurn*> aTurns;
		for (URoadNodePort* b : ports) {
			if (a != b && a->numRights && b->numLefts) {
				aTurns.Add(new URoadTurn(a, this, b));
			}
		}
		portTurns.Add(a, aTurns);
	}
}

void URoadNodeCross::drawDebug(UWorld* world, uint8 debugFlags) {
	URoadNode::drawDebug(world, debugFlags);
	if (debugFlags & DEBUG_ROAD_PATH) {
		DrawDebugPoint(world, position, 6, FColor::Green, true);
	}
	if (debugFlags & DEBUG_ROAD_CROSS) {
		for (TPair<URoadNodePort*, TArray<URoadTurn*>> pair : portTurns) {
			for (URoadTurn* turn : pair.Value) {
				turn->drawDebug(world, 0);
			}
		}
	}
}

TArray<URoadTurn*> URoadNodeCross::getPortTurns(URoadNodePort* port) {
	return portTurns.Contains(port) ?
		portTurns[port] : TArray<URoadTurn*>();
}