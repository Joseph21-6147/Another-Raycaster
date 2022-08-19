// Make Your Own Ray Caster - episode 2
// ====================================

// Youtube: https://youtu.be/PC1RaETIx3Y
// Idea and video by 3DSage - thanks!!

// Port to olc::PixelGameEngine
// By Joseph21, august 19, 2022

// Dependencies:
//   *  olcPixelGameEngine.h - (olc::PixelGameEngine header file) by JavidX9 (see: https://github.com/OneLoneCoder/olcPixelGameEngine)


/* Short description
   -----------------
   This is the second episode of the 3DSage Raycaster series. It builds upon the first episode, and the code is the final result of
   following the video. For better recognition the code is intentionally kept close to the original code by 3DSage. However, where he
   implemented in openGL, I implemented on the PixelGameEngine (PGE).

   The code contains:
     * a refactoring that was done in between episode 1 and 2 (and was not part of the video)
     * wall texturing
     * definition and "opening" of doors
     * floor texturing
     * ceiling texturing

   For other raycasting introductions I'd suggest to check on the following video's by JavidX9:

       FPS part 1 - https://youtu.be/xW8skO7MFYw
       FPS part 2 - https://youtu.be/HEb2akswCcw
       DDA video  - https://youtu.be/NbSee-XM7WA

    Or the implementation of the Permadi tutorial on the olc::PixelGameEngine by Joseph21:

       https://github.com/Joseph21-6147/Raycasting-tutorial-series

    Have fun!
 */

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

// ====================   Constants   ==============================

#define PI  3.1415926535f
#define P2  1.0f * PI / 2.0f
#define P3  3.0f * PI / 2.0f

#define DR  0.0174533f    // one degree in radians

#define MOVE_SPEED     50.0f
#define MOVE_FRACTION   0.1f

// you can define multiple colours for multiple wall types
#define EMPTY     '.'
#define WALL_CBRD 'C'
#define WALL_BRCK 'B'
#define WINDOW    'W'
#define DOOR      'D'

// default colours for sky and floor
#define SKY_COL     olc::CYAN
#define FLR_COL     olc::BLUE

// margin used in bIsEqual() (and checking signedness of sin() and cos() results)
#define EPSILON  0.00001f

// ====================   Hard coded textures   ==============================

// all 32x32 textures [ 0 = black, 1 = white ]
int All_Textures[] = {

    // Checkerboard
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,1,1,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,1,1,1,1,1,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,1,1,1,1,1,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,1,1,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,

    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,

    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,

    // Brick
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,

    // Window
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,

    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,

    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,

    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,

    // Door
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,1,1,1,1,0,0,0,
    0,0,0,1,0,0,0,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,0,0,0,1,0,0,0,
    0,0,0,1,0,0,0,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,0,0,0,1,0,0,0,
    0,0,0,1,0,0,0,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,0,0,0,1,0,0,0,

    0,0,0,1,0,0,0,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,0,0,0,1,0,0,0,
    0,0,0,1,0,0,0,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,0,0,0,1,0,0,0,
    0,0,0,1,0,0,0,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,0,0,0,1,0,0,0,
    0,0,0,1,0,0,0,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,0,0,0,1,0,0,0,
    0,0,0,1,0,0,0,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,0,0,0,1,0,0,0,
    0,0,0,1,0,0,0,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,0,0,0,1,0,0,0,
    0,0,0,1,0,0,0,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,0,0,0,1,0,0,0,
    0,0,0,1,1,1,1,1, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 1,1,1,1,1,0,0,0,

    0,0,0,0,0,0,0,0, 0,0,0,0,0,1,0,1, 1,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,1,1,1,1,0,1, 1,0,1,1,1,1,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,

    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,
};

// ====================   PGE derived class AnotherRayCaster   ==============================

class AnotherRayCaster : public olc::PixelGameEngine {

public:
    AnotherRayCaster() {
        sAppName = "3DSage's RayCaster (episode 2) - by Joseph21";
    }

private:
    // -----   PLAYER   -----

    float px, py, pa;    // player position and angle
    float pdx, pdy;      // displacement depending on angle and speed

