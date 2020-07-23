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

#include "SceneLoader.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

ASceneLoader::ASceneLoader()
{
	PrimaryActorTick.bCanEverTick = false;
	Bounding = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounding"));
	Bounding->SetMobility(EComponentMobility::Static);
	Bounding->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Bounding->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	Bounding->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Bounding->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	Bounding->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Overlap);
	Bounding->OnComponentBeginOverlap.AddDynamic(this, &ASceneLoader::BeginOverlap);
	Bounding->OnComponentEndOverlap.AddDynamic(this, &ASceneLoader::EndOverlap);
	RootComponent = Bounding;
#if WITH_EDITOR
	arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	arrow->AttachToComponent(Bounding, FAttachmentTransformRules::KeepRelativeTransform);
#endif
}

void ASceneLoader::BeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult &SweepResult) {
	APawn* pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (OtherActor == pawn) {
		inLocation = OtherActor->GetActorLocation();
	}
}

void ASceneLoader::EndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex) {
	APawn* pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (OtherActor == pawn) {
		FVector dir = OtherActor->GetActorLocation() - inLocation;
		if (FVector::DotProduct(dir, GetActorForwardVector()) > 0) {
			setLevelLoaded(true);
			OnPlayerMovedIn();
		}
		else {
			setLevelLoaded(false);
			OnPlayerMovedOut();
		}
	}
}

void ASceneLoader::setLevelLoaded(bool load) {
	FString funcName = "RebuildLightSystem";
	FLatentActionInfo latent(0, 1, *funcName, TrafficManager);
	FString lodName = "LOD_" + LevelName.ToString();
	if (load) {
		UGameplayStatics::LoadStreamLevel(this, LevelName, true, false, latent);
		UGameplayStatics::UnloadStreamLevel(this, FName(*lodName), FLatentActionInfo(), false);
#if WITH_EDITOR
		UKismetSystemLibrary::PrintString(this, L"Load Level: " + LevelName.ToString(), true, true, FLinearColor::Green, 10);
#endif
	}
	else {
		UGameplayStatics::LoadStreamLevel(this, FName(*lodName), true, false, FLatentActionInfo());
		UGameplayStatics::UnloadStreamLevel(this, LevelName, latent, false);
#if WITH_EDITOR
		UKismetSystemLibrary::PrintString(this, L"Unload Level: " + LevelName.ToString(), true, true, FLinearColor::Red, 10);
#endif
	}
}
