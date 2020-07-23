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

#include "TrafficLight.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"

ATrafficLight::ATrafficLight() {
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1;

	// create vehicle blocker
	VehicleBlocker = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleBlocker"));
	VehicleBlocker->SetCollisionObjectType(ECollisionChannel::ECC_Vehicle);
	VehicleBlocker->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	VehicleBlocker->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Block);
	VehicleBlocker->InitBoxExtent(FVector(5, 3, 3));
	VehicleBlocker->RelativeLocation = FVector(-3, 1, 0);
	VehicleBlocker->ShapeColor = FColor::Cyan;
	VehicleBlocker->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// create light holders
	red = CreateDefaultSubobject<USceneComponent>(TEXT("Red"));
	red->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	yellow = CreateDefaultSubobject<USceneComponent>(TEXT("Yellow"));
	yellow->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	green = CreateDefaultSubobject<USceneComponent>(TEXT("Green"));
	green->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void ATrafficLight::BeginPlay() {
	if (Master) {
		// slave mode
		Master->slaves.Add(this);
		red->SetVisibility(false, true);
		yellow->SetVisibility(false, true);
		green->SetVisibility(false, true);
		if (Reversed) {
			switch (Master->LightState) {
			case TrafficLightState::Blink:
				Master->reversedState = TrafficLightState::Blink;
				break;
			case TrafficLightState::Red:
				Master->reversedState = TrafficLightState::Green;
				break;
			case TrafficLightState::Yellow:
			case TrafficLightState::Green:
				Master->reversedState = TrafficLightState::Red;
				break;
			}
			setLightState(Master->reversedState);
		}
		else {
			setLightState(Master->LightState);
		}
	}
	else {
		// master mode
		Super::BeginPlay();
		setLightState(LightState, false);
	}
}

void ATrafficLight::Tick(float DeltaSeconds) {
	//if (!master) {
		Super::Tick(DeltaSeconds);
		TickCountdown -= 1; // DeltaSeconds = TickInterval
		if (LightState == TrafficLightState::Blink) {
			// blink mode
			if (TickCountdown <= 0) {
				TickCountdown = BlinkPeriod;
				blinkState = !blinkState;
				yellow->SetVisibility(blinkState, true);
				// update slaves
				for (ATrafficLight* slave : slaves) {
					slave->yellow->SetVisibility(blinkState, true);
				}
			}
		}
		else {
			// individual mode
			if (TickCountdown <= 0) {
				switch (LightState) {
				case TrafficLightState::Green:
					setLightState(TrafficLightState::Yellow);
					break;

				case TrafficLightState::Yellow:
					setLightState(TrafficLightState::Red);
					reversedState = TrafficLightState::Green;
					break;

				case TrafficLightState::Red:
					setLightState(TrafficLightState::Green);
					reversedState = TrafficLightState::Red;
					break;
				}
				updateSlaves();
			}
			else if (TickCountdown <= YellowPeriod && reversedState == TrafficLightState::Green) {
				reversedState = TrafficLightState::Yellow;
				updateSlaves();
			}
		}
	//}
}

void ATrafficLight::setLightState(TrafficLightState newState, bool setCountdown) {
	LightState = newState;
	bool isRed = newState == TrafficLightState::Red;
	bool isYellow = newState == TrafficLightState::Yellow;
	bool isGreen = newState == TrafficLightState::Green;
	red->SetVisibility(isRed, true);
	yellow->SetVisibility(isYellow, true);
	green->SetVisibility(isGreen, true);
	VehicleBlocker->SetCollisionEnabled((isRed || isYellow) ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	if (setCountdown) {
		if (isRed) {
			TickCountdown = RedPeriod;
		}
		else if (isYellow) {
			TickCountdown = YellowPeriod;
		}
		else if (isGreen) {
			TickCountdown = GreenPeriod;
		}
		else {
			TickCountdown = BlinkPeriod;
		}
	}
}

void ATrafficLight::updateSlaves() {
	for (ATrafficLight* slave : slaves) {
		slave->setLightState(slave->Reversed ?
			reversedState : LightState);
	}
}