    float degToRad( float angle ) { return angle * PI / 180.0f; }
    float FixAng( float angle ) {
        if (angle >= 360.0f) { angle -= 360.0f; }
        if (angle < 0.0f) { angle += 360.0f; }
        return angle;
    }

    // draws the player as a tiny square in the map, including a directional line segment
    void drawPlayer() {
        // display little yellow square of 8x8 pixels
        FillRect( px - 4, py - 4, 8, 8, olc::YELLOW );
        // show the looking direction using a little line segment
        DrawLine( px, py, px + pdx * 20.0f, py + pdy * 20.0f, olc::YELLOW );
    }

    // -----   MAP   -----

    int mapX = 8, mapY = 8, mapS = 64;   // grid is 8x8 tiles, each tile is 64x64 in size
    std::string mapW;                    // i'm using a string implementation of the map
    std::string mapF;                    // there's a map for the walls, the floor and the ceiling
    std::string mapC;

    // draws the map with different colouring per wall type
    void drawMap2D() {
        for (int y = 0; y < mapY; y++) {
            for (int x = 0; x < mapX; x++) {
                // filled tiles/walls are white, empty spaces are black
                olc::Pixel tileColour;
                switch (mapW[y * mapX + x]) {
                    case WALL_CBRD: tileColour = olc::WHITE;          break;
                    case WALL_BRCK: tileColour = olc::GREY;           break;
                    case WINDOW:    tileColour = olc::DARK_GREY;      break;
                    case DOOR:      tileColour = olc::VERY_DARK_GREY; break;
                    case EMPTY:     tileColour = olc::BLACK;          break;
                    default :       tileColour = olc::MAGENTA;
                }
                // leave a little space to get a grid layout map drawing
                FillRect(
                    x * mapS + 1,
                    y * mapS + 1,
                    mapS - 2,
                    mapS - 2,
                    tileColour
                );
            }
        }
    }

    // pythagoras distance
    float distance( float ax, float ay, float bx, float by, float ang ) {
        return cos( degToRad( ang )) * (bx - ax) - sin( degToRad( ang )) * (by - ay);
    }

    // comparator for floats - NOTE: 3DSage just used == operator for floats (in C)
    bool bIsEqual( float a, float b ) {
        return (abs( a - b ) < EPSILON);
    }

    // since I'm using string based map definitions instead of integer arrays, I need functions to convert
    // the map cell content into an integer value.

    int nWallIndex( char c ) {
        switch (c) {
            case EMPTY:     return -1;
            case WALL_CBRD: return  0;
            case WALL_BRCK: return  1;
            case WINDOW:    return  2;
            case DOOR:      return  3;
        }
        return -1;
    }

    int nFloorIndex( char c ) {
        switch (c) {
            case EMPTY:     return 0;
            case WALL_CBRD: return 1;
            case WALL_BRCK: return 2;
            case WINDOW:    return 3;
            case DOOR:      return 4;
        }
        return -1;
    }

    int nCeilIndex( char c ) { return nFloorIndex( c ); }

