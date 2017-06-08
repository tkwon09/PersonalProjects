/* ------------------------------------------------
   $File: handmade.cpp
   $Date: 8/17/2015
   $Creator: Casey Muratori
   ------------------------------------------------ */

#include "handmade.h"
#include "handmade_world.cpp"

/*
  DirectSound uses a primary/secondary buffer. The secondary buffer is a ring-buffer. Since we are now on the platform-independent layer we will abandon the complexity of the ring-buffer and just write to a single buffer. Less efficient since we need to do a buffer copy, but the amount of data being processed is very small.
*/
internal void
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
#if 0
        real32 SineValue = sinf(GameState->tSine);
        int16 SampleValue = (int16) (SineValue * ToneVolume);
#else
        int16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
#if 0
        GameState->tSine += 2.0f * Pi32 / (real32) WavePeriod;
        if (GameState->tSine > 2.0f * Pi32)
        {
            GameState->tSine -= 2.0f * Pi32;
        }
#endif
    }
}

internal void
DrawRectangle(game_offscreen_buffer *Buffer, v2 vMin, v2 vMax, real32 R, real32 G, real32 B)
{
    int32 MinX = RoundReal32ToUInt32(vMin.X);
    int32 MinY = RoundReal32ToUInt32(vMin.Y);
    int32 MaxX = RoundReal32ToUInt32(vMax.X);
    int32 MaxY = RoundReal32ToUInt32(vMax.Y);

    if (MinX < 0)
    {
        MinX = 0;
    }

    if (MinY < 0)
    {
        MinY = 0;
    }

    if (MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }

    if (MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
     
    // Preadvance row pointer to the first row of the rectangle
    uint32 Int32R = (uint32) (RoundReal32ToUInt32(R * 255.0f) << 16);
    uint32 Int32G = (uint32) (RoundReal32ToUInt32(G * 255.0f) << 8);
    uint32 Int32B = (uint32) (RoundReal32ToUInt32(B * 255.0f) << 0);
    uint32 Color = Int32R | Int32G | Int32B;
     
    uint8 *Row = (uint8 *) Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch;
    for (int Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Pixel = (uint32 *) Row;
        for (int X = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Color;
        }

        Row += Buffer->Pitch;
    }
}

internal void
DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap,
           real32 RealX, real32 RealY, real32 CAlpha = 1.0f)
{
    
    int32 MinX = RoundReal32ToUInt32(RealX);
    int32 MinY = RoundReal32ToUInt32(RealY);
    int32 MaxX = MinX + Bitmap->Width;
    int32 MaxY = MinY + Bitmap->Height;

    int32 SourceOffsetX = 0;
    if (MinX < 0)
    {
        SourceOffsetX = -MinX;
        MinX = 0;
    }

    int32 SourceOffsetY = 0;
    if (MinY < 0)
    {
        SourceOffsetY = -MinY;
        MinY = 0;
    }

    if (MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }

    if (MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }

    uint32 *SourceRow = Bitmap->Pixels + Bitmap->Width * (Bitmap->Height - 1);
    SourceRow += -SourceOffsetY * Bitmap->Width + SourceOffsetX;
    uint8 *DestRow = (uint8 *) Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch;
    for (int Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Dest = (uint32 *) DestRow;
        uint32 *Source = SourceRow;
        for (int X = MinX; X < MaxX; ++X)
        {
            // Slow Linear Blend
            // C = (1 - t)A + tB
            // TODO: Someday need to talk about premultiplied alpha!
            real32 A = (real32) ((*Source >> 24) & 0xFF) / 255.0f;
            A *= CAlpha;
            real32 SR = (real32) ((*Source >> 16) & 0xFF);
            real32 SG = (real32) ((*Source >> 8) & 0xFF);
            real32 SB = (real32) (*Source & 0xFF);

            real32 DR = (real32)((*Dest >> 16) & 0xFF);
            real32 DG = (real32)((*Dest >> 8) & 0xFF);
            real32 DB = (real32)(*Dest & 0xFF);

            real32 R = (1.0f - A) * DR + A * SR;
            real32 G = (1.0f - A) * DG + A * SG;
            real32 B = (1.0f - A) * DB + A * SB;

            uint32 CR = (uint32) (R + 0.5f) << 16;
            uint32 CG = (uint32) (G + 0.5f) << 8;
            uint32 CB = (uint32) (B + 0.5f);

            *Dest = CR | CG | CB;
           
            ++Dest;
            ++Source;
        }
        DestRow += Buffer->Pitch;
        SourceRow -= Bitmap->Width;
    }
}

// Makes the stuff packed to a resolution of 1 byte.
#pragma pack(push, 1)
struct bitmap_header
{
    // Bitmap File Header
    uint16 FileType;
    uint32 FileSize;
    uint16 Reserved1;
    uint16 Reserved2;
    uint32 BitmapOffset;

    // DIB Header
    uint32 Size;
    int32 Width;
    int32 Height;
    uint16 Planes;
    uint16 BitsPerPixel;
    uint32 Compression;
    uint32 SizeOfBitmap;
    uint32 HorzResolution;
    uint32 VertResolution;
    uint32 ColorsUsed;
    uint32 ColorsImportant;

    // Below are for V4 and above only
    uint32 RedMask;
    uint32 GreenMask;
    uint32 BlueMask;
};
#pragma pack(pop)

