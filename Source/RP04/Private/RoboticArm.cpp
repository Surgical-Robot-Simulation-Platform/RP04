// Fill out your copyright notice in the Description page of Project Settings.


#include "RoboticArm.h"

#include "tiff.h"
#include "Common/UdpSocketBuilder.h"


// Sets default values
ARoboticArm::ARoboticArm()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARoboticArm::BeginPlay()
{
	// Target each joint
	Joints[0] = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("q1")));
	Joints[1] = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("q2")));
	Joints[2] = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("q3")));
	Joints[3] = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("q4")));
	Joints[4] = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("q5")));
	Joints[5] = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("q6")));

	for (const auto* Joint : Joints)
	{
		if (!Joint)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find all 6 joints."));
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("All joints found and targeted."));

	InitializeSocket();
	if (ListenerSocket)
	{
		Super::BeginPlay();
	}
}

void ARoboticArm::InitializeSocket()
{
	LocalIP = TEXT("127.0.0.1");
	LocalPort = 8500;
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

FString ARoboticArm::ReadSocket()
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


ARoboticArm::Coordinates ARoboticArm::ParseCoordinates(FString stream)
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
	Coordinates Coords = {X, Y, Z};
	UE_LOG(LogTemp, Warning, TEXT("Parsed coordinates: %f %f %f"), X, Y, Z);
	return Coords;
}

// ARoboticArm::JointAngles IKSolver(ARoboticArm::Coordinates coordinates)
// {
//}

void ARoboticArm::RotateJoints(JointAngles angles)
{
}


// Called every frame
void ARoboticArm::Tick(float DeltaTime)
{
	FString coordinates = ReadSocket();
	if (coordinates != "")
	{
		Coordinates ParsedCoordinates = ParseCoordinates(coordinates);
		//JointAngles SolvedAngles = IKSolver(ParsedCoordinates);
		//RotateJoints(SolvedAngles);
	}
	Super::Tick(DeltaTime);
}
