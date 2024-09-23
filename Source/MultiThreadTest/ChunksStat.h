#pragma once
#include "Stats/Stats.h"

#if !UE_BUILD_SHIPPING
DECLARE_STATS_GROUP(TEXT("Chunks"), STATGROUP_CHUNKS, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Spawn Column"), STAT_SpawnColumn, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Spawn Chunk"), STAT_SpawnChunk, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Generate Column"), STAT_GenerateChunk, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Generate Chunk Data"), STAT_GenerateChunkGen, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Gen Chunk Data XY"), STAT_GenerateChunkGenXY, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Set Data"), STAT_GenerateChunkSet, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Set Chunk Data"), STAT_SetChunkData, STATGROUP_CHUNKS);

DECLARE_CYCLE_STAT(TEXT("ChunkRegistry Register Chunk"), STAT_ChunkRegistryRegisterChunk,
                   STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("ChunkRegistry On Column Spawn"), STAT_ChunkRegistryColumnSpawn,
                   STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("ChunksManager On Column Spawn"), STAT_ChunksManagerColumnSpawn,
                   STATGROUP_CHUNKS);

DECLARE_CYCLE_STAT(TEXT("Render Chunk"), STAT_RenderChunk, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Chunk Live"), STAT_ChunkRenderLive, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Chunk LoD"), STAT_ChunkRenderLoD, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Render Chunk For Each Piece"), STAT_ChunkRenderForEachPiece,
                   STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Renderer Render Group"), STAT_RendererRenderGroup, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Render Chunk Confirm"), STAT_ChunkRenderConfirm, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Render Chunk Confirm LoD"), STAT_ChunkRenderConfirm_LoD, STATGROUP_CHUNKS);

DECLARE_CYCLE_STAT(TEXT("Chunks Manager Lazy Tick"), STAT_ChunkLazyTick, STATGROUP_CHUNKS);
DECLARE_CYCLE_STAT(TEXT("Chunks Manager Tick"), STAT_ChunkTick, STATGROUP_CHUNKS);


#endif