    // The DDA raycasting algorithm and the rendering of textures walls, floor and ceiling is all
    // handled in this method
    void drawRays2D() {

        int mx, my;                // map coordinates to find ...
        int mp;                    // ... coord of hit in the wall (if any)
        int dof;                   // depth of field - the max distance (in tiles) to check for hits

        float disH, disV, vx, vy;  // will contain distance and ray end point for the line checks

        float rx, ry, ra;          // ray's end point coord and angle
        float xo, yo;              // x and y offsets to get to next line or column of map

        // ----- draw background - blue floor, cyan sky -----

        // the right pane (3d view of the raycaster is at points (530, 0) upper left
        // and (530 + # rays * ray width, 320) lower right
        FillRect( 530,   0, 60 * 8, 160, olc::CYAN );
        FillRect( 530, 160, 60 * 8, 160, olc::BLUE );

        // prepare iteration over field of view - one ray is cast for every degree
        ra = FixAng( pa + 30.0f );        // make sure ra is in [0, 2 * PI)

        // cast 60 rays
        for (int r = 0; r < 60; r++) {

            int vmt = 0, hmt = 0;  // vertical and horizontal map texture number

            // --- Check Vertical Grid Line Collisions ---
            //     ===================================

            dof = 0;
            // reset distance caching variables each iteration
            disV = 1000000;
            // we need tangent (not its inverse) for checking vertical lines (see Notes on part 1)
            float Tan = tan( degToRad( ra ));

                 if (cos( degToRad( ra )) >  EPSILON) { rx = (((int)px >> 6) << 6) + 64.0f   ; ry = (px - rx) * Tan + py; xo =  64; yo = -xo * Tan; }  // ray pointing to right
            else if (cos( degToRad( ra )) < -EPSILON) { rx = (((int)px >> 6) << 6) -  0.0001f; ry = (px - rx) * Tan + py; xo = -64; yo = -xo * Tan; }  // ray pointing to left
            else { rx = px; ry = py; dof = 8; }                                                                                                        // ray vertical

            // walk the horizontal grid lines, converting the ray's end point (world coordinates) into
            // tile coordinates and checking if a wall was hit
            while (dof < 8) {
                // take the rays hit position, divide by 64, use that to find position in map array
                mx = int(rx) >> 6;
                my = int(ry) >> 6;
                mp = my * mapX + mx;
                // if the index is within the map, check if there's a wall there
                if (mp >= 0 && mp < mapX * mapY && mapW[mp] != EMPTY) {   // hit wall
                    vmt = nWallIndex( mapW[mp] );
                    dof = 8;                                   // set dof to 8 to end while loop
                    disV = distance( px, py, rx, ry, ra );     // store info to compare shortest hit length
                } else {  // no hit and dof < 8 --> check next line
                    rx += xo;
                    ry += yo;
                    dof += 1;
                }
            }
            vx = rx;
            vy = ry;

            // --- Check Horizontal Grid Line collisions ---
            //     =====================================

            dof = 0;
            // reset distance caching variables each iteration
            disH = 1000000;
            // we need inverse of tangent for the algo to check horizontal lines (see Notes on part 1)
            Tan = 1.0f / Tan;

                 if (sin( degToRad( ra )) >  EPSILON) { ry = (((int)py >> 6) << 6) -  0.0001f; rx = (py - ry) * Tan + px; yo = -64; xo = -yo * Tan; } // ray pointing up
            else if (sin( degToRad( ra )) < -EPSILON) { ry = (((int)py >> 6) << 6) + 64.0f   ; rx = (py - ry) * Tan + px; yo =  64; xo = -yo * Tan; } // ray pointing down
            else { rx = px; ry = py; dof = 8; }                                                                                                       // ray is horizontal

            // walk the vertical grid lines, converting the ray's end point (world coordinates) into
            // tile coordinates and checking if a wall was hit
            while (dof < 8) {
                // take the rays hit position, divide by 64, use that to find position in map array
                mx = int(rx) >> 6;
                my = int(ry) >> 6;
                mp = my * mapX + mx;
                // if the index is within the map, check if there's a wall there
                if (mp >= 0 && mp < mapX * mapY && mapW[mp] != EMPTY) {   // hit wall
                    hmt = nWallIndex( mapW[mp] );
                    dof = 8;                                   // set dof to 8 to end while loop
                    disH = distance( px, py, rx, ry, ra );     // store info to compare shortest hit length
                } else {  // no hit and dof < 8 --> check next line
                    rx += xo;
                    ry += yo;
                    dof += 1;
                }
            }

            float fShadeFactor;
            bool bHitHorizontal;
            // select shortest distance and determine shading factor and hit type (in rc)
            if (disV < disH) {      // vertical   wall hit
                rx = vx;
                ry = vy;
                disH = disV;
                hmt = vmt;
                fShadeFactor = 0.5f;
                bHitHorizontal = false;
            } else {                // horizontal wall hit
                fShadeFactor = 1.0f;
                bHitHorizontal = true;
            }

            olc::Pixel shadeCol = olc::GREEN;
            // show the ray as a coloured line
            DrawLine( px, py, rx, ry, shadeCol * fShadeFactor );

            // ----- Draw 3D walls -----
            //       =============

            float ca = FixAng( pa - ra );                        // fix fish eye distortion
            disH *= cos( degToRad( ca ));

            float lineH = (mapS * 320) / disH;

            // calculate step formula before line height is capped
            float ty_step = 32.0f / (float)lineH;
            float ty_off = 0.0f;

            if (lineH > 320.0f) { ty_off = (lineH - 320) / 2.0f; lineH = 320.0f; }      // cap line height at 320
            float lineOff = 160 - lineH / 2.0f;          // offset from top of screen

            // put slice on screen - textured rendering
            float ty = ty_off * ty_step + hmt * 32;
            float tx;
            if (bHitHorizontal) {
                tx = int(rx / 2.0f) % 32; if (ra > 180           ) { tx = 31 - tx; } // south textures are mirrored, flip them
            } else {
                tx = int(ry / 2.0f) % 32; if (ra > 90 && ra < 270) { tx = 31 - tx; } // west textures are mirrored, flip them
            }

            for (int y = 0; y < lineH; y++) {
                float c = All_Textures[int(ty) * 32 + int(tx)];
                shadeCol = olc::PixelF( c, c, c );

                switch (hmt) {
                    case 0 : shadeCol = olc::PixelF( c       , c / 2.0f, c / 2.0f ); break;   // checkerboard red
                    case 1 : shadeCol = olc::PixelF( c       , c       , c / 2.0f ); break;   // brick yellow
                    case 2 : shadeCol = olc::PixelF( c / 2.0f, c / 2.0f, c        ); break;   // window blue
                    case 3 : shadeCol = olc::PixelF( c / 2.0f, c       , c / 2.0f ); break;   // door green
                    default: shadeCol = olc::WHITE;
                }
                // draw "pixels" as 8x8 squares
                FillRect( 530 + r * 8, lineOff + y, 8, 1, shadeCol * fShadeFactor );
                ty += ty_step;
            }

            // ----- Draw Floor -----
            //       ==========

            // work your way down from bottom of wall
            for (int y = lineOff + lineH; y < 320; y++) {
                float dy = y - (320 / 2.0f), deg = degToRad( ra ), raFix = cos(degToRad( FixAng( pa - ra )));
                // determine texture coordinates
                tx = px / 2.0f + cos( deg ) * 158 * 32 / dy / raFix;
                ty = py / 2.0f - sin( deg ) * 158 * 32 / dy / raFix;
                // choose different floor textures
                int nmp = nFloorIndex( mapF[ int( ty / 32.0f ) * mapX + int( tx / 32.0f ) ] ) * 32 * 32;
                // sample floor
                float c = All_Textures[ (int( ty ) & 31) * 32 + (int( tx ) & 31) + nmp ] * 0.7f;   // make floor a little darker
                shadeCol = olc::PixelF( c / 1.3f, c / 1.3f, c );
                // draw "pixels" as 8x8 squares
                FillRect( 530 + r * 8, y, 8, 1, shadeCol );

                // ----- Draw ceiling -----
                //       ============

                nmp = nCeilIndex( mapC[ int( ty / 32.0f ) * mapX + int( tx / 32.0f ) ] ) * 32 * 32;
                // sample floor
                c = All_Textures[ (int( ty ) & 31) * 32 + (int( tx ) & 31) + nmp ] * 0.7f;   // make floor a little darker
                shadeCol = olc::PixelF( c / 2.0f, c / 1.2f, c / 2.0f );
                // draw "pixels" as 8x8 squares
                FillRect( 530 + r * 8, 320 - y, 8, 1, shadeCol );
            }

            // ----- End drawing -----
            //       ===========

            // prepare for next iteration
            ra = FixAng( ra - 1 );                         // make sure ra is in [0, 2 * PI)
        }
    }

public:
    bool OnUserCreate() override {

        // init player position
        px = 300.0f;
        py = 300.0f;
        pa = 0.0f;
        pdx =  cos( degToRad( pa ));
        pdy = -sin( degToRad( pa ));

        // init game world / wall map - sizes must match mapX and mapY !!
        mapW += "CCCCCWCC";
        mapW += "C..C...C";
        mapW += "C..D.B.C";
        mapW += "CCDC...C";
        mapW += "B......C";
        mapW += "B....C.C";
        mapW += "B......C";
        mapW += "CCWCWCWC";

        // init floor map - sizes must match mapX and mapY !!
        mapF += "........";
        mapF += "....CC..";
        mapF += "....B...";
        mapF += "........";
        mapF += "..B.....";
        mapF += "........";
        mapF += ".CCCC...";
        mapF += "........";

        // init ceiling map - sizes must match mapX and mapY !!
        mapC += "........";
        mapC += "........";
        mapC += "........";
        mapC += "......C.";
        mapC += ".CWC....";
        mapC += "........";
        mapC += "........";
        mapC += "........";

        return true;
    }

