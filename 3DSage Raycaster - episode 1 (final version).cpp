// Make Your Own Ray Caster - episode 1
// ====================================

// Youtube: https://youtu.be/gYRrGTC7GtA
// Idea and video by 3DSage - thanks!!

// Port to olc::PixelGameEngine
// By Joseph21, august 19, 2022

// Dependencies:
//   *  olcPixelGameEngine.h - (olc::PixelGameEngine header file) by JavidX9 (see: https://github.com/OneLoneCoder/olcPixelGameEngine)


/* Short description
   -----------------
   This is the first episode of the 3DSage Raycaster series. The code is the final result of following the video. For better recognition
   it is intentionally kept close to the original code by 3DSage. However, where he implemented in openGL, I implemented on
   the PixelGameEngine (PGE).

   The code contains:
     * setting up the PGE
     * adding a player with user interaction to move and rotate (no collision detection, that's in episode 2)
     * adding a map and a way to display both the map and the player in it in 2D
     * ray casting method based on DDA algorith
     * wall rendering and some simple forms of shading and colouring the walls

   For other raycasting introductions I'd suggest to check on the following video's by JavidX9:

       FPS part 1 - https://youtu.be/xW8skO7MFYw
       FPS part 2 - https://youtu.be/HEb2akswCcw
       DDA video  - https://youtu.be/NbSee-XM7WA

    Or the implementation of the Permadi tutorial on the olc::PixelGameEngine by Joseph21:

       https://github.com/Joseph21-6147/Raycasting-tutorial-series

    Have fun!
 */

 // ====================   Constants   ==============================

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define PI  3.1415926535f
#define P2  PI / 2.0f
#define P3  3.0f * PI / 2.0f

#define DR  0.0174533f    // one degree in radians

#define MOVE_SPEED     50.0f
#define MOVE_FRACTION   0.1f

// ====================   PGE derived class AnotherRayCaster   ==============================

class AnotherRayCaster : public olc::PixelGameEngine {

public:
    AnotherRayCaster() {
        sAppName = "RayCaster (episode 1) by 3DSage";
    }

private:
    float px, py;      // player position
    float pa;          // player angle
    float pdx, pdy;    // displacement depending on angle and speed

    void drawPlayer() {
        // represent player as a little yellow square of 8x8 pixels
        FillRect( px - 4, py - 4, 8, 8, olc::YELLOW );
        // show the looking direction using a little line segment
        DrawLine( px, py, px + pdx * 5.0f, py + pdy * 5.0f, olc::YELLOW );
    }

    int mapX = 8, mapY = 8, mapS = 64;   // grid is 8x8 tiles, each tile is 64x64 in size
    std::string sMap;                    // I'm using a string implementation of the map

