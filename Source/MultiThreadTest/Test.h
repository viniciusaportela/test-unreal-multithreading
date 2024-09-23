// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThreadSafeQueue.h"
#include "GameFramework/Actor.h"
#include "Test.generated.h"

struct FChunkDataColumn;
class FLoadChunkRunnable;

UCLASS()
class MULTITHREADTEST_API ATest : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATest();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TSharedPtr<TThreadSafeQueue<FIntVector2>> LoadColumnQueue;
	
	/**
	 * List of chunks to create with their chunk data
	 */
	TSharedPtr<TThreadSafeQueue<FChunkDataColumn>> CreateColumnQueue;

	TArray<FLoadChunkRunnable*> LoadChunkRunnables;

	TArray<FRunnableThread*> LoadChunkThreads;
	
	int Count = 0;
};
