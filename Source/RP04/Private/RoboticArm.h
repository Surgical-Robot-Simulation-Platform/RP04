// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoboticArm.generated.h"

UCLASS()
class ARoboticArm : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoboticArm();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Define robotic arm constraints and measurement
	const TArray<FString> JOINT_AXES = {"Y", "Y", "Y", "Y", "Y", "Y"};

	struct JointAngles
	{
		float Q1;
		float Q2;
		float Q3;
		float Q4;
		float Q5;
		float Q6;
	};

	// void MoveEndEffector(FVector DesiredPosition);

private:
	FString LocalIP;
	int32 LocalPort;
	FSocket* ListenerSocket;

	TArray<uint8> ReceivedData;

	UStaticMeshComponent* Joints[6];

	void InitializeSocket();
	FString ReadSocket();
	FVector ParseCoordinates(FString stream);

	void SetJointConfig(float Q1, float Q2, float Q3, float Q4, float Q5, float Q6);
};
