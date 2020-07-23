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

//#include "CoreMinimal.h"
#include "RoadNode.generated.h"

#define DEBUG_ROAD_PATH		(1 << 0)
#define DEBUG_ROAD_CROSS	(1 << 1)
#define DEBUG_ROAD_LIMIT	(1 << 2)
#define DEBUG_ROAD_VECTOR	(1 << 3)

class URoadNode;
class URoadNodePort;
class URoadNodeNormal;
class URoadNodeCross;
class URoadNodeGuide;
class URoadSegment;

UENUM(BlueprintType)
enum class RoadNodeType : uint8 {
	Null	UMETA(DisplayName = "Null Node"),
	Port	UMETA(DisplayName = "Port Node"),
	Normal	UMETA(DisplayName = "Normal Node"),
	Cross	UMETA(DisplayName = "Cross Node"),
	Guide	UMETA(DisplayName = "Guide Node")
};

UENUM(BlueprintType)
enum class RoadTurnType : uint8 {
	None	UMETA(DisplayName = "No Turn"),
	Left	UMETA(DisplayName = "Turn Left"),
	Right	UMETA(DisplayName = "Turn Right")
};

/**
 * 
 */
class URBANTRAFFIC_API URoadNode
{
public:
	URoadNode(URoadSegment* segment, int nextIndex, FVector position);
	virtual ~URoadNode();

	/* Determines the type of this node: port, normal, cross, or guide. */
	virtual RoadNodeType getNodeType() { return RoadNodeType::Null; }

	/* Determines turn type of this node: left, right, or straight. */
	virtual RoadTurnType getTurnType();

	/* Compiles raw data, only called when raw data is set completely. */
	virtual void compileData();

	/* Creates persistant visual debug objects. */
	virtual void drawDebug(UWorld* world, uint8 debugFlags);

	/* Computes target position (in world space) based on this node. */
	virtual FVector computeTarget(int lane, bool invert);
	
	/* Gets the segment that holds this node. */
	URoadSegment* getSegment();

	/* Gets prescaled forward/backward vector. */
	FVector getHeadVector(bool invert, float scale = 1.0f);

	/* Gets prescaled right/left vector. */
	FVector getHandVector(bool invert, float scale = 1.0f);

	/* Gets distance from this node to segment's port. */
	float getLengthOnSegment(bool invert);

	/* Gets distance from this node to next node. */
	float getNodeLength(bool invert);

	/* Checks if distance from the node to any of ports is under threshold. */
	bool isNearAnyPort(float threshold);

	FVector findNearestPointOnVectors(FVector position);

	// ==================================
	// =            Raw Data            =
	// ==================================

	/* The index (in local space) of the next node which connected to this node. */
	int nextIndex;

	/* The position of this node (in world space). */
	FVector position;

	/* The speed limiter. Max value 1.0 means no limit. */
	float speedLimit = 1.0f;

	// ==================================
	// =        Precomputed Data        =
	// ==================================

	/* Precomp, the collection of adjacent-distance pairs. */
	TMap<URoadNode*, float> adjacentNodes;

	/* Precomp, distance to start port. */
	float distanceToStart = 0;
	
	/* Precomp, distance to end port. */
	float distanceToEnd = 0;

protected:
	/* Referrences to the parent segment. */
	URoadSegment* segment;

	/* Precomp, node direction vectors. */
	FVector forward, backward, right, left;

	/* Precomp, distance to the next node. */
	float lengthForward = 0;
	
	/* Precomp, distance to the previous node. */
	float lengthBackward = 0;

public:
	/* Finds node in array which nearest a position. */
	static URoadNode* findNearestNode(FVector position, TArray<URoadNode*> &nodes);
};
