// Copyright Epic Games, Inc. All Rights Reserved.

#include "MazeCharacter.h"
#include "MazeProjectile.h"
#include "Animation/AnimInstance.h"
#include "Engine/TextRenderActor.h"
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


typedef enum _Direction {
	DIRECTION_LEFT,
	DIRECTION_UP,
	DIRECTION_RIGHT,
	DIRECTION_DOWN
} Direction;

typedef enum _MapFlag {
	MAP_FLAG_WALL,
	MAP_FLAG_EMPTY,
	MAP_FLAG_VISITED,
} MapFlag;

const int DIR[4][2] = { {0,-2},{0,2},{-2,0},{2,0} };

void shuffleArray(int array[], int size)
{
	int i, r, temp;

	for (i = 0; i < (size - 1); ++i)
	{
		r = i + (rand() % (size - i));
		temp = array[i];
		array[i] = array[r];
		array[r] = temp;
	}
}

int inRange(int y, int x, int maxSize)
{
	return (y < maxSize - 1 && y > 0) && (x < maxSize - 1 && x > 0);
}

void generateMap(int y, int x, int* map, int maxSize)
{
	int i, nx, ny;
	int directions[4] = {
		DIRECTION_UP,
		DIRECTION_RIGHT,
		DIRECTION_DOWN,
		DIRECTION_LEFT
	};

	map[y + x * maxSize] = MAP_FLAG_VISITED;

	shuffleArray(directions, 4);

	for (i = 0; i < 4; i++)
	{
		// 다음 위치를 구한다.
		nx = x + DIR[directions[i]][0];
		ny = y + DIR[directions[i]][1];

		if (inRange(ny, nx, maxSize) && map[ny+nx*maxSize] == MAP_FLAG_WALL)
		{
			generateMap(ny, nx, map, maxSize);
			// 세로 축 이동인 경우
			if (ny != y)
				map[(ny + y) / 2+x*maxSize] = MAP_FLAG_EMPTY;
			// 가로 축 이동인 경우
			else
				map[y + (x + nx) / 2 * maxSize] = MAP_FLAG_EMPTY;
			map[ny+nx*maxSize] = MAP_FLAG_EMPTY;
		}
	}
}

FVector2D getRandomStartingPoint(int maxSize)
{
	int x = 1 + rand() % (maxSize - 1);
	int y = 1 + rand() % (maxSize - 1);
	if (x % 2 == 0)
		x--;
	if (y % 2 == 0)
		y--;
	return FVector2D(x, y);
}


void AMazeCharacter::GenerateMap()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("HelloWorld"));
	UWorld* const World = GetWorld();

	constexpr int mapSize = 31;
	int map[mapSize][mapSize] = { 0};
	memset(map, MAP_FLAG_WALL, sizeof(map));
	FVector2D startPoint = getRandomStartingPoint(mapSize);
	generateMap(startPoint.Y, startPoint.X, &map[0][0], mapSize);
	/*
	for (int ii = 0; ii < mapSize; ++ii)
	{
		for (int jj = 0; jj < mapSize; ++jj)
		{
			map[ii][jj] = false;
			map[ii][jj] = map[ii][jj] || (ii == 0 || jj == 0 || ii == mapSize - 1 || jj == mapSize - 1);
			map[ii][jj] = map[ii][jj] || ii % 2 == 0 || jj % 2 == 0;
		}
	}

	
	for (int ii = 0; ii < mapSize-1; ++ii)
	{
		for (int jj = 0; jj < mapSize-1; ++jj)
		{
			if (ii % 2 == 0 || jj % 2 == 0)
			{
				continue;
			}


			if (ii == mapSize - 2 && jj == mapSize - 2)
				continue;

			if (ii== mapSize - 2)
			{
				map[ii][jj + 1] = false;
				continue;
			}

			if (jj == mapSize - 2)
			{
				map[ii+1][jj] = false;
				continue;
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
	
	
	*/
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
		for (int ii = 0; ii < mapSize; ++ii)
		{
			
			for (int jj = 0; jj < mapSize; ++jj)
			{
				//if (ii == 0 || jj == 0 || ii == 99 || jj == 99)
				{
					if(MAP_FLAG_WALL == map[ii][jj])
					{
						AWall* wall = World->SpawnActor<AWall>(DefaultWallBp, location, rotation, ActorSpawnParams);
						_lstWall.Add(wall);
						//GEngine->AddOnScreenDebugMessage(uint64 Key, float TimeToDisplay, FColor DisplayColor, const FString & DebugMessage, bool bNewerOnTop = true, const FVector2D & TextScale = FVector2D::UnitVector);
						FString str = FString::Printf(TEXT("%d,%d"), ii, jj);
						wall->AddDebugText(str);
						
					}
				}
				location.Y += 100;
			}

			location.X+=100;
			location.Y = 0;
		}
	}
}