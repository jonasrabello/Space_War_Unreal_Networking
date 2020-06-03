// Fill out your copyright notice in the Description page of Project Settings.

#include "SpaceShip.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"

#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

#include "Engine.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASpaceShip::ASpaceShip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create root component to attach things to
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	//Create the camera component
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCameraComponent"));
	MainCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	//Create the mesh component
	shipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	shipMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	shipMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	// How to get set a mesh from directory
	shipMesh->SetStaticMesh((ConstructorHelpers::FObjectFinderOptional<UStaticMesh>
		(TEXT("/Game/StarterContent/Shapes/Kirsci_Mert_graybox_spaceship_2_Box002.Kirsci_Mert_graybox_spaceship_2_Box002"))).Get());

	//Component Heiarchy
	//Attach any components here
	MainCamera->SetupAttachment(RootComponent);		
	shipMesh->SetupAttachment(RootComponent);
}

void ASpaceShip::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpaceShip, currentVelocity);
	DOREPLIFETIME(ASpaceShip, currentRotation);
}

void ASpaceShip::ControlServer_Implementation(FVector currentVelocity_, FRotator currentRotation_) {

	currentVelocity = currentVelocity_;
	currentRotation = currentRotation_;
}

bool ASpaceShip::ControlServer_Validate(FVector currentVelocity_, FRotator currentRotation_) {
	return true;
}

// Called when the game starts or when spawned
void ASpaceShip::BeginPlay() {
	Super::BeginPlay();
}

// Called every frame
void ASpaceShip::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	timeCopy = DeltaTime;
	FlightControl(DeltaTime);
}

// Called to bind functionality to input
void ASpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	///Axis Binding List
	InputComponent->BindAxis("Thrust", this, &ASpaceShip::ControlThrust);
	InputComponent->BindAxis("Tilt", this, &ASpaceShip::Tilt);
	InputComponent->BindAxis("Turn", this, &ASpaceShip::Turn);
	InputComponent->BindAxis("TurnUp", this, &ASpaceShip::TurnUp);
	InputComponent->BindAxis("Altitude", this, &ASpaceShip::Altitude);
	InputComponent->BindAxis("ViewX", this, &ASpaceShip::AddControllerPitchInput);
	InputComponent->BindAxis("ViewY", this, &ASpaceShip::AddControllerYawInput);

	//Action binding list for speed control
	InputComponent->BindAction("StopShip", IE_Pressed, this, &ASpaceShip::StopShip);
	InputComponent->BindAction("Boost", IE_Pressed, this, &ASpaceShip::Boost);
	InputComponent->BindAction("Boost", IE_Released, this, &ASpaceShip::BoostRelease);
	InputComponent->BindAction("Creep", IE_Pressed, this, &ASpaceShip::Creep);
	InputComponent->BindAction("Creep", IE_Released, this, &ASpaceShip::CreepRelease);
}

void ASpaceShip::FlightControl(float deltaTime_) {

	//Move the ship as long as the velocity is not zero
	newLocation = GetActorLocation() + (currentVelocity * deltaTime_);
	SetActorLocation(newLocation);
	//rotate ship	
	SetActorRotation(currentRotation);
}

void ASpaceShip::ControlThrust(float ThrustValue) {
	//Create a vector for movement using the rotation
	MovementDirection = FRotationMatrix(currentRotation).GetScaledAxis(EAxis::X);
	//Ensure acceleration isn't going above max or below me
	if (acceleration <= maxAcceleration && acceleration >= minAcceleration) {
		//Keep the speed the same whichever way the camera is pointing
		currentVelocity.X = FMath::Clamp(MovementDirection.X, -1.0f, 1.0f) * acceleration;
		currentVelocity.Y = FMath::Clamp(MovementDirection.Y, -1.0f, 1.0f) *  acceleration;
		currentVelocity.Z = FMath::Clamp(MovementDirection.Z, -1.0f, 1.0f) *  acceleration;
		acceleration += (ThrustValue * 2);
	}
	//Resets acceleration to 1 below max/above min if it goes over/under 
	//Necessary otherwise at max/min speeds you'll lose the ability to accelerate/deccelerate
	if (acceleration > maxAcceleration) {
		//Keep the speed the same whichever way the camera is pointing
		currentVelocity.X = FMath::Clamp(MovementDirection.X, -1.0f, 1.0f) * acceleration;
		currentVelocity.Y = FMath::Clamp(MovementDirection.Y, -1.0f, 1.0f) * acceleration;
		currentVelocity.Z = FMath::Clamp(MovementDirection.Z, -1.0f, 1.0f) * acceleration;
		acceleration = (maxAcceleration - 1);
	}
	else if (acceleration < minAcceleration) {
		if (GEngine) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Acceleration: %f"), acceleration));
		}
		//Keep the speed the same whichever way the camera is pointing
		currentVelocity.X = FMath::Clamp(MovementDirection.X, -1.0f, 1.0f) * acceleration;
		currentVelocity.Y = FMath::Clamp(MovementDirection.Y, -1.0f, 1.0f) * acceleration;
		currentVelocity.Z = FMath::Clamp(MovementDirection.Z, -1.0f, 1.0f) * acceleration;
		acceleration = (minAcceleration + 1);
	}

	// Update Server
	if (Role < ROLE_Authority) {
		ControlServer(currentVelocity, currentRotation);
	}
}

