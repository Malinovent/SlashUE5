// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Components/CapsuleComponent.h"
#include "CollisionQueryParams.h"
#include "Items/Treasure.h"

// Sets default values
ABreakableActor::ABreakableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	SetRootComponent(GeometryCollection);
	GeometryCollection->SetGenerateOverlapEvents(true);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    
    Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    Capsule->SetupAttachment(GetRootComponent());
    Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
}

// Called when the game starts or when spawned
void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
}

#include "CollisionQueryParams.h"

void ABreakableActor::SpawnTreasure()
{
	UWorld* World = GetWorld();
	
	if(World && TreasureClasses.Num() > 0)
	{	
		FVector SpawnLocation = GetActorLocation();
		FVector Start = SpawnLocation;
		FVector End = SpawnLocation - FVector::UpVector * 1000.f;
		
		FHitResult FloorLocation;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		
		//Find the floor location on just the z
		if(World->LineTraceSingleByChannel(
			FloorLocation,
			Start,
			End,
			ECC_WorldStatic,
			Params,
			FCollisionResponseParams()
		))
		{
			SpawnLocation.Z = FloorLocation.ImpactPoint.Z;
		}

		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f, 0, 5.f);
		
		const uint8 RandomIndex = FMath::RandRange(0, TreasureClasses.Num() - 1);
		World->SpawnActor<ATreasure>(TreasureClasses[RandomIndex], SpawnLocation, FRotator::ZeroRotator);
	}
}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint)
{
	if(bBroken){ return; }
	bBroken = true;
}

// Called every frame
void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

