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

#include "CharacterBase.h"
#include "PawnSpectatorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// register overlap listeners
	UCapsuleComponent* capsule = GetCapsuleComponent();
	capsule->OnComponentBeginOverlap.AddDynamic(this, &ACharacterBase::BeginOverlap);
	capsule->OnComponentEndOverlap.AddDynamic(this, &ACharacterBase::EndOverlap);
}

void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* input)
{
	Super::SetupPlayerInputComponent(input);
	input->BindAction(IA_INTERACTIVE, IE_Pressed, this, &ACharacterBase::AttachNearestVehicle);
	// setup input component for spectator
	UActorComponent* spectator = GetComponentByClass(UPawnSpectatorComponent::StaticClass());
	if (spectator) {
		input->BindAxis(IA_CAMERA_UP);
		input->BindAxis(IA_CAMERA_RIGHT);
		input->BindAxis(IA_CAMERA_ZOOM);
	}
}

// ================================================================
// ===                  ATTACH/DETACH VEHICLES                  ===
// ================================================================

void ACharacterBase::AttachNearestVehicle() {
	if (nearVehicles.Num()) {
		// link pointer between vehicle and this character
		MyVehicle = nearVehicles[0];
		MyVehicle->MyDriver = this;
		nearVehicles.Reset();
		// attach this actor to vehicle actor
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AttachToActor(MyVehicle, FAttachmentTransformRules::KeepRelativeTransform);
		SetActorRelativeRotation(FRotator());
		FVector loc = MyVehicle->GetSocketRelativeLocation(BONE_FRONTSEAT, true);
		SetActorRelativeLocation(FVector(loc.X - 30, loc.Y - 80, 100), false, nullptr, ETeleportType::TeleportPhysics);
		// notify blueprint event
		BeginGetInVehicle();
	}
}

void ACharacterBase::DetachCurrentVehicle() {
	if (MyVehicle) {
		FVector loc = MyVehicle->GetSocketRelativeLocation(BONE_FRONTSEAT, true);
		SetActorRelativeLocation(FVector(loc.X - 30, loc.Y - 100, 100), false, nullptr, ETeleportType::TeleportPhysics);
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MyVehicle->GetController()->Possess(this);
		MyVehicle->MyDriver = nullptr;
		MyVehicle = nullptr;
	}
}

void ACharacterBase::BeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult &SweepResult) {
	AVehicleBase* vehicle = Cast<AVehicleBase>(OtherActor);
	if (vehicle && !nearVehicles.Contains(vehicle)) {
		nearVehicles.Add(vehicle);
	}
}

void ACharacterBase::EndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex) {
	AVehicleBase* vehicle = Cast<AVehicleBase>(OtherActor);
	if (vehicle) {
		nearVehicles.Remove(vehicle);
	}
}