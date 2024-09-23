#pragma once

struct FGameConstants
{
	static constexpr int32 ChunkSize = 16;
	static constexpr int32 ChunkGroupSize = 16;

	static constexpr int32 WorldHeight = 256;
	static constexpr int32 ChunksInZ = WorldHeight / ChunkSize;

	static constexpr int32 TextureAtlasMinSize = 16;

	static inline FString BlockMaterialPath = TEXT(
		"/Game/Material/M_TestTextureArray.M_TestTextureArray");

	static inline FString BasicColorMaterialPath = TEXT("/Game/Material/M_BasicColor.M_BasicColor");

	static constexpr int32 ScaleMultiplier = 100;

	static constexpr int16 DefaultLiveDistance = 12;
	static constexpr int16 DefaultLoD8Distance = 12;
	static constexpr int16 DefaultLoD4Distance = 12;
	static constexpr int16 DefaultLoD2Distance = 12;
	static constexpr int16 DefaultLoD1Distance = 12;
	static constexpr int16 DefaultUnloadedDistance = 13;

	static constexpr float ChunksManagerTickInterval = 1.0f;

	static constexpr int ChunkRenderingThreads = 24;

	static constexpr float InteractionDistance = 1000.f;

	static constexpr int CreateChunkPerTick = 10;
	static constexpr int RenderChunkPerTick = 10;
	static constexpr int CheckIsReadyToRenderPerLazyTick = 100;
};
