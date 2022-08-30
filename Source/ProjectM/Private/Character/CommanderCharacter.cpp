// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CommanderCharacter.h"
#include "MassMoveToCommandSubsystem.h"
#include "MassFireProjectileTask.h"
#include "MassEntityQuery.h"
#include "MassProjectileDamageProcessor.h"
#include "MassCommonFragments.h"
#include "MassEnemyTargetFinderProcessor.h"
#include "MassAgentComponent.h"
#include "MassEntityView.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntitySpawnDataGeneratorBase.h"
#include "MassSpawnLocationProcessor.h"
#include "MassAgentSubsystem.h"

//----------------------------------------------------------------------//
//  UMassPlayerControllableCharacterTrait
//----------------------------------------------------------------------//
void UMassPlayerControllableCharacterTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddTag<FMassPlayerControllableCharacterTag>();
}

//----------------------------------------------------------------------//
//  ACommanderCharacter
//----------------------------------------------------------------------//
ACommanderCharacter::ACommanderCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACommanderCharacter::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	check(World);
	MoveToCommandSystem = UWorld::GetSubsystem<UMassMoveToCommandSubsystem>(World);
}

void ACommanderCharacter::SetMoveToCommand() const
{
	MoveToCommandSystem->SetMoveToCommandTarget(FVector(0.f, 0.f, 20.f), false); // TODO
}

void ACommanderCharacter::SpawnProjectile() const
{
	// TODO: don't hard-code
	static const float ForwardVectorMagnitude = 300.f;
	static const FVector ProjectileLocationOffset = FVector(0.f, 0.f, 150.f);
	static const float InitialVelocityMagnitude = 4000.0f;

	const UWorld* World = GetWorld();
	const FVector& ActorForward = GetActorForwardVector();
	const FVector ActorFeetLocation = GetActorLocation() - FVector(0.f, 0.f, GetRootComponent()->Bounds.BoxExtent.Z);
	const FVector SpawnLocation = ActorFeetLocation + ActorForward * ForwardVectorMagnitude + ProjectileLocationOffset;
	const FVector InitialVelocity = ActorForward * InitialVelocityMagnitude;
	::SpawnProjectile(World, SpawnLocation, GetActorQuat(), InitialVelocity, ProjectileEntityConfig);
}

void ChangePlayerEntityToSoliderEntity(const UWorld* World, const FMassEntityConfig& EntityConfig, const FTransform &Transform, UMassEntitySubsystem* EntitySubsystem, const int16 &PlayerHealth)
{
	UMassSpawnerSubsystem* SpawnerSystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(World);
	check(SpawnerSystem);

	// TODO: A bit hacky to get first actor here.
	const FMassEntityTemplate* EntityTemplate = EntityConfig.GetOrCreateEntityTemplate(*World->GetLevel(0)->Actors[0], *SpawnerSystem); // TODO: passing SpawnerSystem is a hack
	check(EntityTemplate->IsValid());

	FMassEntitySpawnDataGeneratorResult Result;
	Result.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();
	Result.SpawnData.InitializeAs<FMassTransformsSpawnData>();
	Result.NumEntities = 1;
	FMassTransformsSpawnData& Transforms = Result.SpawnData.GetMutable<FMassTransformsSpawnData>();

	Transforms.Transforms.Reserve(1);
	FTransform& SpawnDataTransform = Transforms.Transforms.AddDefaulted_GetRef();
	SpawnDataTransform = Transform;

	TArray<FMassEntityHandle> SpawnedEntities;
	SpawnerSystem->SpawnEntities(EntityTemplate->GetTemplateID(), Result.NumEntities, Result.SpawnData, Result.SpawnDataProcessor, SpawnedEntities);

	FMassHealthFragment* SpawnedEntityHealthFragment = EntitySubsystem->GetFragmentDataPtr<FMassHealthFragment>(SpawnedEntities[0]);
	check(SpawnedEntityHealthFragment);
	SpawnedEntityHealthFragment->Value = PlayerHealth;
}

