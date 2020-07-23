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

#include "VehicleObserverComponent.h"
#include "VehicleBase.h"
#include "TrafficLight.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UVehicleObserverComponent::UVehicleObserverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	//bAutoActivate = true;
}

void UVehicleObserverComponent::BeginPlay()
{
	Super::BeginPlay();

	vehicle = Cast<AVehicleBase>(GetOwner());
	if (vehicle) {
		UBoxComponent* bound = vehicle->Bounding;
		USkeletalMeshComponent* mesh = vehicle->GetMesh();
		bound->OnComponentBeginOverlap.AddDynamic(this, &UVehicleObserverComponent::BeginOverlap);
		bound->OnComponentEndOverlap.AddDynamic(this, &UVehicleObserverComponent::EndOverlap);
		mesh->OnComponentHit.AddDynamic(this, &UVehicleObserverComponent::OnComponentHit);
	}
}

void UVehicleObserverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//if (vehicle) {
	//	float lane = vehicle->GetAbsoluteLane();
	//	// FIXME: find correct entry port
	//	URoadNodePort* port = (URoadNodePort*)vehicle->prevNode->getSegment()->getNode(0);
	//	if (lane < -0.5f && port->numLefts == 0) {

	//	}
	//}

	// fire control
	if (vehicle && fireComponent) {
		float fireScale = fireComponent->RelativeScale3D.X;
		if (fireScale < FireScale) {
			fireScale += (DeltaTime * 0.6f);
			fireComponent->SetRelativeScale3D(FVector(fireScale));
		}
		if (fireHeath > 0) {
			fireHeath -= (DeltaTime * fireDamage);
			//UE_LOG(LogTemp, Warning, TEXT("%f"), vehicleHeath);
			if (fireHeath <= 0 && ExplosionSound) {
				FVector position = vehicle->GetActorLocation();
				UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, position, 3);
				GiveExplosion(position + FVector(0, 0, -1), 120);
			}
		}
	}
}

void UVehicleObserverComponent::OnComponentHit(UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit) {

	// TODO: Hit by another vehicle
	if (OtherActor != vehicle && VehicleParts.Contains(Hit.BoneName)) {
		float damage = FVector::Dist(Hit.TraceEnd, Hit.TraceStart) / 100;
		GiveDamage(Hit.BoneName, damage);
		//UE_LOG(LogTemp, Warning, TEXT("Hit %s (%f)"), *Hit.BoneName.ToString(), damage);
	}

	//if (OtherActor != vehicle && VehicleParts.Contains(Hit.BoneName)) {
	//	FVehiclePartHealth* partInfo = &VehicleParts[Hit.BoneName];
	//	AVehicleBase* otherVehicle = Cast<AVehicleBase>(OtherActor);
	//	if (otherVehicle) {
	//		float damage = otherVehicle->GetVelocity().Size() / 100;
	//		GiveDamage(Hit.BoneName, damage * partInfo->DamageMultiplier);
	//		UE_LOG(LogTemp, Warning, TEXT("Hit By Another %s (%f)"), *Hit.BoneName.ToString(), damage);
	//	}
	//	else {
	//		float damage = FVector::Dist(Hit.TraceEnd, Hit.TraceStart) / 100;
	//		GiveDamage(Hit.BoneName, damage * partInfo->DamageMultiplier);
	//		UE_LOG(LogTemp, Warning, TEXT("Hit %s (%f)"), *Hit.BoneName.ToString(), damage);
	//	}
	//}
}

void UVehicleObserverComponent::BeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult &SweepResult) {
	if (vehicle && FMath::Abs(vehicle->GetVehicleMovement()->GetForwardSpeed()) > 10) {
		if (Cast<AVehicleBase>(OtherActor)) {
			OnVehicleConflict.Broadcast((AVehicleBase*)OtherActor);
		}
		else if (Cast<ACharacter>(OtherActor)) {
			OnCharacterConflict.Broadcast((ACharacter*)OtherActor);
		}
		else if (Cast<ATrafficLight>(OtherActor)) {
			overlapActors.Add(OtherActor, vehicle->GetActorLocation());
		}

		//overlapActors.Add(OtherActor, vehicle->GetActorLocation());
		//AVehicleBase* otherVehicle = Cast<AVehicleBase>(OtherActor);
		//ACharacter* otherCharacter = Cast<ACharacter>(OtherActor);
		//if (otherVehicle) {
		//	OnVehicleConflict.Broadcast(otherVehicle);
		//}
		//else if (otherCharacter) {
		//	USkeletalMeshComponent* mesh = otherCharacter->GetMesh();
		//	mesh->SetCollisionProfileName(TEXT("Ragdoll"));
		//	mesh->SetSimulatePhysics(true);
		//	UCapsuleComponent* capsule = otherCharacter->GetCapsuleComponent();
		//	capsule->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		//	capsule->SetSimulatePhysics(true);
		//	OnCharacterConflict.Broadcast(otherCharacter);
		//}
	}
}

void UVehicleObserverComponent::EndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex) {
	ATrafficLight* trafficLight = Cast<ATrafficLight>(OtherActor);
	if (trafficLight && overlapActors.Contains(OtherActor)) {
		FVector dir = vehicle->GetActorLocation() - overlapActors[OtherActor];
		float dot = FVector::DotProduct(vehicle->GetActorForwardVector(), dir);
		if (dot > vehicle->Bounding->GetScaledBoxExtent().X * 2) {
			OnSignalConflict.Broadcast(trafficLight);
		}
		overlapActors.Remove(OtherActor);
	}

	//if (overlapActors.Contains(OtherActor)) {
	//	ATrafficLight* light = Cast<ATrafficLight>(OtherActor);
	//	if (light) {
	//		FVector dir = vehicle->GetActorLocation() - overlapActors[OtherActor];
	//		float dot = FVector::DotProduct(vehicle->GetActorForwardVector(), dir);
	//		if (dot > vehicle->Bounding->GetScaledBoxExtent().X * 2) {
	//			OnSignalConflict.Broadcast(light);
	//		}
	//	}
	//	overlapActors.Remove(OtherActor);
	//}
}