    bool OnUserUpdate( float fElapsedTime ) override {

        // slow rotation or movement down if shift is held
        float suf = GetKey( olc::Key::SHIFT ).bHeld ? 1.0f : 10.0f;

        if (GetKey( olc::Key::A ).bHeld) { pa += 20.0f * fElapsedTime * suf; pa = FixAng( pa ); pdx = cos( degToRad( pa )); pdy = -sin( degToRad( pa )); }
        if (GetKey( olc::Key::D ).bHeld) { pa -= 20.0f * fElapsedTime * suf; pa = FixAng( pa ); pdx = cos( degToRad( pa )); pdy = -sin( degToRad( pa )); }

        // calculate index within the map right in front and right behind player
        // the player will retain a distance of 20 / 64th of a tile from the walls
        int xo = 20 * (pdx < 0.0f ? -1 : +1);
        int yo = 20 * (pdy < 0.0f ? -1 : +1);
        int ipx        =  px       / 64.0f;
        int ipx_add_xo = (px + xo) / 64.0f;
        int ipx_sub_xo = (px - xo) / 64.0f;
        int ipy        =  py       / 64.0f;
        int ipy_add_yo = (py + yo) / 64.0f;
        int ipy_sub_yo = (py - yo) / 64.0f;
        // do movement with collision detection - you can slide along the walls since x and y
        // direction are CD'ed separately.
        if (GetKey( olc::Key::W ).bHeld) {
            if (mapW[ipy        * mapX + ipx_add_xo] == EMPTY) { px += pdx * 20.0f * fElapsedTime * suf; }
            if (mapW[ipy_add_yo * mapX + ipx       ] == EMPTY) { py += pdy * 20.0f * fElapsedTime * suf; }
        }
        if (GetKey( olc::Key::S ).bHeld) {
            if (mapW[ipy        * mapX + ipx_sub_xo] == EMPTY) { px -= pdx * 20.0f * fElapsedTime * suf; }
            if (mapW[ipy_sub_yo * mapX + ipx       ] == EMPTY) { py -= pdy * 20.0f * fElapsedTime * suf; }
        }

        if (GetKey( olc::Key::E ).bPressed) {
            int nxo = 0; if (pdx < 0) { nxo = -25; } else { nxo = 25; }
            int nyo = 0; if (pdy < 0) { nyo = -25; } else { nyo = 25; }
            int nipx_add_xo = (px + nxo) / 64.0f;
            int nipy_add_yo = (py + nyo) / 64.0f;
            if (mapW[ nipy_add_yo * mapX + nipx_add_xo ] == DOOR) { mapW[ nipy_add_yo * mapX + nipx_add_xo ] = EMPTY; }
        }

        // make background dark grey
        Clear( olc::VERY_DARK_GREY );

        drawMap2D();

        drawRays2D();

        drawPlayer();

        return true;
    }

    bool OnUserDestroy() override {

        // clean up code here
        return true;
    }
};

// ====================   main()   ==============================

// keep the screen dimensions constant and vary the resolution by adapting the pixel size
#define SCREEN_X   1024
#define SCREEN_Y    512
#define PIXEL_X       1
#define PIXEL_Y       1

int main()
{
	AnotherRayCaster demo;
	if (demo.Construct( SCREEN_X / PIXEL_X, SCREEN_Y / PIXEL_Y, PIXEL_X, PIXEL_Y ))
		demo.Start();

	return 0;
}


