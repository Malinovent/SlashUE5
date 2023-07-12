// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "Components/CapsuleComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "Perception/PawnSensingComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Slash/DebugMacros.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SetPeripheralVisionAngle(45);
	PawnSensing->SightRadius = 5000.f;
	
}


void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	if(HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
	
	//Cache the AI Controller, Check the AI Controller documentation
	EnemyController = Cast<AAIController>(GetController());

	MoveToTarget(CurrentPatrolTarget);

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	if(Attributes && Attributes->IsAlive())
	{
		DirectionalHitReact(ImpactPoint);
	}
	else
	{
		Die();
	}
	
	
	if(HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
	
}

void AEnemy::DirectionalHitReact(const FVector& ImpactPoint)
{
	//output an angle between 0 and 360 to determine which of 4 react animations to play,
	//therefore determining which direction the enemy will move in
	const FVector Forward = GetActorForwardVector();
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit == |Forward| * |ToHit| * cos(theta)
	// |Forward| = 1, |ToHit| = 1, so Forward * ToHit == cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	// Take the inverse cosine of both sides to get theta by itself
	double Theta = FMath::Acos(CosTheta);
	// Convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);

	//If CrossProduct is positive, the hit is on the left side of the enemy
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if(CrossProduct.Z < 0)
	{
		Theta *= -1;
	}

	//Play Hit React Montage
	FName Section("FromBack");
	if(Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");
	}
	else if(Theta >= -135.f  && Theta < -45.f )
	{
		Section = FName("FromLeft");
	}
	else if(Theta >= 45.f  && Theta < 135.f )
	{
		Section = FName("FromRight");
	}
	
	PlayHitReactMontage(Section);
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if(Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
		CombatTarget = EventInstigator->GetPawn();
		
		if(HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(true);
			HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
		}
	}

	if(CurrentState != EEnemyState::EES_Chasing || CurrentState != EEnemyState::EES_Attacking)
	{
		CurrentState = EEnemyState::EES_Chasing;
		MoveToTarget(CombatTarget);
	}

	return DamageAmount;
}


void AEnemy::CheckCombatTarget()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("COMBAT"));
	//If there is no combat target in the combat radius, then go back to business
	if(!InTargetRange(CombatTarget, CombatRadius))
	{
		CombatTarget = nullptr;
		if(HealthBarWidget) HealthBarWidget->SetVisibility(false);
		CurrentState = EEnemyState::EES_Patrolling;
		MoveToTarget(CurrentPatrolTarget);
		UE_LOG(LogTemp, Warning, TEXT("Lose Interest"));
	}
	else if(!InTargetRange(CombatTarget, AttackRadius) && CurrentState != EEnemyState::EES_Chasing)
	{
		//Outside attack range, chase character
		CurrentState = EEnemyState::EES_Chasing;
		GetCharacterMovement()->MaxWalkSpeed = 300;
		MoveToTarget(CombatTarget);
		UE_LOG(LogTemp, Warning, TEXT("Chasing"));
	}
	else if(InTargetRange(CombatTarget, AttackRadius) && CurrentState != EEnemyState::EES_Attacking)
	{
		//Inside attack range, attack character
		CurrentState = EEnemyState::EES_Attacking;
		UE_LOG(LogTemp, Warning, TEXT("Attack"));

	}
	
}

void AEnemy::CheckPatrolTarget()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PATROLLING"));
	if(InTargetRange(CurrentPatrolTarget, PatrolRadius))
	{
		CurrentPatrolTarget = ChoosePatrolTarget();
		//MoveToTarget(CurrentPatrolTarget);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, FMath::RandRange(WaitMin, WaitMax)); //Gets the world timer manager
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	//UE_LOG(LogTemp, Warning, TEXT("Pawn Seen!"));
	if(CurrentState == EEnemyState::EES_Chasing) return;	//If already chasing, don't do anything	
	if(SeenPawn->ActorHasTag("SlashCharacter"))
	{
		//Go to chase
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("I SEE YOU"));
		GetWorldTimerManager().ClearTimer(PatrolTimer);
		GetCharacterMovement()->MaxWalkSpeed = 300;
		CombatTarget = SeenPawn;
		
		if(CurrentState != EEnemyState::EES_Attacking)
		{
			CurrentState = EEnemyState::EES_Chasing;
			MoveToTarget(CombatTarget);
		}
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(CurrentState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}
}


void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::PlayHitReactMontage(FName SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
	
}

void AEnemy::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if(AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
		AnimInstance->Montage_JumpToSection("Death", DeathMontage);
		CurrentState = EEnemyState::EES_Dead;
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if(HealthBarWidget) HealthBarWidget->SetVisibility(false);
		SetLifeSpan(3.f);
	}
}

bool AEnemy::InTargetRange(const AActor* Target, double Radius) const
{
	if(Target == nullptr) return false;
	
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();

	DRAW_SPHERE_SINGLEFRAME(GetActorLocation(), FColor::Green);
	DRAW_SPHERE_SINGLEFRAME(Target->GetActorLocation(), FColor::Red);
	
	
	return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if(EnemyController == nullptr || Target == nullptr) return;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(15.f);
	EnemyController->MoveTo(MoveRequest);
	//TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints(); //Return a TArray of pathpoints on the navmesh
	
	
	/*
	//Set the AI's move to patrol target
	if(EnemyController && CurrentPatrolTarget)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(CurrentPatrolTarget);
		MoveRequest.SetAcceptanceRadius(15.f);
		FNavPathSharedPtr NavPath;
		EnemyController->MoveTo(MoveRequest, &NavPath);
		TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints(); //Return a TArray of pathpoints on the navmesh
	}
	*/
}

AActor* AEnemy::ChoosePatrolTarget()
{
	if(PatrolTargets.Num() <= 1){ return nullptr; }

	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("In Target Range")));
	TArray<AActor*> ValidTargets;

	for(AActor* Target : PatrolTargets)
	{
		if(Target != CurrentPatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}
	
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Looking for patrol target")));
	return  ValidTargets[FMath::RandRange(0, ValidTargets.Num() - 1)];
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(CurrentPatrolTarget);
}

