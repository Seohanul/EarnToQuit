// Copyright Epic Games, Inc. All Rights Reserved.

#include "MazeCharacter.h"
#include "MazeProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Engine.h"
#include "Wall.h"

//////////////////////////////////////////////////////////////////////////
// AMazeCharacter

AMazeCharacter::AMazeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));
}

void AMazeCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

}

//////////////////////////////////////////////////////////////////////////// Input

void AMazeCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	// Bind fire event
	PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, this, &AMazeCharacter::OnPrimaryAction);
	PlayerInputComponent->BindAction("GenerateMap", IE_Released, this, &AMazeCharacter::GenerateMap);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMazeCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMazeCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMazeCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AMazeCharacter::LookUpAtRate);
}

void AMazeCharacter::OnPrimaryAction()
{
	// Trigger the OnItemUsed Event
	OnUseItem.Broadcast();
}

void AMazeCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnPrimaryAction();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AMazeCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AMazeCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMazeCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMazeCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMazeCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

bool AMazeCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMazeCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AMazeCharacter::EndTouch);

		return true;
	}
	
	return false;
}

void AMazeCharacter::GenerateMap()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("HelloWorld"));
	UWorld* const World = GetWorld();

	constexpr int mapSize = 100;
	bool map[mapSize+2][mapSize+2] = { false };
	for (int ii = 0; ii < mapSize; ++ii)
	{
		for (int jj = 0; jj < mapSize; ++jj)
		{
			if (ii % 2 == 0 || jj % 2 == 0)
			{
				map[ii][jj] = true;
			}
			else
			{
				map[ii][jj] = false;
			}
		}
	}

	
	for (int ii = 0; ii < mapSize; ++ii)
	{
		for (int jj = 0; jj < mapSize; ++jj)
		{
			if (ii % 2 == 0 || jj % 2 == 0)
			{
				continue;
			}

			if (ii == mapSize - 2)
			{
				map[ii][jj + 1] = false;
				continue;
			}

			if (jj == mapSize - 2)
			{
				map[ii + 1][jj] = false;
			}

			if (rand() % 2 == 0)
			{
				map[ii][jj + 1] = false;
			}
			else
			{
				map[ii + 1][jj] = false;
			}
		}
	}
	
	

	if (World != nullptr && nullptr != DefaultWallBp)
	{

		for (auto& pWall : _lstWall)
		{
			World->DestroyActor(pWall);
		}
		_lstWall.RemoveAll([](AWall*) {return true; });
		
		FActorSpawnParameters ActorSpawnParams;

		FVector location(0,0,0);
		FRotator rotation(0,0,0);
		for (int ii = 0; ii < 100; ++ii)
		{
			
			for (int jj = 0; jj < 100; ++jj)
			{
				//if (ii == 0 || jj == 0 || ii == 99 || jj == 99)
				{
					if( map[ii][jj])
					{
						_lstWall.Add(World->SpawnActor<AWall>(DefaultWallBp, location, rotation, ActorSpawnParams));
					}
				}
				location.Y += 100;
			}

			location.X+=100;
			location.Y = 0;
		}
	}
}