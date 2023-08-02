#include <raylib.h>
#include <math.h>

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

        // mouse rotation implementation
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
        //dash_size += dashCooldownProgress / 100;
        if (!dash.canDash) {
            dash.dashCooldownTimer += GetFrameTime();
            
            if (dash.dashCooldownTimer >= dash.dashCooldown) {
                dash.canDash = true;
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("dash cooldown:", screenWidth / 100, screenHeight - 30, 5 ,LIGHTGRAY);
            DrawRectangleV({screenWidth / 9 ,screenHeight - 26}, {dash_size * dashCooldownProgress  , 2}, RED);
            DrawRectanglePro(rec, {rec.width / 2, rec.height / 2}, angle, GRAY);
            
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
