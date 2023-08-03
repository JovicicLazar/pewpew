#include <raylib.h>
#include <math.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <iostream>

using namespace std;

#define MAX_PARTICLES 100

bool CheckCollisionRec(Rectangle rec1, Rectangle rec2) {
    return (rec1.x < rec2.x + rec2.width && rec1.x + rec1.width / 2  > rec2.x &&
            rec1.y < rec2.y + rec2.height  && rec1.y + rec1.height / 2 > rec2.y);
}

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
} Enemy;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 radius;
    Color color;
    bool active;
} Particle;

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "pewpew");

    SetTargetFPS(60);

    Rectangle rec;
    rec.height = screenHeight / 20;
    rec.width = screenHeight / 20;
    rec.x =  screenWidth / 2 - rec.width / 2; // Center the rectangle horizontally
    rec.y = screenHeight / 2 - rec.height / 2; // Center the rectangle vertically

    Dash dash;
    dash.isDashing = false;
    dash.canDash = true;
    dash.dashSpeed = 20.0f;
    dash.dashCooldown = 0.5f;
    dash.dashCooldownTimer = 0.5f;
    dash.dashDuration = 0.15f; // Duration of the dash in seconds
    dash.dashDurationTimer = 0.0f;
    dash.dashDirection = {0, 0};

    float angle = 0.0f;

    float dash_size = 100;
    float dash_max_size = 100;

    Bullet bullets[100]; // An array to store the spawned bullets
    int numBullets = 0;

    Enemy enemies[10];
    int numEnemies = 0;

    Particle particles[MAX_PARTICLES];
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }

    while (!WindowShouldClose())
    {
        float dashCooldownProgress = dash.dashCooldownTimer / dash.dashCooldown;

        // getting position of a mouse
        Vector2 mousePos = GetMousePosition();

        // moving implementation
        if (IsKeyDown(KEY_W)) rec.y    -= 6.0f;
        if (IsKeyDown(KEY_A)) rec.x  -= 6.0f;
        if (IsKeyDown(KEY_S)) rec.y  += 6.0f;
        if (IsKeyDown(KEY_D)) rec.x += 6.0f;
        

        // wrapping implementation 
        if (rec.x <= -rec.width) rec.x = screenWidth + rec.width;
        if (rec.x > screenWidth + rec.width) rec.x = 0 - rec.width;
        if (rec.y < - rec.height) rec.y = screenHeight + rec.height;
        if (rec.y > screenHeight + rec.height) rec.y = 0 - rec.height;

        // rotation implm
        angle = atan2f(mousePos.y - (rec.y + rec.height / 2), mousePos.x - (rec.x + rec.width / 2));
        angle = angle * RAD2DEG;

        // dash implementation w s a d + pace
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
            dash.isDashing = true;
            dash.canDash = false;
            dash.dashDurationTimer = 0.0f;
            dash.dashCooldownTimer = 0.0f;
        }

        if (dash.isDashing) {
            rec.x += dash.dashDirection.x * dash.dashSpeed;
            rec.y += dash.dashDirection.y * dash.dashSpeed;

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

        // shooooooting rra rra rra 
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (numBullets < 100) { // Limit the number of bullets to 100
                Rectangle newRect;
                newRect.width = 17;
                newRect.height = 17;
                newRect.x = rec.x; // Center the rectangle on the mouse click position
                newRect.y = rec.y;

                Color newColor = (Color){ GetRandomValue(50, 255), GetRandomValue(50, 255), GetRandomValue(50, 255), 255 };

                Vector2 direction;
                direction.x = GetMouseX() - newRect.x - newRect.width/2; // Calculate direction vector towards the mouse
                direction.y = GetMouseY() - newRect.y - newRect.height/2;
                float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
                direction.x /= length;
                direction.y /= length;

                bullets[numBullets].rect = newRect;
                bullets[numBullets].color = newColor;
                bullets[numBullets].direction = direction;
                bullets[numBullets].rotation = angle;
                bullets[numBullets].active = true;
                bullets[numBullets].damage = 10.0f;
                numBullets++;
            }
        }

        for (int i = 0; i < numBullets; i++) {
            bullets[i].rect.x += bullets[i].direction.x * 15;
            bullets[i].rect.y += bullets[i].direction.y * 15;

            // Despawn bullets that have left the screen
            if (bullets[i].rect.x > screenWidth || bullets[i].rect.y > screenHeight ||
                bullets[i].rect.x + bullets[i].rect.width < 0 || bullets[i].rect.y + bullets[i].rect.height < 0) {
                // Move the last rectangle in the array to the current index and decrement the count
                bullets[i] = bullets[numBullets - 1];
                numBullets--;
            }
        }

        //// spawn enemies
        //playerRect.x = GetMouseX() - playerRect.width / 2;
        //playerRect.y = GetMouseY() - playerRect.height / 2;
        if (GetRandomValue(0, 100) < 2 && numEnemies < 10) { // Chance to spawn new enemy: 2%
            Rectangle newRect;
            newRect.width = 30;
            newRect.height = 30;
            newRect.x = GetRandomValue(0, screenWidth - newRect.width);
            newRect.y = GetRandomValue(0, screenHeight - newRect.height);

            int a = GetRandomValue(0, 10);
            int b = GetRandomValue(0, 10);

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

            enemies[numEnemies].rect = newRect;
            enemies[numEnemies].color = RED;
            enemies[numEnemies].health = 50;
            numEnemies++;
        }

        // Update the positions of the enemies to follow the player
        for (int i = 0; i < numEnemies; i++) {
            float moveSpeed = 2.0f; // Adjust the speed of the enemies
            Vector2 direction;
            direction.x = rec.x - enemies[i].rect.x - 15;
            direction.y = rec.y - enemies[i].rect.y - 15;
            float length = sqrtf(direction.x * direction.x + direction.y * direction.y);

            // Normalize the direction vector
            direction.x /= length;
            direction.y /= length;

            // Move the enemy towards the player
            enemies[i].rect.x += direction.x * moveSpeed;
            enemies[i].rect.y += direction.y * moveSpeed;

                for (int j = 0; j < numBullets; j++) {
                    if (bullets[j].active && CheckCollisionRec(bullets[j].rect, enemies[i].rect)) {
                        enemies[i].health -= bullets[j].damage;
                        
                        if(enemies[i].health <= 0){

                            // Create particle explosion at enemy position
                            for (int k = 0; k < MAX_PARTICLES; k++) {
                                if (!particles[k].active) {
                                    particles[k].active = true;
                                    particles[k].position.x = enemies[i].rect.x + enemies[i].rect.width / 2;
                                    particles[k].position.y = enemies[i].rect.y + enemies[i].rect.height / 2;
                                    particles[k].velocity.x = GetRandomValue(-10, 10);
                                    particles[k].velocity.y = GetRandomValue(-10, 10);
                                    particles[k].radius = { (float)GetRandomValue(3, 6), (float)GetRandomValue(3, 6)};
                                    particles[k].color = bullets[j].color;
                                }
                            }
                            enemies[i].color = {0,0,0,0};
                            enemies[i] = enemies[numEnemies - 1];
                            numEnemies--;
                        }

                        bullets[j].color = {0,0,0,0};
                        bullets[j].active = false;
                        bullets[j].damage = 0.0f;
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
        BeginDrawing();
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
            std::string s = std::to_string(numBullets);
            char const *pchar = s.c_str(); 
            DrawText("dash cooldown:", screenWidth / 100, screenHeight - 30, 5 ,LIGHTGRAY);
            DrawText(pchar, 0, 0, 10 ,WHITE);
            DrawRectangleV({screenWidth / 9 ,screenHeight - 26}, {dash_size * dashCooldownProgress  , 2}, RED);
            DrawRectanglePro(rec, {rec.width / 2, rec.height / 2}, angle, GRAY);

            
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
