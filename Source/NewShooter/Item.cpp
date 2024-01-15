// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
AItem::AItem() :
	ItemName(FString("Default")),
	ItemCount(30),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup),
	// Item interp variables
	ZCurveTime(0.7f),
	ItemInterpStartLocation(FVector(0.f)),
	CameraTargetLocation(FVector(0.f)),
	bInterping(false),
	ItemInterpX(0.f),
	ItemInterpY(0.f),
	InterpInitialYawOffset(0.f)
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	// Hide pickup widget
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
	SetActiveStars();

	// Setup overlap for AreaSphere
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	// Set Item properties based on ItemState
	SetItemProperties(ItemState);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle item interping when interping state
	ItemInterp(DeltaTime);
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(1);
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(-1);
		}
	}
}

void AItem::SetActiveStars()
{
	for (int32 i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}

	for (int32 i = 1; i <= static_cast<int>(ItemRarity) + 1; i++)
	{
		ActiveStars[i] = true;
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:
			
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AreaSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		
		break;
		
	case EItemState::EIS_EquipInterping:

		PickupWidget->SetVisibility(false);
		
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		break;
		
	case EItemState::EIS_PickedUp:
		break;
		
	case EItemState::EIS_Equipped:

		PickupWidget->SetVisibility(false);
			
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
		break;
		
	case EItemState::EIS_Falling:
			
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		break;
		
	case EItemState::EIS_MAX:
		break;
	}
}

void AItem::StartItemCurve(AShooterCharacter* Char)
{
	// Store a handle to the Character;
	Character = Char;

	// Store init location of the item
	ItemInterpStartLocation = GetActorLocation();

	bInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);

	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishInterping, ZCurveTime);

	// Get initial yaw of the camera
	const float CameraRotationYaw = Character->GetFollowCamera()->GetComponentRotation().Yaw;

	// Get initial yaw of the item
	const float ItemRotationYaw = GetActorRotation().Yaw;

	// Difference between camera nad item yaw's
	InterpInitialYawOffset = ItemRotationYaw - CameraRotationYaw;
}

void AItem::FinishInterping()
{
	bInterping = false;
	
	if (Character)
	{
		Character->GetPickupItem(this);
	}

	// Set scale back to normal
	SetActorScale3D(FVector(1.f));
}

void AItem::ItemInterp(float DeltaTime)
{
	if (!bInterping) return;

	if (Character && ItemZCurve)
	{
		// Time passed when we start timer
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);

		// Get curve value corresponding to elapsed time
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

		// Get the item's initial location when the curve started
		FVector ItemLocation = ItemInterpStartLocation;

		// Get location in front of the camera
		const FVector CameraInterpLocation {Character->GetCameraInterpLocation()};

		// Vector from Item to Camera Interp Location (only z)
		const FVector ItemToCamera {FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z)};

		// Scale factor to multiply with Curve Value
		const float DeltaZ = ItemToCamera.Size();
		
		// Interpolated X and Y values 
		const FVector CurrentLocation = GetActorLocation();
		const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.0f);
		const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.0f);

		// Set X and Y of item location to interp values
		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;

		// Adding curve value to the Z component of the initial location (scaled by DeltaZ)
		ItemLocation.Z += CurveValue * DeltaZ;
		
		// Update items location according to the curve scaled by deltaZ
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		// Camera rotation this frame
		const FRotator CameraRotation {Character->GetFollowCamera()->GetComponentRotation()};

		// Camera rotation plus initial yaw offset
		FRotator ItemRotation {0.f, CameraRotation.Yaw + InterpInitialYawOffset, 0.f};

		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
		
	}
}

