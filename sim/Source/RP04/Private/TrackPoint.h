// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrackPoint.generated.h"

UCLASS()
class ATrackPoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATrackPoint();

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

	UStaticMeshComponent* Sphere;
	void InitializeSocket();
	FString ReadSocket();
	FVector ParseCoordinates(FString stream);

	void MovePoint(FVector coords);
};
