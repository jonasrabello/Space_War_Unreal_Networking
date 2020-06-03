// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Weapon.h"
#include "ShipEngine.h"
#include "Shield.h"
#include "Durability.h"
#include "Camera/CameraComponent.h"

#include "SpaceShip.generated.h"



UCLASS()
class LEVELCONCEPT_API ASpaceShip : public ACharacter {
	GENERATED_BODY()

public:
	ASpaceShip(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* shipMesh;

	UPROPERTY(EditAnywhere)
		UCameraComponent* MainCamera;

	///Advanced Variable List
	UPROPERTY(Replicated)
		FVector currentVelocity;
	UPROPERTY(Replicated)
		FRotator currentRotation;
	
	FVector MovementDirection;
	FVector Location;
	FVector newLocation;
	FRotator newRotation;

	AWeapon* weapon;
	AShipEngine* engine;
	AShield* shield;
	ADurability* durability;


	///Primitive Variable List
	float acceleration = 0.0f;
	float rotationSpeed = 0.5f;
	float maxAcceleration = 5500.0f;
	float minAcceleration = -5500.0f;
	float altitudeAdjust = 0.0f;
	float altitudeRaiseValue = 200.0f;
	int health = 100;
	int shield_helath = 0;
	int damage = 0;
	float fire_rate = 1.0f;


	//Storage variables	
	float tempAcceleration;
	float tempMaxAcceleration;
	//copy deltaTime from Tick() -- will be used to adjust acceleration back to tempAcceleration
	float timeCopy;

	///Mechanic Method List
	//speed controls

	UFUNCTION(Server, Reliable, WithValidation)
		void ControlServer(FVector currentVelocity_, FRotator currentRotation_);
	void ControlServer_Implementation(FVector currentVelocity_, FRotator currentRotation_);
	bool ControlServer_Validate(FVector currentVelocity_, FRotator currentRotation_);

	void FlightControl(float deltaTime_);
	void StopShip();
	void Boost();
	void BoostRelease();
	void Creep();
	void CreepRelease();
	void ControlThrust(float thrustValue_);
	//rotation controls
	void Tilt(float tiltValue);
	void Turn(float turn);
	void TurnUp(float turnUp);
	void Altitude(float altitude);



	//Setters and Getters
	AShipEngine* GetEngine();
	void SetEngine(AShipEngine* engine_);
	AWeapon* GetWeapon();
	void SetWeapon(AWeapon* weapon_);
	AShield* GetShield();
	void SetShield(AShield* shield_);
	ADurability* GetDurability();
	void SetDurability(ADurability* durability_);
	//int GetHealth();
	//void SetHealth(int health_);
	//float GetRotationSpeed();
	//void SetRotationSpeed(float rotationSpeed_);
	//float GetAccelerationRate();
	//void SetAccelerationRate(float accelerationRate_);
	//float GetTopSpeed();
	//void SetTopSpeed(float topSpeed_);
	//int GetDamage();
	//int SetDamage(float damage_);
	//float GetFireRate();
	//void SetFireRate(float fireRate_);

};