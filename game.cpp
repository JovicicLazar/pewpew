#include <raylib.h>
#include <math.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <iostream>

using namespace std;

#define MAX_PARTICLES 100

#define MAX_PARTICLES_BULLET 15

bool CheckCollisionRec(Rectangle rec1, Rectangle rec2) {
    return (rec1.x < rec2.x + rec2.width && rec1.x + rec1.width / 2  > rec2.x &&
            rec1.y < rec2.y + rec2.height  && rec1.y + rec1.height / 2 > rec2.y);
}
unsigned char Clamp(int value) {
    if (value < 0) {
        return 0;
    } else if (value > 255) {
        return 255;
    } else {
        return (unsigned char)value;
    }
}

// custom enums
enum GameState {
    STATE_START,
    STATE_GAME,
    STATE_PLAY_AGAIN,
    STATE_PAUSE
};

// custom types
typedef struct {
    Rectangle rec;
    int health;
    float speed;
} Player;

typedef struct {
    bool isDashing;          // Indicates whether the dash is currently active
    bool canDash;            // Indicates whether the player can perform a dash
    float dashSpeed;         // Speed of the dash
    float dashCooldown;      // Cooldown time before the player can dash again
    float dashCooldownTimer; // Timer to keep track of the dash cooldown
    float dashDuration;      // Duration of the dash in seconds
    float dashDurationTimer;
    Vector2 dashDirection;    // Direction of the dash
} Dash;

typedef struct {
    Rectangle rect;
    Color color;
    float rotation;
    Vector2 direction;
    bool active;
    float damage;
} Bullet;

typedef struct {
    Rectangle rect;
    Color color;
    float health;
    float speed;
    int color_strength;
    float damage;
} Enemy;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 radius;
    Color color;
    bool active;
} Particle;