internal loaded_bitmap
DEBUGLoadBMP(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *FileName)
{
    loaded_bitmap Result = {};
     
    debug_read_file_result ReadResult = ReadEntireFile(Thread, FileName);
    if (ReadResult.ContentsSize != 0)
    {
        bitmap_header *Header = (bitmap_header *) ReadResult.Contents;
      
        // Pixels assumed to be 4-byte aligned
        uint32 *Pixels = (uint32 *) ((uint8 *) ReadResult.Contents + Header->BitmapOffset);
        Result.Pixels = Pixels;
        Result.Width = Header->Width;
        Result.Height = Header->Height;

        // NOTE: if you are using this generically for some reason,
        // remember BMP files can be top-down, compression etc. etc.
        // this is not complete BMP loading code
      
        // NOTE: Byte order in memory is determined by the header, so
        // we have to read out the masks and convert the pixels
        // ourselves.

        // IMPORTANT: Casy is using BMP version 4 or above, but our photoshop exports only in BMP version 3
        // V4 = 108 bytes, channel order may vary
        // V3 = 40 bytes, channels should be AA RR GG BB using 32-bit
        // So technically the v4 code doesn't run with our assets.
        Assert(Header->Compression == 0);
        if (Header->Size == 108)
        {
            uint32 RedMask = Header->RedMask;
            uint32 GreenMask = Header->GreenMask;
            uint32 BlueMask = Header->BlueMask;
            uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);

            /* STUDY:
             * (BitmapColor & RedMask) -> produces just the 8 bits of red.
             * 0x xx RR GG BB <- how do we move it to the proper spot?
             *
             * We use the scans in order to get the correct shift.
             */
            bit_scan_result RedScan = FindLeastSignificantSetBit(RedMask);
            bit_scan_result GreenScan = FindLeastSignificantSetBit(GreenMask);
            bit_scan_result BlueScan = FindLeastSignificantSetBit(BlueMask);
            bit_scan_result AlphaScan = FindLeastSignificantSetBit(AlphaMask);
                        
            Assert(RedScan.Found);
            Assert(GreenScan.Found);
            Assert(BlueScan.Found);
            Assert(AlphaScan.Found);
      
            int32 RedShift = 16 - (int32) RedScan.Index;
            int32 GreenShift = 8 - (int32) GreenScan.Index;
            int32 BlueShift = 0 - (int32) BlueScan.Index;
            int32 AlphaShift = 24 - (int32) AlphaScan.Index;

            uint32 *SourceDest = Pixels;
            int32 IncrementY = 1;
            for (int32 Y = 0; Y < Result.Height; ++Y)
            {
                for (int32 X = 0; X < Result.Width; ++X)
                {
                    uint32 C = *SourceDest;

                    uint32 SwizeledAlpha = RotateLeft(C & AlphaMask, AlphaShift);
                    uint32 SwizeledRed = RotateLeft(C & RedMask, RedShift);
                    uint32 SwizeledGreen = RotateLeft(C & GreenMask, GreenShift);
                    uint32 SwizeledBlue = RotateLeft(C & BlueMask, BlueShift);
                    *SourceDest++ = SwizeledAlpha | SwizeledRed | SwizeledGreen | SwizeledBlue;
                }
            }
        }
        else
        {
            // Only supporting V3+ for now
            Assert(Header->Size == 40);
        }
    }
    return Result;
}

inline low_entity *
GetLowEntity(game_state *GameState, uint32 Index)
{
    low_entity *Result = 0;
    
    if ((Index > 0) && (Index < GameState->LowEntityCount))
    {
        Result = GameState->LowEntities + Index;
    }

    return Result;
}

inline v2
GetCameraSpaceP(game_state *GameState, low_entity *EntityLow)
{
    // NOTE : Map the entity into camera space
    world_difference Diff = Subtract(GameState->World, &EntityLow->P, &GameState->CameraP);
    v2 Result = Diff.dXY;

    return Result;
}

inline high_entity *
MakeEntityHighFrequency(game_state *GameState, low_entity *EntityLow, uint32 LowIndex, v2 CameraSpaceP)
{
    high_entity *EntityHigh = 0;

    Assert(EntityLow->HighEntityIndex == 0);
    if (EntityLow->HighEntityIndex == 0)
    {
        if (GameState->HighEntityCount < ArrayCount(GameState->HighEntities_))
        {
            uint32 HighIndex = GameState->HighEntityCount++;
            EntityHigh = GameState->HighEntities_ + HighIndex;

            EntityHigh->P = CameraSpaceP;
            EntityHigh->dP = V2(0, 0);
            EntityHigh->ChunkZ = EntityLow->P.ChunkZ;
            EntityHigh->FacingDirection = 0;
            EntityHigh->LowEntityIndex = LowIndex;

            EntityLow->HighEntityIndex = HighIndex;
        }
        else
        {
            InvalidCodePath;
        }
    }

    return EntityHigh;
}

inline high_entity *
MakeEntityHighFrequency(game_state *GameState, uint32 LowIndex)
{
    high_entity *EntityHigh = 0;
    
    low_entity *EntityLow = GameState->LowEntities + LowIndex;
    if (EntityLow->HighEntityIndex)
    {
        EntityHigh = GameState->HighEntities_ + EntityLow->HighEntityIndex;
            
    }
    else
    {   
        v2 CameraSpaceP = GetCameraSpaceP(GameState, EntityLow);
        EntityHigh = MakeEntityHighFrequency(GameState, EntityLow, LowIndex, CameraSpaceP);
    }

    return EntityHigh;
}

inline entity
ForceEntityIntoHigh(game_state *GameState, uint32 LowIndex)
{
    entity Result = {};

    if ((LowIndex > 0) && (LowIndex < GameState->LowEntityCount))
    {
        Result.LowIndex = LowIndex;
        Result.Low = GameState->LowEntities + LowIndex;
        Result.High = MakeEntityHighFrequency(GameState, LowIndex);
    }
    
    return Result;
}

