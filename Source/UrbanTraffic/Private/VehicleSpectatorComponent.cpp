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

#include "VehicleSpectatorComponent.h"
#include "VehicleBase.h"

UVehicleSpectatorComponent::UVehicleSpectatorComponent() {
	InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
	InternalCamera->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	InternalCamera->ComponentTags.Add("Rotatable");
	AddSpectatorCamera(InternalCamera, false);
}

void UVehicleSpectatorComponent::SetSpectatorCamera(int cameraId) {
	Super::SetSpectatorCamera(cameraId);
	if (cameraId == 1) {
		AVehicleBase* vehicle = Cast<AVehicleBase>(GetOwner());
		USceneComponent* root = vehicle->GetRootComponent();
		// add left planars
		mirrorLeft = NewObject<UPlanarReflectionComponent>(vehicle);
		mirrorLeft->AttachToComponent(root, FAttachmentTransformRules::KeepRelativeTransform, "mirror_l");
		mirrorLeft->RelativeScale3D = FVector(0.004f, 0.008f, 1);
		mirrorLeft->PrefilterRoughness = 0;
		mirrorLeft->DistanceFromPlaneFadeoutEnd = 0;
		mirrorLeft->PrefilterRoughnessDistance = 0;
		mirrorLeft->ScreenPercentage = 50;
		mirrorLeft->ExtraFOV = 20;
		mirrorLeft->RegisterComponent();
		// add right planars
		mirrorRight = NewObject<UPlanarReflectionComponent>(vehicle);
		mirrorRight->AttachToComponent(root, FAttachmentTransformRules::KeepRelativeTransform, "mirror_r");
		mirrorRight->RelativeScale3D = FVector(0.004f, 0.008f, 1);
		mirrorRight->PrefilterRoughness = 0;
		mirrorRight->DistanceFromPlaneFadeoutEnd = 0;
		mirrorRight->PrefilterRoughnessDistance = 0;
		mirrorRight->ScreenPercentage = 30;
		mirrorRight->ExtraFOV = 0;
		mirrorRight->RegisterComponent();
	}
	else {
		// remove reflection planars
		if (mirrorLeft) {
			mirrorLeft->DestroyComponent();
			mirrorLeft = nullptr;
		}
		if (mirrorRight) {
			mirrorRight->DestroyComponent();
			mirrorRight = nullptr;
		}
	}
}