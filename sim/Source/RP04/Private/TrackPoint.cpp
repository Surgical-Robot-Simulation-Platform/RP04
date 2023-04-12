// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackPoint.h"

#include "Json.h"
#include "JsonUtilities.h"
#include "Common/UdpSocketBuilder.h"

// Sets default values
ATrackPoint::ATrackPoint()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATrackPoint::BeginPlay()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	InitializeSocket();
	if (ListenerSocket)
	{
		Super::BeginPlay();
	}
}

void ATrackPoint::InitializeSocket()
{
	LocalIP = TEXT("127.0.0.1");
	LocalPort = 16000;
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

FString ATrackPoint::ReadSocket()
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
			FString Data = FString(BytesRead, (char*)ReceivedData.GetData());
			return Data;
		}
	}
	return FString();
}

FVector ATrackPoint::ParseCoordinates(FString stream)
{
	TArray<FString> MessageParts;
	stream.ParseIntoArray(MessageParts, TEXT(" "), true);
	float X = FCString::Atof(*MessageParts[0]) * 1000;
	float Y = FCString::Atof(*MessageParts[1]) * 1000;
	float Z = FCString::Atof(*MessageParts[2]) * 200;
	UE_LOG(LogTemp, Error, TEXT("%f %f %f"), X, Y, Z);
	Z = FMath::Abs(Z - 150);
	if (Z < 35)
	{
		Z = 35;
	}
	FVector pos = FVector(X, Y, Z);
	return pos;
}


// Called every frame
void ATrackPoint::Tick(float DeltaTime)
{
	const FString coordinates = ReadSocket();
	if (!coordinates.IsEmpty())
	{
		SetActorRelativeLocation(ParseCoordinates(coordinates));
	}

	Super::Tick(DeltaTime);
}

void ATrackPoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	// Close the socket connection when the simulation stops
	if (ListenerSocket != nullptr)
	{
		ListenerSocket->Close();
		UE_LOG(LogTemp, Warning, TEXT("Socket has been closed."));
	}
	else UE_LOG(LogTemp, Warning, TEXT("Socket did not close."))
}
