// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Wall.generated.h"

UCLASS()
class MAZE_API AWall : public AActor
{
	GENERATED_BODY()
		UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		class UBoxComponent* CollisionComp;
public:	
	// Sets default values for this actor's properties
	AWall();
	class UTextRenderComponent* _text;
	void AddDebugText(const FString& str);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	

};
