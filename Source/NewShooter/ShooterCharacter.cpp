// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Item.h"
#include "Weapon.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
// BASE RATES FOR TURNING/LOOKING UP
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	bAiming(false),
// TURN RATES AND LOOK UP/DOWN FOR AIMING/NOT AIMING
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
// TURN RATES FOR AND LOOK UP/DOWN MODIFIERS FOR MOUSE
	HipTurnRateModifier(1.f),
	HipLookUpRateModifier(1.f),
	AimingTurnRateModifier(0.22f),
	AimingLookUpRateModifier(0.22f),
// CAMERA FOV VALUES
	CameraDefaultFOV(0.f),
	CameraZoomedFOV(35.f),
	ZoomInterpSpeed(20.f),
	CameraCurrentFOV(0.f),
// CROSSHAIR SPREAD FACTOR
	CrosshairSpreadMultiplier(0.f),
	CrosshairAimFactor(0.5f),
	CrosshairInAirFactor(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairShootingFactor(0.f),
// BULLET FIRE TIMER VARIABLES
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
// AUTOMATIC GUN FIRE RATE
	AutomaticFireRate(0.1f),
	bShouldFire(true),
// ITEM TRACE VARIABLES
	bShouldTraceForItems(false),
	OverlappedItemCount(0),
// CAMERA INTERP LOCATIONS VARIABLES
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
// AMMO VARIABLES
	Starting9mmAmmo(85),
	StartingARAmmo(120),
// COMBAT VARIABLES
	CombatState(ECombatState::ECS_Unoccupied)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; // Camera follow distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera")); 
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to spring arm
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// DONT ROTATE WHEN CONTROLLER ROTATE. CONTROLLER ONLY AFFECT CAMERA.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	

	// CONFIGURE CHARACTER MOVEMENT
	GetCharacterMovement()->bOrientRotationToMovement = false; // CHARACTER MOVE IN THE DIRECTION OF INPUT
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // AT THIS ROTATION RATE
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	AssignPlayerController();

	if (PlayerController)
	{
		// Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller.
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// PawnClientRestart can run more than once in an Actor's lifetime, so start by clearing out any leftover mappings.
			Subsystem->ClearAllMappings();

			// Add each mapping context, along with their priority values. Higher values outprioritize lower values.
			Subsystem->AddMappingContext(DefaultInputMappingContext, DefaultInputMappingPriority);
		}
	}

	// Spawn default weapon and equip it
	EquipWeapon(SpawnDefaultWeapon());

	InitializeAmmoMap();
}

void AShooterCharacter::AssignPlayerController()
{
	if (!PlayerController)
	{
		PlayerController = Cast<APlayerController>(GetController());
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleCameraZoom(DeltaTime);

	// Change look sensitivity depends player aim
	HandleTurnRates();

	CalculateCrosshairSpread(DeltaTime);

	TraceForItems();
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveForwardAction)
		{
			PlayerEnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AShooterCharacter::MoveForward);
		}

		if (MoveRightAction)
		{
			PlayerEnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AShooterCharacter::MoveRight);
		}
		
		if (TurnRateAction)
        {
        	PlayerEnhancedInputComponent->BindAction(TurnRateAction, ETriggerEvent::Triggered, this, &AShooterCharacter::TurnAtRate);
        }
        		
		if (LookUpRateAction)
		{
			PlayerEnhancedInputComponent->BindAction(LookUpRateAction, ETriggerEvent::Triggered, this, &AShooterCharacter::LookUpAtRate);
		}

		if (TurnAction)
		{
			PlayerEnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Turn);
		}
        		
		if (LookUpAction)
		{
			PlayerEnhancedInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &AShooterCharacter::LookUp);
		}

		if (JumpAction)
		{
			PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AShooterCharacter::Jump);
		}

		if (FireButtonAction)
		{
			PlayerEnhancedInputComponent->BindAction(FireButtonAction, ETriggerEvent::Started, this, &AShooterCharacter::FireButtonPressed);
			PlayerEnhancedInputComponent->BindAction(FireButtonAction, ETriggerEvent::Completed, this, &AShooterCharacter::FireButtonReleased);
		}

		if (AimAction)
		{
			PlayerEnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AShooterCharacter::AimingModePressed);
			PlayerEnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AShooterCharacter::AimingModeReleased);
		}
		
		if (SelectAction)
		{
			PlayerEnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Started, this, &AShooterCharacter::SelectButtonPressed);
			PlayerEnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Completed, this, &AShooterCharacter::SelectButtonReleased);
		}
		
	}
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::MoveForward(const FInputActionValue& Value)
{
	if((Controller != nullptr) && (Value[0] != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation{0, Rotation.Yaw, 0};

		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, Value[0]);
	}
}

