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

#include "VehicleHardwareComponent.h"
#include "VehicleBase.h"
#include "Kismet/KismetSystemLibrary.h"

UVehicleHardwareComponent::UVehicleHardwareComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
	bAutoActivate = false;
}

void UVehicleHardwareComponent::BeginPlay()
{
	Super::BeginPlay();
	// auto connect
	if (bAutoActivate) {
		arduino.open(TCHAR_TO_ANSI(*SerialPort), Baudrate);
	}
	// register handling events
	AVehicleBase* vehicle = Cast<AVehicleBase>(GetOwner());
	if (vehicle) {
		vehicle->addHandlingListener(this);
		if (IncludeSensorData) {
			sensors.Reset();
			vehicle->GetComponents<UObstacleSensorComponent>(sensors, false);
			for (UObstacleSensorComponent* sensor : sensors) {
				sensor->NormalizeRange = FVector2D(255, 0);
			}
			//sensors.Reset();
			//TArray<UObject*> children;
			//vehicle->CollectDefaultSubobjects(children, false);
			//for (UObject* child : children) {
			//	UObstacleSensorComponent* sensor = Cast<UObstacleSensorComponent>(child);
			//	if (sensor) {
			//		sensor->NormalizeRange = FVector2D(255, 0);
			//		sensors.Add(sensor);
			//	}
			//}
		}
	}
}

void UVehicleHardwareComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	AVehicleBase* vehicle = Cast<AVehicleBase>(GetOwner());
	if (vehicle) {
		// steering
		float wheelAngle = vehicle->WheelAngle * 2.0f;
		uint16_t steeringPulse = (uint16_t)FMath::GetMappedRangeValueClamped(FVector2D(-70, 70), FVector2D(WHEEL_PULSE_MIN, WHEEL_PULSE_MAX), wheelAngle);
		//uint16_t steeringPulse = (uint16_t)FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(MAX_STEERING_PULSE, MIN_STEERING_PULSE), vehicle->Steering * 1.5f);
		controlData[ARDUINO_OFFSET_STEERING] = (uint8_t)((steeringPulse & 0xff00) >> 8);
		controlData[ARDUINO_OFFSET_STEERING+1] = (uint8_t)((steeringPulse & 0x00ff));
		// throttle
		float throttle = FMath::Clamp(vehicle->Speed / vehicle->MaxSpeedLimit, -1.0f, 1.0f);
		//float throttle = vehicle->Throttle;
		controlData[ARDUINO_OFFSET_THROTTLE] = (uint8_t)(FMath::Abs(throttle) * 63);
		if (throttle > 0.01f) {
			controlData[ARDUINO_OFFSET_THROTTLE] |= ARDUINO_FLAG_THROTTLE_FORWARD;
		}
		else if (throttle < -0.01f) {
			controlData[ARDUINO_OFFSET_THROTTLE] |= ARDUINO_FLAG_THROTTLE_BACKWARD;
		}
		// send data
		arduino.write(controlData, 8);
		//UE_LOG(LogTemp, Warning, TEXT("%s"), *FString((const char*)controlData));

		if (IncludeSensorData) {
			sensorData[ARDUINO_OFFSET_NUMSENSORS] = (uint8_t)sensors.Num();
			for (UObstacleSensorComponent* sensor : sensors) {
				int i = ARDUINO_OFFSET_FIRSTSENSOR + sensor->SensorDataOrder;
				sensorData[i] = (uint8_t)sensor->GetNormalizedValue();
			}
			// send data
			arduino.write(controlData, 16);
			//UE_LOG(LogTemp, Warning, TEXT("%s"), *FString((const char*)sensorData));
		}
	}
}

void UVehicleHardwareComponent::Connect(FString port, int baud) {
	this->SerialPort = port;
	this->Baudrate = baud;
	arduino.close();
	arduino.open(TCHAR_TO_ANSI(*port), baud);
}

void UVehicleHardwareComponent::Disconnect() {
	arduino.close();
}

void UVehicleHardwareComponent::OnGearChanged_Implementation(int value) {
	controlData[ARDUINO_OFFSET_GEAR] = (uint8_t)value;
}

void UVehicleHardwareComponent::OnBrakeChanged_Implementation(bool state) {
	if (state) {
		controlData[ARDUINO_OFFSET_LIGHT] |= ARDUINO_FLAG_LIGHT_HANDBRAKE;
	}
	else {
		controlData[ARDUINO_OFFSET_LIGHT] &= ~ARDUINO_FLAG_LIGHT_HANDBRAKE;
	}
}

void UVehicleHardwareComponent::OnLightChanged_Implementation(bool state) {
	if (state) {
		controlData[ARDUINO_OFFSET_LIGHT] |= ARDUINO_FLAG_LIGHT_HALOGEN;
	}
	else {
		controlData[ARDUINO_OFFSET_LIGHT] &= ~ARDUINO_FLAG_LIGHT_HALOGEN;
	}
}

void UVehicleHardwareComponent::OnSideLightChanged_Implementation(bool right, bool left) {
	if (left) {
		controlData[ARDUINO_OFFSET_LIGHT] |= ARDUINO_FLAG_LIGHT_LEFTSIDE;
	}
	else {
		controlData[ARDUINO_OFFSET_LIGHT] &= ~ARDUINO_FLAG_LIGHT_LEFTSIDE;
	}
	if (right) {
		controlData[ARDUINO_OFFSET_LIGHT] |= ARDUINO_FLAG_LIGHT_RIGHTSIDE;
	}
	else {
		controlData[ARDUINO_OFFSET_LIGHT] &= ~ARDUINO_FLAG_LIGHT_RIGHTSIDE;
	}
}