void UVehicleObserverComponent::GiveFire(float DamagePerSecond) {
	fireDamage = FMath::Max(0.0f, DamagePerSecond);
	if (vehicle && FireTemplate && !fireComponent) {
		USceneComponent* root = vehicle->GetRootComponent();
		// attach and play fire particle
		fireComponent = NewObject<UParticleSystemComponent>(vehicle);
		fireComponent->AttachToComponent(root, FAttachmentTransformRules::KeepRelativeTransform);
		fireComponent->RelativeScale3D = FVector(0.5f);
		fireComponent->SetTemplate(FireTemplate);
		fireComponent->RegisterComponent();
		if (FireSound) {
			// attach and play fire sound
			UAudioComponent* fireAudio = NewObject<UAudioComponent>(vehicle);
			fireAudio->AttachToComponent(root, FAttachmentTransformRules::KeepRelativeTransform);
			fireAudio->SetSound(FireSound);
			fireAudio->RegisterComponent();
			fireAudio->Play();
		}
	}
}

static float calculateExplosionDamage(FVector& impulseVector, float baseDamage) {
	float distance = impulseVector.Size();
	float damageReduce = distance * 0.1f; // each centimeter reduce 0.1 damage
	float damage = FMath::Max(0.0f, baseDamage - damageReduce); // damage always positive
	impulseVector *= (damage / distance);
	return damage;
}

void UVehicleObserverComponent::GiveExplosion(FVector ExplosionLocation, float BaseDamage)
{
	USkeletalMeshComponent* mesh = vehicle->GetMesh();
	FVector chassisImpulse = vehicle->GetActorLocation() - ExplosionLocation;
	// apply damage and impulse for parts
	for (TPair<FName, FVehiclePartHealth> pair : VehicleParts) {
		FVector partImpulse = vehicle->GetSocketRelativeLocation(pair.Key) + chassisImpulse;
		float damage = calculateExplosionDamage(partImpulse, BaseDamage);
		GiveDamage(pair.Key, damage);
		mesh->AddImpulse(partImpulse * 600, pair.Key);
	}
	// apply impulse for chassis
	calculateExplosionDamage(chassisImpulse, BaseDamage);
	mesh->AddImpulse(chassisImpulse * 16000);
}

void UVehicleObserverComponent::GiveDamage(FName PartName, float BaseDamage) {
	USkeletalMeshComponent* mesh = vehicle->GetMesh();
	FVehiclePartHealth* partInfo = &VehicleParts[PartName];
	float prevHeath = partInfo->currentHealth;
	float damage = BaseDamage * partInfo->DamageMultiplier;
	if (prevHeath > 0 && damage > 0.1f) {
		partInfo->currentHealth -= damage;
		//GLog->Log(FString::Printf(L"%s : %f / %f", *Hit.BoneName.ToString(), hitPart->currentHealth, prevHeath));
		if (partInfo->currentHealth <= 0) {
			mesh->SetConstraintProfile(PartName, TEXT("Detached"));
			if (AutoRemoveDelay > 0) {
				FTimerHandle timerHandle;
				FTimerDelegate timerDelegate;
				timerDelegate.BindUFunction(this, FName("RemoveDestroyedPart"), PartName, partInfo->brokenMeshComponent);
				vehicle->GetWorldTimerManager().SetTimer(timerHandle, timerDelegate, 1, false, AutoRemoveDelay);
			}
			//GLog->Log(FString::Printf(L"%s destroyed", *Hit.BoneName.ToString()));
		}
		else if (prevHeath > 90 && partInfo->currentHealth <= 90) {
			if (partInfo->BrokenMesh) {
				// attach broken part
				UStaticMeshComponent* brokenMesh = partInfo->brokenMeshComponent = NewObject<UStaticMeshComponent>(mesh);
				brokenMesh->AttachToComponent(mesh, FAttachmentTransformRules::KeepRelativeTransform, PartName);
				brokenMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				brokenMesh->SetStaticMesh(partInfo->BrokenMesh);
				brokenMesh->SetVectorParameterValueOnMaterials(PAINT_PRIMARY, (FVector)vehicle->VehiclePaint.PrimaryColor);
				brokenMesh->SetVectorParameterValueOnMaterials(PAINT_SECONDARY, (FVector)vehicle->VehiclePaint.SecondaryColor);
				brokenMesh->RegisterComponent();
				// hide original part
				mesh->HideBoneByName(FName(*(PartName.ToString() + "_ok")), EPhysBodyOp::PBO_None);
			}
			// enable "Broken" profile for part
			mesh->SetConstraintProfile(PartName, TEXT("Broken"));
			//GLog->Log(FString::Printf(L"%s broken", *Hit.BoneName.ToString()));
		}
	}
}

void UVehicleObserverComponent::RemoveDestroyedPart(FName PartName, UStaticMeshComponent* BrokenMeshComponent)
{
	vehicle->GetMesh()->HideBoneByName(PartName, EPhysBodyOp::PBO_Term);
	VehicleParts.Remove(PartName);
	if (BrokenMeshComponent) {
		BrokenMeshComponent->DestroyComponent();
	}
}