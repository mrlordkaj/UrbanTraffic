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

#include "PawnSpectatorComponent.h"
#include "GameFramework/Actor.h"

UPawnSpectatorComponent::UPawnSpectatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// create chase spring arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->RelativeLocation = FVector(0, 0, 110);
	SpringArm->RelativeRotation = FRotator(-10, 0, 0);
	SpringArm->TargetArmLength = 800;
	SpringArm->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	// create chase camera
	ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CharseCamera"));
	ChaseCamera->AttachToComponent(SpringArm, FAttachmentTransformRules::KeepRelativeTransform);
	cameras.Add(ChaseCamera);
}

//void UPawnSpectatorComponent::BeginPlay() {
//	Super::BeginPlay();
//	//TArray<USceneComponent*> children;
//	//GetChildrenComponents(false, children);
//	//UE_LOG(LogTemp, Warning, TEXT("%d"), children.Num());
//}

void UPawnSpectatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) {
	AActor* owner = GetOwner();
	if (owner) {
		float y = owner->GetInputAxisValue(IA_CAMERA_UP);
		float x = owner->GetInputAxisValue(IA_CAMERA_RIGHT);
		if (!cameraIndex) {
			// update camera zoom (SpringArm length)
			float w = owner->GetInputAxisValue(IA_CAMERA_ZOOM);
			if (((w > 0 && SpringArm->TargetArmLength < 2000) ||
				(w < 0 && SpringArm->TargetArmLength > 100))) {
				SpringArm->TargetArmLength += w * 100;
			}
			// update SpringArm rotation
			FRotator rot = SpringArm->RelativeRotation;
			rot.Pitch = FMath::ClampAngle(rot.Pitch - y, -89, 89);
			rot.Yaw += x;
			SpringArm->SetRelativeRotation(rot);
		}
		else {
			// update active camera rotation
			UCameraComponent* cam = cameras[cameraIndex];
			if (cam->ComponentHasTag(TEXT("Rotatable"))) {
				FRotator rot = cam->RelativeRotation;
				rot.Pitch = FMath::ClampAngle(rot.Pitch - y, -89, 89);
				rot.Yaw += x;
				cam->SetRelativeRotation(rot);
			}
		}
	}
}

void UPawnSpectatorComponent::SetSpectatorCamera(int cameraId) {
	cameras[cameraIndex]->SetActive(false);
	cameraIndex = FMath::Clamp(cameraId, 0, cameras.Num() - 1);
	cameras[cameraIndex]->SetActive(true);
}

void UPawnSpectatorComponent::ToggleSpectatorCamera() {
	int index = cameraIndex + 1;
	if (index >= cameras.Num()) {
		index = 0;
	}
	SetSpectatorCamera(index);
}

void UPawnSpectatorComponent::AddSpectatorCamera(UCameraComponent* camera, bool autoActive) {
	if (!cameras.Contains(camera)) {
		cameras.Add(camera);
		if (autoActive) {
			SetSpectatorCamera(cameras.Num() - 1);
		}
		else {
			camera->bAutoActivate = false;
		}
	}
}

void UPawnSpectatorComponent::RemoveSpectatorCamera(UCameraComponent* camera) {
	cameras.Remove(camera);
}

void UPawnSpectatorComponent::ClearSpectatorCameras() {
	while (cameras.Num() > 1) {
		cameras.RemoveAt(1);
	}
}