void AShooterCharacter::MoveRight(const FInputActionValue& Value)
{
	if((Controller != nullptr) && (Value[0] != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation{0, Rotation.Yaw, 0};

		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, Value[0]);
	}
}

void AShooterCharacter::TurnAtRate(const FInputActionValue& Value)
{
	AddControllerYawInput(Value[0] * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(const FInputActionValue& Value)
{
	AddControllerPitchInput(Value[0] * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(const FInputActionValue& Value)
{
	const float TurnValue = bAiming ? Value[0] * AimingTurnRateModifier : Value[0] * HipTurnRateModifier;
	
	AddControllerYawInput(TurnValue);
}

void AShooterCharacter::LookUp(const FInputActionValue& Value)
{
	const float LookUpValue = bAiming ? Value[0] * AimingLookUpRateModifier : Value[0] * HipLookUpRateModifier;
	
	AddControllerPitchInput(LookUpValue);
}

void AShooterCharacter::Jump()
{
	Super::Jump();
}

void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr) return;

	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		PlayFireSound();

		FireBullet();
	
		PlayGunFireMontage();

		// Start bullet fire timer for crosshairs
		StartCrosshairBulletFire();
	
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		// Tentative beam location - still need to trace from gun
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else // no crosshair trace hit
	{
		// OutBeamLocation is the End location of the line trace
	}
	
	// Perform a second trace, from barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart = MuzzleSocketLocation;
	const FVector StartToEnd { OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd { OutBeamLocation + StartToEnd * 1.25f};

	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit) // Object between barrel and BeamEndPoint???
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	
	return false;
}

void AShooterCharacter::AimingModePressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingModeReleased()
{
	bAiming = false;
}

void AShooterCharacter::HandleCameraZoom(float DeltaTime)
{
	// Aiming button pressed?
	if (bAiming)
	{
		// Interpolate to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		// Interpolate to default FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::HandleTurnRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{0.f, 600.f};
	FVector2D VelocityMultiplierRange{0.f, 1.f};
	FVector Velocity { GetVelocity() };
	Velocity.Z = 0;

	// Calculate velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	// Calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling())
	{
		// Spread the crosshair slowly when in the air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else // in the ground
	{
		// Shrink the crosshair rapidly when in the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	if (bAiming) // aiming
	{
		// Spread the crosshair when in no aiming
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, ZoomInterpSpeed);
	}
	else // no aiming
	{
		// Shrink the crosshair when in no aiming
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, ZoomInterpSpeed);
	}

	if (bFiringBullet) // True 0.05 second after shoot
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 30.f);
	}
	
	CrosshairSpreadMultiplier =
		CrosshairAimFactor +
		CrosshairVelocityFactor + 
		CrosshairInAirFactor +
		CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(
		CrosshairShootTimer,
		this,
		&AShooterCharacter::FinishCrosshairBulletFire,
		ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;
	
	if (WeaponHasAmmo())
	{
		bShouldFire = true;
	
		if (bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		// RELOAD WEAPON
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// Get Viewport Size
	FVector2d ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshair
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshair
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld
	(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if(bScreenToWorld)
	{
		// Trace form Crosshair world location outward
		const FVector Start { CrosshairWorldPosition };
		const FVector End {Start + CrosshairWorldDirection * 50'000};
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	
	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				// Show item pickup widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);	
			}

			// We hit an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					// We are hitting a different AItem this frame
					// Or AItem is null
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}

			// Store a reference to HitItem for next frame
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// No longer overlapping any items
		// Item last frame should not showing widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

void AShooterCharacter::IncrementOverlappedItemCount(int Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation {FollowCamera->GetComponentLocation()};
	const FVector CameraForward {FollowCamera->GetForwardVector()};

	// DesiredLocation = CameraWorldLocation + Forward * A + Up * B

	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f,0.f, CameraInterpElevation);
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	auto Weapon = Cast<AWeapon>(Item);

	if (Weapon)
	{
		SwapWeapon(Weapon);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	// Check the TSubclassOf Variable
	if (DefaultWeaponClass)
	{
		// Spawn weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip)
	{
		// Get hand socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));

		if (HandSocket)
		{
			// attach weapon to the hand socket ("RightHandSocket")
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		// Set EquippedWeapon to the newly spawned weapon
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this);
	}
}

void AShooterCharacter::SelectButtonReleased()
{
	
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr)
		return false;

	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::FireBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("barrel_socket");

	if (BarrelSocket)
	{
		const FTransform BarrelSocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, BarrelSocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(BarrelSocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd)
		{
			// Spawn impact particles after update BeamEndPoint
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}
		
			// Spawn beam particles 	
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, BarrelSocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName{"Target"}, BeamEnd);
				}
			}
		}
	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}



