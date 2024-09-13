// File: src/main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <string>
// Constants
const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;
const float PI = 3.14159265f;

// Colors
const sf::Color COLOR_WHITE = sf::Color::White;
const sf::Color COLOR_BLUE = sf::Color::Blue;
const sf::Color COLOR_RED = sf::Color::Red;
const sf::Color COLOR_BLACK = sf::Color::Black;
const sf::Color COLOR_YELLOW = sf::Color::Yellow;
const sf::Color COLOR_GREEN = sf::Color::Green;

// Game States
enum class GameState { MENU, PLAY, GAME_OVER };

// Difficulty Levels
enum class Difficulty { EASY = 1, MEDIUM = 2, HARD = 3 };

// Utility Functions
float degToRad(float degrees) {
    return degrees * PI / 180.f;
}

sf::Vector2f calculatePosition(float angleDeg, float radius, sf::Vector2f center) {
    float angleRad = degToRad(angleDeg);
    return sf::Vector2f(center.x + radius * std::cos(angleRad),
                        center.y + radius * std::sin(angleRad));
}

// Classes

// Player Class
class Player {
public:
    Player(sf::Vector2f center, float ringRadius) :
        center(center), ringRadius(ringRadius), angle(0.f), direction(1) {
        shape.setRadius(playerRadius);
        shape.setFillColor(COLOR_BLUE);
        shape.setOrigin(playerRadius, playerRadius);
        updatePosition();
    }

    void update() {
        angle += playerSpeed * direction;
        if (angle >= 360.f) angle -= 360.f;
        if (angle < 0.f) angle += 360.f;
        updatePosition();
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }

    void changeDirection() {
        direction *= -1;
    }

    std::unique_ptr<class Bullet> shoot();

    float getAngle() const { return angle; }

private:
    void updatePosition() {
        position = calculatePosition(angle, ringRadius, center);
        shape.setPosition(position);
    }

    sf::CircleShape shape;
    sf::Vector2f center;
    float ringRadius;
    float angle; // in degrees
    int direction; // 1 for clockwise, -1 for counter-clockwise
    sf::Vector2f position;

    const float playerRadius = 15.f;
    const float playerSpeed = 2.f; // degrees per frame
};

// Bullet Class
class Bullet {
public:
    Bullet(sf::Vector2f startPos, float angleDeg) :
        position(startPos), angle(angleDeg) {
        // Define triangle shape
        shape.setPointCount(3);
        shape.setFillColor(COLOR_GREEN);
        shape.setOrigin(5.f, 5.f);

        // Define points relative to (0,0)
        shape.setPoint(0, sf::Vector2f(10.f, 0.f));
        shape.setPoint(1, sf::Vector2f(-5.f, 5.f));
        shape.setPoint(2, sf::Vector2f(-5.f, -5.f));

        shape.setPosition(position);
        shape.setRotation(angle);
        // Calculate velocity
        float angleRad = degToRad(angle);
        velocity = sf::Vector2f(bulletSpeed * std::cos(angleRad),
                                bulletSpeed * std::sin(angleRad));
    }

    void update() {
        position += velocity;
        shape.move(velocity);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }

    bool isOffScreen(unsigned int width, unsigned int height) const {
        return (position.x < 0 || position.x > width ||
                position.y < 0 || position.y > height);
    }

    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

private:
    sf::ConvexShape shape;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float angle; // in degrees

    const float bulletSpeed = 5.f;
};

// Explosion Class
class Explosion {
public:
    Explosion(sf::Vector2f pos) : position(pos), frame(0) {
        circle.setRadius(5.f);
        circle.setFillColor(COLOR_YELLOW);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setPosition(position);
    }

    void update() {
        frame++;
        float newRadius = circle.getRadius() + 1.f;
        circle.setRadius(newRadius);
        circle.setOrigin(newRadius, newRadius);
        // Fade out
        sf::Color color = circle.getFillColor();
        if (color.a > 5) color.a -= 5;
        circle.setFillColor(color);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(circle);
    }

    bool isFinished() const {
        return frame > explosionDuration;
    }

private:
    sf::CircleShape circle;
    sf::Vector2f position;
    int frame;
    const int explosionDuration = 30;
};

