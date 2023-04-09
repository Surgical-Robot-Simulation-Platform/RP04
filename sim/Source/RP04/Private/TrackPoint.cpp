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
	Point = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("Sphere")));
	if (!Point)
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
			UE_LOG(LogTemp, Warning, TEXT("Received %d bytes of data: %s"), BytesRead,
			       *Data);
			return Data;
		}
	}
	return FString();
}

void ATrackPoint::MovePoint(FString stream)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(stream);

	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		for (auto& JsonElem : JsonObject->Values)
		{
			FString Key = JsonElem.Key;
			TSharedPtr<FJsonValue> JsonValue = JsonElem.Value;
			float x, y, z;
			float x_rot, y_rot, z_rot;
			int count = 0;
			if (JsonValue->Type == EJson::Array)
			{
				TArray<TSharedPtr<FJsonValue>> ArrayValue = JsonValue->AsArray();

				// Extract each sequence of values between the '[' and ']' characters.
				for (auto& ArrayElem : ArrayValue)
				{
					if (ArrayElem->Type == EJson::Array)
					{
						TArray<TSharedPtr<FJsonValue>> InnerArrayValue = ArrayElem->AsArray();

						// Do something with the sequence of values.
						for (auto& InnerArrayElem : InnerArrayValue)
						{
							if (InnerArrayElem->Type == EJson::Number)
							{
								float Value = InnerArrayElem->AsNumber();
								if (count == 0)
								{
									x = Value;
								}
								else if (count == 1)
								{
									y = Value;
								}
								else if (count == 2)
								{
									z = Value;
								}
								else if (count == 3)
								{
									x_rot = Value;
								}
								else if (count == 4)
								{
									y_rot = Value;
								}
								else if (count == 5)
								{
									z_rot = Value;
								}
							}
							count++;
							if (count == 6)
							{
								UE_LOG(LogTemp, Warning, TEXT("Position: %f, %f, %f\tRotation: %f, %f, %f"), x, y, z,
								       x_rot,
								       y_rot, z_rot);
								FVector pos(x * 200, y * 200, z * 200);
								FRotator rot(x_rot, y_rot, z_rot);
								Point->SetRelativeLocationAndRotation(pos, rot);
								return;
							}
						}
					}
				}
			}
		}
	}
}


// Called every frame
void ATrackPoint::Tick(float DeltaTime)
{
	FString coordinates = ReadSocket();
	if (!coordinates.IsEmpty())
	{
		MovePoint(coordinates);
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
