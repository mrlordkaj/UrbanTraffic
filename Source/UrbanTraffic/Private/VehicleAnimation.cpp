/*
 * The MIT License
 *
 * Copyright 2020 Thinh Pham.
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

#include "VehicleAnimation.h"
#include "VehicleBase.h"
#include "VehicleAnimationInterface.h"
#include "VehicleAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UVehicleAnimation::UVehicleAnimation()
{
	timeline = NewObject<UTimelineComponent>(GetOuter());
	timeline->SetLooping(false);
	timeline->SetIgnoreTimeDilation(true);
}

void UVehicleAnimation::setupTimeline(FName doorName, UCurveFloat* openCurve)
{
	// check for correct animation class
	AVehicleBase* vehicle = Cast<AVehicleBase>(GetOuter());
	if (vehicle) {
		UAnimInstance* anim = vehicle->GetMesh()->GetAnimInstance();
		if (UKismetSystemLibrary::DoesImplementInterface(anim, UVehicleAnimationInterface::StaticClass())) {
			myDoorName = doorName;
			animInstance = anim;
			// update event
			FOnTimelineFloat interpFunc;
			interpFunc.BindUFunction(this, TEXT("TimelineFloatReturn"));
			timeline->AddInterpFloat(openCurve, interpFunc);
			// finish event
			FOnTimelineEvent finishFunc;
			finishFunc.BindUFunction(this, TEXT("TimelineFinished"));
			timeline->SetTimelineFinishedFunc(finishFunc);
		}
	}
}

void UVehicleAnimation::playTimeline(bool reverse, float duration)
{
	if (animInstance) {
		IVehicleAnimationInterface::Execute_BeginDoorAnimation(animInstance, myDoorName);
		timeline->SetPlayRate(timeline->GetTimelineLength() / duration);
		if (reverse) {
			timeline->ReverseFromEnd();
		}
		else {
			timeline->PlayFromStart();
		}
	}
}

void UVehicleAnimation::TimelineFloatReturn(float value)
{
	IVehicleAnimationInterface::Execute_UpdateDoorAnimation(animInstance, myDoorName, value);
}

void UVehicleAnimation::TimelineFinished()
{
	IVehicleAnimationInterface::Execute_EndDoorAnimation(animInstance, myDoorName);
}
