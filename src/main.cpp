#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <fstream>
using namespace std;

// Define colors
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

// Define game variables
int cellSize = 30;
int cellCount = 18;
int offset = 75;

double lastUpdateTime = 0;

// Check if an element is present in a deque
bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

// Check if a specified interval has passed since the last update event
bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

// Class representing the Snake
class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

    // Draw the snake
    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    // Update the snake's position
    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    // Reset the snake to its initial state
    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

// Class representing the food
class Food
{
public:
    Vector2 position;
    Texture2D texture;

    // Constructor to initialize the food's position and load its texture
    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("../graphics/steak.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }

    // Destructor to unload the food's texture
    ~Food()
    {
        UnloadTexture(texture);
    }

    // Draw the food
    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    // Generate a random cell within the game grid
    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    // Generate a random position for the food that is not occupied by the snake
    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

// Class representing the game
class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    int highScore = 0;
    Sound eatSound;
    Sound wallSound;
    Sound moveSound;

    // Constructor to initialize the game and load sounds
    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("../sounds/eat.mp3");
        wallSound = LoadSound("../sounds/wall.mp3");
        moveSound = LoadSound("../sounds/move.mp3");
    }

    // Destructor to unload sounds
    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        UnloadSound(moveSound);
        CloseAudioDevice();
    }

    // Draw the game
    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    // Update the game state
    void Update()
    {
        if (running)
        {
            snake.Update();
            PlaySound(moveSound);
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    // Check if the snake has collided with the food
    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            if (score > highScore)
            {
                highScore = score;
            }
            PlaySound(eatSound);
        }
    }

    // Check if the snake has collided with the edges of the game grid
    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1)
        {
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    // Perform actions when the game is over
    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
    }

    // Check if the snake has collided with its own tail
    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }

    // Save the high score to a file
    void SaveHighScore()
    {
        ofstream file("highscore.txt"); // Open the file for writing
        if (file.is_open())
        {
            file << highScore; // Write the high score to the file
            file.close();      // Close the file
        }
    }

    // Load the high score from a file
    void LoadHighScore()
    {
        ifstream file("highscore.txt"); // Open the file for reading
        if (file.is_open())
        {
            file >> highScore; // Read the high score from the file
            file.close();      // Close the file
        }
    }
};

// Draw the intro screen
void DrawIntroScreen()
{
    ClearBackground(green);
    DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
    DrawText("Snake", offset + 10, 200, 80, darkGreen);
    DrawText("Press Enter to Start", offset + 10, offset + cellSize * cellCount / 2 - 10, 30, darkGreen);
    DrawText("Press Esc to Exit", offset + 10, offset + cellSize * cellCount / 2 + 30, 30, darkGreen);
    DrawText("Created by Rafay Khattak", offset + 10, 570, 30, darkGreen);
}

int main()
{
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Snake - PC Edition");
    SetTargetFPS(60);

    Game game = Game();
    game.LoadHighScore();
    bool introScreen = true;

    while (WindowShouldClose() == false)
    {
        BeginDrawing();

        if (introScreen)
        {
            DrawIntroScreen();

            if (IsKeyPressed(KEY_ENTER))
            {
                introScreen = false;
            }
            else if (IsKeyPressed(KEY_ESCAPE))
            {
                break;
            }
        }
        else
        {
            if (EventTriggered(0.2))
            {
                game.Update();
            }

            if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
            {
                game.snake.direction = {0, -1};
                game.running = true;
            }
            if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1)
            {
                game.snake.direction = {0, 1};
                game.running = true;
            }
            if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
            {
                game.snake.direction = {-1, 0};
                game.running = true;
            }
            if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1)
            {
                game.snake.direction = {1, 0};
                game.running = true;
            }

            // Drawing
            ClearBackground(green);
            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
            DrawText("Snake", offset - 5, 20, 40, darkGreen);
            // Display high score
            DrawText(TextFormat("High Score: %i", game.highScore), offset + 330, 25, 30, darkGreen);
            DrawText(TextFormat("%i", game.score), offset - 30, offset + cellSize * cellCount + 10, 40, darkGreen);
            game.Draw();
        }
        game.SaveHighScore();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
