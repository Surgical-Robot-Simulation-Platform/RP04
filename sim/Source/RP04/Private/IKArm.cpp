// Fill out your copyright notice in the Description page of Project Settings.


#include "IKArm.h"
#include "Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"

// Sets default values
AIKArm::AIKArm()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIKArm::BeginPlay()
{
	USkeletalMeshComponent* SkeletalMeshComponent = FindComponentByClass<USkeletalMeshComponent>();
	if (!SkeletalMeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find skeletal mesh!"));
		return;
	}
	Super::BeginPlay();
	
}

// Called every frame
void AIKArm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