void ASpaceShip::StopShip() {
	currentVelocity.X = 0.0f;
	currentVelocity.Y = 0.0f;
	currentVelocity.Z = 0.0f;
	acceleration = 0.0f;

	// Update Server
	if (Role < ROLE_Authority) {
		ControlServer(currentVelocity, currentRotation);
	}

}

void ASpaceShip::Boost() {
	tempAcceleration = acceleration;
	tempMaxAcceleration = maxAcceleration;
	acceleration *= 5;
	maxAcceleration *= 10;
	currentVelocity.X = 1.0f * acceleration;
	currentVelocity.Y = 1.0f * acceleration;
	currentVelocity.Z = 1.0f * acceleration;

	// Update Server
	if (Role < ROLE_Authority) {
		ControlServer(currentVelocity, currentRotation);
	}
}

void ASpaceShip::BoostRelease() {
	acceleration = tempAcceleration;
	maxAcceleration = tempMaxAcceleration;
	currentVelocity.X = 1.0f * acceleration;
	currentVelocity.Y = 1.0f * acceleration;
	currentVelocity.Z = 1.0f * acceleration;

	// Update Server
	if (Role < ROLE_Authority) {
		ControlServer(currentVelocity, currentRotation);
	}
}

void ASpaceShip::Creep() {
	tempAcceleration = acceleration;
	tempMaxAcceleration = maxAcceleration;
	acceleration /= 5;
	maxAcceleration /= 5;
	currentVelocity.X = 1.0f * acceleration;
	currentVelocity.Y = 1.0f * acceleration;
	currentVelocity.Z = 1.0f * acceleration;

	// Update Server
	if (Role < ROLE_Authority) {
		ControlServer(currentVelocity, currentRotation);
	}
}

void ASpaceShip::CreepRelease() {
	acceleration = tempAcceleration;
	maxAcceleration = tempMaxAcceleration;
	currentVelocity.X = 1.0f * acceleration;
	currentVelocity.Y = 1.0f * acceleration;
	currentVelocity.Z = 1.0f * acceleration;

	// Update Server
	if (Role < ROLE_Authority) {
		ControlServer(currentVelocity, currentRotation);
	}

	if (currentVelocity.X <= 0.0f || currentVelocity.Y <= 0.0f || currentVelocity.Z <= 0) {
		StopShip();
	}
}

void ASpaceShip::Tilt(float tiltValue) {
	currentRotation.Roll += rotationSpeed * tiltValue;

	// Update Server
	if (Role < ROLE_Authority) {
		ControlServer(currentVelocity, currentRotation);
	}
}

void ASpaceShip::Turn(float turn) {
	currentRotation.Yaw += rotationSpeed * turn;

	// Update Server
	if (Role < ROLE_Authority) {
		ControlServer(currentVelocity, currentRotation);
	}
}

void ASpaceShip::TurnUp(float turnUp) {
	currentRotation.Pitch += rotationSpeed * turnUp;

	// Update Server
	if (Role < ROLE_Authority) {
		ControlServer(currentVelocity, currentRotation);
	}
}

void ASpaceShip::Altitude(float altitude) {
	Location.Z += altitudeRaiseValue * altitude;
}

void ASpaceShip::SetEngine(AShipEngine* engine_) {
	engine = engine_;
	maxAcceleration = maxAcceleration + engine->AShipEngine::GetAdditionalAcceleration();
	minAcceleration = minAcceleration + engine->AShipEngine::GetAdditionalAcceleration() * -1.0f;
	rotationSpeed = rotationSpeed + engine->AShipEngine::GetAdditionalSpeed();
}

void ASpaceShip::SetWeapon(AWeapon* weapon_) {
	weapon = weapon_;
	damage = weapon->AWeapon::GetDamage();
}

void ASpaceShip::SetShield(AShield* shield_) {
	shield = shield_;
	shield_helath = shield->AShield::GetShieldHealth();
}

void ASpaceShip::SetDurability(ADurability* durability_) {
	durability = durability_;
	health = health + durability->ADurability::GetAdditionalHealth();
}

AShipEngine* ASpaceShip::GetEngine() {
	return engine;
}

AWeapon* ASpaceShip::GetWeapon() {
	return weapon;
}
AShield* ASpaceShip::GetShield() {
	return shield;
}
ADurability* ASpaceShip::GetDurability() {
	return durability;
}