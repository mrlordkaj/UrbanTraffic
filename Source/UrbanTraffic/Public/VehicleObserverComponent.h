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

#include "TrafficLight.h"
#include "VehicleBase.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleObserverComponent.generated.h"

class UParticleSystemComponent;
class USoundCue;

USTRUCT(BlueprintType)
struct URBANTRAFFIC_API FVehiclePartHealth
{
	GENERATED_BODY()

	/* The mesh to shown when the part is broken. */
	UPROPERTY(EditAnywhere)
	UStaticMesh* BrokenMesh;

	/* Damage multiplier, higher value easier to destroy. */
	UPROPERTY(EditAnywhere, meta = (UIMin = "0", ClampMin = "0"))
	float DamageMultiplier = 1;

	/* Current health, destroy the part when this value down to zero. */
	float currentHealth = 100;

	UStaticMeshComponent* brokenMeshComponent;
};

UCLASS( ClassGroup=(UrbanTraffic), meta=(BlueprintSpawnableComponent) )
class URBANTRAFFIC_API UVehicleObserverComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSignalConflict, ATrafficLight*, TrafficLight);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleConflict, AVehicleBase*, Vehicle);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterConflict, ACharacter*, Character);

public:
	UVehicleObserverComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void GiveExplosion(FVector ExplosionLocation, float BaseDamage);

	UFUNCTION(BlueprintCallable)
	void GiveDamage(FName PartName, float BaseDamage);

	UPROPERTY(BlueprintAssignable)
	FOnSignalConflict OnSignalConflict;

	UPROPERTY(BlueprintAssignable)
	FOnVehicleConflict OnVehicleConflict;

	UPROPERTY(BlueprintAssignable)
	FOnCharacterConflict OnCharacterConflict;

private:
	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION()
	void RemoveDestroyedPart(FName PartName, UStaticMeshComponent* BrokenMeshComponent);

	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FVehiclePartHealth> VehicleParts;

	UPROPERTY(EditDefaultsOnly)
	float AutoRemoveDelay;
	
	/* Cached pointer for owner vehicle. */
	AVehicleBase* vehicle;

	/* Stores overlapped traffic lights. */
	TMap<AActor*, FVector> overlapActors;

// ================================================================
// ===                         FIRE DAMAGE                      ===
// ================================================================

public:
	UFUNCTION(BlueprintCallable)
	void GiveFire(float DamagePerSecond);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float FireScale = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	UParticleSystem* FireTemplate;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	USoundCue* ExplosionSound;

	/* Attached particle component when fire activated. */
	UParticleSystemComponent* fireComponent;

	/* Damage per seconds given by fire. */
	float fireDamage;

	/* When this value down to zero, the vehicle will explode. */
	float fireHeath = 100.0f;
};