    void drawMap2D() {
        for (int y = 0; y < mapY; y++) {
            for (int x = 0; x < mapX; x++) {
                // filled tiles/walls are white, empty spaces are black
                olc::Pixel tileColour;
                switch (sMap[y * mapX + x]) {
                    case '#': tileColour = olc::WHITE; break;
                    case '*': tileColour = olc::BLUE;  break;
                    case '.': tileColour = olc::BLACK; break;
                    default : tileColour = olc::GREY;
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
    float dist( float ax, float ay, float bx, float by, float ang ) {
        return sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));
    }

#define EPSILON  0.00001f     // margin used in bIsEqual()

    // comparator for floats
    bool bIsEqual( float a, float b ) {
        return (abs( a - b ) < EPSILON);
    }

    void drawRays2D() {

        int mx, my;           // map coordinates to find ...
        int mp;               // ... coord of hit in the wall (if any)
        int dof;              // depth of field - the max distance to check for hits

        float disH, hx, hy;   // will contain distance and ray end point for horizontal
        float disV, vx, vy;   // resp. vertical line checks
        float disT;

        char hc, vc;          // store type of wall that was hit (if any)

        float rx, ry, ra;     // ray's x and y coord and rays angle
        float xo, yo;         // x and y offsets to get to next line or column of map

        // make sure angle is in [0, 2 * PI)
        auto cycle_clamp = [=] (float &angle ) {
            while (angle <  0.0f     ) { angle += 2.0f * PI; }
            while (angle >= 2.0f * PI) { angle -= 2.0f * PI; }
        };

        // prepare iteration over field of view - one ray is cast for every degree
        ra = pa - DR * 30.0f;     // set ray angle to player angle -/- 30 degree
        cycle_clamp( ra );        // make sure ra is in [0, 2 * PI)

        // cast 60 rays
        for (int r = 0; r < 60; r++) {

            // --- Check Horizontal Grid Line collisions ---
            //     =====================================

            dof = 0;
            // reset distance caching variables each iteration
            disH = 1000000;
            hx = px;
            hy = py;

            // we need negative inverse of tangent - see notes why that's the case
            float aTan = -1 / tan( ra );

            // ray is looking up on the screen
            if (ra > PI) {
                // truncate ray's y position to nearest 64 value - divide by 64 and multiply by 64 again, and subtract a small amount for accuracy
                // NOTE - this small amount is needed to have the ray position land in the correct tile when converting from
                //        world to tile coordinates and checking the map array
                ry = ((int(py) >> 6) << 6) - 0.0001f;
                // ray's x value is the distance between the player and the ray's y position times the inverse tangent, offset by players x position
                rx = (py - ry) * aTan + px;
                // determine offsets per step
                yo = -64;
                xo = -yo * aTan;
            }
            // ray is looking down on the screen
            if (ra < PI) {
                // truncate ray's y position to nearest 64 value - divide by 64 and multiply by 64 again, and add 64 to get to next horizontal grid line
                ry = ((int(py) >> 6) << 6) + 64;
                // ray's x value is the distance between the player and the ray's y position times the inverse tangent, offset by players x position
                rx = (py - ry) * aTan + px;
                // determine offsets per step
                yo =  64;
                xo = -yo * aTan;
            }
            // ray is looking straight right or left
            if (bIsEqual( ra, 0.0f ) || bIsEqual( ra, PI)) {
                rx = px;
                ry = py;
                dof = 8;   // distH will be the initial 1000000
            }

            // walk the horizontal grid lines, converting the ray's end point (world coordinates) into
            // tile coordinates and checking if a wall was hit
            while (dof < 8) {
                // take the rays hit position, divide by 64, use that to find position in map array
                mx = int(rx) >> 6;
                my = int(ry) >> 6;
                mp = my * mapX + mx;
                // if the index is within the map, check if there's a wall there
                if (mp >= 0 && mp < mapX * mapY && sMap[mp] != '.') {   // hit wall
                    // store info to compare shortest hit length
                    hx = rx;
                    hy = ry;
                    disH = dist( px, py, hx, hy, ra );
                    // store type of wall hit
                    hc = sMap[mp];
                    // set dof to 8 to end while loop
                    dof = 8;
                } else {  // no hit and dof < 8 --> check next line
                    rx += xo;
                    ry += yo;
                    dof += 1;
                }
            }

            // --- Check Vertical Grid Line Collisions ---
            //     ===================================

            dof = 0;
            // reset distance caching variables each iteration
            disV = 1000000;
            vx = px;
            vy = py;

            // we need negative tangent (not its inverse) - see notes why that's the case
            float nTan = -tan( ra );

            // ray is looking left on the screen
            if (ra > P2 && ra < P3) {
                // truncate ray's x position to nearest 64 value - divide by 64 and multiply by 64 again, and subtract a small amount for accuracy
                // NOTE - this small amount is needed to have the ray position land in the correct tile when converting from
                //        world to tile coordinates and checking the map array
                rx = ((int(px) >> 6) << 6) - 0.0001f;
                // ray's x value is the distance between the player and the ray's y position times the inverse tangent, offset by players x position
                ry = (px - rx) * nTan + py;
                // determine offsets per step
                xo = -64;
                yo = -xo * nTan;
            }
            // ray is looking right on the screen
            if (ra < P2 || ra > P3) {
                // truncate ray's x position to nearest 64 value - divide by 64 and multiply by 64 again, and add 64 to get to next vertical grid line
                rx = ((int(px) >> 6) << 6) + 64;
                // ray's y value is the distance between the player and the ray's y position times the negative tangent, offset by players y position
                ry = (px - rx) * nTan + py;
                // determine offsets per step
                xo =  64;
                yo = -xo * nTan;
            }
            // ray is looking straight up or down
            if (bIsEqual( ra, P2 ) || bIsEqual( ra, P3 )) {
                rx = px;
                ry = py;
                dof = 8;
            }

            // walk the vertical grid lines, converting the ray's end point (world coordinates) into
            // tile coordinates and checking if a wall was hit
            while (dof < 8) {
                // take the rays hit position, divide by 64, use that to find position in map array
                mx = int(rx) >> 6;
                my = int(ry) >> 6;
                mp = my * mapX + mx;
                // if the index is within the map, check if there's a wall there
                if (mp >= 0 && mp < mapX * mapY && sMap[mp] != '.') {   // hit wall
                    // store info to compare shortest hit length
                    vx = rx;
                    vy = ry;
                    disV = dist( px, py, vx, vy, ra );
                    // store type of wall hit
                    vc = sMap[mp];
                    // set dof to 8 to end while loop
                    dof = 8;
                } else {  // no hit and dof < 8 --> check next line
                    rx += xo;
                    ry += yo;
                    dof += 1;
                }
            }

            float fShadeFactor = 1.0f;
            char rc;
            // select shortest distance and determine shading factor and hit type (in rc)
            if (disV < disH) { rx = vx; ry = vy; disT = disV; rc = vc; fShadeFactor = 0.9f; }  // vertical   wall hit
            if (disH < disV) { rx = hx; ry = hy; disT = disH; rc = hc; fShadeFactor = 0.7f; }  // horizontal wall hit

            olc::Pixel shadeCol;
            // select colour based on the wall char encountered in the hit
            switch (rc) {
                case '#': shadeCol = olc::RED;   break;
                case '*': shadeCol = olc::BLUE;  break;
                case '.': shadeCol = olc::BLACK; break;
                default : shadeCol = olc::GREY;
            }
            // show the ray as a coloured line
            DrawLine( px, py, rx, ry, shadeCol * fShadeFactor );

            // ----- Draw 3D walls -----
            //       =============

            // determine distance between player and ray angles
            float ca = pa - ra;
            cycle_clamp( ca );
            disT *= cos( ca );                         // fix fish eye distortion

            float lineH = (mapS * 320) / disT;
            if (lineH > 320.0f) { lineH = 320.0f; }    // cap line height at 320
            float lineO = 160 - lineH / 2.0f;          // offset from top of screen
            // put slice on screen - 3DSage uses an openGL call with a line width of 8 pixels.
            // I use a FillRect() call
            FillRect( r * 8 + 530, lineO, 8, lineH, shadeCol * fShadeFactor );

            // ----- End wall drawing -----
            //       ================

            // prepare next iteration
            ra += DR;                                  // increase ray angle by one degree
            cycle_clamp( ra );                         // make sure ra is in [0, 2 * PI)
        }
    }

public:
    bool OnUserCreate() override {

        // init player position
        px = 300.0f;
        py = 300.0f;
        pa = 0.0f;
        pdx = cos( pa ) * MOVE_SPEED * MOVE_FRACTION;
        pdy = sin( pa ) * MOVE_SPEED * MOVE_FRACTION;

        // create map
        sMap += "########";
        sMap += "#......#";
        sMap += "#.#.*###";
        sMap += "##..#..#";
        sMap += "#......#";
        sMap += "#..**..#";
        sMap += "#......#";
        sMap += "########";

        return true;
    }


    bool OnUserUpdate( float fElapsedTime ) override {

        float move_factor = MOVE_SPEED * MOVE_FRACTION;
        // slow rotation down if shift is held
        float rot_factor = GetKey( olc::Key::SHIFT ).bHeld ? 0.1f : 1.0f;

        if (GetKey( olc::Key::A ).bHeld) { pa -= 5.0f * rot_factor * fElapsedTime; if (pa < 0.0f     ) { pa += 2.0f * PI; } pdx = cos( pa ) * move_factor; pdy = sin( pa ) * move_factor; }
        if (GetKey( olc::Key::D ).bHeld) { pa += 5.0f * rot_factor * fElapsedTime; if (pa > 2.0f * PI) { pa -= 2.0f * PI; } pdx = cos( pa ) * move_factor; pdy = sin( pa ) * move_factor; }
        if (GetKey( olc::Key::W ).bHeld) { px += pdx * MOVE_SPEED * fElapsedTime; py += pdy * MOVE_SPEED * fElapsedTime; }
        if (GetKey( olc::Key::S ).bHeld) { px -= pdx * MOVE_SPEED * fElapsedTime; py -= pdy * MOVE_SPEED * fElapsedTime; }

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

