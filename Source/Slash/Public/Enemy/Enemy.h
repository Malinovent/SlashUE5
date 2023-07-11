// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UAnimMontage;
class UAttributeComponent; 
class UHealthBarComponent;
class AAIController;
class UPawnSensingComponent;

UCLASS()
class SLASH_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:

	AEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
	void DirectionalHitReact(const FVector& ImpactPoint);
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void CheckCombatTarget();
	void CheckPatrolTarget();
	UFUNCTION()
	void PawnSeen(APawn* Pawn);


	
protected:
	virtual void BeginPlay() override;
	void PlayHitReactMontage(FName SectionName);
	void Die();
	bool InTargetRange(const AActor* Target, double Radius) const;
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
	
	UPROPERTY(BlueprintReadWrite)
	EEnemyState CurrentState = EEnemyState::EES_Patrolling;

	

private:
	
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;
	
	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = VFX)
	UParticleSystem* HitParticles;

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY()
	AActor* CombatTarget;
	
	UPROPERTY(EditAnywhere)
	double CombatRadius = 50.f;

	FTimerHandle PatrolTimer;  // The world timer keeps tracks of the time in the game world
	void PatrolTimerFinished(); // CALL BACK: The function that will be called when the timer is finished

	
	UPROPERTY()
	AAIController* EnemyController;
	/**
	* Navigation: Create an array of Patrol Targets, a current target and set them to be accessible in the editor
	*/
	UPROPERTY(EditAnywhere, Category = "AI Navigation", BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	AActor* CurrentPatrolTarget;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolRadius = 200.f;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMax = 10.f;

	/**
	* Sensing: 
	*/
	UPROPERTY(VisibleAnywhere, Category = "AI Sensing")
	UPawnSensingComponent* PawnSensing;
	
public:
	
};