int main() {

    const int screenWidth  = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "pewpew");

    SetTargetFPS(60);

    enum GameState gameState = STATE_START;

    Rectangle playerRec;
    playerRec.height             = screenHeight / 20;
    playerRec.width              = screenHeight / 20;
    playerRec.x                  = screenWidth / 2 - playerRec.width / 2; 
    playerRec.y                  = screenHeight / 2 - playerRec.height / 2; 

    Player player;
    player.rec                   = playerRec;
    player.health                = 10000;
    player.speed                 = 6.0f;

    Dash dash;
    dash.isDashing               = false;
    dash.canDash                 = true;
    dash.dashSpeed               = 20.0f;
    dash.dashCooldown            = 0.5f;
    dash.dashCooldownTimer       = 0.5f;
    dash.dashDuration            = 0.15f;
    dash.dashDurationTimer       = 0.0f;
    dash.dashDirection           = {0, 0};

    float angle           = 0.0f;
    float dash_bar_size   = (float)(screenWidth / 8);
    float health_bar_size = (float)(screenWidth / 4);
    float dash_max_size   = 100.0f;
    int score             = 0;

    Bullet bullets[100];
    int numBullets = 0;

    Enemy enemies[10];
    int numEnemies = 0;

    Particle particles[MAX_PARTICLES];
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }

    Particle bullet_particles[MAX_PARTICLES_BULLET];
    for (int i = 0; i < MAX_PARTICLES_BULLET; i++) {
        bullet_particles[i].active = false;
    }

    while (!WindowShouldClose())
    {
        float dashCooldownProgress   = dash.dashCooldownTimer / dash.dashCooldown;
        float healthBarProgress      = (float)player.health / 10000;

        // pause 
        Vector2 mousePos             = GetMousePosition();
        if(IsKeyDown(KEY_P) && gameState == STATE_GAME) gameState = STATE_PAUSE;
        
        if(gameState != STATE_PAUSE && gameState != STATE_PLAY_AGAIN) {
            // moving mechanic
            if (IsKeyDown(KEY_W)) player.rec.y -= 6.0f;
            if (IsKeyDown(KEY_A)) player.rec.x -= 6.0f;
            if (IsKeyDown(KEY_S)) player.rec.y += 6.0f;
            if (IsKeyDown(KEY_D)) player.rec.x += 6.0f;
            
            // wrapping mechanic 
            if (player.rec.x <= -player.rec.width) player.rec.x               = screenWidth + player.rec.width;
            if (player.rec.x > screenWidth + player.rec.width) player.rec.x   = 0 - player.rec.width;
            if (player.rec.y < - player.rec.height) player.rec.y              = screenHeight + player.rec.height;
            if (player.rec.y > screenHeight + player.rec.height) player.rec.y = 0 - player.rec.height;

            // mouse rotation mechanic
            angle = atan2f(mousePos.y - (player.rec.y + player.rec.height / 2), mousePos.x - (player.rec.x + player.rec.width / 2));
            angle = angle * RAD2DEG;

            // dash mechanic (w s a d + space)
            dash.dashDirection.x = 0;
            dash.dashDirection.y = 0;

            if (IsKeyDown(KEY_W)) dash.dashDirection.y = -1;
            if (IsKeyDown(KEY_A)) dash.dashDirection.x = -1;
            if (IsKeyDown(KEY_S)) dash.dashDirection.y = 1;
            if (IsKeyDown(KEY_D)) dash.dashDirection.x = 1;
            
            float dashMagnitude = sqrtf(dash.dashDirection.x * dash.dashDirection.x + dash.dashDirection.y * dash.dashDirection.y);
            if (dashMagnitude > 0) {
                dash.dashDirection.x /= dashMagnitude;
                dash.dashDirection.y /= dashMagnitude;
            }

            if (IsKeyPressed(KEY_SPACE) && dash.canDash) {
                dash.isDashing         = true;
                dash.canDash           = false;
                dash.dashDurationTimer = 0.0f;
                dash.dashCooldownTimer = 0.0f;
            }

            if (dash.isDashing) {
                player.rec.x += dash.dashDirection.x * dash.dashSpeed;
                player.rec.y += dash.dashDirection.y * dash.dashSpeed;

                dash.dashDurationTimer += GetFrameTime();

                if (dash.dashDurationTimer >= dash.dashDuration) {
                    dash.isDashing = false;
                }
            }

            if (!dash.canDash) {
                dash.dashCooldownTimer += GetFrameTime();
                
                if (dash.dashCooldownTimer >= dash.dashCooldown) {
                    dash.canDash = true;
                }
            }

            // shooting mechanic (rra rrra rrrrra)
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (numBullets < 100) { 
                    Rectangle newRect;
                    newRect.width  = player.rec.width / 1.5f;
                    newRect.height = player.rec.height / 1.5f;
                    newRect.x      = player.rec.x; 
                    newRect.y      = player.rec.y;

                    Color newColor = (Color){ GetRandomValue(50, 255), GetRandomValue(50, 255), GetRandomValue(50, 255), 255 };

                    Vector2 direction;
                    direction.x  = GetMouseX() - newRect.x - newRect.width/2; // Calculate direction vector towards the mouse
                    direction.y  = GetMouseY() - newRect.y - newRect.height/2;
                    float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
                    direction.x /= length;
                    direction.y /= length;

                    bullets[numBullets].rect      = newRect;
                    bullets[numBullets].color     = newColor;
                    bullets[numBullets].direction = direction;
                    bullets[numBullets].rotation  = angle;
                    bullets[numBullets].active    = true;
                    bullets[numBullets].damage    = 10.0f;
                    numBullets++;
                }
            }

            for (int i = 0; i < numBullets; i++) {
                bullets[i].rect.x += bullets[i].direction.x * 15;
                bullets[i].rect.y += bullets[i].direction.y * 15;

                if (bullets[i].rect.x > screenWidth || bullets[i].rect.y > screenHeight || bullets[i].rect.x + bullets[i].rect.width < 0 || bullets[i].rect.y + bullets[i].rect.height < 0) {
                    bullets[i] = bullets[numBullets - 1];
                    numBullets--;
                }
            }

            //// enemies spawn mechanic
            if (GetRandomValue(0, 100) < 2 && numEnemies < 10) { // Chance to spawn new enemy: 2%
                Rectangle newRect;
                newRect.width  = player.rec.width + 8;
                newRect.height = player.rec.height + 8;
                newRect.x      = GetRandomValue(0, screenWidth - newRect.width);
                newRect.y      = GetRandomValue(0, screenHeight - newRect.height);
                int a          = GetRandomValue(0, 10);
                int b          = GetRandomValue(0, 10);

                if(a % 2 == 0) {
                    newRect.x -= screenWidth;
                } else {
                    newRect.x += screenWidth;
                }

                if(b % 2 == 0) {
                    newRect.y -= screenWidth;
                } else {
                    newRect.y += screenWidth;
                }

                enemies[numEnemies].rect           = newRect;
                enemies[numEnemies].color          = {255, 0, 0, 255};
                enemies[numEnemies].health         = 50;
                enemies[numEnemies].color_strength = 255;
                enemies[numEnemies].speed          = (float)GetRandomValue(0.2f, 6.0f);
                enemies[numEnemies].damage         = enemies[numEnemies].speed * 10;
                numEnemies++;
            }

            // Update the positions of the enemies to follow the player
            for (int i = 0; i < numEnemies; i++) {
                Vector2 direction;
                direction.x  = player.rec.x - enemies[i].rect.x - 15;
                direction.y  = player.rec.y - enemies[i].rect.y - 15;
                float length = sqrtf(direction.x * direction.x + direction.y * direction.y);

                direction.x       /= length;
                direction.y       /= length;
                enemies[i].rect.x += direction.x * enemies[i].speed;
                enemies[i].rect.y += direction.y * enemies[i].speed;

                if (!dash.isDashing && CheckCollisionRec(player.rec, enemies[i].rect)) {
                    player.health -= (int)enemies[i].damage;

                    if( player.health <= 0 ) {
                        gameState = STATE_PLAY_AGAIN;
                        player.health = 10000;
                        numBullets = 0;
                        numEnemies = 0;
                        player.rec.x = screenWidth / 2 - playerRec.width / 2; 
                        player.rec.y= screenHeight / 2 - playerRec.height / 2; 
                        score = 0;
                    }

                }

                    for (int j = 0; j < numBullets; j++) {
                        if (bullets[j].active && CheckCollisionRec(bullets[j].rect, enemies[i].rect)) {

                            for (int k = 0; k < MAX_PARTICLES_BULLET; k++) {
                                if (!bullet_particles[k].active) {
                                    bullet_particles[k].active     = true;
                                    bullet_particles[k].position.x = bullets[j].rect.x + bullets[j].rect.width / 2;
                                    bullet_particles[k].position.y = bullets[j].rect.y + bullets[j].rect.height / 2;
                                    bullet_particles[k].velocity.x = GetRandomValue(-10, 10);
                                    bullet_particles[k].velocity.y = GetRandomValue(-10, 10);
                                    bullet_particles[k].radius     = { (float)GetRandomValue(3, 6), (float)GetRandomValue(3, 6)};
                                    bullet_particles[k].color      = bullets[j].color;
                                }
                            }
                            
                            enemies[i].health         -= bullets[j].damage;
                            enemies[i].color_strength -= 30;
                            enemies[i].color           = {Clamp(enemies[i].color_strength),0,0, 255};
                            
                            if(enemies[i].health <= 0){

                                // Create particle explosion at enemy position
                                for (int k = 0; k < MAX_PARTICLES; k++) {
                                    if (!particles[k].active) {
                                        particles[k].active     = true;
                                        particles[k].position.x = enemies[i].rect.x + enemies[i].rect.width / 2;
                                        particles[k].position.y = enemies[i].rect.y + enemies[i].rect.height / 2;
                                        particles[k].velocity.x = GetRandomValue(-10, 10);
                                        particles[k].velocity.y = GetRandomValue(-10, 10);
                                        particles[k].radius     = { (float)GetRandomValue(3, 6), (float)GetRandomValue(3, 6)};
                                        particles[k].color      = enemies[i].color;
                                    }
                                }
                                
                                enemies[i].color = {0,0,0,0};
                                enemies[i]       = enemies[numEnemies - 1];
                                numEnemies--;

                                score += 127;
                            }
                            
                            //bullets[j].color = {0,0,0,0};
                            bullets[j].active = false;
                            bullets[j].damage = 0.0f;
                            bullets[i]        = bullets[numBullets - 1];
                            numBullets--;
                        }
                    }

            }
            
            // Update the positions of the particles for the explosion effect
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].active) {
                    particles[i].position.x += particles[i].velocity.x;
                    particles[i].position.y += particles[i].velocity.y;
                    particles[i].velocity.y += 0.1f; // Apply gravity to the particles

                    // Despawn particles when they go out of the screen or fade out
                    if (particles[i].position.y > screenHeight || particles[i].color.a <= 0) {
                        particles[i].active = false;
                    }
                }
            }

            // Update the positions of the particles for the explosion effect
            for (int i = 0; i < MAX_PARTICLES_BULLET; i++) {
                if (bullet_particles[i].active) {
                    bullet_particles[i].position.x += bullet_particles[i].velocity.x;
                    bullet_particles[i].position.y += bullet_particles[i].velocity.y;
                    bullet_particles[i].velocity.y += 0.1f; // Apply gravity to the particles

                    // Despawn particles when they go out of the screen or fade out
                    if (bullet_particles[i].position.y > screenHeight || bullet_particles[i].color.a <= 0) {
                        bullet_particles[i].active = false;
                    }
                }
            }
        }

        // main update loop
        BeginDrawing();
        if(gameState == STATE_START) {
            if (IsKeyDown(KEY_SPACE)) {
                gameState = STATE_GAME;
            }

            BeginDrawing();
                ClearBackground(BLACK);
                DrawText("Press [SPACE] to start", screenWidth / 2 - MeasureText("Press [SPACE] to start", 20) / 2, screenHeight / 2, 20, DARKGRAY);
            EndDrawing();

        } else if(gameState == STATE_PLAY_AGAIN) {
            if (IsKeyDown(KEY_SPACE)) {
                gameState = STATE_GAME;
            }

            BeginDrawing();
                ClearBackground(BLACK);
                DrawText("Press [SPACE] to play again", screenWidth / 2 - MeasureText("Press [SPACE] to play again", 20) / 2, screenHeight / 2, 20, DARKGRAY);
            EndDrawing();

        } else if(gameState == STATE_PAUSE) {
            if (IsKeyDown(KEY_SPACE)) {
                gameState = STATE_GAME;
            }

            BeginDrawing();
                ClearBackground(BLACK);
                DrawText("Game paused! Press [SPACE] to continue", screenWidth / 2 - MeasureText("Game paused! Press [SPACE] to continue", 20) / 2, screenHeight / 2, 20, DARKGRAY);
            EndDrawing();

        } else {
            ClearBackground(BLACK);
            for (int i = 0; i < numBullets; i++) {
                DrawRectanglePro(bullets[i].rect, {bullets[i].rect.width / 2, bullets[i].rect.height / 2}, bullets[i].rotation, bullets[i].color);
            }

            for (int i = 0; i < numEnemies; i++) {
                DrawRectangleRec(enemies[i].rect, enemies[i].color);
            }

            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].active) {
                    DrawRectangleV(particles[i].position, {particles[i].radius.x, particles[i].radius.y}, particles[i].color);
                }
            }

            for (int i = 0; i < MAX_PARTICLES_BULLET; i++) {
                if (bullet_particles[i].active) {
                    DrawRectangleV(bullet_particles[i].position, {bullet_particles[i].radius.x, bullet_particles[i].radius.y}, bullet_particles[i].color);
                }
            }

            std::string s = std::to_string(score);
            std::string pom = "score: " + s;
            char const *pchar = pom.c_str(); 

            DrawText("dash cooldown:", screenWidth / 100, screenHeight - 30, 5 ,LIGHTGRAY);
            DrawText(pchar, 0, 0, player.rec.height / 2 ,WHITE);
            DrawRectangleV({screenWidth / 9 ,screenHeight - 26}, {dash_bar_size * dashCooldownProgress  , player.rec.height / 10}, BLUE);
            DrawRectangleV({50, 50}, {health_bar_size * healthBarProgress  , player.rec.height / 10}, RED);
            DrawRectanglePro(player.rec, {player.rec.width / 2, player.rec.height / 2}, angle, GRAY);

        }
            
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