inline void
MakeEntityLowFrequency(game_state * GameState, uint32 LowIndex)
{
    low_entity *EntityLow = &GameState->LowEntities[LowIndex];
    uint32 HighIndex = EntityLow->HighEntityIndex;
    if (HighIndex)
    {
        uint32 LastHighIndex = GameState->HighEntityCount - 1;
        
        // NOTE : Keep the high entity array compact, move last to deleted
        if (HighIndex != LastHighIndex)
        {

            high_entity *LastEntity = GameState->HighEntities_ + LastHighIndex;
            high_entity *DelEntity = GameState->HighEntities_ + HighIndex;

            *DelEntity = *LastEntity;            
            GameState->LowEntities[LastEntity->LowEntityIndex].HighEntityIndex = HighIndex;
        }
        --GameState->HighEntityCount;
        EntityLow->HighEntityIndex = 0;
    }
}

inline bool32
ValidateEntityPairs(game_state *GameState)
{
    bool32 Valid = true;
    for (uint32 HighEntityIndex = 1; HighEntityIndex < GameState->HighEntityCount; ++HighEntityIndex)
    {
        high_entity *High = GameState->HighEntities_ + HighEntityIndex;
        Valid = Valid && GameState->LowEntities[High->LowEntityIndex].HighEntityIndex == HighEntityIndex;        
    }

    return Valid;
}

inline void
OffsetAndCheckFrequencyByArea(game_state *GameState, v2 Offset, rectangle2 CameraBounds)
{
    
    for (uint32 HighEntityIndex = 1; HighEntityIndex < GameState->HighEntityCount;)
    {
        high_entity *High = GameState->HighEntities_ + HighEntityIndex; // NOTE : Casey accesses arrays like this to get a ref to pointer
        
        High->P += Offset;
        if (IsInRectangle(CameraBounds, High->P))
        {
            ++HighEntityIndex;
        }
        else
        {
            Assert(GameState->LowEntities[High->LowEntityIndex].HighEntityIndex == HighEntityIndex);
            MakeEntityLowFrequency(GameState, High->LowEntityIndex);
        }
    }
}

/* STUDY:
 * Day 046 Not overcomplicating things. No predesign, no
 * preOOP. The code is pretty much telling us what we need to do. When
 * something is clear that it needs to happen, we do it. For instance
 * we needed controllers and players that mapped together. So we just
 * made them. The code and the needs of the code will always guide
 * you. Ignoring the needs of the code forces you to shoehorn stuff
 * together. Let yourself get in the situation where you need
 * something before you write it.
 */

struct add_low_entity_result
{
    low_entity *Low;
    uint32 LowIndex;
};

internal add_low_entity_result
AddLowEntity(game_state *GameState, entity_type Type, world_position *P)
{
    Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities));
    uint32 EntityIndex = GameState->LowEntityCount++;

    low_entity *EntityLow = GameState->LowEntities + EntityIndex;
    *EntityLow = {};
    EntityLow->Type = Type;
    
    if (P)
    {
        EntityLow->P = *P;
        ChangeEntityLocation(&GameState->WorldArena, GameState->World, EntityIndex, 0, P);
    }

    add_low_entity_result Result;
    Result.Low = EntityLow;
    Result.LowIndex = EntityIndex;

    // TODO : Do we need to have a begin/end paradigm for adding entities
    // so that they can be brought into the high set when they are added
    // and are in the camera region?
    
    return Result;
}

internal add_low_entity_result
AddWall(game_state * GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Wall, &P);
    
    Entity.Low->P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    Entity.Low->Height = GameState->World->TileSideInMeters;
    Entity.Low->Width = Entity.Low->Height;
    Entity.Low->Collides = true;

    return Entity;
}

internal add_low_entity_result
AddPlayer(game_state * GameState)
{
    world_position P = GameState->CameraP;
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Hero, &P);

    Entity.Low->HitPointMax = 3;
    Entity.Low->HitPoint[2].FilledAmount = HIT_POINT_SUB_COUNT;
    Entity.Low->HitPoint[0] = Entity.Low->HitPoint[1] = Entity.Low->HitPoint[2];
        
    Entity.Low->Height = 0.5f; //1.4f;
    Entity.Low->Width = 1.0f;
    Entity.Low->Collides = true;
    
    if (GameState->CameraFollowingEntityIndex == 0)
    {
        GameState->CameraFollowingEntityIndex = Entity.LowIndex;
    }
    
    return Entity;
}

internal add_low_entity_result
AddMonstar(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Monstar, &P);

    Entity.Low->Height = 0.5f;
    Entity.Low->Width = 1.0f;
    Entity.Low->Collides = true;

    return Entity;
}

internal add_low_entity_result
AddFamiliar(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Familiar, &P);

    Entity.Low->Height = 0.5f;
    Entity.Low->Width = 1.0f;
    Entity.Low->Collides = false;
    
    return Entity;
}

