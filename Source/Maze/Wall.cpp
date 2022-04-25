// Fill out your copyright notice in the Description page of Project Settings.


#include "Wall.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"

// Sets default values
AWall::AWall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	CollisionComp->InitBoxExtent(FVector(1, 1, 1));
	//CollisionComp->InitSphereRadius(5.0f);
	//CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	{
		_text = CreateDefaultSubobject<UTextRenderComponent>("TextComponent");
		_text->SetTextRenderColor(FColor::Green);
		_text->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform, "");
	}
}

// Called when the game starts or when spawned
void AWall::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWall::AddDebugText(const FString& str)
{
	_text->SetText(FText::FromString(str));
}