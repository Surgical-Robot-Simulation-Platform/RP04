// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UdpSocketBuilder.h"
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

private:
	FString LocalIP;
	int32 LocalPort;
	FSocket* ListenerSocket;

	TArray<uint8> ReceivedData;

	void InitializeSocket();
	void ReadSocket();
};