internal bool32
TestWall(real32 WallX, real32 RelX, real32 RelY, real32 PlayerDeltaX, real32 PlayerDeltaY,
         real32 *tMin, real32 MinY, real32 MaxY)
{
    // TODO : Handle this better when implementing collision detection for real.
    bool32 Hit = false;

    real32 tEpsilon = 0.001f;
    if (PlayerDeltaX != 0.0f)
    {
        real32 tResult = (WallX - RelX) / PlayerDeltaX;
        real32 Y = RelY + tResult * PlayerDeltaY;
        if ((tResult >= 0.0f) && (*tMin > tResult))
        {
            if ((Y >= MinY) && (Y <= MaxY))
            {
                *tMin = Maximum(0.0f, tResult - tEpsilon);
                Hit = true;
            } 
        }
    }
    
    return Hit;
}
/* STUDY: We currently disallow the move that would
 * have pushed the hero into the wall. We can correct
 * the velocity, but the hero cannot move even if we
 * correct his velocity. What we need to do is see if
 * he WOULD have collided and then correct his
 * velocity and try to move him again. ANOTHER
 * PROBLEM: what if he collides then?
 *
 * Need to deal with the discreteness directly.
 *
 * Day 045:
 * One solution is to do a "Search in t"
 * You take small portions of the time loop and extrude
 * an area or volume to see if there was a collision
 * in the middle of the time step. Then you need to
 * take into account the momentum that propels the
 * object along the wall.
 *
 * Problem: It can be unbounded! What if you move
 * against a curved wall? Then you need to keep
 * accounting for the momentum and recalculating!
 *
 * In The Witness:
 * Instead of searching in time, search in position.
 * Take the location where the object would be. You
 * know the span of how far the object would have
 * been. Then, just search the cells in the span.
 * Find the closest point in each cell closest to
 * the destination point. 100% perfect, predictable.
 *
 * It also has the ability to ignore small edges if
 * you could have pathed there! (Unlike Unity...)
 *
 * Check tiles can move to, check if we can enter it,
 * check the closest point on the edges. Need to use
 * the flood fill pattern in order to prevent
 * tunneling through diagonal obstacles.
 *
 * What do we do about the guy not being a single
 * point? Can just account for the radius of the
 * ellipse representing the character's collision.
 * Can run into problems with irregular collision
 * shapes. Minkowski shape algebra can help.
 *
 * Negative: have to be able to search in p! Needs
 * to care about the surrounding topology. Doesn't
 * handle bounciness well.
 *
 * Day 047:
 * We will do search in t because of its simplicity.
 */

/* STUDY Day 050 Minkowski-based collision detection. Instead of
 * checking a shape of "radius" r against all others, if we shrank
 * it down to a point, what would we have to do to other shapes
 * with "radius" s so that it fits anywhere the original square
 * would? Answer: sweep r around the other s-length
 * shapes. G.J.K. algorithm allows this sort of Minkowski sum on
 * convex shapes for ray-casting and collision detection.
 *
 * For a rectangle, just add its radius to all other retangles to
 * check.
 */
internal void
MoveEntity(game_state *GameState, entity Entity, real32 dt, v2 ddP)
{
    world *World = GameState->World;

    real32 ddPLength = LengthSq(ddP);
    if (ddPLength > 1.0f)
    {
        ddP *= (1.0f / SquareRoot(ddPLength));
    }

    real32 PlayerSpeed = 50.0f;
    ddP *= PlayerSpeed;

    // TODO: This is friction. ODE here!
    ddP += -8.0f * Entity.High->dP;
    
    v2 OldPlayerP = Entity.High->P;
    v2 PlayerDelta = (0.5f * ddP * Square(dt)) + (Entity.High->dP * dt);
    
    Entity.High->dP = ddP * dt + Entity.High->dP;
    v2 NewPlayerP = OldPlayerP + PlayerDelta;
    
    // p(t) = p0 + t*d -> length of d is equal to total movement, t = 1. aka starts with max possible t
    // Do a bounded number of iterations to change trajectory
    for (uint32 Iteration = 0; Iteration < 4; ++Iteration)
    {        
        real32 tMin = 1.0f;
        v2 WallNormal = {};
        uint32 HitHighEntityIndex = 0;

        v2 DesiredPosition = Entity.High->P + PlayerDelta;

        for (uint32 TestHighEntityIndex = 1; TestHighEntityIndex < GameState->HighEntityCount; ++TestHighEntityIndex)
        {
            if (TestHighEntityIndex != Entity.Low->HighEntityIndex)
            {
                entity TestEntity;
                TestEntity.High = GameState->HighEntities_ + TestHighEntityIndex;
                TestEntity.LowIndex = TestEntity.High->LowEntityIndex;
                TestEntity.Low = GameState->LowEntities + TestEntity.LowIndex;
                if (TestEntity.Low->Collides)
                {
                    real32 DiameterW = TestEntity.Low->Width + Entity.Low->Width;
                    real32 DiameterH = TestEntity.Low->Height + Entity.Low->Height;

                    v2 MinCorner = -0.5f * V2(DiameterW, DiameterH);
                    v2 MaxCorner = 0.5f * V2(DiameterW, DiameterH);

                    v2 Rel = Entity.High->P - TestEntity.High->P;

                    // for each wall.. ts = (wx - p0x) / dx
                    if (TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMin, MinCorner.Y, MaxCorner.Y))
                    {
    WallNormal = V2(-1, 0);
    HitHighEntityIndex = TestHighEntityIndex;
}

                    if (TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMin, MinCorner.Y, MaxCorner.Y))
                    {
    WallNormal = V2(1, 0);
    HitHighEntityIndex = TestHighEntityIndex;
}

                    if (TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMin, MinCorner.X, MaxCorner.X))
                    {
    WallNormal = V2(0, -1);
    HitHighEntityIndex = TestHighEntityIndex;
}

                    if (TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMin, MinCorner.X, MaxCorner.X))
                    {
    WallNormal = V2(0, 1);
    HitHighEntityIndex = TestHighEntityIndex;
}
}
}
}
        
        Entity.High->P += tMin * PlayerDelta;
        if (HitHighEntityIndex)
        {
    Entity.High->dP = Entity.High->dP - 1*Inner(Entity.High->dP, WallNormal) * WallNormal;
    PlayerDelta = DesiredPosition - Entity.High->P;
    PlayerDelta = PlayerDelta - 1*Inner(PlayerDelta, WallNormal) * WallNormal;

    high_entity *HitHigh = GameState->HighEntities_ + HitHighEntityIndex;
    low_entity *HitLow = GameState->LowEntities + HitHigh->LowEntityIndex;
    // TODO : Stairs
    //Entity.High->AbsTileZ += HitLow->dAbsTileZ;
}
        else
        {
    break;
}
}

    // TODO : Change to using the acceleration vector
    if ((Entity.High->dP.X == 0.0f) && (Entity.High->dP.Y == 0.0f))
    {
        // NOTE : Leave the facing direction as it is.
    }
    else if (AbsoluteValue(Entity.High->dP.X) > AbsoluteValue(Entity.High->dP.Y))
    {
        if (Entity.High->dP.X > 0)
        {
            Entity.High->FacingDirection = 0;
        }
        else
        {
            Entity.High->FacingDirection = 2;
        }
    }
    else
    {
        if (Entity.High->dP.Y > 0)
        {
            Entity.High->FacingDirection = 1;
        }
        else
        {
            Entity.High->FacingDirection = 3;
        }
    }

    world_position NewP = MapIntoChunkSpace(GameState->World, GameState->CameraP, Entity.High->P);

    // TODO : Bundle these together as the position update
    ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity.LowIndex, &Entity.Low->P, &NewP);
    Entity.Low->P = NewP;
}

