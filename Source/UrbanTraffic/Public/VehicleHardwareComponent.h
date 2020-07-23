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

#include "ObstacleSensorComponent.h"
#include "Tools/SimpleSerial.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleHardwareComponent.generated.h"

// data offsets
#define ARDUINO_OFFSET_STEERING	2
#define ARDUINO_OFFSET_THROTTLE	4
#define ARDUINO_OFFSET_GEAR		5
#define ARDUINO_OFFSET_LIGHT	6
#define ARDUINO_OFFSET_NUMSENSORS	2
#define ARDUINO_OFFSET_FIRSTSENSOR	3

// data flags
#define ARDUINO_FLAG_THROTTLE_FORWARD	(1 << 7)
#define ARDUINO_FLAG_THROTTLE_BACKWARD	(1 << 6)
#define ARDUINO_FLAG_LIGHT_HALOGEN		(1 << 0)
#define ARDUINO_FLAG_LIGHT_HANDBRAKE	(1 << 1)
#define ARDUINO_FLAG_LIGHT_LEFTSIDE		(1 << 2)
#define ARDUINO_FLAG_LIGHT_RIGHTSIDE	(1 << 3)

 // hardware ranges
#define WHEEL_ANGLE_MID     92
#define WHEEL_ANGLE_MIN     44
#define WHEEL_ANGLE_MAX     140
#define WHEEL_PULSE_MIN		15828
#define WHEEL_PULSE_MAX		31860
#define MOTOR_ANALOG_MIN    48
#define MOTOR_ANALOG_MAX    255
#define MOTOR_ANALOG_RANGE  (MOTOR_ANALOG_MAX-MOTOR_ANALOG_MIN)

UCLASS( ClassGroup=(UrbanTraffic), meta=(BlueprintSpawnableComponent) )
class URBANTRAFFIC_API UVehicleHardwareComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVehicleHardwareComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void Connect(FString SerialPort, int Baudrate);

	UFUNCTION(BlueprintCallable)
	void Disconnect();

	UFUNCTION(BlueprintNativeEvent)
	void OnGearChanged(int Gear);

	UFUNCTION(BlueprintNativeEvent)
	void OnBrakeChanged(bool HandBrake);

	UFUNCTION(BlueprintNativeEvent)
	void OnLightChanged(bool LightState);

	UFUNCTION(BlueprintNativeEvent)
	void OnSideLightChanged(bool RightSide, bool LeftSide);
			
protected:
	/* The name of device's serial port. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SerialPort;

	/* The baudrate of device's serial port. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Baudrate = 9600;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IncludeSensorData;

private:
	/* The collection of vehicle sensors. */
	TArray<UObstacleSensorComponent*> sensors;

	/* Arduino serial handler. */
	SimpleSerial arduino;

	/* Serialization of control signal to be sent. */
	uint8_t controlData[8] = { '$', '#', 0, 0, 0, 0, 0, '\r' };

	/* Serialization of sensor data to be sent. */
	uint8_t sensorData[16] = { '$', '@', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\r' };
};
