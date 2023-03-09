// Fill out your copyright notice in the Description page of Project Settings.


#include "RoboticArm.h"

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


FVector ARoboticArm::ParseCoordinates(FString stream)
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

// void ARoboticArm::MoveEndEffector(FVector DesiredPosition)
// {

// }


void ARoboticArm::SetJointConfig(float Q1, float Q2, float Q3, float Q4, float Q5, float Q6)
{
	FRotator Q1_rotate, Q2_rotate, Q3_rotate, Q4_rotate, Q5_rotate, Q6_rotate;


	Q1_rotate = FRotator(Joints[0]->GetRelativeRotation().Pitch, Q1, Joints[0]->GetRelativeRotation().Roll);
	Q2_rotate = FRotator(Q2, Joints[1]->GetRelativeRotation().Yaw, Joints[1]->GetRelativeRotation().Roll);
	Q3_rotate = FRotator(Joints[2]->GetRelativeRotation().Pitch, Q3,
	                     Joints[2]->GetRelativeRotation().Roll);
	Q4_rotate = FRotator(Q4, Joints[3]->GetRelativeRotation().Yaw, Joints[3]->GetRelativeRotation().Roll);
	Q5_rotate = FRotator(Q5, Joints[4]->GetRelativeRotation().Yaw, Joints[4]->GetRelativeRotation().Roll);
	Q6_rotate = FRotator(Q6, Joints[5]->GetRelativeRotation().Yaw, Joints[5]->GetRelativeRotation().Roll);

	const FRotator Rotations[6] = {Q1_rotate, Q2_rotate, Q3_rotate, Q4_rotate, Q5_rotate, Q6_rotate};
	for (int i = 0; i < 6; i++)
	{
		Joints[i]->SetRelativeRotation(Rotations[i]);
	}
}

// Called every frame
void ARoboticArm::Tick(float DeltaTime)
{
	FString coordinates = ReadSocket();
	Super::Tick(DeltaTime);
}