internal void
SetCamera(game_state *GameState, world_position NewCameraP)
{
    world *World = GameState->World;

    Assert(ValidateEntityPairs(GameState));
    
    world_difference dCameraP = Subtract(World, &NewCameraP, &GameState->CameraP);
    GameState->CameraP = NewCameraP;

    // TODO : Totally picking these numbers arbitrarily!
    uint32 TileSpanX = 17 * 3;
    uint32 TileSpanY = 9 * 3;
    rectangle2 CameraBounds = RectCenterDim(V2(0, 0), World->TileSideInMeters * V2((real32) TileSpanX, (real32) TileSpanY));

    v2 EntityOffsetForFrame = -dCameraP.dXY;
    OffsetAndCheckFrequencyByArea(GameState, EntityOffsetForFrame, CameraBounds);

    Assert(ValidateEntityPairs(GameState));
    
    // TODO : Do this in terms of tile chunks!
    world_position MinChunkP = MapIntoChunkSpace(World, NewCameraP, GetMinCorner(CameraBounds));
    world_position MaxChunkP = MapIntoChunkSpace(World, NewCameraP, GetMaxCorner(CameraBounds));

    for (int32 ChunkY = MinChunkP.ChunkY; ChunkY <= MaxChunkP.ChunkY; ++ChunkY)
    {
        for (int32 ChunkX = MinChunkP.ChunkX; ChunkX <= MaxChunkP.ChunkX; ++ChunkX)
        {
            world_chunk *Chunk = GetWorldChunk(World, ChunkX, ChunkY, NewCameraP.ChunkZ);
            if (Chunk)
            {
                for (world_entity_block *Block = &Chunk->FirstBlock; Block; Block = Block->Next)
                {
                    for (uint32 EntityIndexIndex = 0; EntityIndexIndex < Block->EntityCount; ++EntityIndexIndex)
                    {
                        uint32 LowEntityIndex = Block->LowEntityIndex[EntityIndexIndex];
                        low_entity *Low = GameState->LowEntities + LowEntityIndex;                        
                        if (Low->HighEntityIndex == 0)
                        {
                            v2 CameraSpaceP = GetCameraSpaceP(GameState, Low);
                            if (IsInRectangle(CameraBounds, CameraSpaceP))
                            {
                                MakeEntityHighFrequency(GameState, Low, LowEntityIndex, CameraSpaceP);
                            }
                        }
                    }
                }
            }
        }
    }
    Assert(ValidateEntityPairs(GameState));
}

internal void
PushPiece(entity_visible_piece_group *Group, loaded_bitmap *Bitmap,
          v2 Offset, real32 OffsetZ, v2 Align, real32 Alpha = 1.0f, real32 EntityZC = 1.0f)
{
    Assert(Group->PieceCount < ArrayCount(Group->Pieces));
    entity_visible_piece *Piece = Group->Pieces + Group->PieceCount++;
    Piece->Bitmap = Bitmap;
    Piece->Offset = Offset - Align;
    Piece->OffsetZ = OffsetZ;
    Piece->A = Alpha;
    Piece->EntityZC = EntityZC;
}

inline entity
EntityFromHighIndex(game_state *GameState, uint32 HighEntityIndex)
{
    entity Result = {};

    /* STUDY : Day 059 Casey doesn't like the amount of addressing
     * we're doing here. Need profiling to see if it's bad.
     */
    if (HighEntityIndex)
    {
        Assert(HighEntityIndex < ArrayCount(GameState->HighEntities_));
        Result.High = GameState->HighEntities_ + HighEntityIndex;
        Result.LowIndex = Result.High->LowEntityIndex;
        Result.Low = GameState->LowEntities + Result.LowIndex;
    }

    return Result;
}
inline void
UpdateFamiliar(game_state *GameState, entity Entity, real32 dt)
{
    entity ClosestHero = {};
    real32 ClosestHeroDSq = Square(10.0f); // NOTE : 10 meter maximum search!
    for (uint32 HighEntityIndex = 1; HighEntityIndex < GameState->HighEntityCount; ++HighEntityIndex)
    {
        entity TestEntity = EntityFromHighIndex(GameState, HighEntityIndex);

        if (TestEntity.Low->Type == EntityType_Hero)
        {
            real32 TestDSq = LengthSq(TestEntity.High->P - Entity.High->P);
            if (ClosestHeroDSq > TestDSq)
            {
                ClosestHero = TestEntity;
                ClosestHeroDSq = TestDSq;
            }
        }
    }

    v2 ddP = {};
    if (ClosestHero.High && (ClosestHeroDSq > Square(3.0f)))
    {
        // TODO : PULL SPEED OUT OF MOVE ENTITY
        real32 Acceleration = 0.5f;
        real32 OneOverLength = Acceleration / SquareRoot(ClosestHeroDSq);
        ddP = OneOverLength * (ClosestHero.High->P - Entity.High->P);
    }
    
    MoveEntity(GameState, Entity, dt, ddP);
}

