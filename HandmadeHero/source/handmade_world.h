#if !defined(HANDMADE_WORLD_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

// TODO : Replace with v3
struct world_difference
{
    v2 dXY;
    real32 dZ;
};

/* STUDY : Day 056 At the point where we are removing tilemap stuff
 * and moving to entities only. Technically we can use a world_chunk
 * and an offset within the chunk to get an exact position. Seems
 * simple right? But what if we need positional information through a
 * reference? Then there's no way for the entity to know which
 * world_chunk it's in. The AbsTileX, Y, Z are the only way for it to
 * know which chunk it occupies. Is there a way to support indirection
 * while not storing this extra information?
 */
struct world_position
{
    // TODO : Puzzler! How can we get rid of chunk coordinates here,
    // and still allow reference to entities to be able to figure out
    // where they are (or rather, which world_chunk they are in?)
    
    // NOTE: Used to be tile chunk indexes. Now they are simply locations where nearby entities reside.
    int32 ChunkX;
    int32 ChunkY;
    int32 ChunkZ;

    // NOTE: These are offsets from the chunk center
    // NOTE : _ denotes that it shouldn't be something to change a lot
    v2 Offset_;
};

// TODO : Could make this just tile_chunk and then allow multiple tile_chunks per X/Y/Z
struct world_entity_block
{
    uint32 EntityCount;
    uint32 LowEntityIndex[16];
    world_entity_block *Next;
};

struct world_chunk
{
    int32 ChunkX;
    int32 ChunkY;
    int32 ChunkZ;

    /* STUDY : Day 057 As world entity blocks fill up, he doesn't know
     * whether a block copy would be too expensive. Since these would
     * only really fill up during world creation time and not play
     * time, it is probably fine to do block copies. Since querying
     * should be more frequent, should be better to not use pointers
     * to avoid unecessary memory lookups.
     */
    // TODO : Profile this and determine if a pointer would be better here!
    world_entity_block FirstBlock;
    
    // STUDY : External chaining
    world_chunk *NextInHash;
};

struct world
{
    real32 TileSideInMeters;
    real32 ChunkSideInMeters;
    
    world_entity_block *FirstFree;

    /* STUDY : Day 056 Two ways to implement: pointers or tile_chunk
     * has some notion of whether it's filled or not. Reason we
     * implemented the second version is to avoid double indirection,
     * pointer chasing. Problem is that the tile_entity_block inside
     * the tile_chunk structure can start ballooning and causing empty
     * slots to take up too much space.
     */
    // TODO : WorldChunkHash should probably switch to pointer IF
    // tile entity blocks continue to be stored en masse directly in the tile chunk!
    // NOTE : At the moment, this must be a power of 2
    world_chunk ChunkHash[4096];
};

#define HANDMADE_WORLD_H
#endif
