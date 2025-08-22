
/* Minimal Raycaster in C + SDL2
 * Keys: W/S = forward/back, A/D = strafe, ←/→ = rotate, Esc = quit
 */
#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define W 1400
#define H 1200

// Map (1=wall, 0=empty). Keep it small and enclosed.
#define MAP_W 16
#define MAP_H 16
static int worldMap[MAP_H][MAP_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
    {1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,1},
    {1,0,1,0,0,0,1,0,0,0,1,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,0,1,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,1},
    {1,1,1,0,1,0,1,0,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,0,1,3,1,1,0,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1},
    {1,1,1,0,1,1,1,1,0,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,0,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
    {1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};


static void draw_vline(uint32_t *pixels, int x, int y0, int y1, uint32_t color){
    if (x < 0 || x >= W) return;
    if (y0 < 0) y0 = 0; if (y1 >= H) y1 = H-1;
    for (int y = y0; y <= y1; ++y) pixels[y*W + x] = color;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { fprintf(stderr, "SDL_Init: %s\n", SDL_GetError()); return 1; }
    SDL_Window *win = SDL_CreateWindow("Raycaster (C + SDL2)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, 0);
    if (!win) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); return 1; }
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);

    // Player state
    double posX = 3.5, posY = 3.5; // start position
    double dirX = 1.0, dirY = 0.0; // facing east
    double planeX = 0.0, planeY = 0.66; // camera plane (FOV ~ 66 deg)

    bool running = true;
    const Uint8 *ks = SDL_GetKeyboardState(NULL);
    uint32_t *pixels = NULL; int pitch = 0;

    while (running) {
        // Input/events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // Movement
        double moveSpeed = 0.06;   // adjust for step size
        double rotSpeed  = 0.04;   // rotation speed

        // Forward/back
        if (ks[SDL_SCANCODE_W]) {
            double nx = posX + dirX * moveSpeed;
            double ny = posY + dirY * moveSpeed;
            if (worldMap[(int)posY][(int)nx] == 0) posX = nx;
            if (worldMap[(int)ny][(int)posX] == 0) posY = ny;
        }
        if (ks[SDL_SCANCODE_S]) {
            double nx = posX - dirX * moveSpeed;
            double ny = posY - dirY * moveSpeed;
            if (worldMap[(int)posY][(int)nx] == 0) posX = nx;
            if (worldMap[(int)ny][(int)posX] == 0) posY = ny;
        }
        // Strafe
        if (ks[SDL_SCANCODE_A]) {
            double nx = posX - planeX * moveSpeed;
            double ny = posY - planeY * moveSpeed;
            if (worldMap[(int)posY][(int)nx] == 0) posX = nx;
            if (worldMap[(int)ny][(int)posX] == 0) posY = ny;
        }
        if (ks[SDL_SCANCODE_D]) {
            double nx = posX + planeX * moveSpeed;
            double ny = posY + planeY * moveSpeed;
            if (worldMap[(int)posY][(int)nx] == 0) posX = nx;
            if (worldMap[(int)ny][(int)posX] == 0) posY = ny;
        }
        // Rotate left/right
        if (ks[SDL_SCANCODE_RIGHT]) {
            double oldDirX = dirX;
            dirX = dirX*cos(rotSpeed) - dirY*sin(rotSpeed);
            dirY = oldDirX*sin(rotSpeed) + dirY*cos(rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX*cos(rotSpeed) - planeY*sin(rotSpeed);
            planeY = oldPlaneX*sin(rotSpeed) + planeY*cos(rotSpeed);
        }
        if (ks[SDL_SCANCODE_LEFT]) {
            double oldDirX = dirX;
            dirX = dirX*cos(-rotSpeed) - dirY*sin(-rotSpeed);
            dirY = oldDirX*sin(-rotSpeed) + dirY*cos(-rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX*cos(-rotSpeed) - planeY*sin(-rotSpeed);
            planeY = oldPlaneX*sin(-rotSpeed) + planeY*cos(-rotSpeed);
        }

        // Render
        SDL_LockTexture(tex, NULL, (void**)&pixels, &pitch);
        // simple sky/floor
        for (int i = 0; i < W*H; ++i) pixels[i] = (i < W*(H/2)) ? 0xFF87CEEB : 0xFF404040;

        for (int x = 0; x < W; ++x) {
            double cameraX = 2.0 * x / (double)W - 1.0; // -1 to 1
            double rayDirX = dirX + planeX * cameraX;
            double rayDirY = dirY + planeY * cameraX;

            int mapX = (int)posX;
            int mapY = (int)posY;

            // Length of ray from one x or y-side to next
            double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1.0 / rayDirX);
            double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1.0 / rayDirY);
            double sideDistX, sideDistY;

            int stepX, stepY;
            int side; // 0=x-side, 1=y-side
            // Calculate step and initial sideDist
            if (rayDirX < 0) { stepX = -1; sideDistX = (posX - mapX) * deltaDistX; }
            else { stepX = 1; sideDistX = (mapX + 1.0 - posX) * deltaDistX; }
            if (rayDirY < 0) { stepY = -1; sideDistY = (posY - mapY) * deltaDistY; }
            else { stepY = 1; sideDistY = (mapY + 1.0 - posY) * deltaDistY; }

            // DDA
            int hit = 0;
            while (!hit) {
                if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
                else { sideDistY += deltaDistY; mapY += stepY; side = 1; }
                if (mapX < 0 || mapX >= MAP_W || mapY < 0 || mapY >= MAP_H) { hit = 1; break; }
                if (worldMap[mapY][mapX] > 0) hit = 1;
            }

            // Perpendicular distance to avoid fisheye
            double perpWallDist;
            if (side == 0) perpWallDist = (mapX - posX + (1 - stepX)/2.0) / rayDirX;
            else           perpWallDist = (mapY - posY + (1 - stepY)/2.0) / rayDirY;
            if (perpWallDist < 1e-6) perpWallDist = 1e-6;

            int lineHeight = (int)(H / perpWallDist);
            int drawStart = -lineHeight/2 + H/2;
            int drawEnd   =  lineHeight/2 + H/2;

            // Pick wall color by cell value, darken if y-side
            int cell = (mapY >= 0 && mapY < MAP_H && mapX >= 0 && mapX < MAP_W) ? worldMap[mapY][mapX] : 1;
            uint32_t base = (cell == 2) ? 0xFFCC8844 : 0xFF77AA77;
            if (side == 1) { // darker for y-sides
                uint8_t a = (base >> 24) & 0xFF, r = (base >> 16) & 0xFF, g = (base >> 8) & 0xFF, b = base & 0xFF;
                r = (uint8_t)(r * 0.7); g = (uint8_t)(g * 0.7); b = (uint8_t)(b * 0.7);
                base = (a<<24) | (r<<16) | (g<<8) | b;
            }

            draw_vline(pixels, x, drawStart, drawEnd, base);
        }
        SDL_UnlockTexture(tex);

        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