inline void
UpdateMonstar(game_state *GameState, entity Entity, real32 dt)
{
    
}

#include <stdlib.h>
// NOTE: Defining it using the handle
/* STUDY : foo
 */
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    // Check the terminator button - Base of the buttons array = total button count
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == ArrayCount(Input->Controllers[0].Buttons));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
     
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        // NOTE: Reserve entity slot 0 for the null entity
        AddLowEntity(GameState, EntityType_Null, 0);
        GameState->HighEntityCount = 1;
        
        // Assuming that program runs in data folder. Using debug read file function using function pointer.
        GameState->Backdrop = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_background.bmp");
        GameState->Shadow = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_shadow.bmp");
        GameState->Tree = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_tree.bmp");
        
        hero_bitmaps *Bitmap;
        Bitmap = GameState->HeroBitmaps;
      
        Bitmap->Body = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_right_body.bmp");
        Bitmap->InnerCoat = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_right_coat_inner.bmp");
        Bitmap->OuterCoat = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_right_coat_outer.bmp");
        Bitmap->Align = V2(23, 64);
        ++Bitmap;
      
        Bitmap->Body = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_back_body.bmp");
        Bitmap->InnerCoat = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_back_coat_inner.bmp");
        Bitmap->OuterCoat = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_back_coat_outer.bmp");
        Bitmap->Align = V2(24, 64);
        ++Bitmap;
      
        Bitmap->Body = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_left_body.bmp");
        Bitmap->InnerCoat = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_left_coat_inner.bmp");
        Bitmap->OuterCoat = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_left_coat_outer.bmp");
        Bitmap->Align = V2(25, 64);
        ++Bitmap;
      
        Bitmap->Body = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_front_body.bmp");
        Bitmap->InnerCoat = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_front_coat_inner.bmp");
        Bitmap->OuterCoat = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_front_coat_outer.bmp");
        Bitmap->Align = V2(25, 64);
        ++Bitmap;
      
        InitializeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state), (uint8 *) Memory->PermanentStorage + sizeof(game_state));
      
        GameState->World = PushStruct(&GameState->WorldArena, world);
        world * World = GameState->World;
        InitializeWorld(World, 1.4f);
        
        uint32 TilesPerWidth = 17;
        uint32 TilesPerHeight = 9;
        
        uint32 ScreenBaseX = 0;
        uint32 ScreenBaseY = 0;
        uint32 ScreenBaseZ = 0;
        uint32 ScreenX = ScreenBaseX;
        uint32 ScreenY = ScreenBaseY;
        uint32 AbsTileZ = ScreenBaseZ;

        // TODO: Replace all this with real world generation!
        bool32 DoorLeft = false;
        bool32 DoorRight = false;
        bool32 DoorTop = false;
        bool32 DoorBottom = false;
        bool32 DoorUp = false;
        bool32 DoorDown = false;
        for (uint32 ScreenIndex = 0; ScreenIndex < 1000; ++ScreenIndex)
        {
            // TODO: Random number generator!
            uint32 RandomChoice;
            //if (DoorUp || DoorDown)
            {
                RandomChoice = rand() % 2;
            }
#if 0
            else
            {
                RandomChoice = rand() % 3;
            }
#endif
            
            bool32 CreatedZDoor = false;
            if (RandomChoice == 2)
            {
                CreatedZDoor = true;
                if (AbsTileZ == ScreenBaseZ)
                {
                    DoorUp = true;
                }
                else
                {
                    DoorDown = true;
                }
            }
            else if (RandomChoice == 1)
            {
                DoorRight = true;
            }
            else 
            {
                DoorTop = true;
            }
           
            for (uint32 TileY = 0; TileY < TilesPerHeight; ++TileY)
            {
                for (uint32 TileX = 0; TileX < TilesPerWidth; ++TileX)
                {
                    uint32 AbsTileX = ScreenX * TilesPerWidth + TileX;
                    uint32 AbsTileY = ScreenY * TilesPerHeight + TileY;

                    uint32 TileValue = 1;
                    if (TileX == 0 && (!DoorLeft || TileY != (TilesPerHeight / 2)))
                    {
                        TileValue = 2;
                    }
            
                    if (TileX == (TilesPerWidth - 1) && (!DoorRight || TileY != (TilesPerHeight / 2)))
                    {
                        TileValue = 2;
                    }
             
                    if (TileY == 0 && (!DoorBottom || TileX != (TilesPerWidth / 2)))
                    {
                        TileValue = 2;
                    }
             
                    if (TileY == (TilesPerHeight - 1) && (!DoorTop || TileX != (TilesPerWidth / 2)))
                    {
                        TileValue = 2;
                    }

                    if (TileX == 10 && (TileY == 6))
                    {
                        if (DoorUp)
                        {
                            TileValue = 3;
                        }
                        if (DoorDown)
                        {
                            TileValue = 4;
                        }
                    }
                  
                    if (TileValue == 2)
                    {
                        AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);
                    }
                }
            }
            DoorLeft = DoorRight;
            DoorBottom = DoorTop;

            if (CreatedZDoor)
            {
                DoorDown = !DoorDown;
                DoorUp = !DoorUp;
            }
            else
            {
                DoorUp = false;
                DoorDown = false;
            }
           
            DoorRight = false;
            DoorTop = false;
           
            if (RandomChoice == 2)
            {
                if (AbsTileZ == ScreenBaseZ)
                {
                    AbsTileZ = ScreenBaseZ + 1;
                }
                else
                {
                    AbsTileZ = ScreenBaseZ;
                }
            
            }
            else if (RandomChoice == 1)
            {
                ScreenX += 1;
            }
            else
            {
                ScreenY += 1;
            }
        }