void ACommanderCharacter::ChangePlayerToAISoldier()
{
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
	check(EntitySubsystem);

	UMassAgentComponent* AgentComponent = Cast<UMassAgentComponent>(GetComponentByClass(UMassAgentComponent::StaticClass()));
	check(AgentComponent);

	const FMassEntityHandle& PlayerEntityHandle = AgentComponent->GetEntityHandle();
	FMassEntityView PlayerEntityView(*EntitySubsystem, PlayerEntityHandle);
	FTransformFragment* PlayerEntityTransformFragment = PlayerEntityView.GetFragmentDataPtr<FTransformFragment>();
	FMassHealthFragment* PlayerEntityHealthFragment = PlayerEntityView.GetFragmentDataPtr<FMassHealthFragment>();

	check(PlayerEntityTransformFragment);
	check(PlayerEntityHealthFragment);

	ChangePlayerEntityToSoliderEntity(GetWorld(), SoldierEntityConfig, PlayerEntityTransformFragment->GetTransform(), EntitySubsystem, PlayerEntityHealthFragment->Value);
}

void ACommanderCharacter::DidDie_Implementation()
{
}

void ACommanderCharacter::InitializeFromMassSoldier(const int32 MassEntityIndex, const int32 MassEntitySerialNumber)
{
	FMassEntityHandle MassSoldierEntityToInitializeWith = FMassEntityHandle(MassEntityIndex, MassEntitySerialNumber);

	UMassAgentSubsystem* AgentSubsystem = UWorld::GetSubsystem<UMassAgentSubsystem>(GetWorld());
	check(AgentSubsystem);

	AgentSubsystem->GetOnMassAgentComponentEntityAssociated().AddLambda([this, MassSoldierEntityToInitializeWith](const UMassAgentComponent& AgentComponent)
	{
		if (&AgentComponent == Cast<UMassAgentComponent>(GetComponentByClass(UMassAgentComponent::StaticClass())))
		{
			InitializeFromMassSoldierInternal(MassSoldierEntityToInitializeWith);
		}
	});
}

void ACommanderCharacter::InitializeFromMassSoldierInternal(FMassEntityHandle MassSoldierEntityToInitializeWith)
{
	check(MassSoldierEntityToInitializeWith.IsSet());

	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
	check(EntitySubsystem);

	const FMassHealthFragment& SoldierHealthFragment = EntitySubsystem->GetFragmentDataChecked<FMassHealthFragment>(MassSoldierEntityToInitializeWith);
	int16 NewPlayerHealth = SoldierHealthFragment.Value;

	const FTransformFragment& SoldierTransformFragment = EntitySubsystem->GetFragmentDataChecked<FTransformFragment>(MassSoldierEntityToInitializeWith);
	FTransform NewPlayerTransform = SoldierTransformFragment.GetTransform();

	EntitySubsystem->DestroyEntity(MassSoldierEntityToInitializeWith);

	UMassAgentComponent* AgentComponent = Cast<UMassAgentComponent>(GetComponentByClass(UMassAgentComponent::StaticClass()));
	check(AgentComponent);

	const FMassEntityHandle& PlayerEntityHandle = AgentComponent->GetEntityHandle();

	FMassHealthFragment* PlayerEntityHealthFragment = EntitySubsystem->GetFragmentDataPtr<FMassHealthFragment>(PlayerEntityHandle);
	check(PlayerEntityHealthFragment);
	
	PlayerEntityHealthFragment->Value = NewPlayerHealth;
	NewPlayerTransform.SetLocation(NewPlayerTransform.GetLocation() + FVector(0.f, 0.f, RootComponent->Bounds.BoxExtent.Z));
	SetActorTransform(NewPlayerTransform);
}

bool ACommanderCharacter::IsPlayerOnTeam1()
{
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
	check(EntitySubsystem);

	UMassAgentComponent* AgentComponent = Cast<UMassAgentComponent>(GetComponentByClass(UMassAgentComponent::StaticClass()));
	check(AgentComponent);

	const FMassEntityHandle& PlayerEntityHandle = AgentComponent->GetEntityHandle();
	FMassEntityView PlayerEntityView(*EntitySubsystem, PlayerEntityHandle);

	FTeamMemberFragment* PlayerEntityTeamMemberFragment = PlayerEntityView.GetFragmentDataPtr<FTeamMemberFragment>();
	check(PlayerEntityTeamMemberFragment);

	return PlayerEntityTeamMemberFragment->IsOnTeam1;
}