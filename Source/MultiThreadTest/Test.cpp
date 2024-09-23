// Fill out your copyright notice in the Description page of Project Settings.


#include "Test.h"

#include "ChunkHelper.h"
#include "LoadChunkRunnable.h"
#include "WorldGenerator.h"
#include "WorldGenerator.h"
#include "Constants/GameConstants.h"


// Sets default values
ATest::ATest()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	LoadColumnQueue = MakeShared<TThreadSafeQueue<FIntVector2>>();
	CreateColumnQueue = MakeShared<TThreadSafeQueue<FChunkDataColumn>>();
}

// Called when the game starts or when spawned
void ATest::BeginPlay()
{
	Super::BeginPlay();

	const auto WorldGenerator = NewObject<UWorldGenerator>();

	const auto LoadChunkRunnablesAndThreads = FLoadChunkRunnable::Create(
		WorldGenerator, LoadColumnQueue, CreateColumnQueue, 24);
	LoadChunkRunnables = LoadChunkRunnablesAndThreads.Runnables;
	LoadChunkThreads = LoadChunkRunnablesAndThreads.Threads;

	const auto PlayerPos = FVector{0,0,0};
	const auto PlayerColPos = UChunkHelper::ToChunkPos(PlayerPos);

	const auto PosAroundPlayer = UChunkHelper::GetPositionsAround(
		PlayerColPos,
		FGameConstants::DefaultUnloadedDistance - 1
	);

	for (const auto& Pos : PosAroundPlayer)
	{
		LoadColumnQueue->Enqueue(Pos);
	}
}

// Called every frame
void ATest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(0, 0.1f, FColor::Blue, FString::Printf(TEXT("Count: %d"), Count));

	if (!CreateColumnQueue->IsEmpty())
	{
		CreateColumnQueue->Dequeue();
		Count++;
	}
}

