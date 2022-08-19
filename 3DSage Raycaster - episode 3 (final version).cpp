// Make Your Own Ray Caster - episode 3
// ====================================

// Youtube: https://youtu.be/w0Bm4IA-Ii8
// Idea and video by 3DSage - thanks!!

// Port to olc::PixelGameEngine
// By Joseph21, august 19, 2022

// Dependencies:
//   *  olcPixelGameEngine.h - (olc::PixelGameEngine header file) by JavidX9 (see: https://github.com/OneLoneCoder/olcPixelGameEngine)
//   *  Dungeons.ppm, sky.ppm, sprites.ppm - sprite files
//   *  screens.h, screen Title.cpp, screen Lost.cpp, screen Won.cpp - screen definition code files


/* Short description
   -----------------
   This is the third and final episode of the 3DSage Raycaster series. It builds upon the second episode, and the code is the final result of
   following the video. For better recognition the code is intentionally kept close to the original code by 3DSage. However, where he
   implemented in openGL, I implemented on the PixelGameEngine (PGE).

   The code contains:
     * more advanced textures and texturing code for walls, floor, ceiling, sky etc.
     * screens for startup (title), for win and for loose situations
     * game states
     * a way to render objects - both static (key, lamp) and dynamic (enemy)

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

#include "Textures/Dungeon.ppm"     // defines int All_Textures[]
#include "Textures/sky.ppm"         // defines int sky[]
#include "Textures/sprites.ppm"     // key, lamp, enemy, defined in int sprites[]
#include "screens.h"                // declares int won[], lost[] and title[]

// ====================   Constants   ==============================

#define PI 3.1415926535f
#define P2  1.0f * PI / 2.0f
#define P3  3.0f * PI / 2.0f

#define DR  0.0174533f    // one degree in radians

#define MOVE_SPEED     50.0f
#define MOVE_FRACTION   0.1f

// you can define multiple types for multiple wall types
#define EMPTY      '.'
#define WALL_CBRD  'C'
#define WALL_BRCK  'B'
#define WINDOW     'W'
#define DOOR       'D'
#define WALL_LGTH  'L'
#define WALL_END   'E'

#define FLOOR_WOOD '1'
#define FLOOR_BRCK '2'
#define FLOOR_1    '3'
#define FLOOR_2    '4'
#define FLOOR_3    '5'
#define FLOOR_4    '6'

#define CEIL_WOOD  '1'
#define CEIL_BRCK  '2'
#define CEIL_1     '3'
#define CEIL_2     '4'
#define CEIL_3     '5'
#define CEIL_4     '6'

// default colours for sky and floor
#define SKY_COL     olc::CYAN
#define FLR_COL     olc::BLUE

// margin used in bIsEqual() (and checking signedness of sin() and cos() results)
#define EPSILON  0.00001f

// determines rendering of the 3d view
#define FOV           60.0f
#define SLICE_DEG      0.5f
#define NR_SLICES      (FOV / SLICE_DEG)
#define SLICE_WIDTH    8

// determines rendering of the 2d map and size of the world
#define TILE_SIZE     64


// keep the screen dimensions constant and vary the resolution by adapting the pixel size
#define SCREEN_X    960
#define SCREEN_Y    640
#define PIXEL_X       1
#define PIXEL_Y       1

#define SCREEN_DELAY 2.0f

// ====================   PGE derived class AnotherRayCaster   ==============================

class AnotherRayCaster : public olc::PixelGameEngine {

public:
    AnotherRayCaster() {
        sAppName = "3DSage's RayCaster (episode 3) - by Joseph21";
    }

private:

    // -----   GAME STATE   -----

    int gameState = 0;
    float timer   = 0.0f;
    float fade    = 0.0f;

    // -----   PLAYER   -----

    float px, py, pa;    // player position and angle
    float pdx, pdy;      // displacement depending on angle and speed

    float degToRad( float angle ) { return angle * PI / 180.0f; }
    float FixAng( float angle ) {
        if (angle >= 360.0f) { angle -= 360.0f; }
        if (angle <    0.0f) { angle += 360.0f; }
        return angle;
    }

    // -----   OBJECTS   -----

    typedef struct {
        int type;     // static, key, enemy  [ could this be an enum type ? ]
        int state;    // on off [ could this be a bool ? ]
        int nMap;     // texture to show - index in the texture array ig
        float x, y, z;  // position [ could this be an flc::vi3d type ? ]

    } mySprite;
    mySprite sp[4];
    // depth buffer
    int depth[SCREEN_X / SLICE_WIDTH];

    void drawSprite( float fElapsedTime ) {
        // turn the sprite off if the players position is with a range of the center of the sprite
        // this has the effect of a "pick up"
        int nPUrange = 30;
        if (px < sp[0].x + nPUrange && px > sp[0].x - nPUrange &&
            py < sp[0].y + nPUrange && py > sp[0].y - nPUrange) {
            // pick up key object
            sp[0].state = 0;
        }
        // check on killed by enemy
        nPUrange = 30;
        if (px < sp[3].x + nPUrange && px > sp[3].x - nPUrange &&
            py < sp[3].y + nPUrange && py > sp[3].y - nPUrange) {
            gameState = 4;
        }

        // enemy attack
        int spx     = int(  sp[3].x       / mapS), spy     = int(  sp[3].y        / mapS);   // normal grid position
        int spx_add = int( (sp[3].x + 15) / mapS), spy_add = int( (sp[3].y + 15 ) / mapS);   // normal grid position plus  offset
        int spx_sub = int( (sp[3].x - 15) / mapS), spy_sub = int( (sp[3].y - 15 ) / mapS);   // normal grid position minus offset

        float fAttackSpeed = 15.0f;
        if (sp[3].x > px && mapW[ spy     * 8 + spx_sub ] == EMPTY) { sp[3].x -= fAttackSpeed * fElapsedTime; }   // if the player is to east of enemy, make enemy move to east
        if (sp[3].x < px && mapW[ spy     * 8 + spx_add ] == EMPTY) { sp[3].x += fAttackSpeed * fElapsedTime; }   // if the player is to west of enemy, make enemy move to west
        if (sp[3].y > py && mapW[ spy_sub * 8 + spx     ] == EMPTY) { sp[3].y -= fAttackSpeed * fElapsedTime; }   // if the player is to north of enemy, make enemy move to north
        if (sp[3].y < py && mapW[ spy_add * 8 + spx     ] == EMPTY) { sp[3].y += fAttackSpeed * fElapsedTime; }   // if the player is to south of enemy, make enemy move to south

        for (int s = 0; s < 4; s++) {
            // work out temp value of sprite position -/- player position
            float sx = sp[s].x - px;
            float sy = sp[s].y - py;
            float sz = sp[s].z;
            // rotate the sprite around the player - need players position with sine and cosine
            float CS = cos( degToRad( pa )), SN = sin( degToRad( pa ));
            float a = sy * CS + sx * SN;   // "rotation matrix"
            float b = sx * CS - sy * SN;
            sx = a;
            sy = b;

            // "multiply by a large constant and divide by our screen depth
            // reposition that in the center of our screen using half the screen width and height
            sx = (sx * 108.0f / sy) + ((SCREEN_X / SLICE_WIDTH) / 2);   // convert to screen x, y
            sy = (sz * 108.0f / sy) + ((SCREEN_Y / SLICE_WIDTH) / 2);

            // draw the sprite with index s
            int scale = 32 * 80 / b;   // scale according to z-depth
            // set limits to prevent scale too big
            scale = std::max( 0, std::min( SCREEN_X / SLICE_WIDTH, scale ));

            // texture
            float t_x = 0;
            float t_y = 31;
            float t_x_step = 31.5f / float( scale );   // height of texture divided by scale (rounding issue fixed)
            float t_y_step = 32.0f / float( scale );

            for (int x = sx - scale / 2; x < sx + scale / 2; x++) {
                t_y = 31;
                for (int y = 0; y < scale; y++) {
                    // draw "point" - only
                    //   * if the sprite is "on"
                    //   * if its on screen, and
                    //   * if it's closer than what's in z-buffer
                    if (sp[s].state == 1 && x > 0 && x < (SCREEN_X / SLICE_WIDTH) && b < depth[ x ]) {

                        // display the sprite using texture from sprites[]
                        int nPixel = (int( t_y ) * 32 + int( t_x )) * 3 + (sp[s].nMap * 32 * 32 * 3);

                        int red   = sprites[nPixel + 0];
                        int green = sprites[nPixel + 1];
                        int blue  = sprites[nPixel + 2];
                        // apply colour filtering on pure magenta
                        if (!(red == 255 && green == 0 && blue == 255)) {
                            FillRect( x * 8, (sy - y) * 8, 8, 8, olc::Pixel( red, green, blue ));
                        }
                        t_y -= t_y_step; if (t_y < 0) { t_y = 0; }
                    }
                }
                t_x += t_x_step;
            }
        }
    }

    // -----   MAP   -----

    int mapX = 8, mapY = 8, mapS = TILE_SIZE;   // grid is 8x8 tiles, each tile is mapS x mapS pixels
    std::string mapW;                    // i'm using a string implementation of the map
    std::string mapF;                    // there's a map for the walls, the floor and the ceiling
    std::string mapC;

    // pythagoras distance
    float distance( float ax, float ay, float bx, float by, float ang ) {
        return cos( degToRad( ang )) * (bx - ax) - sin( degToRad( ang )) * (by - ay);
    }

    // comparator for floats - NOTE: 3DSage just used == operator for floats (in C)
    bool bIsEqual( float a, float b ) {
        return (abs( a - b ) < EPSILON);
    }

/*
 * The maps are string based, so each cell / tile is represented by a character.
 * The wall, ceiling and floor types are defined as constants with the associated character.
 * These characters are used to define a level - both the floor, the walls and the ceiling.
 * The following three functions are needed to map from the character type of a wall / floor / ceiling
 * to it's corresponding texture in the All_Textures[] array.
 *
 * map <--> character <--> constant / type <--> index <--> texture
 */

    // returns the index in the sprite array that corresponds with
    // wall type 'c'
    int nWallIndex( char c ) {
        switch (c) {
            case EMPTY:     return -1;
            case WALL_CBRD: return  0;
            case WALL_BRCK: return  3;
            case WINDOW:    return  2;
            case DOOR:      return  4;
            case WALL_LGTH: return  8;
            case WALL_END:  return  1;
        }
        return -1;
    }
    // returns the index in the sprite array that corresponds with
    // floor type 'c'
    int nFloorIndex( char c ) {

        switch (c) {
            case FLOOR_WOOD: return 0;
            case FLOOR_BRCK: return 3;
            case FLOOR_1   : return 6;
            case FLOOR_2   : return 7;
            case FLOOR_3   : return 8;
            case FLOOR_4   : return 9;
        }
        return -1;
    }

    // returns the index in the sprite array that corresponds with
    // ceiling type 'c'
    int nCeilIndex(  char c ) {
        switch (c) {
            case CEIL_WOOD: return 0;
            case CEIL_BRCK: return 3;
            case CEIL_1   : return 6;
            case CEIL_2   : return 7;
            case CEIL_3   : return 8;
            case CEIL_4   : return 9;
        }
        return -1;
    }

    // The DDA raycasting algorithm and the rendering of textures walls, floor and ceiling is all
    // handled in this method
    void drawRays2D() {

        int mx, my;  // map coordinates to find ...
        int mp;      // ... coord of hit in the wall (if any)
        int dof;     // depth of field - the max distance (in tiles) to check for hits

        float disH, disV, vx, vy;  // will contain distance and ray end point for the line checks

        float rx, ry, ra;     // ray's end point coord and angle
        float xo, yo;         // x and y offsets to get to next line or column of map

        // prepare iteration over field of view - one ray is cast for every degree
        ra = FixAng( pa + 0.5f * FOV );        // make sure ra is in [0, 360)

        for (int r = 0; r < NR_SLICES; r++) {

            int vmt = 0, hmt = 0;  // vertical and horizontal map texture number

            // --- Check Vertical Grid Line Collisions ---
            //     ===================================

            dof = 0;
            // reset distance caching variables each iteration
            disV = 1000000;
            // we need tangent (not its inverse) for checking vertical lines (see Notes on part 1)
            float Tan = tan( degToRad( ra ));

                 if (cos( degToRad( ra )) >  EPSILON) { rx = (((int)px / mapS) * mapS) + mapS   ; ry = (px - rx) * Tan + py; xo =  mapS; yo = -xo * Tan; }  // ray pointing to right
            else if (cos( degToRad( ra )) < -EPSILON) { rx = (((int)px / mapS) * mapS) - 0.0001f; ry = (px - rx) * Tan + py; xo = -mapS; yo = -xo * Tan; }  // ray pointing to left
            else { rx = px; ry = py; dof = 8; }                                                                                                             // ray vertical

            // walk the horizontal grid lines, converting the ray's end point (world coordinates) into
            // tile coordinates and checking if a wall was hit
            while (dof < 8) {
                // take the rays hit position, divide by 64, use that to find position in map array
                mx = int(rx) / mapS;
                my = int(ry) / mapS;
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

                 if (sin( degToRad( ra )) >  EPSILON) { ry = (((int)py / mapS) * mapS) - 0.0001f; rx = (py - ry) * Tan + px; yo = -mapS; xo = -yo * Tan; } // ray pointing up
            else if (sin( degToRad( ra )) < -EPSILON) { ry = (((int)py / mapS) * mapS) + mapS   ; rx = (py - ry) * Tan + px; yo =  mapS; xo = -yo * Tan; } // ray pointing down
            else { rx = px; ry = py; dof = 8; }                                                                                                            // ray is horizontal

            // walk the vertical grid lines, converting the ray's end point (world coordinates) into
            // tile coordinates and checking if a wall was hit
            while (dof < 8) {
                // take the rays hit position, divide by 64, use that to find position in map array
                mx = int(rx) / mapS;
                my = int(ry) / mapS;
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

            auto check_index = [=]( const std::string &msg, int ix, int low, int hgh ) {
                bool correct = true;
                if (ix < low || ix >= hgh) {
                    std::cout << "ERROR: DrawRays2D() --> index out of range! " << msg << " Index = " << ix << " low = " << low << " high = " << hgh << std::endl;
                    correct = false;
                }
                return correct;
            };

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

            depth[r] = disH;   // save this slice's depth

            // ----- Draw 3D walls -----
            //       =============

            float ca = FixAng( pa - ra );                        // fix fish eye distortion
            disH *= cos( degToRad( ca ));

            float lineH = (mapS * SCREEN_Y) / disH;

            // calculate step formula before line height is capped
            float ty_step = 32.0f / (float)lineH;
            float ty_off = 0.0f;

            if (lineH > SCREEN_Y) { ty_off = (lineH - SCREEN_Y) / 2.0f; lineH = SCREEN_Y; }      // cap line height at screen height
            float lineOff = (SCREEN_Y / 2) - lineH / 2.0f;          // offset from top of screen

            // put slice on screen - textured rendering
            float ty = ty_off * ty_step; // + hmt * 32;
            float tx;
            if (bHitHorizontal) {
                tx = int(rx / 2.0f) % 32; if (ra > 180           ) { tx = 31 - tx; } // south textures are mirrored, flip them
            } else {
                tx = int(ry / 2.0f) % 32; if (90 < ra && ra < 270) { tx = 31 - tx; } // west textures are mirrored, flip them
            }

            for (int y = 0; y < lineH; y++) {
                // display the wall using texture from All_Textures[] using SLICE_WIDTH x SLICE_WIDTH "pixels"
                int nPixel = (int( ty ) * 32 + int( tx )) * 3 + (hmt * 32 * 32 * 3);

                check_index( "wall drawing", nPixel, 0, 32*32*10*3 );  // there are 6 sprites, each having 32x32 pixels. Each pixel contains 3 values

                int red   = All_Textures[nPixel + 0] * fShadeFactor;
                int green = All_Textures[nPixel + 1] * fShadeFactor;
                int blue  = All_Textures[nPixel + 2] * fShadeFactor;
                FillRect( r * SLICE_WIDTH, lineOff + y, SLICE_WIDTH, 1, olc::Pixel( red, green, blue ));

                ty += ty_step;
            }

            // ----- Draw Floor -----
            //       ==========

            // work your way down from bottom of wall
            for (int y = lineOff + lineH; y < SCREEN_Y; y++) {
                float dy = y - (SCREEN_Y / 2.0f), deg = degToRad( ra ), raFix = cos(degToRad( FixAng( pa - ra )));
                // determine texture coordinates - the constant 158 is based on the fov and the aspect ratio
                // since we doubled the view,
                tx = px / 2.0f + cos( deg ) * 158 * 2 * 32 / dy / raFix;
                ty = py / 2.0f - sin( deg ) * 158 * 2 * 32 / dy / raFix;

                if (!check_index( "floor drawing 1", int( ty / 32.0f ) * mapX + int( tx / 32.0f ), 0, 8*8 )) {
                    std::cout << "px = " << px << " py = " << py << std::endl;
                    std::cout << "dy = " << dy << " deg = " << deg << " raFix = " << raFix << std::endl;
                    std::cout << "tx = " << tx << " ty = " << ty << std::endl;
                }

                // sample floor
                int nmp = nFloorIndex( mapF[ int( ty / 32.0f ) * mapX + int( tx / 32.0f ) ] ) * 32 * 32;
                int nPixel = ((int( ty ) & 31) * 32 + (int( tx ) & 31)) * 3 + nmp * 3;

                check_index( "floor drawing 2", nPixel, 0, 32*32*10*3 );

                int red   = All_Textures[nPixel + 0] * 0.7f;  // darken floor a little bit just to add variation
                int green = All_Textures[nPixel + 1] * 0.7f;
                int blue  = All_Textures[nPixel + 2] * 0.7f;
                // draw "pixels" as SLICE_WIDTH x SLICE_WIDTH squares
                FillRect( r * SLICE_WIDTH, y, SLICE_WIDTH, 1, olc::Pixel( red, green, blue ));

                // ----- Draw ceiling -----
                //       ============

                // sample ceiling
                nmp = nCeilIndex( mapC[ int( ty / 32.0f ) * mapX + int( tx / 32.0f ) ] ) * 32 * 32;
                // only draw ceiling if index is > 0 - so we can see the sky if there is no ceiling
                if (nmp > 0) {
                    nPixel = ((int( ty ) & 31) * 32 + (int( tx ) & 31)) * 3 + nmp * 3;
                    red   = All_Textures[nPixel + 0];
                    green = All_Textures[nPixel + 1];
                    blue  = All_Textures[nPixel + 2];
                    // draw "pixels" as SLICE_WIDTH x  squares
                    FillRect( r * SLICE_WIDTH, SCREEN_Y - y, SLICE_WIDTH, 1, olc::Pixel( red, green, blue ));
                }
            }

            // ----- End drawing -----
            //       ===========

            // prepare for next iteration
            ra = FixAng( ra - SLICE_DEG );                         // make sure ra is in [0, 2 * PI)
        }
    }

    void drawSky() {
        for (int y = 0; y < 40; y++) {
            for (int x = 0; x < 120; x++) {
                // shift sky with player angle
                int xo = int(pa) * 2 - x;
                if (xo < 0) { xo += 120; }
                xo = xo % 120;

                // display the wall using texture from All_Textures[] using SLICE_WIDTH x SLICE_WIDTH "pixels"
                int nPixel = (y * 120 + xo) * 3;
                int red   = sky[nPixel + 0];
                int green = sky[nPixel + 1];
                int blue  = sky[nPixel + 2];
                FillRect( x * SLICE_WIDTH, y * SLICE_WIDTH, SLICE_WIDTH, SLICE_WIDTH, olc::Pixel( red, green, blue ));
            }
        }
    }

    void screen( int v ) {
        int *T;  // texture pointer for screen denoted by v
        switch (v) {
            case 1: T = title; break;
            case 2: T = won;   break;
            case 3: T = lost;  break;
            // error checking on the input parameter
            default: std::cout << "ERROR: screen() --> index not recognized (must be in [1,3]: " << v << std::endl;
        }

        for (int y = 0; y < SCREEN_Y; y++) {
            for (int x = 0; x < SCREEN_X; x++) {
                // display the screen using the selected texture, using 1x1 pixels
                int nPixel = (y * SCREEN_X + x) * 3;
                int red   = float( T[nPixel + 0] ) * fade;
                int green = float( T[nPixel + 1] ) * fade;
                int blue  = float( T[nPixel + 2] ) * fade;
                Draw( x, y, olc::Pixel( red, green, blue ));
            }
        }
    }

    // these are needed for screen generation only
    olc::Sprite *ptrStrtScr = nullptr;
    olc::Sprite *ptrWinScr  = nullptr;
    olc::Sprite *ptrLoseScr = nullptr;

public:

    void init() {

        // init player position
        px = 150.0f;
        py = 400.0f;
        pa =  90.0f;
        pdx =  cos( degToRad( pa ));
        pdy = -sin( degToRad( pa ));

        // init game world / wall map - sizes must match mapX and mapY !!
        mapW.clear();
        mapW += "CCCBCWCC";
        mapW += "E..B...C";
        mapW += "C..D.B.C";
        mapW += "BBDB...C";
        mapW += "C......C";
        mapW += "C....C.C";
        mapW += "B......C";
        mapW += "CCWCWCWC";

        // init floor map - sizes must match mapX and mapY !!
        mapF.clear();
        mapF += "11111111";
        mapF += "12213331";
        mapF += "12223331";
        mapF += "11213331";
        mapF += "13332221";
        mapF += "13332221";
        mapF += "13332221";
        mapF += "11111111";

        // init ceiling map - sizes must match mapX and mapY !!
        mapC.clear();
        mapC += "0000....";
        mapC += "0330....";
        mapC += "0333..4.";
        mapC += "0030....";
        mapC += ".434....";
        mapC += "........";
        mapC += "........";
        mapC += "........";

        auto init_sp_index = [=]( int index, int type, int state, int nMap, int x, int y, int z ) {
            sp[index].type  = type;
            sp[index].state = state;
            sp[index].nMap  = nMap;  // index into texture array sprites[]
            sp[index].x     = x;
            sp[index].y     = y;
            sp[index].z     = z;
        };

        init_sp_index( 0, 1, 1, 0, 1.5f * mapS, 5.0f * mapS, 20 );     // key - height of 20 will put it on the floor
        init_sp_index( 1, 2, 1, 1, 1.5f * mapS, 4.5f * mapS,  0 );     // light 1
        init_sp_index( 2, 2, 1, 1, 3.5f * mapS, 4.5f * mapS,  0 );     // light 2
        init_sp_index( 3, 3, 1, 2, 2.5f * mapS, 2.0f * mapS, 20 );     // enemy
    }

    bool OnUserCreate() override {

        init();

        ptrWinScr  = new olc::Sprite( "Textures/background Won.png"  );
        ptrLoseScr = new olc::Sprite( "Textures/background Lost.png" );

        return true;
    }

    bool OnUserUpdate( float fElapsedTime ) override {

        if (gameState == 0) {     // ========== initialisation ==========
            init();
            timer = 0.0f;
            fade = 0.0f;
            gameState = 1;
        } else
        if (gameState == 1) {     // ========== start screen ==========
            screen( 1 );
            if (fade < 1.0f) { fade += fElapsedTime; }
            if (fade > 1.0f) { fade = 1.0f; }
            timer += fElapsedTime;
            if (timer > SCREEN_DELAY) {    // wait for a fixed amount of time (should be 3.0f seconds)
                timer = 0.0f;
                fade = 0.0f;
                gameState = 2;
            }
        } else
        if (gameState == 2) {    // ========== main game loop ==========

            // slow rotation or movement down if shift is held
            float suf = GetKey( olc::Key::SHIFT ).bHeld ? 1.0f : 10.0f;

            if (GetKey( olc::Key::A ).bHeld) { pa += 20.0f * fElapsedTime * suf; pa = FixAng( pa ); pdx = cos( degToRad( pa )); pdy = -sin( degToRad( pa )); }
            if (GetKey( olc::Key::D ).bHeld) { pa -= 20.0f * fElapsedTime * suf; pa = FixAng( pa ); pdx = cos( degToRad( pa )); pdy = -sin( degToRad( pa )); }

            // calculate index within the map right in front and right behind player
            // the player will retain a distance of 20 / 64th of a tile from the walls
            int xo = 20 * (pdx < 0.0f ? -1 : +1);
            int yo = 20 * (pdy < 0.0f ? -1 : +1);
            float fMapS = float( mapS );
            int ipx        =  px       / fMapS;
            int ipx_add_xo = (px + xo) / fMapS;
            int ipx_sub_xo = (px - xo) / fMapS;
            int ipy        =  py       / fMapS;
            int ipy_add_yo = (py + yo) / fMapS;
            int ipy_sub_yo = (py - yo) / fMapS;
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

            // open door if your in front of it and press E
            // added condition that "key" must be picked up
            if (GetKey( olc::Key::E ).bPressed && sp[0].state == 0) {
                int nxo = 0; if (pdx < 0) { nxo = -25; } else { nxo = 25; }
                int nyo = 0; if (pdy < 0) { nyo = -25; } else { nyo = 25; }
                int nipx_add_xo = (px + nxo) / float( mapS );
                int nipy_add_yo = (py + nyo) / float( mapS );
                if (mapW[ nipy_add_yo * mapX + nipx_add_xo ] == DOOR) { mapW[ nipy_add_yo * mapX + nipx_add_xo ] = EMPTY; }
            }

            // make background dark grey
            Clear( olc::VERY_DARK_GREY );

            drawSky();

            drawRays2D();

            drawSprite( fElapsedTime );

            // check if game was won - if so, jump to next game state
            if (int(px) / mapS == 1 && int(py) / mapS == 1) {
                timer = 0.0f;
                fade = 0.0f;
                gameState = 3;
            }
        } else
        if (gameState == 3) {    // ========== game was won ==========
            screen(2);
            if (fade < 1.0f) { fade += fElapsedTime; }
            if (fade > 1.0f) { fade = 1.0f; }
            timer += fElapsedTime;
            if (timer > SCREEN_DELAY) {
                fade = 0.0f;
                timer = 0.0f;
                gameState = 0;
            }
        } else
        if (gameState == 4) {    // ========== game was lost ==========
            screen(3);
            if (fade < 1.0f) { fade += fElapsedTime; }
            if (fade > 1.0f) { fade = 1.0f; }
            timer += fElapsedTime;
            if (timer > SCREEN_DELAY) {
                fade = 0.0f;
                timer = 0.0f;
                gameState = 0;
            }
        }

        return true;
    }

    bool OnUserDestroy() override {

        // clean up code here
        return true;
    }
};

// ====================   main   ==============================

int main()
{
	AnotherRayCaster demo;
	if (demo.Construct( SCREEN_X / PIXEL_X, SCREEN_Y / PIXEL_Y, PIXEL_X, PIXEL_Y ))
		demo.Start();

	return 0;
}

