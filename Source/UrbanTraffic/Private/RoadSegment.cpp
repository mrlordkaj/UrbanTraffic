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

#include "RoadSegment.h"
#include "RoadNode.h"
#include "RoadTurn.h"
#include "DrawDebugHelpers.h"

URoadSegment::URoadSegment(AUrbanTraffic* manager) {
	this->manager = manager;
}

URoadSegment::~URoadSegment() {
	crossNode = nullptr;
	ports.Reset();
	nodes.Empty();
	segmentLength = 0;
}

void URoadSegment::compileData(TArray<URoadNode*> newNodes) {
	// reset previous data
	URoadSegment::~URoadSegment();

	// set and compile new nodes
	nodes.Append(newNodes);
	//TArray<URoadNodeNormal*> normals;
	for (URoadNode* node : nodes) {
		switch (node->getNodeType()) {
		case RoadNodeType::Port:
			ports.Add((URoadNodePort*)node);
			break;

		case RoadNodeType::Cross:
			crossNode = (URoadNodeCross*)node;
			break;

		//case RoadNodeType::Normal:
		//	normals.Add((URoadNodeNormal*)node);
		//	break;
		}
		node->compileData();
		segmentLength += node->getNodeLength(false);
	}

	// adjust speed limit for nodes next to limited nodes.
	const float invMaxDst = 1.0f / 4000.0f;
	for (URoadNode* node : nodes) {
		if (node->speedLimit <= 0.8f) {
			float baseLimit = 1 - node->speedLimit;
			// compute limit for next nodes
			if (node->nextIndex >= 0) {
				float curLimit = node->speedLimit;
				int curId = node->nextIndex;
				URoadNode* curNode = node;
				while (curId < nodes.Num() && curLimit < 1) {
					curLimit += curNode->getNodeLength(false) * invMaxDst * baseLimit;
					curNode = nodes[curId++];
					if (curLimit < curNode->speedLimit) {
						curNode->speedLimit = curLimit;
					}
				}
			}
			// compute limit for previous nodes
			{
				float curLimit = node->speedLimit;
				int curId = (node->nextIndex < 0 ? nodes.Num() : node->nextIndex) - 2;
				URoadNode* curNode = node;
				while (curId >= 0 && curLimit < 1) {
					curLimit += curNode->getNodeLength(true) * invMaxDst * baseLimit;
					curNode = nodes[curId--];
					if (curLimit < curNode->speedLimit) {
						curNode->speedLimit = curLimit;
					}
				}
			}
		}
	}

	// calculate length on segment for nodes
	float lengthFromStart = 0;
	for (URoadNode* node : nodes) {
		node->distanceToStart = lengthFromStart;
		node->distanceToEnd = segmentLength - lengthFromStart;
		lengthFromStart += node->getNodeLength(false);
	}
}

void URoadSegment::drawDebug(UWorld* world, uint8 debugFlags) {
	for (URoadNode* node : nodes) {
		node->drawDebug(world, debugFlags);
	}
}

float URoadSegment::getLengthOnSegment(FVector position, bool invert) {
	// find nearest node
	URoadNode* nearestNode = nullptr;
	float minDst = TNumericLimits<float>::Max();
	for (URoadNode* node : nodes) {
		float dst = FVector::Dist(position, node->position);
		if (dst < minDst) {
			nearestNode = node;
			minDst = dst;
		}
	}
	float lengthOnSegment = 0;
	if (nearestNode) {
		float nodeLength = nearestNode->getLengthOnSegment(false);
		FVector nodeDir = position - nearestNode->position;
		float dot = -1;
		// check forward vector first
		if (nearestNode != nodes.Last()) {
			dot = FVector::DotProduct(nodeDir, nearestNode->getHeadVector(false));
			lengthOnSegment = nodeLength + dot;
		}
		// if negative, check backward vector
		if (dot < 0 && nearestNode != nodes[0]) {
			dot = FVector::DotProduct(nodeDir, nearestNode->getHeadVector(true));
			lengthOnSegment = nodeLength - dot;
		}
		// still negative? then use node's lengthOnSegment as result
		if (dot < 0) {
			lengthOnSegment = nodeLength;
		}
	}
	// return length on segment based invert flag
	return invert ? (segmentLength - lengthOnSegment) : lengthOnSegment;
}

float URoadSegment::getSegmentLength() {
	return segmentLength;
}

URoadNodeCross* URoadSegment::getCrossNode() {
	return crossNode;
}

URoadNode* URoadSegment::getNode(int index) {
	return nodes[index];
}

TArray<URoadNode*> URoadSegment::collectNodes(bool invert) {
	TArray<URoadNode*> rs(nodes);
	if (invert) Algo::Reverse(rs);
	return rs;
}

TArray<URoadNodePort*> URoadSegment::collectPorts(bool invert) {
	TArray<URoadNodePort*> rs(ports);
	if (invert) Algo::Reverse(rs);
	return rs;
}

TArray<URoadNodePort*> URoadSegment::collectEntryPorts() {
	TArray<URoadNodePort*> rs;
	for (URoadNodePort* port : ports) {
		if (port->numRights) {
			rs.Add(port);
		}
	}
	return rs;
}

int URoadSegment::computeMaxVehicles(int laneDensity) {
	int totalLanes = 0;
	for (URoadNodePort* port : ports) {
		int lanes = port->numLefts + port->numRights;
		if (lanes > totalLanes) {
			totalLanes = lanes;
		}
	}
	return laneDensity * totalLanes;
}