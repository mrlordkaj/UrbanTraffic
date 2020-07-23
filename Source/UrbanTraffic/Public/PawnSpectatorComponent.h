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

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PawnSpectatorComponent.generated.h"

UCLASS(ClassGroup = (UrbanTraffic), meta = (BlueprintSpawnableComponent))
class URBANTRAFFIC_API UPawnSpectatorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPawnSpectatorComponent();
	//virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	UFUNCTION()
	void ToggleSpectatorCamera();
	
	UFUNCTION(BlueprintCallable)
	virtual void SetSpectatorCamera(int CameraIndex);
	
	UFUNCTION(BlueprintCallable)
	void AddSpectatorCamera(UCameraComponent* Camera, bool bAutoActive);

	UFUNCTION(BlueprintCallable)
	void RemoveSpectatorCamera(UCameraComponent* Camera);

	UFUNCTION(BlueprintCallable)
	void ClearSpectatorCameras();

protected:
	/* Spring arm for the default camera. */
	UPROPERTY(EditDefaultsOnly)
	USpringArmComponent* SpringArm;

	/* The default camera, at zero index. */
	UPROPERTY(EditDefaultsOnly)
	UCameraComponent* ChaseCamera;

private:
	/* List of all cameras. */
	TArray<UCameraComponent*> cameras;

	/* Index of the activating camera. */
	int cameraIndex;
};
