// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeProjectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;

UCLASS(config=Game)
class AMazeProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	UBoxComponent*		CollisionComp;


public:
	AMazeProjectile();
};