#if 0
        while (GameState->LowEntityCount < (ArrayCount(GameState->LowEntities) - 16))
        {
            uint32 Coordinate = 1024 + GameState->LowEntityCount;
            AddWall(GameState, Coordinate, Coordinate, Coordinate);
        }
#endif
        
        world_position NewCameraP = {};
        uint32 CameraTileX = ScreenBaseX * TilesPerWidth + 17 / 2;
        uint32 CameraTileY = ScreenBaseY * TilesPerWidth + 9 / 2;
        uint32 CameraTileZ = ScreenBaseZ * TilesPerWidth;
        NewCameraP = ChunkPositionFromTilePosition(GameState->World, CameraTileX, CameraTileY, CameraTileZ);

        AddMonstar(GameState, CameraTileX + 2, CameraTileY + 2, CameraTileZ);

        for (int FamiliarIndex = 0; FamiliarIndex < 1; ++FamiliarIndex)
        {
            int32 FamiliarOffsetX = (rand() % 14) - 7;
            int32 FamiliarOffsetY = (rand() % 6) - 3;
            if (FamiliarOffsetX != 0 || FamiliarOffsetY != 0)
            {
                AddFamiliar(GameState, CameraTileX + FamiliarOffsetX, CameraTileY + FamiliarOffsetY, CameraTileZ);
            }
        }

        SetCamera(GameState, NewCameraP);

        Memory->IsInitialized = true;
    }
    
    world *World = GameState->World;
     
    real32 TileSideInPixels = 60;
    real32 MetersToPixels = (real32) TileSideInPixels / World->TileSideInMeters;

    real32 LowerLeftX = -(real32) TileSideInPixels / 2;
    real32 LowerLeftY = (real32) Buffer->Height;

    //
    // NOTE: 
    //         
    for (int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        uint32 LowIndex = GameState->PlayerIndexForController[ControllerIndex];
        if (LowIndex == 0)
        {
            if (Controller->Start.EndedDown)
            {
                uint32 EntityIndex = AddPlayer(GameState).LowIndex;
                GameState->PlayerIndexForController[ControllerIndex] = EntityIndex;
            }  
        }
        else
        {
            entity ControllingEntity = ForceEntityIntoHigh(GameState, LowIndex);
            v2 ddP = {};

            if (Controller->IsAnalog)
            {
                // NOTE: Use analog movement tuning
                ddP = V2(Controller->StickAverageX, Controller->StickAverageY);
            }
            else
            {            
                // NOTE: Use digital movement tuning
                                
                if (Controller->MoveUp.EndedDown)
                {
                    ddP.Y += 1.0f;
                }
                if (Controller->MoveDown.EndedDown)
                {
                    ddP.Y += -1.0f;
                }
                if (Controller->MoveLeft.EndedDown)
                {
                    ddP.X += -1.0f;
                }
                if (Controller->MoveRight.EndedDown)
                {
                    ddP.X += 1.0f;
                }
            }

            if (Controller->ActionUp.EndedDown)
            {
                ControllingEntity.High->dZ = 3.0f;
            }
            
            MoveEntity(GameState, ControllingEntity, Input->dtForFrame, ddP);
        }
    }

    entity CameraFollowingEntity = ForceEntityIntoHigh(GameState, GameState->CameraFollowingEntityIndex);
    if (CameraFollowingEntity.High)
    {
        world_position NewCameraP = GameState->CameraP;
        
        NewCameraP.ChunkZ = CameraFollowingEntity.Low->P.ChunkZ;

#if 0
        if (CameraFollowingEntity.High->P.X > 8.5f * GameState->World->TileSideInMeters)
        {
            NewCameraP.ChunkX += 17;
        }
        if (CameraFollowingEntity.High->P.X <= -8.5f * GameState->World->TileSideInMeters)
        {
            NewCameraP.ChunkX -= 17;
        }
        if (CameraFollowingEntity.High->P.Y > 4.5f * GameState->World->TileSideInMeters)
        {
            NewCameraP.ChunkY += 9;
        }
        if (CameraFollowingEntity.High->P.Y <= -4.5f * GameState->World->TileSideInMeters)
        {
            NewCameraP.ChunkY -= 9;
        }
#else
        NewCameraP = CameraFollowingEntity.Low->P;
#endif
        
        // TODO : Map new entities in and old entities out!!!
        // TODO : Mapping tiles and stairs into the entity set!

        SetCamera(GameState, NewCameraP);
    }

    //
    // NOTE: Render
    // 
#if 1
    DrawRectangle(Buffer, V2(0, 0), V2((real32) Buffer->Width, (real32) Buffer->Height), 0.5f, 0.5f, 0.5f);
#else
    DrawBitmap(Buffer, &GameState->Backdrop, 0, 0);