// Enemy Base Class
class Enemy {
public:
    Enemy(sf::Vector2f pos, sf::Vector2f vel, float size, sf::Color color) :
        position(pos), velocity(vel), size(size), color(color) {
        shape.setSize(sf::Vector2f(size, size));
        shape.setFillColor(color);
        shape.setOrigin(size / 2.f, size / 2.f);
        shape.setPosition(position);
    }

    virtual void update() {
        position += velocity;
        shape.setPosition(position);
    }

    virtual void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }

    virtual sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

    virtual bool isBoss() const { return false; }

protected:
    sf::RectangleShape shape;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float size;
    sf::Color color;
};

// Different Enemy Types
class CircleEnemy : public Enemy {
public:
    CircleEnemy(sf::Vector2f pos, sf::Vector2f vel, float size, sf::Color color) :
        Enemy(pos, vel, size, color) {
        // Override shape to be a circle
        circleShape.setRadius(size / 2.f);
        circleShape.setFillColor(color);
        circleShape.setOrigin(size / 2.f, size / 2.f);
        circleShape.setPosition(position);
    }

    void update() override {
        position += velocity;
        circleShape.setPosition(position);
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(circleShape);
    }

    sf::FloatRect getBounds() const override {
        return circleShape.getGlobalBounds();
    }

private:
    sf::CircleShape circleShape;
};

class BossEnemy : public Enemy {
public:
    BossEnemy(sf::Vector2f pos, sf::Vector2f vel, float size, sf::Color color) :
        Enemy(pos, vel, size, color), health(10) {
        shape.setSize(sf::Vector2f(size, size));
        shape.setFillColor(color);
        shape.setOrigin(size / 2.f, size / 2.f);
        shape.setPosition(position);
    }

    void takeDamage() {
        health--;
        if (health <= 0) {
            isAlive = false;
        }
    }

    bool alive() const { return isAlive; }

    bool isBoss() const override { return true; }

    void draw(sf::RenderWindow& window) override {
        // Draw health bar
        sf::RectangleShape healthBarBack(sf::Vector2f(size, 5.f));
        healthBarBack.setFillColor(sf::Color::Red);
        healthBarBack.setPosition(position.x - size / 2.f, position.y - size / 2.f - 10.f);

        sf::RectangleShape healthBarFront(sf::Vector2f(size * (health / 10.f), 5.f));
        healthBarFront.setFillColor(sf::Color::Green);
        healthBarFront.setPosition(position.x - size / 2.f, position.y - size / 2.f - 10.f);

        window.draw(shape);
        window.draw(healthBarBack);
        window.draw(healthBarFront);
    }

private:
    int health;
    bool isAlive = true;
};

// Bullet Implementation
std::unique_ptr<Bullet> Player::shoot() {
    return std::make_unique<Bullet>(position, angle);
}

// EnemyManager Class
class EnemyManager {
public:
    EnemyManager(sf::Vector2f center, float ringRadius, Difficulty difficulty) :
        center(center), ringRadius(ringRadius), difficulty(difficulty) {
        srand(static_cast<unsigned int>(time(0)));
        spawnTimer = 0;
        spawnInterval = 60 / static_cast<int>(difficulty); // Lower difficulty, slower spawn
    }

