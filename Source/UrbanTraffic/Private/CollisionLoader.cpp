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

#include "CollisionLoader.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Misc/Paths.h"
#include <string>
#include <fstream>
#include <iostream>

ACollisionLoader::ACollisionLoader()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent->SetMobility(EComponentMobility::Static);
}

void ACollisionLoader::BeginPlay()
{
	Super::BeginPlay();
	// build primitive collision from data file
	UWorld* world = GetWorld();
	AActor* collAct = nullptr;
	USceneComponent* collCom = nullptr;
	/*std::ifstream is(*(FPaths::ProjectDir() + dataFile));*/
	std::ifstream is(*(FPaths::GameDir() + DataFile));
	std::string line;
	while (std::getline(is, line)) {
		if (line.compare("break") == 0) {
			// break command closes previous grand collision
			collCom = nullptr; // trigger next line is the first
			collAct = nullptr;
			continue;
		}
		// breakdown line into params
		TArray<FString> params;
		FString(line.c_str()).ParseIntoArray(params, _T(","));
		if (collCom == nullptr) {
			// first line define grand collision
			if (GenerateActors) {
				// spawn grand actor, attach to this
				collAct = world->SpawnActor<AActor>();
				collCom = NewObject<USceneComponent>(collAct, TEXT("RootComponent"));
				collAct->SetRootComponent(collCom);
				collAct->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
#if WITH_EDITOR
				collAct->SetActorLabel(params[0].TrimStartAndEnd());
#endif
			}
			else {
				// create grand collision component
				collCom = NewObject<USceneComponent>(this);
				collCom->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			}
			// apply grand component transform
			collCom->SetRelativeLocation(FVector(
				FCString::Atof(*params[1]),
				FCString::Atof(*params[2]),
				FCString::Atof(*params[3])
			));
			collCom->SetRelativeRotation(FQuat(
				FCString::Atof(*params[4]),
				FCString::Atof(*params[5]),
				FCString::Atof(*params[6]),
				FCString::Atof(*params[7])
			));
			collCom->SetMobility(EComponentMobility::Static);
			collCom->RegisterComponent();
		}
		else {
			// other lines define collision parts upon type (first param)
			switch (FCString::Atoi(*params[0])) {
				//case COLLISION_MESH: {
				//	FString meshName = params[1].TrimStartAndEnd();
				//	FString meshPath = "StaticMesh'/Game/LibertyCity/Collisions/" + meshName + "." + meshName + "'";
				//	UStaticMeshComponent* meshCom = NewObject<UStaticMeshComponent>(collisionAsActor ? collAct : this);
				//	meshCom->AttachToComponent(collCom, FAttachmentTransformRules::KeepRelativeTransform);
				//	meshCom->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, *meshPath)));
				//	meshCom->SetVisibility(false);
				//	meshCom->SetCollisionProfileName(FName("BlockAllDynamic"));
				//	meshCom->SetMobility(EComponentMobility::Static);
				//	meshCom->RegisterComponent();
				//} break;

			case COLLISION_BOX: {
				UBoxComponent* boxCom = NewObject<UBoxComponent>(GenerateActors ? collAct : this);
				boxCom->AttachToComponent(collCom, FAttachmentTransformRules::KeepRelativeTransform);
				boxCom->SetRelativeLocation(FVector(
					FCString::Atof(*params[1]),
					FCString::Atof(*params[2]),
					FCString::Atof(*params[3])
				));
				boxCom->SetBoxExtent(FVector(
					FCString::Atof(*params[4]),
					FCString::Atof(*params[5]),
					FCString::Atof(*params[6])
				));
				boxCom->ShapeColor = FColor::White;
				boxCom->SetCollisionProfileName(FName("BlockAllDynamic"));
				boxCom->SetMobility(EComponentMobility::Static);
				boxCom->RegisterComponent();
			} break;

			case COLLISION_SPHERE: {
				USphereComponent* sphereCom = NewObject<USphereComponent>(GenerateActors ? collAct : this);
				sphereCom->AttachToComponent(collCom, FAttachmentTransformRules::KeepRelativeTransform);
				sphereCom->SetRelativeLocation(FVector(
					FCString::Atof(*params[1]),
					FCString::Atof(*params[2]),
					FCString::Atof(*params[3])
				));
				sphereCom->SetSphereRadius(FCString::Atof(*params[4]));
				sphereCom->ShapeColor = FColor::Black;
				sphereCom->SetCollisionProfileName(FName("BlockAllDynamic"));
				sphereCom->SetMobility(EComponentMobility::Static);
				sphereCom->RegisterComponent();
			} break;
			}
		}
	}
	is.close();
}