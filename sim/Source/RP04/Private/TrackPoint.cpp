// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackPoint.h"

#include "Common/UdpSocketBuilder.h"

// Sets default values
ATrackPoint::ATrackPoint()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATrackPoint::BeginPlay()
{
	Sphere = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("Sphere")));
	if (!Sphere)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find point."));
		return;
	}

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	UE_LOG(LogTemp, Warning, TEXT("Track point found.."));
	InitializeSocket();
	if (ListenerSocket)
	{
		Super::BeginPlay();
	}
}

void ATrackPoint::InitializeSocket()
{
	LocalIP = TEXT("127.0.0.1");
	LocalPort = 12347;
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
			UE_LOG(LogTemp, Warning, TEXT("Received %d bytes of data: %s"), BytesRead,
			       *Data);
			return Data;
		}
	}
	return FString();
}

FVector ATrackPoint::ParseCoordinates(FString stream)
{
	int32 OpenParenIdx = stream.Find(TEXT("("), ESearchCase::IgnoreCase, ESearchDir::FromStart,
	                                 stream.Find(TEXT(","), ESearchCase::IgnoreCase, ESearchDir::FromStart) + 1);

	// Get the index of the last close parenthesis
	int32 CloseParenIdx = stream.Find(TEXT(")"), ESearchCase::IgnoreCase, ESearchDir::FromStart);

	// Get the sub-string between the open and close parentheses
	FString XYZString = stream.Mid(OpenParenIdx + 1, CloseParenIdx - OpenParenIdx - 1);

	// Split the string on commas to get the x, y, and z values as separate strings
	TArray<FString> XYZComponents;
	XYZString.ParseIntoArray(XYZComponents, TEXT(","), true);

	// Convert the strings to floats, convert from metres to cm
	float X = FCString::Atof(*XYZComponents[0]) * 100;
	float Y = FCString::Atof(*XYZComponents[1]) * 100;
	float Z = FCString::Atof(*XYZComponents[2]) * 100;
	UE_LOG(LogTemp, Warning, TEXT("Parsed coordinates: %f %f %f"), X, Y, Z);
	return FVector(X, Y, X);
}

void ATrackPoint::MovePoint(FVector coords)
{
	Sphere->SetRelativeLocation(coords);
}


// Called every frame
void ATrackPoint::Tick(float DeltaTime)
{
	FString coordinates = ReadSocket();
	if (!coordinates.IsEmpty())
	{
		FVector coords = ParseCoordinates(coordinates);
		MovePoint(coords);
	}
	Super::Tick(DeltaTime);
}
