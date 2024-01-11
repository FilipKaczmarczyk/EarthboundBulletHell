// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class NEWSHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	APlayerController* PlayerController;

	void AssignPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	int32 DefaultInputMappingPriority = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* TurnRateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookUpRateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* TurnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FireButtonAction;

	void MoveForward(const FInputActionValue& Value);

	void MoveRight(const FInputActionValue& Value);

	void TurnAtRate(const FInputActionValue& Value);

	void LookUpAtRate(const FInputActionValue& Value);

	void Turn(const FInputActionValue& Value);

	void LookUp(const FInputActionValue& Value);

	void Jump();

	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	void AimingModePressed();
	void AimingModeReleased();

	void HandleCameraZoom(float DeltaTime);

	void HandleTurnRates();

	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();
	
	UFUNCTION()
	void FinishCrosshairBulletFire();

	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer();
	
	UFUNCTION()
	void AutoFireReset();

	/** Line trace for items under the crosshair */
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	/** Trace for items if OverlappedItemCount > 0 */
	void TraceForItems();

	void SpawnDefaultWeapon();

private:
	/** Camera Holder */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	class USpringArmComponent* CameraBoom;
	
	/** Camera follows player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	class UCameraComponent* FollowCamera;

	/** Base turn rate (deg/sec) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	float BaseTurnRate = 15;
	
	/** Base look up/down rate (deg/sec) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	float BaseLookUpRate = 15;
	
	/** Turn rate when not aiming*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	float HipTurnRate;

	/** Base look up/down rate when not aiming*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	float HipLookUpRate;
	
	/** Turn rate when aiming*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	float AimingTurnRate;

	/** Base look up/down rate when aiming*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	float AimingLookUpRate;

	/** Turn rate modifier when not aiming (mouse only)*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"));
	float HipTurnRateModifier;

	/** Look up/down modifier when not aiming (mouse only)*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"));
	float HipLookUpRateModifier;
	
	/** Turn rate modifier when aiming (mouse only)*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"));
	float AimingTurnRateModifier;

	/** Look up/down modifier when aiming (mouse only)*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"));
	float AimingLookUpRateModifier;

	/** Sound spawned when shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	class USoundCue* FireSound;
	
	/** Flash spawned at barrel when shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	UParticleSystem* MuzzleFlash;

	/** Montage for firing weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	UAnimMontage* HipFireMontage;
	
	/** Particles spawned when bullet impact world element*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	UParticleSystem* ImpactParticles;
	
	/** Smoke trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"));
	UParticleSystem* BeamParticles;

	/** Are we in aiming mode? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"));
	bool bAiming;
	
	/** Default Camera FOV */
	float CameraDefaultFOV;
	
	/** Camera FOV in Aiming Mode */
	float CameraZoomedFOV;
	
	/** Camera Current FOV in this frame */
	float CameraCurrentFOV;

	/** Speed for zooming when aim */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"));
	float ZoomInterpSpeed;
	
	/** Determines the spread of the crosshair */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"));
	float CrosshairSpreadMultiplier;
	
	/** Velocity factor influencing spreads */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"));
	float CrosshairVelocityFactor;

	/** In air factor influencing spreads */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"));
	float CrosshairInAirFactor;
	
	/** Aiming factor influencing spreads */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"));
	float CrosshairAimFactor;
	
	/** Shooting factor influencing spreads */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"));
	float CrosshairShootingFactor;

	float ShootTimeDuration;
	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;

	/** Fire button pressed */
	bool bFireButtonPressed;

	/** True when we can fire. False when waiting for the timer */
	bool bShouldFire;

	/** Rate of automatic gun fire */
	float AutomaticFireRate;

	/** Sets a timer between gunshots*/
	FTimerHandle AutoFireTimer;

	/** True when we should trace every frame for item */
	bool bShouldTraceForItems;

	/** Number of overlapped AItems */
	int OverlappedItemCount;

	/** Current trace item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items,  meta = (AllowPrivateAccess = "true"));
	class AItem* HitItemLastFrame;

	/** Currently equipped weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat,  meta = (AllowPrivateAccess = "true"));
	class AWeapon* EquippedWeapon;

	/** Set this in Blueprints for the default weapon class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat,  meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass; 
	
public:
	
	AShooterCharacter();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE APlayerController* GetPlayerController() const { return PlayerController; };

	FORCEINLINE bool GetAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int GetOverlappedItemCount() const { return OverlappedItemCount; }

	/** Add/Subtract fo/from OverlappedItemCount and update bShouldTraceForItems */
	void IncrementOverlappedItemCount(int Amount); 
};
