#if !defined(HANDMADE_H)
/* ------------------------------------------------
   $File: handmade.h
   $Date: 8/17/2015
   $Creator: Casey Muratori
   ------------------------------------------------ */

#include "handmade_platform.h"

/*
  STUDY:
  Cross-platform design 1: Virtualize the operating system out to the game.

  Abstract platform operations as virtual operating systems, but are implemented in platform-specific code. Use handles to structs that have different definitions based on the platform it's being compiled on.

  Casey doesn't like this approach because an OS needs a lot of flexibility but a game is much more specialized. So why should we virtualize a ton of OS functionality when it just adds complexity to our game?

  Cross-platform design 2: Virtualize the game as services to the platform.

  Write all the OS code as specific as it needs to be. Game gives OS graphics, sounds, user's input. OS sends network data for the game and handles file I/O. That's pretty much it. Prevents the platform layer from inheriting too much complexity from the OS. Reason this API design gives a lot of benefits is because there is large disparity between the flexibility between the services IN THIS CASE.

  SUMMARY: Maintain a strict boundary between platform-specific code and platform-independent code.
 */

/*
  STUDY: Semantic compression/Compression-oriented programming:
  Don't assume API works and call it, build API from the ground up.
  Write the usage code first, make sure it works, THEN elevate them into structures.
  If you're stuck, pound out some code. Eventually it will inform you of what to do next.
  Code is procedural by nature, complex object systems DO NOT CHANGE WHAT HAPPENS ON THE CPU.
*/

#define Minimum(A, B) ((A < B)? (A) : (B))
#define Maximum(A, B) ((A > B)? (A) : (B))

//
//
//
 

struct memory_arena
{
    memory_index Size;
    uint8 *Base;
    memory_index Used;
};

internal void
InitializeArena(memory_arena *Arena, memory_index Size, uint8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

// Macros so we can push any type we want >:D
#define PushStruct(Arena, type) (type *) PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *) PushSize_(Arena, (Count) * sizeof(type))
void*
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    return Result;
}

#include "handmade_math.h"
#include "handmade_intrinsics.h"
#include "handmade_world.h"

struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    uint32 *Pixels;
};

struct hero_bitmaps
{
    v2 Align;
     
    loaded_bitmap Body;
    loaded_bitmap InnerCoat;
    loaded_bitmap OuterCoat;
};

/* STUDY Day 052 : In OOP, virtual Update function on Entity. Then you
   have to call all virtual Update functions on all Entities that
   exist! The act of looping over them and calling virtual functions
   will get too slow at some point. The High/Low/Dormant structure
   solves this. Doesn't solve memory footprint problem.
 */

struct high_entity
{
    v2 P; // NOTE : Relative to the camera
    v2 dP;
    uint32 ChunkZ;
    uint32 FacingDirection;

    real32 tBob;
    
    real32 Z;
    real32 dZ;

    uint32 LowEntityIndex;
};

enum entity_type
{
    EntityType_Null,

    EntityType_Hero,
    EntityType_Wall,
    EntityType_Familiar,
    EntityType_Monstar,
};

#define HIT_POINT_SUB_COUNT 4
struct hit_point
{
    // TODO : Bake this down into one variable
    uint8 Flags;
    uint8 FilledAmount;
};

struct low_entity
{
    entity_type Type;
    
    world_position P;
    real32 Width, Height;

    // NOTE : This is for "stairs"
    bool32 Collides;
    int32 dAbsTileZ;

    uint32 HighEntityIndex;

    // TODO : Should hitpoints themselves be entities?
    uint32 HitPointMax;
    hit_point HitPoint[16];
};

struct entity
{
    uint32 LowIndex;
    low_entity *Low;
    high_entity *High;
};

struct entity_visible_piece
{
    loaded_bitmap *Bitmap;
    v2 Offset;
    real32 OffsetZ;
    real32 EntityZC;

    real32 R, G, B, A;
    v2 Dim;
};

struct entity_visible_piece_group
{
    uint32 PieceCount;
    entity_visible_piece Pieces[8];
};

struct game_state
{
    memory_arena WorldArena;
    world *World;

    // TODO: Should we allow split-screen?
    uint32 CameraFollowingEntityIndex;
    world_position CameraP;

    uint32 PlayerIndexForController[ArrayCount(((game_input *)0)->Controllers)];
    
    uint32 LowEntityCount;
    low_entity LowEntities[100000];

    uint32 HighEntityCount;
    high_entity HighEntities_[256];
    
    loaded_bitmap Backdrop;
    loaded_bitmap Shadow;
    hero_bitmaps HeroBitmaps[4];

    loaded_bitmap Tree;
};

#define HANDMADE_H
#endif