    void update(float deltaTime, std::vector<std::unique_ptr<Enemy>>& enemies) {
        spawnTimer++;
        if (spawnTimer >= spawnInterval) {
            spawnTimer = 0;
            spawnEnemy(enemies);
        }
    }

private:
    void spawnEnemy(std::vector<std::unique_ptr<Enemy>>& enemies) {
        // Randomly choose spawn side
        int side = rand() % 4; // 0: top, 1: bottom, 2: left, 3: right
        sf::Vector2f pos;
        sf::Vector2f vel;
        float speed = 1.f + static_cast<int>(difficulty) * 0.5f;

        switch (side) {
            case 0: // Top
                pos = sf::Vector2f(rand() % WINDOW_WIDTH, -30.f);
                vel = sf::Vector2f(center.x - pos.x, center.y - pos.y);
                break;
            case 1: // Bottom
                pos = sf::Vector2f(rand() % WINDOW_WIDTH, WINDOW_HEIGHT + 30.f);
                vel = sf::Vector2f(center.x - pos.x, center.y - pos.y);
                break;
            case 2: // Left
                pos = sf::Vector2f(-30.f, rand() % WINDOW_HEIGHT);
                vel = sf::Vector2f(center.x - pos.x, center.y - pos.y);
                break;
            case 3: // Right
                pos = sf::Vector2f(WINDOW_WIDTH + 30.f, rand() % WINDOW_HEIGHT);
                vel = sf::Vector2f(center.x - pos.x, center.y - pos.y);
                break;
        }

        // Normalize velocity
        float length = std::sqrt(vel.x * vel.x + vel.y * vel.y);
        vel /= length;
        vel *= speed;

        // Randomly decide enemy type
        int type = rand() % (3 + (static_cast<int>(difficulty) >= 3 ? 1 : 0)); // More types on higher difficulty
        if (type == 0) {
            // Square Enemy
            enemies.emplace_back(std::make_unique<Enemy>(pos, vel, 30.f, COLOR_RED));
        }
        else if (type == 1) {
            // Circle Enemy
            enemies.emplace_back(std::make_unique<CircleEnemy>(pos, vel, 25.f, COLOR_RED));
        }
        else if (type == 2 && static_cast<int>(difficulty) >= 3) {
            // Boss Enemy
            enemies.emplace_back(std::make_unique<BossEnemy>(pos, vel, 60.f, COLOR_YELLOW));
        }
    }

    sf::Vector2f center;
    float ringRadius;
    Difficulty difficulty;
    int spawnTimer;
    int spawnInterval;
};

