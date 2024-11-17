
#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};
Color red = {255, 0, 0, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool eventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

bool ElementInDeque(Vector2 element, deque<Vector2> deq)
{
    for (unsigned int i = 0; i < deq.size(); i++)
    {
        if (Vector2Equals(deq[i], element))
        {
            return true;
        }
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}}; // starting cells of the snake!
    Vector2 direction = {1, 0};                                         // direction of the snake
    bool addSegment = false;

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

    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food
{

public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("food.png"); // this is the image of the food
        texture = LoadTextureFromImage(image);        // this is the texture of the food
        UnloadImage(image);                           // this is the image of the food
        position = GenerateRandomPos(snakeBody);
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandowPos()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {

        Vector2 position = GenerateRandowPos();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandowPos();
        }
        return position;
    }
};

class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound wallSound;

    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("eat.mp3");
        wallSound = LoadSound("wall.mp3");
    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

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

    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        PlaySound(wallSound);
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }

    void ResetGame()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = true;
        score = 0;
    }
};

vector<int> LoadScores()
{
    vector<int> scores;
    ifstream file("scores.txt");
    int score;
    while (file >> score)
    {
        scores.push_back(score);
    }
    file.close();
    return scores;
}

void SaveScores(const vector<int> &scores)
{
    ofstream file("scores.txt");
    for (int score : scores)
    {
        file << score << endl;
    }
    file.close();
}

void DrawStartMenu(const vector<int> &scores)
{
    ClearBackground(green);

    // Define the padding and text sizes
    int padding = 25; // Moderate padding around the text
    int textSizeTitle = 60;  // Slightly larger text size for title
    int textSizeStart = 50;  // Slightly larger text size for "Press ENTER to Start"
    int textSizeScores = 40; // Slightly larger text size for scores
    int textSizeTopScores = 50; // Slightly larger text size for "Top Scores:"

    // Measure text dimensions for dynamic rectangle size
    int titleWidth = MeasureText("Yeti's Retro Snake", textSizeTitle);
    int startTextWidth = MeasureText("Press ENTER to Start", textSizeStart);
    int topScoresWidth = MeasureText("Top Scores:", textSizeTopScores);

    // Get the width of the longest text line for the rectangle width
    int maxWidth = max({titleWidth, startTextWidth, topScoresWidth});
    maxWidth += padding * 3;  // Add more padding to the width for a larger rectangle

    // Estimate the total height needed for the rectangle
    int totalHeight = padding * 4 + textSizeTitle + textSizeStart + textSizeTopScores;  // Moderate height padding
    int scoreHeight = (scores.size() < 3 ? scores.size() : 3) * (textSizeScores + padding);
    totalHeight += scoreHeight;

    // Define the rectangle's position (centered)
    int startX = (GetScreenWidth() - maxWidth) / 2;
    int startY = (GetScreenHeight() - totalHeight) / 2;

    // Draw the border with moderately thick lines
    Color borderColor = darkGreen;
    int borderThickness = 6; // Balanced border thickness
    DrawRectangleLinesEx(Rectangle{(float)startX, (float)startY, (float)maxWidth, (float)totalHeight}, borderThickness, borderColor);

    // Inside the border, adjust positioning for each element
    int titleY = startY + padding;
    int startGameY = titleY + textSizeTitle + padding;
    int topScoresY = startGameY + textSizeStart + padding;
    int scoreY = topScoresY + textSizeTopScores + padding;

    // Draw game title with a larger font
    DrawText("Yeti's Retro Snake", startX + maxWidth / 2 - titleWidth / 2, titleY, textSizeTitle, darkGreen);

    // Draw "Press ENTER to Start" with a larger font
    DrawText("Press ENTER to Start", startX + maxWidth / 2 - startTextWidth / 2, startGameY, textSizeStart, darkGreen);

    // Draw "Top Scores:" with a larger font
    DrawText("Top Scores:", startX + maxWidth / 2 - topScoresWidth / 2, topScoresY, textSizeTopScores, darkGreen);

    // Display the top 3 scores, centered and with proper suffix
    for (int i = 0; i < 3 && i < scores.size(); i++)
    {
        // Determine the suffix for the rank (1st, 2nd, 3rd)
        const char* rankSuffix;
        if (i == 0)
            rankSuffix = "st";
        else if (i == 1)
            rankSuffix = "nd";
        else if (i == 2)
            rankSuffix = "rd";

        // Display the score with the rank using larger text
        DrawText(TextFormat("%d%s: %i", i + 1, rankSuffix, scores[i]), 
                 startX + maxWidth / 2 - MeasureText(TextFormat("%d%s: %i", i + 1, rankSuffix, scores[i]), textSizeScores) / 2,
                 scoreY + i * (textSizeScores + padding), textSizeScores, darkGreen);
    }
}

int main()
{
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    vector<int> scores = LoadScores();
    Game game;

    bool gameStarted = false;

    while (!WindowShouldClose())
    {
        if (gameStarted)
        {
            if (eventTriggered(0.15))
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

            ClearBackground(green);
            DrawText("Yeti's Retro Snake", offset - 5, 20, 40, darkGreen);
            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
            DrawText(TextFormat("Score: %i", game.score), offset - 6, offset + cellSize * cellCount + 10, 40, darkGreen);
            game.Draw();

            if (!game.running)
            {
                scores.push_back(game.score);
                sort(scores.begin(), scores.end(), greater<int>());
                scores.resize(3); // Keep top 3 scores
                SaveScores(scores);
                game.ResetGame();
                gameStarted = false;
            }
        }
        else
        {
            DrawStartMenu(scores);

            if (IsKeyPressed(KEY_ENTER))
            {
                game.ResetGame();
                gameStarted = true;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