#endif
    real32 ScreenCenterX = 0.5f * (real32) Buffer->Width;
    real32 ScreenCenterY = 0.5f * (real32) Buffer->Height;

    entity_visible_piece_group PieceGroup;
    for (uint32 HighEntityIndex = 1; HighEntityIndex < GameState->HighEntityCount; ++HighEntityIndex)
    {
        PieceGroup.PieceCount = 0;
        
        high_entity *HighEntity = GameState->HighEntities_ + HighEntityIndex;
        low_entity *LowEntity = GameState->LowEntities + HighEntity->LowEntityIndex;

        /* STUDY : Day 059 Starting to write updates for familiar and
         * enemy. This starting point seems good. Probably can pull
         * collision detection code into here and deal with physics
         * stuff that apply to all entities be done here too. We have
         * enough usage code in order to justify collapsing things
         * into a structure with a bitmap, offsets, and alpha. See
         * entity_visible_piece.
         */
        entity Entity;
        Entity.LowIndex = HighEntity->LowEntityIndex;
        Entity.Low = LowEntity;
        Entity.High = HighEntity;
//        UpdateEntity(GameState, Entity, dt);

        real32 dt = Input->dtForFrame;
        
        // TODO : This is incorrect, should be computed after update!!!!
        real32 ShadowAlpha = 1.0f - 0.5f * HighEntity->Z;
        if (ShadowAlpha < 0)
        {
            ShadowAlpha = 0.0f;
        }
        
        hero_bitmaps *HeroBitmaps = &GameState->HeroBitmaps[HighEntity->FacingDirection];
        
        switch(LowEntity->Type)
        {
            case EntityType_Hero:
            {
                // TODO : Z!!!
                PushPiece(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
                PushPiece(&PieceGroup, &HeroBitmaps->InnerCoat, V2(0, 0), 0, HeroBitmaps->Align);
                PushPiece(&PieceGroup, &HeroBitmaps->Body, V2(0, 0), 0, HeroBitmaps->Align);
                PushPiece(&PieceGroup, &HeroBitmaps->OuterCoat, V2(0, 0), 0, HeroBitmaps->Align);

                
            } break;
            
            case EntityType_Wall:
            {
                PushPiece(&PieceGroup, &GameState->Tree, V2(0, 0), 0, V2(24, 32));
            } break;

            case EntityType_Familiar:
            {
                UpdateFamiliar(GameState, Entity, dt);
                Entity.High->tBob += dt;
                if (Entity.High->tBob > (2.0 * Pi32))
                {
                    Entity.High->tBob -= (2.0 * Pi32);
                }
                real32 BobSin = Sin(2.0f * Entity.High->tBob);
                /* STUDY : Day 060 PushPiece is in pixel space, so
                 * down is up. The final renderer will handle meter
                 * space vs. pixel space better!
                 */
                PushPiece(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, (0.5f * ShadowAlpha) + 0.2f * BobSin, 0.0f);
                PushPiece(&PieceGroup, &HeroBitmaps->OuterCoat, V2(0, 0), 10.0f * BobSin, HeroBitmaps->Align);
            } break;

            case EntityType_Monstar:
            {
                UpdateMonstar(GameState, Entity, dt);
                PushPiece(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha);
                PushPiece(&PieceGroup, &HeroBitmaps->Body, V2(0, 0), 0, HeroBitmaps->Align);
            } break;

            default:
            {
                InvalidCodePath;
            } break;
        }
        
        real32 ddZ = -9.8f;
        HighEntity->Z = 0.5f * ddZ * Square(dt) + HighEntity->dZ * dt + HighEntity->Z;
        HighEntity->dZ = ddZ * dt + HighEntity->dZ;
        if (HighEntity->Z < 0)
        {
            HighEntity->Z = 0;
        }
        
        real32 EntityGroundPointX = ScreenCenterX + MetersToPixels * HighEntity->P.X;
        real32 EntityGroundPointY = ScreenCenterY - MetersToPixels * HighEntity->P.Y;
        real32 EntityZ = -MetersToPixels * HighEntity->Z;
#if 0
        v2 PlayerLeftTop = {PlayerGroundPointX - 0.5f * MetersToPixels * LowEntity->Width,
                            PlayerGroundPointY - 0.5f * MetersToPixels * LowEntity->Height};
        v2 EntityWidthHeight = {LowEntity->Width, LowEntity->Height};
        DrawRectangle(Buffer, PlayerLeftTop, PlayerLeftTop + 0.9f * MetersToPixels * EntityWidthHeight,
                      1.0f, 1.0f, 1.0f);
#endif
        
        for (uint32 PieceIndex = 0; PieceIndex < PieceGroup.PieceCount; ++PieceIndex)
        {
            entity_visible_piece *Piece = PieceGroup.Pieces + PieceIndex;
            v2 Center = {EntityGroundPointX + Piece->Offset.X,
                         EntityGroundPointY + Piece->Offset.Y + Piece->OffsetZ + Piece->EntityZC * EntityZ};
            if (Piece->Bitmap)
            {
                DrawBitmap(Buffer, Piece->Bitmap, Center.X, Center.Y, Piece->A);
            }
            else
            {
                v2 HalfDim = 0.5f * MetersToPixels * Piece->Dim;
                DrawRectangle(Buffer, Center - HalfDim, Center + HalfDim, Piece->R, Piece->G, Piece->B);
            }
        }
    }
}

// NOTE: Defining function use handle
// NOTE: extern "C" will prevent C++ name mangling (happens to overloaded and templatized functions)
// forces compiler to use C
extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 242);
}

/*
internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    Pitch - number added to pointer to beginning of a row to move it to next row
        Stride - same as above but for a point to the end of a row
     
                                             uint8 *Row = (uint8 *) Buffer->Memory; // it's a void * so need to cast it
    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32 *Pixel = (uint32 *) Row;
        for (int X = 0; X < Buffer->Width; ++X)
        {
            //MEMORY:    RR GG BB xx
            //LOADED IN: xx BB GG RR
            //WANTED:    xx RR GG BB
            //MEMORY:    BB GG RR xx

            uint8 Blue = (uint8) (X + BlueOffset);
            uint8 Green = (uint8) (Y + GreenOffset);
            *Pixel++ = ((Green << 16) | Blue);
        }
        Row += Buffer->Pitch;
    }
}
*/
