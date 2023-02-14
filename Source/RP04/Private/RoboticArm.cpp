// Fill out your copyright notice in the Description page of Project Settings.


#include "RoboticArm.h"

// Sets default values
ARoboticArm::ARoboticArm()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARoboticArm::BeginPlay()
{
	InitializeSocket();
	if (ListenerSocket)
	{
		Super::BeginPlay();
	}
}

void ARoboticArm::InitializeSocket()
{
	LocalIP = TEXT("127.0.0.1");
	LocalPort = 40000;
	FIPv4Address IPv4Address;
	FIPv4Address::Parse(LocalIP, IPv4Address);
	ListenerSocket = FUdpSocketBuilder(TEXT("ListenerSocket"))
	                 .AsReusable()
	                 .BoundToAddress(IPv4Address)
	                 .BoundToPort(LocalPort);

	if (ListenerSocket)
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket created succesfully on %s:%d"), *LocalIP, LocalPort);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Socket could not be created on %s:%d"), *LocalIP, LocalPort);
	}
}

void ARoboticArm::ReadSocket()
{
	TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	uint32 Size;
	if (ListenerSocket && ListenerSocket->HasPendingData(Size))
	{
		ReceivedData.SetNumUninitialized(Size);
		int32 BytesRead = 0;
		ListenerSocket->RecvFrom(ReceivedData.GetData(), Size, BytesRead, *RemoteAddress);

		if (BytesRead > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Received %d bytes of data: %p"), BytesRead,
			       ReceivedData.GetData());
		}
	}
}


// Called every frame
void ARoboticArm::Tick(float DeltaTime)
{
	UStaticMeshComponent* Test = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("p1_hinge")));
	if (Test)
	{
		FString ComponentName = Test->GetName();
		FVector ForceDirection = FVector::BackwardVector * 100.0f;
		Test->AddForce(ForceDirection);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find component"));
	}
	ReadSocket();
	Super::Tick(DeltaTime);
}