// Game Class
class Game {
public:
    Game() :
        window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Circle Shooter Game"),
        state(GameState::MENU),
        selectedDifficulty(Difficulty::EASY),
        enemyManager(center, ringRadius, selectedDifficulty),
        playerInstance
        (center, ringRadius),
        shakeDuration(0),
        shakeMagnitude(0.f),
        score(0)
    {
        window.setFramerateLimit(60);
        if (!font.loadFromFile("assets/fonts/Arial.ttf")) {
            // Handle error
            // For simplicity, we'll proceed without a custom font
        }
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    // Window and Rendering
    sf::RenderWindow window;

    // Game State
    GameState state;

    // Difficulty
    Difficulty selectedDifficulty;

    // Player
    sf::Vector2f center = sf::Vector2f(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);
    float ringRadius = 200.f;
    Player playerInstance = Player(center, ringRadius);

    // Enemies and Bullets
    EnemyManager enemyManager;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Bullet>> bullets;
    std::vector<std::unique_ptr<Explosion>> explosions;

    // Menu
    sf::Font font;
    int difficultyIndex = 0;
    std::vector<Difficulty> difficulties = { Difficulty::EASY, Difficulty::MEDIUM, Difficulty::HARD };

    // Game Over
    int score;

    // Screen Shake
    int shakeDuration;
    float shakeMagnitude;
    sf::Vector2f originalViewCenter;

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            // Close Window
            if (event.type == sf::Event::Closed)
                window.close();

            if (state == GameState::MENU) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Space) {
                        state = GameState::PLAY;
                        // Initialize game variables
                        enemies.clear();
                        bullets.clear();
                        explosions.clear();
                        score = 0;
                        enemyManager = EnemyManager(center, ringRadius, selectedDifficulty);
                    }
                    else if (event.key.code == sf::Keyboard::Up) {
                        difficultyIndex = (difficultyIndex - 1 + difficulties.size()) % difficulties.size();
                        selectedDifficulty = difficulties[difficultyIndex];
                        enemyManager = EnemyManager(center, ringRadius, selectedDifficulty);
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        difficultyIndex = (difficultyIndex + 1) % difficulties.size();
                        selectedDifficulty = difficulties[difficultyIndex];
                        enemyManager = EnemyManager(center, ringRadius, selectedDifficulty);
                    }
                }
            }
            else if (state == GameState::PLAY) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Left) {
                        playerInstance.changeDirection();
                    }
                    else if (event.key.code == sf::Keyboard::Right) {
                        playerInstance.changeDirection();
                    }
                    else if (event.key.code == sf::Keyboard::Space) {
                        bullets.emplace_back(playerInstance.shoot());
                        // Change direction after shooting
                        playerInstance.changeDirection();
                    }
                }
            }
            else if (state == GameState::GAME_OVER) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::R) {
                        state = GameState::MENU;
                        selectedDifficulty = Difficulty::EASY;
                        difficultyIndex = 0;
                    }
                    else if (event.key.code == sf::Keyboard::Q) {
                        window.close();
                    }
                }
            }
        }
    }

    void update() {
        if (state == GameState::PLAY) {
            playerInstance.update();

            // Update bullets
            for (auto it = bullets.begin(); it != bullets.end(); ) {
                (*it)->update();
                if ((*it)->isOffScreen(WINDOW_WIDTH, WINDOW_HEIGHT)) {
                    it = bullets.erase(it);
                }
                else {
                    ++it;
                }
            }

            // Update enemies
            enemyManager.update(1.f / 60.f, enemies);
            for (auto it = enemies.begin(); it != enemies.end(); ) {
                (*it)->update();

                // Check if enemy reached the ring
                float distance = std::sqrt(std::pow((*it)->getBounds().left + (*it)->getBounds().width / 2.f - center.x, 2) +
                                           std::pow((*it)->getBounds().top + (*it)->getBounds().height / 2.f - center.y, 2));
                if (distance <= ringRadius + 30.f) { // 30.f is arbitrary
                    state = GameState::GAME_OVER;
                }

                // Remove if off-screen (optional)
                if ((*it)->getBounds().left < -50.f || (*it)->getBounds().left > WINDOW_WIDTH + 50.f ||
                    (*it)->getBounds().top < -50.f || (*it)->getBounds().top > WINDOW_HEIGHT + 50.f) {
                    it = enemies.erase(it);
                }
                else {
                    ++it;
                }
            }

            // Check collisions
            for (auto enemyIt = enemies.begin(); enemyIt != enemies.end(); ) {
                bool enemyRemoved = false;
                for (auto bulletIt = bullets.begin(); bulletIt != bullets.end(); ) {
                    if ((*enemyIt)->getBounds().intersects((*bulletIt)->getBounds())) {
                        // Create explosion
                        explosions.emplace_back(std::make_unique<Explosion>((*enemyIt)->getBounds().getPosition() + sf::Vector2f((*enemyIt)->getBounds().width / 2.f, (*enemyIt)->getBounds().height / 2.f)));

                        // Screen shake
                        shakeDuration = 10;
                        shakeMagnitude = 5.f;

                        // Handle boss health
                        BossEnemy* boss = dynamic_cast<BossEnemy*>(enemyIt->get());
                        if (boss) {
                            boss->takeDamage();
                            if (!boss->alive()) {
                                score += 5; // Boss gives more points
                                enemyIt = enemies.erase(enemyIt);
                                enemyRemoved = true;
                            }
                        }
                        else {
                            score += 1;
                            enemyIt = enemies.erase(enemyIt);
                            enemyRemoved = true;
                        }

                        bulletIt = bullets.erase(bulletIt);
                        break;
                    }
                    else {
                        ++bulletIt;
                    }
                }
                if (!enemyRemoved) {
                    ++enemyIt;
                }
            }

            // Update explosions
            for (auto it = explosions.begin(); it != explosions.end(); ) {
                (*it)->update();
                if ((*it)->isFinished()) {
                    it = explosions.erase(it);
                }
                else {
                    ++it;
                }
            }

            // Update screen shake
            if (shakeDuration > 0) {
                float offsetX = (std::rand() % static_cast<int>(shakeMagnitude * 2)) - shakeMagnitude;
                float offsetY = (std::rand() % static_cast<int>(shakeMagnitude * 2)) - shakeMagnitude;
                window.setView(sf::View(sf::FloatRect(0.f, 0.f, WINDOW_WIDTH, WINDOW_HEIGHT)));
                window.setPosition(sf::Vector2i(static_cast<int>(offsetX), static_cast<int>(offsetY)));
                shakeDuration--;
            }
            else {
                window.setPosition(sf::Vector2i(0, 0));
            }
        }
    }

    void render() {
        window.clear(COLOR_BLACK);

        if (state == GameState::MENU) {
            drawMenu();
        }
        else if (state == GameState::PLAY) {
            // Draw ring
            sf::CircleShape ring;
            ring.setRadius(ringRadius);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2.f);
            ring.setOutlineColor(COLOR_WHITE);
            ring.setOrigin(ringRadius, ringRadius);
            ring.setPosition(center);
            window.draw(ring);

            // Draw player
            playerInstance.draw(window);

            // Draw bullets
            for (auto& bullet : bullets) {
                bullet->draw(window);
            }

            // Draw enemies
            for (auto& enemy : enemies) {
                enemy->draw(window);
            }

            // Draw explosions
            for (auto& explosion : explosions) {
                explosion->draw(window);
            }

            // Draw score
            sf::Text scoreText;
            scoreText.setFont(font);
            scoreText.setCharacterSize(24);
            scoreText.setFillColor(COLOR_WHITE);
            scoreText.setString("Score: " + std::to_string(score));
            scoreText.setPosition(10.f, 10.f);
            window.draw(scoreText);
        }
        else if (state == GameState::GAME_OVER) {
            drawGameOver();
        }

        window.display();
    }

    void drawMenu() {
        // Draw title
        sf::Text title;
        title.setFont(font);
        title.setCharacterSize(48);
        title.setFillColor(COLOR_WHITE);
        title.setString("Circle Shooter Game");
        title.setPosition(WINDOW_WIDTH / 2.f - title.getGlobalBounds().width / 2.f, 100.f);
        window.draw(title);

        // Draw start instruction
        sf::Text startInstr;
        startInstr.setFont(font);
        startInstr.setCharacterSize(24);
        startInstr.setFillColor(COLOR_WHITE);
        startInstr.setString("Press SPACE to Start");
        startInstr.setPosition(WINDOW_WIDTH / 2.f - startInstr.getGlobalBounds().width / 2.f, 200.f);
        window.draw(startInstr);

        // Draw difficulty selection
        sf::Text difficultyText;
        difficultyText.setFont(font);
        difficultyText.setCharacterSize(24);
        difficultyText.setFillColor(COLOR_WHITE);
        difficultyText.setString("Select Difficulty (Up/Down):");
        difficultyText.setPosition(WINDOW_WIDTH / 2.f - difficultyText.getGlobalBounds().width / 2.f, 300.f);
        window.draw(difficultyText);

        // Display current difficulty
        std::string diffStr;
        switch (selectedDifficulty) {
            case Difficulty::EASY: diffStr = "Easy"; break;
            case Difficulty::MEDIUM: diffStr = "Medium"; break;
            case Difficulty::HARD: diffStr = "Hard"; break;
        }

        sf::Text currentDiff;
        currentDiff.setFont(font);
        currentDiff.setCharacterSize(24);
        currentDiff.setFillColor(COLOR_YELLOW);
        currentDiff.setString(diffStr);
        currentDiff.setPosition(WINDOW_WIDTH / 2.f - currentDiff.getGlobalBounds().width / 2.f, 350.f);
        window.draw(currentDiff);
    }

    void drawGameOver() {
        // Draw Game Over Text
        sf::Text overText;
        overText.setFont(font);
        overText.setCharacterSize(48);
        overText.setFillColor(COLOR_RED);
        overText.setString("Game Over!");
        overText.setPosition(WINDOW_WIDTH / 2.f - overText.getGlobalBounds().width / 2.f, 150.f);
        window.draw(overText);

        // Draw Score
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(COLOR_WHITE);
        scoreText.setString("Enemies Defeated: " + std::to_string(score));
        scoreText.setPosition(WINDOW_WIDTH / 2.f - scoreText.getGlobalBounds().width / 2.f, 250.f);
        window.draw(scoreText);

        // Draw Restart and Quit Instructions
        sf::Text restartInstr;
        restartInstr.setFont(font);
        restartInstr.setCharacterSize(24);
        restartInstr.setFillColor(COLOR_WHITE);
        restartInstr.setString("Press R to Restart or Q to Quit");
        restartInstr.setPosition(WINDOW_WIDTH / 2.f - restartInstr.getGlobalBounds().width / 2.f, 350.f);
        window.draw(restartInstr);
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
