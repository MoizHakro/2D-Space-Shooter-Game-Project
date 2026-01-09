#include <iostream>
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <optional>

#define M_PI 3.14159265358979323846

// ============================================================================
// GAME STATE ENUM
// ============================================================================
enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER,
    HIGHSCORE,
	LOADING,
	OPTIONS 
};

// ============================================================================
// BULLET
// ============================================================================
struct Bullet {
	// Bullet sprite and direction
    sf::Sprite sprite;
    sf::Vector2f direction;
    float speed = 500.f;
	// Constructor
    Bullet(const sf::Texture& texture, float x, float y, float dirX, float dirY)
        : sprite(texture), direction(dirX, dirY)
    {
        sprite.setPosition({ x, y });
        sprite.setScale({ 1.5f, 1.5f });
        float angleDeg = std::atan2(dirY, dirX) * 180.f / 3.14159f;
        sprite.setRotation(sf::degrees(angleDeg + 90.f));
    }
	// Get bullet bounds and position
    sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }
    const sf::Vector2f& getPosition() const { return sprite.getPosition(); }
	// Update bullet position
    void update(sf::Time dt) {
        sprite.move(direction * speed * dt.asSeconds());
    }
	// Render bullet
    void render(sf::RenderWindow& target) {
        target.draw(sprite);
    }
};

// ============================================================================
// EXPLOSION
// ============================================================================
struct Explosion {
	// Explosion frames and sprite
    const std::vector<sf::Texture>* frames;
    sf::Sprite sprite;
    int currentFrame = 0;
    float frameTimer = 0.f;
    float frameDuration = 0.05f;
    bool finished = false;
	// Constructor
    Explosion(const std::vector<sf::Texture>* explosionFrames, float x, float y)
        : frames(explosionFrames), sprite((*explosionFrames)[0])
    {
        if (!frames->empty()) {
            sf::FloatRect bounds = sprite.getLocalBounds();
            sprite.setOrigin({ bounds.size.x / 2.0f, bounds.size.y / 1.5f });
            sprite.setPosition({ x, y });
            sprite.setScale({ 2.f, 2.f });
        }
    }
	// Update explosion animation
    void update(sf::Time dt) {
        if (finished || frames->empty()) return;
        frameTimer += dt.asSeconds();
        if (frameTimer >= frameDuration) {
            frameTimer = 0.f;
            currentFrame++;
            if (currentFrame >= static_cast<int>(frames->size())) {
                finished = true;
            } else {
                sprite.setTexture((*frames)[currentFrame], true);
            }
        }
    }
	// Render explosion
    void render(sf::RenderWindow& target) {
        if (!finished) target.draw(sprite);
    }
	// Check if explosion animation is finished
    bool isFinished() const { return finished; }
};

// ============================================================================
// POWERUP
// ============================================================================
struct Powerup {
	// Powerup types
    enum Type { SCORE_BONUS = 0, HEAL = 1, TRIPLE_SHOT = 2 };
	// Powerup sprite and type
    sf::Sprite sprite;
    Type type;
    float speed = 100.f;
	// Constructor
    Powerup(const sf::Texture& texture, Type t, float x, float y)
        : sprite(texture), type(t)
    {
        sprite.setPosition({ x, y });
        sprite.setScale({ 0.04f, 0.04f });
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
    }
	// Update powerup position
    void update(sf::Time dt) {
        sprite.move({ 0.f, speed * dt.asSeconds() });
    }
	// Render powerup
    void render(sf::RenderWindow& target) { target.draw(sprite); }
    sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }
    Type getType() const { return type; }
};

// ============================================================================
// ASTEROID
// ============================================================================
struct Asteroid {
    sf::Sprite sprite;
	// Asteroid attributes
    float speed = 20.f;
    float rotationSpeed = 45.f;
    int health = 15;
    bool isAlive = true;
	// Constructor
    Asteroid(const sf::Texture& texture, float startX, float startY)
        : sprite(texture)
    {
        sprite.setPosition({ startX, startY });
        sprite.setScale({ 0.8f, 0.8f });
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
    }
	// Get asteroid bounds and position
    sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }
    const sf::Vector2f& getPosition() const { return sprite.getPosition(); }
	// Damage asteroid
    void takeDamage(int damage) {
        health -= damage;
        if (health <= 0) isAlive = false;
    }
	// Update asteroid position and rotation
    void update(sf::Time dt) {
        if (!isAlive) return;
        float newY = sprite.getPosition().y + (speed * dt.asSeconds());
        sprite.setPosition({ sprite.getPosition().x, newY });
        sprite.rotate(sf::degrees(rotationSpeed * dt.asSeconds()));
    }
	// Render asteroid
    void render(sf::RenderWindow& target) {
        if (isAlive) target.draw(sprite);
    }
};

// ============================================================================
// ENEMY
// ============================================================================
struct Enemy {
	// Enemy sprite
    sf::Sprite sprite;
	// Enemy attributes
    float speed = 50.f;
    int hp = 70;
    float startX;
    float sineTimer = 0.f;
    float shootCooldown;
    float shootTimer = 0.f;
	// Constructor
    Enemy(const sf::Texture& texture, float x, float y)
        : sprite(texture), startX(x)
    {
        shootCooldown = static_cast<float>(rand() % 40 + 20) / 10.f;
        sprite.setPosition({ x, y });
        sprite.setScale({ 0.11f, 0.11f });
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
    }
	// Get enemy bounds and position
    sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }
    const sf::Vector2f& getPosition() const { return sprite.getPosition(); }
    int getHp() const { return hp; }
	// damage enemy
    void takeDamage(int damage) {
        hp -= damage;
        if (hp < 0) hp = 0;
    }
	// Update enemy state
    void update(sf::Time dt, std::vector<Bullet>& bullets, const sf::Texture& bulletTex) {
        sineTimer += dt.asSeconds();
        float newY = sprite.getPosition().y + (speed * dt.asSeconds());
        float newX = startX + (std::sin(sineTimer * 0.5f) * 100.f);
        float spriteWidth = sprite.getGlobalBounds().size.x;
        newX = std::max(spriteWidth / 2.f, std::min(newX, 1200.f - spriteWidth / 2.f));
        sprite.setPosition({ newX, newY });

        shootTimer += dt.asSeconds();
        if (shootTimer >= shootCooldown) {
            shootTimer = 0.f;
            bullets.emplace_back(bulletTex, sprite.getPosition().x, sprite.getPosition().y, 0.f, 1.f);
        }
    }
	// Render enemy
    void render(sf::RenderWindow& target) { target.draw(sprite); }
};

// ============================================================================
// BOSS
// ============================================================================
struct Boss {
    sf::Sprite sprite;
    sf::RectangleShape hpBarOuter, hpBarInner;
	// Boss attributes
    int hp, maxHp;
    float speed = 75.f;
    bool movingRight = true;
    float attackTimer = 0.f, attackMax = 1.25f;
    float bulletSpeed;  // Configurable bullet speed

	// Constructor
    Boss(const sf::Texture& texture, int health, float bSpeed) 
        : sprite(texture), hp(health), maxHp(health), bulletSpeed(bSpeed) 
    {
        sprite.setScale({ 0.20f, 0.20f });
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });

        hpBarOuter.setSize({ 200.f, 20.f });
        hpBarOuter.setFillColor(sf::Color::Transparent);
        hpBarOuter.setOutlineColor(sf::Color::Red);
        hpBarOuter.setOutlineThickness(2.f);

        hpBarInner.setSize({ 200.f, 20.f });
        hpBarInner.setFillColor(sf::Color::Red);
    }
	// Update boss state
    void update(sf::Time dt, sf::Vector2u windowSize, std::vector<Bullet>& enemyBullets, const sf::Texture& bulletTex) {
        sf::Vector2f pos = sprite.getPosition();
        float windowWidth = static_cast<float>(windowSize.x);
		// Move boss left and right
        if (movingRight) {
            pos.x += speed * dt.asSeconds();
            if (pos.x + sprite.getGlobalBounds().size.x / 2.f > windowWidth) movingRight = false;
        } else {
            pos.x -= speed * dt.asSeconds();
            if (pos.x - sprite.getGlobalBounds().size.x / 2.f < 0.f) movingRight = true;
        }
        if (pos.y < 150.f) pos.y += 50.f * dt.asSeconds();
        sprite.setPosition(pos);
		// Update HP bar
        hpBarOuter.setPosition({ pos.x - 100.f, pos.y - 100.f });
        hpBarInner.setPosition({ pos.x - 100.f, pos.y - 100.f });
        float hpPercent = std::max(0.f, static_cast<float>(hp) / static_cast<float>(maxHp));
        hpBarInner.setSize({ 200.f * hpPercent, 20.f });
		// Handle attacks
        attackTimer += dt.asSeconds();
        if (attackTimer >= attackMax) {
            attackTimer = 0.f;
            float spawnX = pos.x;
            float spawnY = pos.y + sprite.getGlobalBounds().size.y / 2.f;
            
            // Create bullets with custom speed
            Bullet b1(bulletTex, spawnX, spawnY, 0.f, 1.f);
            b1.speed = bulletSpeed;
            enemyBullets.push_back(b1);
			// Side bullets
            float angleLeft = -25.f * 3.14159f / 180.f;
            Bullet b2(bulletTex, spawnX, spawnY, std::sin(angleLeft), std::cos(angleLeft));
            b2.speed = bulletSpeed;
            enemyBullets.push_back(b2);
			// Side bullets
            float angleRight = 25.f * 3.14159f / 180.f;
            Bullet b3(bulletTex, spawnX, spawnY, std::sin(angleRight), std::cos(angleRight));
            b3.speed = bulletSpeed;
            enemyBullets.push_back(b3);
        }
    }
	// Render boss
    void render(sf::RenderWindow& target) {
        target.draw(sprite);
        target.draw(hpBarOuter);
        target.draw(hpBarInner);
    }
	//  damage boss
    void takeDamage(int damage) { hp -= damage; }
    sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    bool isAlive() const { return hp > 0; }
};

// ============================================================================
// PLAYER
// ============================================================================
struct Player {
    const std::vector<sf::Texture>* textures;
    sf::Sprite sprite;
    sf::Vector2f velocity;
	// Player attributes
    float movementSpeed = 400.f;
    float attackCooldown = 0.2f;
    float attackTimer;
    float tripleShotTimer = 0.f;
    float rotationSpeed = 150.f;
    int currentFrame = 2;
    float animTimer = 0.f;
    float animSpeed = 0.05f;

	// Constructor
    Player(const std::vector<sf::Texture>& tex) : textures(&tex), sprite(tex[2]) {
        attackTimer = attackCooldown;
        sprite.setScale({ 1.65f, 1.65f });
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
    }
	// Get player position and bounds
    const sf::Vector2f& getPosition() const { return sprite.getPosition(); }
    sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }
    sf::Angle getRotation() const { return sprite.getRotation(); }

	// Set player position
    void setPosition(float x, float y) { sprite.setPosition({ x, y }); }
    bool canAttack() { return attackTimer >= attackCooldown; }
    void resetAttackTimer() { attackTimer = 0.f; }
    void activateTripleShot(float duration) { tripleShotTimer = duration; }
    bool isTripleShotActive() const { return tripleShotTimer > 0.f; }
	
    // Update player state
    void update(sf::Time dt, const sf::Vector2u& windowSize) {
        if (attackTimer < attackCooldown) attackTimer += dt.asSeconds();
        if (tripleShotTimer > 0.f) tripleShotTimer -= dt.asSeconds();
		// Reset velocity
        velocity = { 0.f, 0.f };
        bool movingLeft = false, movingRight = false;

		// Handle input
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) velocity.y = -1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) velocity.y = 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) { velocity.x = -1.f; movingLeft = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) { velocity.x = 1.f; movingRight = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) sprite.rotate(sf::degrees(-rotationSpeed * dt.asSeconds()));
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) sprite.rotate(sf::degrees(rotationSpeed * dt.asSeconds()));
		
        // Update animation frame
        animTimer += dt.asSeconds();
        if (animTimer >= animSpeed) {
            animTimer = 0.f;
            int targetFrame = 2;
            if (movingLeft) targetFrame = 0;
            else if (movingRight) targetFrame = 4;
            if (currentFrame < targetFrame) currentFrame++;
            else if (currentFrame > targetFrame) currentFrame--;
            if (textures && currentFrame >= 0 && currentFrame < static_cast<int>(textures->size()))
                sprite.setTexture((*textures)[currentFrame]);
        }
		// Normalize velocity and move player
        if (velocity.x != 0.f || velocity.y != 0.f) {
            float length = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            velocity /= length;
            sprite.move(velocity * movementSpeed * dt.asSeconds());
        }
		// Keep player within window bounds
        sf::Vector2f pos = sprite.getPosition();
        sf::FloatRect bounds = sprite.getGlobalBounds();
        float halfW = bounds.size.x / 2.f, halfH = bounds.size.y / 2.f;
        if (pos.x - halfW < 0.f) pos.x = halfW;
        if (pos.x + halfW > static_cast<float>(windowSize.x)) pos.x = static_cast<float>(windowSize.x) - halfW;
        if (pos.y - halfH < 0.f) pos.y = halfH;
        if (pos.y + halfH > static_cast<float>(windowSize.y)) pos.y = static_cast<float>(windowSize.y) - halfH;
        sprite.setPosition(pos);
    }
	// Render player
    void render(sf::RenderWindow& target) { target.draw(sprite); }
};

// ============================================================================
// STAR FIELD
// ============================================================================
struct Star {
    sf::CircleShape shape;
    float speed;
};

struct StarField {
    std::vector<Star> stars;
    sf::Vector2u windowSize;
	// Constructor
    StarField(int count, sf::Vector2u winSize) : windowSize(winSize) {
        stars.resize(count);
        for (auto& star : stars) {
            float x = static_cast<float>(rand() % windowSize.x);
            float y = static_cast<float>(rand() % windowSize.y);
            float size = static_cast<float>((rand() % 3) + 1);
            star.shape.setRadius(size);
            star.shape.setPosition({ x, y });
            int brightness = (rand() % 100) + 155;
            star.shape.setFillColor(sf::Color(255, 255, 255, brightness));
            star.speed = size * 40.f;
        }
    }
	// Update star positions
    void update(sf::Time dt) {
        for (auto& star : stars) {
            star.shape.move({ 0.f, star.speed * dt.asSeconds() });
            if (star.shape.getPosition().y > windowSize.y) {
                float x = static_cast<float>(rand() % windowSize.x);
                star.shape.setPosition({ x, -5.f });
            }
        }
    }
	// Render stars
    void render(sf::RenderWindow& target) {
        for (const auto& star : stars) target.draw(star.shape);
    }
};

// ============================================================================
// SCROLLING BACKGROUND
// ============================================================================
struct ScrollingBackground {
    sf::Sprite bg1, bg2;
    float scrollSpeed;
    float textureHeight;
	// Constructor
    ScrollingBackground(const sf::Texture& texture, float speed)
        : bg1(texture), bg2(texture), scrollSpeed(speed)
    {
        textureHeight = static_cast<float>(texture.getSize().y);
        bg1.setPosition({ 0.f, 0.f });
        bg2.setPosition({ 0.f, -textureHeight });
    }
	// Update background positions
    void update(sf::Time dt) {
        float movement = scrollSpeed * dt.asSeconds();
        bg1.move({ 0.f, movement });
        bg2.move({ 0.f, movement });
        sf::Vector2f pos1 = bg1.getPosition(), pos2 = bg2.getPosition();
        if (pos1.y >= textureHeight) bg1.setPosition({ 0.f, pos2.y - textureHeight });
        if (pos2.y >= textureHeight) bg2.setPosition({ 0.f, pos1.y - textureHeight });
    }
	// Render background
    void render(sf::RenderWindow& target) {
        target.draw(bg1);
        target.draw(bg2);
    }
};

// ============================================================================
// SCREEN SHAKE
// ============================================================================
struct ScreenShake {
    float shakeAmount = 0.f;
    float shakeDuration = 0.f;
    float shakeTimer = 0.f;
    float maxShakeDuration = 0.f;
	// Start screen shake
    void shake(float amount, float duration) {
        shakeAmount = amount;
        shakeDuration = duration;
        maxShakeDuration = duration;
        shakeTimer = 0.f;
    }
	// Update shake timer
    void update(sf::Time dt) {
        if (shakeTimer < shakeDuration) shakeTimer += dt.asSeconds();
    }
	// Get current shake offset
    sf::Vector2f getOffset() const {
        if (shakeTimer >= shakeDuration) return { 0.f, 0.f };
        float intensity = (shakeDuration - shakeTimer) / maxShakeDuration;
        float offsetX = std::sin(shakeTimer * 50.f) * shakeAmount * intensity;
        float offsetY = std::sin(shakeTimer * 70.f) * shakeAmount * intensity;
        return { offsetX, offsetY };
    }
};

// ============================================================================
// HUD
// ============================================================================
struct HUD {
    sf::Font font;
    sf::Text scoreText;
    sf::Texture heartTex;
    std::vector<sf::Sprite> hearts;
    int score = 0;
    int currentHearts = 10;
    int maxHearts = 10;
    int enemiesDefeated = 0;

    // Power-up notification
    sf::Text powerupText;
    float powerupMessageTimer = 0.f;
    float powerupMessageDuration = 2.f;  // Show message for 2 seconds
    bool showPowerupMessage = false;

	// Constructor
    HUD() : scoreText(font), powerupText(font) {
        font.openFromFile("assests/font/Xirod.otf");
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition({ 10.f, 10.f });

        // Setup power-up text
        powerupText.setCharacterSize(18);
        powerupText.setFillColor(sf::Color::Yellow);
        powerupText.setOutlineColor(sf::Color::Black);
        powerupText.setOutlineThickness(1.f);
		powerupText.setPosition({ 10.f, 85.f }); // Below hearts
    }

	// Load HUD assets
    bool loadAssets() {
        if (!heartTex.loadFromFile("assests/textures/player/heart.png")) return false;
        hearts.clear();
        for (int i = 0; i < maxHearts; i++) {
            sf::Sprite heart(heartTex);
            heart.setScale({ 0.5f, 0.5f });
            heart.setPosition({ 10.f + (i * 25.f), 50.f });
            hearts.push_back(heart);
        }
        return true;
    }

	// Display power-up message
    void showPowerup(const std::string& message) {
        powerupText.setString(message);
        showPowerupMessage = true;
        powerupMessageTimer = 0.f;
    }

	// Score and health management
    void addScore(int points) { score += points; scoreText.setString("Score: " + std::to_string(score)); }
    int getScore() const { return score; }
    void loseHeart() { if (currentHearts > 0) currentHearts--; }
    bool isAlive() const { return currentHearts > 0; }
    void heal(int amount) { currentHearts = std::min(currentHearts + amount, maxHearts); }
    void addEnemyDefeated() { enemiesDefeated++; }
    const sf::Font& getFont() const { return font; }

	// Dynamic spawn rate multiplier based on score
    float getSpawnRateMultiplier() const {
    // Increase spawn rate based on score
    // Every 100 points increases spawn rate by 0.2
        float multiplier = 1.0f + (score / 100) * 0.2f;
        // Cap at 2.2 maximum
        return std::min(multiplier, 2.2f);
    }

	// Update HUD elements
    void update(sf::Time dt) { 
        scoreText.setString("Score: " + std::to_string(score)); 

        // Update power-up message timer
        if (showPowerupMessage) {
            powerupMessageTimer += dt.asSeconds();
            if (powerupMessageTimer >= powerupMessageDuration) {
                showPowerupMessage = false;
            }
        }
    }

	// Render HUD elements
    void render(sf::RenderWindow& target) {
        target.draw(scoreText);
        for (int i = 0; i < currentHearts; i++) target.draw(hearts[i]);

        // Draw power-up message with fade effect
        if (showPowerupMessage) {
            float alpha = 1.f - (powerupMessageTimer / powerupMessageDuration);
            sf::Color color = powerupText.getFillColor();
            color.a = static_cast<std::uint8_t>(255 * alpha);
            powerupText.setFillColor(color);

            sf::Color outlineColor = powerupText.getOutlineColor();
            outlineColor.a = static_cast<std::uint8_t>(255 * alpha);
            powerupText.setOutlineColor(outlineColor);

            target.draw(powerupText);
        }
    }

	// Reset HUD state
    void reset() {
        score = 0;
        currentHearts = maxHearts;
        enemiesDefeated = 0;
        scoreText.setString("Score: 0");
        showPowerupMessage = false;
        powerupMessageTimer = 0.f;
    }
};

// ============================================================================
// MENU BUTTON
// ============================================================================
struct MenuIconButton {
    sf::RectangleShape* rectangle = nullptr;
    sf::Vector2f originalScale = { 1.f, 1.f };
    sf::Vector2f originalPosition;
    float animationTimer = 0.f;
    float animationDuration = 0.15f;
    bool isAnimating = false;
    bool isSelected = false;
    std::string label;
};

// ============================================================================
// MENU
// ============================================================================
struct Menu {
    sf::Font font;
    sf::Sprite* menuBackground = nullptr;
    std::vector<MenuIconButton> iconButtons;
    int selectedIconIndex = 0;
    bool startPressed = false;
    bool exitPressed = false;
    bool highScorePressed = false;
    bool optionsPressed = false;  // Add this member variable

    ~Menu() {
        if (menuBackground) delete menuBackground;
        for (auto& btn : iconButtons) if (btn.rectangle) delete btn.rectangle;
    }

    bool loadAssets(const sf::Texture& menuBgTexture) {
        font.openFromFile("assests/font/Xirod.otf");
        menuBackground = new sf::Sprite(menuBgTexture);
        float scaleX = 1200.f / static_cast<float>(menuBgTexture.getSize().x);
        float scaleY = 900.f / static_cast<float>(menuBgTexture.getSize().y);
        menuBackground->setScale({ scaleX, scaleY });

        std::vector<std::string> buttonLabels = { "START GAME", "OPTIONS", "HIGH SCORE", "EXIT GAME" };
        float buttonWidth = 240.f, buttonHeight = 55.f, buttonX = 95.f, startY = 575.f, spacing = 67.f;

        for (size_t i = 0; i < 4; i++) {
            sf::RectangleShape* rect = new sf::RectangleShape({ buttonWidth, buttonHeight });
            rect->setFillColor(sf::Color::Transparent);
            rect->setPosition({ buttonX, startY + (i * spacing) });

            MenuIconButton btn;
            btn.rectangle = rect;
            btn.originalPosition = rect->getPosition();
            btn.isSelected = (i == 0);
            btn.label = buttonLabels[i];
            iconButtons.push_back(btn);
        }
        return true;
    }

    void handleInput(const sf::Event& event, const sf::RenderWindow& window) {
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Up) {
                selectedIconIndex = (selectedIconIndex - 1 + 4) % 4;
                for (size_t i = 0; i < iconButtons.size(); i++) iconButtons[i].isSelected = (static_cast<int>(i) == selectedIconIndex);
            } else if (keyEvent->code == sf::Keyboard::Key::Down) {
                selectedIconIndex = (selectedIconIndex + 1) % 4;
                for (size_t i = 0; i < iconButtons.size(); i++) iconButtons[i].isSelected = (static_cast<int>(i) == selectedIconIndex);
            } else if (keyEvent->code == sf::Keyboard::Key::Enter) {
                if (selectedIconIndex == 0) startPressed = true;
                else if (selectedIconIndex == 1) optionsPressed = true;  // Add this
                else if (selectedIconIndex == 2) highScorePressed = true;
                else if (selectedIconIndex == 3) exitPressed = true;
            }
        }

        if (const auto* moveEvent = event.getIf<sf::Event::MouseMoved>()) {
            sf::Vector2f mousePos = window.mapPixelToCoords({ moveEvent->position.x, moveEvent->position.y });
            for (size_t i = 0; i < iconButtons.size(); i++) {
                if (iconButtons[i].rectangle && iconButtons[i].rectangle->getGlobalBounds().contains(mousePos)) {
                    selectedIconIndex = static_cast<int>(i);
                    for (size_t j = 0; j < iconButtons.size(); j++) iconButtons[j].isSelected = (j == i);
                }
            }
        }

        if (const auto* clickEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (clickEvent->button == sf::Mouse::Button::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords({ clickEvent->position.x, clickEvent->position.y });
                for (size_t i = 0; i < iconButtons.size(); i++) {
                    if (iconButtons[i].rectangle && iconButtons[i].rectangle->getGlobalBounds().contains(mousePos)) {
                        if (i == 0) startPressed = true;
                        else if (i == 1) optionsPressed = true;  // Add this
                        else if (i == 2) highScorePressed = true;
                        else if (i == 3) exitPressed = true;
                    }
                }
            }
        }
    }
    
    void update(sf::Time dt) {}

    void render(sf::RenderWindow& target) {
        if (menuBackground) target.draw(*menuBackground);
        for (auto& btn : iconButtons) {
            if (btn.rectangle) {
                target.draw(*btn.rectangle);
                if (btn.isSelected) {
                    sf::FloatRect rectBounds = btn.rectangle->getGlobalBounds();
                    sf::RectangleShape highlight(rectBounds.size);
                    highlight.setPosition(rectBounds.position);
                    highlight.setFillColor(sf::Color::Transparent);
                    highlight.setOutlineColor(sf::Color::White);
                    highlight.setOutlineThickness(3.f);
                    target.draw(highlight);
                }
            }
        }
    }

    bool isStartPressed() const { return startPressed; }
    bool isHighScorePressed() const { return highScorePressed; }
    bool isOptionsPressed() const { return optionsPressed; }
    bool isExitPressed() const { return exitPressed; }
    
    void reset() { 
        startPressed = exitPressed = highScorePressed = optionsPressed = false;
        selectedIconIndex = 0; 
    }
};

// ============================================================================
// PAUSE MENU
// ============================================================================
struct PauseMenuButton {
    sf::RectangleShape shape;
    sf::Text* text = nullptr;
    bool isHovered = false;
    int action = 0; // 0=NONE, 1=CONTINUE, 2=TOGGLE_MUSIC, 3=EXIT_GAME
};

struct PauseMenu {
    enum Action { NONE = 0, CONTINUE = 1, TOGGLE_MUSIC = 2, EXIT_GAME = 3 };
	// Pause menu attributes
    sf::Font font;
    sf::Text* titleText = nullptr;
    std::vector<PauseMenuButton> buttons;
    sf::RectangleShape pauseBar1, pauseBar2;
    sf::FloatRect pauseIconBounds;
    int selectedIndex = 0;
    bool isPaused_ = false;
    bool musicOn = true;
    Action lastAction = NONE;
    float iconPulseTimer = 0.f;
    float iconAlpha = 200.f;
	// Constructor
    PauseMenu() {
        float barWidth = 12.f, barHeight = 35.f, barSpacing = 8.f, iconX = 1140.f, iconY = 20.f;
        pauseBar1.setSize({ barWidth, barHeight });
        pauseBar1.setFillColor(sf::Color(255, 255, 255, 200));
        pauseBar1.setPosition({ iconX, iconY });
        pauseBar2.setSize({ barWidth, barHeight });
        pauseBar2.setFillColor(sf::Color(255, 255, 255, 200));
        pauseBar2.setPosition({ iconX + barWidth + barSpacing, iconY });
        pauseIconBounds = sf::FloatRect({ iconX - 5.f, iconY - 5.f }, { (barWidth * 2) + barSpacing + 10.f, barHeight + 10.f });
    }
	// Destructor
    ~PauseMenu() {
        if (titleText) delete titleText;
        for (auto& btn : buttons) if (btn.text) delete btn.text;
    }
	// Load pause menu assets
    bool loadAssets(const sf::Font& f) {
        font = f;
        titleText = new sf::Text(font, "GAME PAUSED", 60);
        titleText->setFillColor(sf::Color::White);
        titleText->setOutlineColor(sf::Color::Black);
        titleText->setOutlineThickness(3.f);
        sf::FloatRect titleBounds = titleText->getLocalBounds();
        titleText->setOrigin({ titleBounds.size.x / 2.f, titleBounds.size.y / 2.f });
        titleText->setPosition({ 600.f, 200.f });
		// Define buttons
        std::vector<std::pair<std::string, Action>> buttonData = { {"CONTINUE", CONTINUE}, {"MUSIC: ON", TOGGLE_MUSIC}, {"EXIT GAME", EXIT_GAME} };
        float buttonWidth = 300.f, buttonHeight = 60.f, startY = 350.f, spacing = 80.f;
		// Create buttons
        for (size_t i = 0; i < buttonData.size(); i++) {
            PauseMenuButton btn;
            btn.shape.setSize({ buttonWidth, buttonHeight });
            btn.shape.setFillColor(sf::Color(0, 0, 0, 100));
            btn.shape.setOutlineColor(sf::Color(255, 255, 255, 150));
            btn.shape.setOutlineThickness(2.f);
            btn.shape.setOrigin({ buttonWidth / 2.f, buttonHeight / 2.f });
            btn.shape.setPosition({ 600.f, startY + (i * spacing) });
            btn.text = new sf::Text(font, buttonData[i].first, 28);
            btn.text->setFillColor(sf::Color::White);
            sf::FloatRect textBounds = btn.text->getLocalBounds();
            btn.text->setOrigin({ textBounds.size.x / 2.f, textBounds.size.y / 2.f });
            btn.text->setPosition({ 600.f, startY + (i * spacing) - 5.f });
            btn.action = buttonData[i].second;
            buttons.push_back(btn);
        }
        return true;
    }
	// Handle input for pause menu
    void handleInput(const sf::Event& event, const sf::RenderWindow& window) {
        if (!isPaused_) {
            if (const auto* clickEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
                if (clickEvent->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords({ clickEvent->position.x, clickEvent->position.y });
                    if (pauseIconBounds.contains(mousePos)) isPaused_ = true;
                }
            }
            if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Escape || keyEvent->code == sf::Keyboard::Key::P) isPaused_ = true;
            }
            return;
        }
		// Handle pause menu keyboard input
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Up) {
                selectedIndex = (selectedIndex - 1 + static_cast<int>(buttons.size())) % static_cast<int>(buttons.size());
            } else if (keyEvent->code == sf::Keyboard::Key::Down) {
                selectedIndex = (selectedIndex + 1) % static_cast<int>(buttons.size());
            } else if (keyEvent->code == sf::Keyboard::Key::Enter) {
                lastAction = static_cast<Action>(buttons[selectedIndex].action);
                if (lastAction == TOGGLE_MUSIC) {
                    musicOn = !musicOn;
                    buttons[selectedIndex].text->setString(musicOn ? "MUSIC: ON" : "MUSIC: OFF");
                } else if (lastAction == CONTINUE) {
                    isPaused_ = false;
                }
            } else if (keyEvent->code == sf::Keyboard::Key::Escape) {
                isPaused_ = false;
                lastAction = CONTINUE;
            }
        }
		// Handle pause menu mouse movement
        if (const auto* clickEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (clickEvent->button == sf::Mouse::Button::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords({ clickEvent->position.x, clickEvent->position.y });
                for (size_t i = 0; i < buttons.size(); i++) {
                    if (buttons[i].shape.getGlobalBounds().contains(mousePos)) {
                        lastAction = static_cast<Action>(buttons[i].action);
                        if (lastAction == TOGGLE_MUSIC) {
                            musicOn = !musicOn;
                            buttons[i].text->setString(musicOn ? "MUSIC: ON" : "MUSIC: OFF");
                        } else if (lastAction == CONTINUE) {
                            isPaused_ = false;
                        }
                    }
                }
            }
        }
    }
	// Update pause menu state
    void update(sf::Time dt) {
        iconPulseTimer += dt.asSeconds();
        iconAlpha = 180.f + 40.f * std::sin(iconPulseTimer * 3.f);
        sf::Color iconColor(255, 255, 255, static_cast<std::uint8_t>(iconAlpha));
        pauseBar1.setFillColor(iconColor);
        pauseBar2.setFillColor(iconColor);
		// Update button hover states
        for (size_t i = 0; i < buttons.size(); i++) {
            buttons[i].isHovered = (static_cast<int>(i) == selectedIndex);
            if (buttons[i].isHovered) {
                buttons[i].shape.setFillColor(sf::Color(50, 50, 80, 180));
                buttons[i].shape.setOutlineColor(sf::Color::White);
            } else {
                buttons[i].shape.setFillColor(sf::Color(0, 0, 0, 100));
                buttons[i].shape.setOutlineColor(sf::Color(255, 255, 255, 150));
            }
        }
    }
	// Render pause icon
    void renderIcon(sf::RenderWindow& target) { target.draw(pauseBar1); target.draw(pauseBar2); }
	// Render pause menu
    void renderMenu(sf::RenderWindow& target) {
        sf::RectangleShape overlay({ 1200.f, 900.f });
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        target.draw(overlay);
        if (titleText) target.draw(*titleText);
        for (auto& btn : buttons) { target.draw(btn.shape); if (btn.text) target.draw(*btn.text); }
    }
	// Getters and setters
    bool isPaused() const { return isPaused_; }
    void setPaused(bool p) { isPaused_ = p; }
    Action getLastAction() { return lastAction; }
    void resetAction() { lastAction = NONE; }
    bool isMusicOn() const { return musicOn; }
};

// ============================================================================
// GAME OVER
// ============================================================================
struct GameOver {
	// Animation frames
    const std::vector<sf::Texture>* frames = nullptr;
    sf::Sprite* animSprite = nullptr;
    int currentFrame = 0;
    float duration = 0.1f;
    float elapsedTime = 0.f;
    Menu menu;
    bool showMenu = false;
	// Destructor
    ~GameOver() { if (animSprite) delete animSprite; }
	// Initialize game over animation and menu
    void init(const std::vector<sf::Texture>& f, float frameDuration, const sf::Texture& gameOverBg) {
        frames = &f;
        duration = frameDuration;
        if (animSprite) delete animSprite;
        if (!frames->empty()) {
            animSprite = new sf::Sprite((*frames)[0]);
            animSprite->setScale({ 2.5f, 2.5f });
            sf::FloatRect bounds = animSprite->getLocalBounds();
            animSprite->setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
            animSprite->setPosition({ 600.f, 450.f });
        }
        menu.loadAssets(gameOverBg);
        reset();
    }
	// Reset game over state
    void reset() {
        showMenu = false;
        currentFrame = 0;
        elapsedTime = 0.f;
        menu.reset();
        if (frames && !frames->empty() && animSprite) animSprite->setTexture((*frames)[0]);
    }
	// Handle input for game over menu
    void handleInput(const sf::Event& event, const sf::RenderWindow& window) {
        if (showMenu) menu.handleInput(event, window);
    }
	// Update game over animation and menu
    void update(sf::Time dt) {
        if (!showMenu) {
            if (frames && !frames->empty() && animSprite) {
                elapsedTime += dt.asSeconds();
                if (elapsedTime >= duration) {
                    elapsedTime = 0.f;
                    currentFrame++;
                    if (currentFrame >= static_cast<int>(frames->size())) {
                        showMenu = true;
                    } else {
                        animSprite->setTexture((*frames)[currentFrame], true);
                    }
                }
            } else {
                showMenu = true;
            }
        }
        menu.update(dt);
    }
	// Render game over animation or menu
    void render(sf::RenderWindow& window) {
        if (!showMenu && animSprite) window.draw(*animSprite);
        else menu.render(window);
    }
	// Getters for menu actions
    bool isRetryPressed() const { return menu.isStartPressed(); }
    bool isOptionsPressed() const { return menu.isOptionsPressed(); }
    bool isHighScorePressed() const { return menu.isHighScorePressed(); }
    bool isExitPressed() const { return menu.isExitPressed(); }
};

// ============================================================================
// OPTIONS MENU
// ============================================================================
struct OptionsMenu {
	// Background
    sf::Font font;
    sf::Sprite* background = nullptr;
    sf::Texture optionsBgTex;

    // Buttons
    std::vector<sf::RectangleShape> buttons;
    std::vector<sf::Text*> buttonTexts;
    int selectedIndex = 0;

    // Music state
    bool musicOn = true;

    // Image display
    bool showingImage = false;
    int currentImageIndex = -1;
    sf::Texture image1Tex, image2Tex;
    sf::Sprite* imageSprite = nullptr;

    // Actions
    bool backPressed = false;
	// Destructor
    ~OptionsMenu() {
        if (background) delete background;
        if (imageSprite) delete imageSprite;
        for (auto* text : buttonTexts) if (text) delete text;
    }
	// Load options menu assets
    bool loadAssets(const sf::Font& f) {
        font = f;

        // Load options background from file
        if (!optionsBgTex.loadFromFile("assests/textures/menu/options.png")) {
            // Fallback: create a dark background if file not found
            sf::Image img;
            img.resize({ 1200, 900 }, sf::Color(20, 20, 40));
            optionsBgTex.loadFromImage(img);
        }
		// Create background sprite
        background = new sf::Sprite(optionsBgTex);
        background->setScale({
            1200.f / static_cast<float>(optionsBgTex.getSize().x),
            900.f / static_cast<float>(optionsBgTex.getSize().y)
        });

        // Load images for controls and credits
        if (!image1Tex.loadFromFile("assests/textures/menu/controls.png")) {
            sf::Image img;
            img.resize({ 800, 600 }, sf::Color(50, 50, 100));
            image1Tex.loadFromImage(img);
        }
        if (!image2Tex.loadFromFile("assests/textures/menu/credits.png")) {
            sf::Image img;
            img.resize({ 800, 600 }, sf::Color(100, 50, 50));
            image2Tex.loadFromImage(img);
        }

        // Create 3 buttons only: Music Toggle, Controls, Credits
        std::vector<std::string> labels = { "MUSIC: ON", "CONTROLS", "CREDITS" };
        float buttonWidth = 300.f, buttonHeight = 60.f;
        float startY = 350.f, spacing = 100.f;
		// Create buttons and texts
        for (size_t i = 0; i < labels.size(); i++) {
            sf::RectangleShape btn({ buttonWidth, buttonHeight });
            btn.setFillColor(sf::Color::Transparent);  // Fully transparent
            btn.setOutlineColor(sf::Color(255, 255, 255, 150));
            btn.setOutlineThickness(2.f);
            btn.setOrigin({ buttonWidth / 2.f, buttonHeight / 2.f });
            btn.setPosition({ 600.f, startY + (i * spacing) });
            buttons.push_back(btn);
			// Create button text
            sf::Text* text = new sf::Text(font, labels[i], 28);
            text->setFillColor(sf::Color::White);
            sf::FloatRect textBounds = text->getLocalBounds();
            text->setOrigin({ textBounds.size.x / 2.f, textBounds.size.y / 2.f });
            text->setPosition({ 600.f, startY + (i * spacing) - 5.f });
            buttonTexts.push_back(text);
        }

        return true;
    }
	// Handle input for options menu
    void handleInput(const sf::Event& event, const sf::RenderWindow& window) {
        // If showing an image, ESC closes it
        if (showingImage) {
            if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Escape) {
                    showingImage = false;
                    currentImageIndex = -1;
                    if (imageSprite) {
                        delete imageSprite;
                        imageSprite = nullptr;
                    }
                }
            }
            return;
        }

        // Keyboard navigation
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Up) {
                selectedIndex = (selectedIndex - 1 + static_cast<int>(buttons.size())) % static_cast<int>(buttons.size());
            }
            else if (keyEvent->code == sf::Keyboard::Key::Down) {
                selectedIndex = (selectedIndex + 1) % static_cast<int>(buttons.size());
            }
            else if (keyEvent->code == sf::Keyboard::Key::Enter) {
                executeAction(selectedIndex);
            }
            else if (keyEvent->code == sf::Keyboard::Key::Escape) {
                backPressed = true;  // ESC goes back to menu
            }
        }

        // Mouse hover
        if (const auto* moveEvent = event.getIf<sf::Event::MouseMoved>()) {
            sf::Vector2f mousePos = window.mapPixelToCoords({ moveEvent->position.x, moveEvent->position.y });
            for (size_t i = 0; i < buttons.size(); i++) {
                if (buttons[i].getGlobalBounds().contains(mousePos)) {
                    selectedIndex = static_cast<int>(i);
                }
            }
        }

        // Mouse click
        if (const auto* clickEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (clickEvent->button == sf::Mouse::Button::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords({ clickEvent->position.x, clickEvent->position.y });
                for (size_t i = 0; i < buttons.size(); i++) {
                    if (buttons[i].getGlobalBounds().contains(mousePos)) {
                        executeAction(static_cast<int>(i));
                    }
                }
            }
        }
    }
	// Execute action based on selected button
    void executeAction(int index) {
        switch (index) {
        case 0:  // Toggle Music
            musicOn = !musicOn;
            buttonTexts[0]->setString(musicOn ? "MUSIC: ON" : "MUSIC: OFF");
            {
                sf::FloatRect textBounds = buttonTexts[0]->getLocalBounds();
                buttonTexts[0]->setOrigin({ textBounds.size.x / 2.f, textBounds.size.y / 2.f });
            }
            break;
        case 1:  // Show Controls image
            showImage(0);
            break;
        case 2:  // Show Credits image
            showImage(1);
            break;
        }
    }
	// Show image overlay
    void showImage(int imageIndex) {
        currentImageIndex = imageIndex;
        showingImage = true;
		// Create sprite for the image
        if (imageSprite) delete imageSprite;

        sf::Texture& tex = (imageIndex == 0) ? image1Tex : image2Tex;
        imageSprite = new sf::Sprite(tex);
		// Center the image
        sf::FloatRect bounds = imageSprite->getLocalBounds();
        imageSprite->setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
        imageSprite->setPosition({ 600.f, 450.f });

        // Scale to fit screen
        float scaleX = 1200.f / bounds.size.x;
        float scaleY = 900.f / bounds.size.y;
        float scale = std::min(scaleX, scaleY);
        imageSprite->setScale({ scale, scale });
    }
	// Update options menu state
    void update(sf::Time dt) {
        for (size_t i = 0; i < buttons.size(); i++) {
            if (static_cast<int>(i) == selectedIndex) {
                buttons[i].setFillColor(sf::Color(255, 255, 255, 30));  // Slight highlight when selected
                buttons[i].setOutlineColor(sf::Color::White);
            }
            else {
                buttons[i].setFillColor(sf::Color::Transparent);
                buttons[i].setOutlineColor(sf::Color(255, 255, 255, 150));
            }
        }
    }
	// Render options menu
    void render(sf::RenderWindow& target) {
        if (background) target.draw(*background);

        // Draw buttons
        for (size_t i = 0; i < buttons.size(); i++) {
            target.draw(buttons[i]);
            if (buttonTexts[i]) target.draw(*buttonTexts[i]);
        }

        // Draw hint text
        sf::Text hint(font, "Press ESC to go back", 20);
        hint.setFillColor(sf::Color(200, 200, 200));
        hint.setPosition({ 50.f, 850.f });
        target.draw(hint);

        // If showing an image, draw overlay and image
        if (showingImage && imageSprite) {
            sf::RectangleShape overlay({ 1200.f, 900.f });
            overlay.setFillColor(sf::Color(0, 0, 0, 200));
            target.draw(overlay);

            target.draw(*imageSprite);

            sf::Text closeHint(font, "Press ESC to close", 24);
            closeHint.setFillColor(sf::Color::White);
            sf::FloatRect hintBounds = closeHint.getLocalBounds();
            closeHint.setOrigin({ hintBounds.size.x / 2.f, hintBounds.size.y / 2.f });
            closeHint.setPosition({ 600.f, 850.f });
            target.draw(closeHint);
        }
    }
	// Getters
    bool isBackPressed() const { return backPressed; }
    bool isMusicOn() const { return musicOn; }
	// Reset options menu state
    void reset() {
        backPressed = false;
        selectedIndex = 0;
        showingImage = false;
        currentImageIndex = -1;
        if (imageSprite) {
            delete imageSprite;
            imageSprite = nullptr;
        }
    }
};

// ============================================================================
// MAIN GAME CLASS
// ============================================================================
class Game {
public:
	// Window and timing
    sf::RenderWindow window;
    sf::Clock clock;
    GameState currentState = GameState::MENU;

    // Audio
    sf::Music gameMusic, menuMusic;
    sf::SoundBuffer shootBuffer, explosionBuffer, bossHitBuffer;
    sf::Sound shootSound, explosionSound, bossHitSound;

    // Textures
    sf::Texture bgTex, menuBgTex, highScoreBgTex, gameOverBgTex;
    sf::Texture coinTex, healTex, boltTex, asteroidTex, bulletTex, playerBulletTex, bossTex;
    std::vector<sf::Texture> playerTextures, enemyTextures;
    std::vector<sf::Texture> explosionFrames, playerExplosionFrames, gameOverExplosionFrames;
    sf::Sprite* highScoreSprite = nullptr;

    // Game Objects
    Player* player = nullptr;
    ScrollingBackground* background = nullptr;
    StarField* stars = nullptr;
    Boss* activeBoss = nullptr;
    std::vector<Enemy> enemies;
    std::vector<Bullet> enemyBullets, playerBullets;
    std::vector<Explosion> explosions;
    std::vector<Asteroid> asteroids;
    std::vector<Powerup> powerups;

    // UI
    Menu menu;
    HUD hud;
    GameOver gameOverScreen;
    PauseMenu pauseMenu;
    ScreenShake screenShake;
    OptionsMenu optionsMenu; 

    // State
	int currentHighScore = 0;  // Loaded from file
	bool bossSpawned = false;  // Track if a boss is currently spawned
    int bossCount = 0;           // Track number of bosses defeated
    int nextBossScore = 500;     // Score threshold for next boss
	float spawnTimer = 0.f, spawnTimerMax = 4.25f; // Enemy spawn timer
	float asteroidSpawnTimer = 0.f, asteroidSpawnTimerMax = 20.f; // Asteroid spawn timer
    GameState previousState = GameState::MENU;  // Track where we came from

    // Loading screen
    float loadingTimer = 0.f; 
    float loadingDuration = 3.f;  // 3 seconds
    sf::Texture loadingBgTex;
    sf::Sprite* loadingSprite = nullptr;

	// Constructor
    Game() : shootSound(shootBuffer), explosionSound(explosionBuffer), bossHitSound(bossHitBuffer) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        window.create(sf::VideoMode({ 1200, 900 }), "Space Shooter", sf::Style::Close | sf::Style::Titlebar);
        window.setFramerateLimit(144);
        currentState = GameState::LOADING;
        currentHighScore = loadHighScore();
        
        // Load and display loading screen FIRST
        if (!loadingBgTex.loadFromFile("assests/textures/menu/loading.png")) {
            sf::Image img;
            img.resize({ 1200, 900 }, sf::Color(20, 20, 40));
            loadingBgTex.loadFromImage(img);
        }
        loadingSprite = new sf::Sprite(loadingBgTex);
        loadingSprite->setScale({
            1200.f / static_cast<float>(loadingBgTex.getSize().x),
            900.f / static_cast<float>(loadingBgTex.getSize().y)
        });
        
        // Render loading screen immediately
        window.clear();
        window.draw(*loadingSprite);
        window.display();
        
        // Now load all other assets (loading screen is visible during this)
        loadAssets();
        initObjects();
        
        // Transition to menu
        currentState = GameState::MENU;
        menuMusic.play();
    }
	// Destructor
    ~Game() {
        if (player) delete player;
        if (background) delete background;
        if (stars) delete stars;
        if (activeBoss) delete activeBoss;
		if (highScoreSprite) delete highScoreSprite;
		if (loadingSprite) delete loadingSprite;
    }
	// Load high score from file
    int loadHighScore() {
        std::ifstream file("highscore.txt");
        int score = 0;
        if (file.is_open()) { file >> score; file.close(); }
        return score;
    }
	// Save high score to file
    void saveHighScore(int score) {
        std::ofstream file("highscore.txt");
        if (file.is_open()) { file << score; file.close(); }
    }
	// LOAD ALL ASSETS
    void loadAssets() {
        // Audio
        gameMusic.openFromFile("assests/audio/gamebm.mp3");
        gameMusic.setLooping(true); gameMusic.setVolume(40.f);
        menuMusic.openFromFile("assests/audio/menubm.mp3");
        menuMusic.setLooping(true); menuMusic.setVolume(50.f);
        shootBuffer.loadFromFile("assests/audio/shoot.mp3");
        shootSound.setBuffer(shootBuffer); shootSound.setVolume(10.f);
        explosionBuffer.loadFromFile("assests/audio/explosion.mp3");
        explosionSound.setBuffer(explosionBuffer); explosionSound.setVolume(80.f);
        bossHitBuffer.loadFromFile("assests/audio/explosion.mp3");
        bossHitSound.setBuffer(bossHitBuffer); bossHitSound.setPitch(2.0f); bossHitSound.setVolume(60.f);

        // Textures
        if (!bgTex.loadFromFile("assests/textures/background/background22.png")) {
            sf::Image img; img.resize({ 800, 600 }, sf::Color::Black); bgTex.loadFromImage(img);
        }
        if (!menuBgTex.loadFromFile("assests/textures/menu/menubg.png")) menuBgTex = bgTex;
        if (!highScoreBgTex.loadFromFile("assests/textures/menu/highscore.png")) highScoreBgTex = menuBgTex;
        
        // Set texture AFTER loading it
        highScoreSprite = new sf::Sprite(highScoreBgTex);
        highScoreSprite->setScale({ 
            1200.f / static_cast<float>(highScoreBgTex.getSize().x), 
            900.f / static_cast<float>(highScoreBgTex.getSize().y) 
        });
		// Game over background
        if (!gameOverBgTex.loadFromFile("assests/textures/menu/menubg4.png")) gameOverBgTex = menuBgTex;
		// Power-up, asteroid and boss textures
        coinTex.loadFromFile("assests/textures/powerups/p3.png");
        healTex.loadFromFile("assests/textures/powerups/p2.png");
        boltTex.loadFromFile("assests/textures/powerups/p1.png");
        asteroidTex.loadFromFile("assests/textures/enemy/asteroid.png");
        bulletTex.loadFromFile("assests/textures/enemy/bullet2.png");
        playerBulletTex.loadFromFile("assests/textures/player/bullet2.png");
        bossTex.loadFromFile("assests/textures/enemy/boss.png");

        // Explosion frames
        for (int i = 1; i <= 5; i++) {
            sf::Texture tex;
            if (tex.loadFromFile("assests/textures/enemy animation/explosion" + std::to_string(i) + ".png"))
                explosionFrames.push_back(tex);
        }
        playerExplosionFrames = explosionFrames;

        // Game over explosion
        sf::Image explosionSheet;
        if (explosionSheet.loadFromFile("assests/textures/player/explosion.jpg")) {
            explosionSheet.createMaskFromColor(sf::Color::White);
            int cols = 3, rows = 2;
            int fw = explosionSheet.getSize().x / cols, fh = explosionSheet.getSize().y / rows;
            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {
                    sf::Image tempImg; tempImg.resize({ static_cast<unsigned>(fw), static_cast<unsigned>(fh) });
                    tempImg.copy(explosionSheet, { 0, 0 }, sf::IntRect({ x * fw, y * fh }, { fw, fh }));
                    sf::Texture frame; frame.loadFromImage(tempImg);
                    gameOverExplosionFrames.push_back(frame);
                }
            }
        } else {
            gameOverExplosionFrames = playerExplosionFrames;
        }

        // Player textures
        sf::Texture t1, t2, t3, t4, t5;
        t1.loadFromFile("assests/textures/player/spaceship1.png");
        t2.loadFromFile("assests/textures/player/spaceship2.png");
        t3.loadFromFile("assests/textures/player/spaceship3.png");
        t4.loadFromFile("assests/textures/player/spaceship4.png");
        t5.loadFromFile("assests/textures/player/spaceship5.png");
        playerTextures = { t1, t2, t3, t4, t5 };

        // Enemy textures
        for (int i = 1; i <= 6; i++) {
            sf::Texture tex;
            if (tex.loadFromFile("assests/textures/enemy/enemy" + std::to_string(i) + ".png"))
                enemyTextures.push_back(tex);
            else enemyTextures.push_back(t3);
        }
    }

	// Initialize game objects
    void initObjects() {
        menu.loadAssets(menuBgTex);
        hud.loadAssets();
        gameOverScreen.init(gameOverExplosionFrames, 0.10f, gameOverBgTex);
        pauseMenu.loadAssets(hud.getFont());
        optionsMenu.loadAssets(hud.getFont());  // Changed - only pass font now

        player = new Player(playerTextures);
        player->setPosition(600.f, 750.f);
        background = new ScrollingBackground(bgTex, 50.f);
        stars = new StarField(25, window.getSize());
    }

	// Reset game state
    void resetGame() {
        enemies.clear();
        if (activeBoss) { delete activeBoss; activeBoss = nullptr; }
        bossSpawned = false;
        bossCount = 0;              // Reset boss counter
        nextBossScore = 500;        // Reset next boss threshold
        enemyBullets.clear(); playerBullets.clear();
        explosions.clear(); asteroids.clear(); powerups.clear();
        hud.reset(); hud.loadAssets();
        player->setPosition(600.f, 750.f);
    }

	// Main game loop
    void run() {
        while (window.isOpen()) {
            sf::Time dt = clock.restart();
            processEvents();
            update(dt);
            render();
        }
    }

	// Event processing
    void processEvents() {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (currentState == GameState::MENU) menu.handleInput(*event, window);
            else if (currentState == GameState::PLAYING) pauseMenu.handleInput(*event, window);
            else if (currentState == GameState::GAME_OVER) gameOverScreen.handleInput(*event, window);
            else if (currentState == GameState::OPTIONS) optionsMenu.handleInput(*event, window);  // Add this
            else if (currentState == GameState::HIGHSCORE) {
                if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyEvent->code == sf::Keyboard::Key::Escape) {
						currentState = previousState;  // Return to previous state
                        if (previousState == GameState::MENU) {
                            menu.reset();
                        }
                        else if (previousState == GameState::GAME_OVER) {
                            gameOverScreen.menu.reset();  // Reset the game over menu's button states
                        }
                    }
                }
            }
        }
    }

	// Update function
    void update(sf::Time dt) {
        // game state menu
        if (currentState == GameState::MENU) {
            menu.update(dt);
            if (menuMusic.getStatus() != sf::Music::Status::Playing) menuMusic.play();
            if (menu.isStartPressed()) {
                currentState = GameState::PLAYING;
                menuMusic.stop(); gameMusic.play(); menu.reset();
            }
            if (menu.isHighScorePressed()) { 
                previousState = GameState::MENU;
                currentState = GameState::HIGHSCORE; menu.reset(); 
            }
            if (menu.isOptionsPressed()) {
                previousState = GameState::MENU;
                currentState = GameState::OPTIONS; menu.reset(); 
            }
            if (menu.isExitPressed()) window.close();
        }
        // game state options
        else if (currentState == GameState::OPTIONS) {
            optionsMenu.update(dt);
            // Apply music setting
            menuMusic.setVolume(optionsMenu.isMusicOn() ? 50.f : 0.f);
            if (optionsMenu.isBackPressed()) {
                currentState = previousState;
                optionsMenu.reset();
            }
        }
		// game state playing
        else if (currentState == GameState::PLAYING) {
            updatePlaying(dt);
        }

		// game state game over
        else if (currentState == GameState::GAME_OVER) {
            gameOverScreen.update(dt);
            gameMusic.stop();
            if (menuMusic.getStatus() != sf::Music::Status::Playing) menuMusic.play();
            if (gameOverScreen.isRetryPressed()) { 
                menuMusic.stop();
                resetGame();
                currentState = GameState::PLAYING;
                gameMusic.play();
            }
            else if (gameOverScreen.isOptionsPressed()) {
                previousState = GameState::GAME_OVER;
                currentState = GameState::OPTIONS;
                gameOverScreen.menu.reset();
            }
            else if (gameOverScreen.isHighScorePressed()) {
                previousState = GameState::GAME_OVER;
                currentState = GameState::HIGHSCORE; 
                gameOverScreen.menu.reset(); 
			}
            else if (gameOverScreen.isExitPressed()) window.close();
        }
    }

	// Update playing state
    void updatePlaying(sf::Time dt) {
		// Handle pause menu
        pauseMenu.update(dt);
		// Check pause menu actions
        auto pauseAction = pauseMenu.getLastAction();
        if (pauseAction == PauseMenu::TOGGLE_MUSIC) {
            gameMusic.setVolume(pauseMenu.isMusicOn() ? 40.f : 0.f);
            pauseMenu.resetAction();
        }
        else if (pauseAction == PauseMenu::EXIT_GAME) {
            resetGame(); currentState = GameState::MENU;
            pauseMenu.resetAction(); pauseMenu.setPaused(false);
        }
		// If paused, skip updates
        if (pauseMenu.isPaused()) return;
		// Update game objects
        background->update(dt);
        stars->update(dt);
        player->update(dt, window.getSize());
        hud.update(dt);
        screenShake.update(dt);

        // Player shooting
        if (player->canAttack()) {
            player->resetAttackTimer();
            shootSound.play();
            float angleRad = player->getRotation().asRadians();
            float dirX = std::sin(angleRad), dirY = -std::cos(angleRad);
            playerBullets.emplace_back(playerBulletTex, player->getPosition().x - 12.5f, player->getPosition().y, dirX, dirY);
            if (player->isTripleShotActive()) {
                float offsetRad = 15.f * (3.14159f / 180.f);
                playerBullets.emplace_back(playerBulletTex, player->getPosition().x - 15.f, player->getPosition().y,
                    std::sin(angleRad - offsetRad), -std::cos(angleRad - offsetRad));
                playerBullets.emplace_back(playerBulletTex, player->getPosition().x - 15.f, player->getPosition().y,
                    std::sin(angleRad + offsetRad), -std::cos(angleRad + offsetRad));
            }
        }

        // Enemy spawning
        if (!activeBoss) {
            spawnTimer += dt.asSeconds();
            if (spawnTimer >= spawnTimerMax / hud.getSpawnRateMultiplier()) {
                spawnTimer = 0.f;
                float randX = static_cast<float>(rand() % (window.getSize().x - 50));
                int texIndex = enemyTextures.empty() ? 0 : rand() % static_cast<int>(enemyTextures.size());
                enemies.emplace_back(enemyTextures[texIndex], randX, -50.f);
            }
        }

        // Update enemies
        for (size_t i = 0; i < enemies.size(); i++) {
            enemies[i].update(dt, enemyBullets, bulletTex);
            if (player->getGlobalBounds().findIntersection(enemies[i].getGlobalBounds())) {
                for (int k = 0; k < 5 && hud.isAlive(); k++) hud.loseHeart();
                explosionSound.play();
                explosions.emplace_back(&explosionFrames, enemies[i].getPosition().x, enemies[i].getPosition().y);
                screenShake.shake(4.f, 0.3f);
                if (!hud.isAlive()) { gameOverScreen.reset(); currentState = GameState::GAME_OVER; }
                enemies.erase(enemies.begin() + i); i--;
            }
            else if (enemies[i].getPosition().y > window.getSize().y) {
                enemies.erase(enemies.begin() + i); i--;
            }
        }
		// Update enemy bullets
        if (!hud.isAlive()) {
            if (hud.getScore() > currentHighScore) { currentHighScore = hud.getScore(); saveHighScore(currentHighScore); }
            gameOverScreen.reset(); currentState = GameState::GAME_OVER;
        }

        // Asteroid spawning
        asteroidSpawnTimer += dt.asSeconds();
        if (asteroidSpawnTimer >= asteroidSpawnTimerMax) {
            asteroidSpawnTimer = 0.f;
            asteroids.emplace_back(asteroidTex, static_cast<float>(rand() % window.getSize().x), -50.f);
        }

        // Update Asteroids 
        for (auto it = asteroids.begin(); it != asteroids.end();) {
            it->update(dt);
            if (player->getGlobalBounds().findIntersection(it->getGlobalBounds())) {
                for (int k = 0; k < 5 && hud.isAlive(); k++) hud.loseHeart();
                explosionSound.play();
                screenShake.shake(4.f, 0.2f);
                if (!hud.isAlive()) { gameOverScreen.reset(); currentState = GameState::GAME_OVER; }
                it = asteroids.erase(it); continue;
            }
			// Player bullets vs Asteroids
            for (auto bulletIt = playerBullets.begin(); bulletIt != playerBullets.end();) {
				// Bullet hits asteroid
                if (it->getGlobalBounds().findIntersection(bulletIt->getGlobalBounds())) {
                    it->takeDamage(1);
                    explosions.emplace_back(&explosionFrames, it->getPosition().x, it->getPosition().y);
                    bulletIt = playerBullets.erase(bulletIt);
                    screenShake.shake(4.f, 0.15f);
                }
				// No hit, move to next bullet
                else ++bulletIt;
            }
			// Remove asteroid if destroyed or out of bounds
            if (!it->isAlive) {
				explosionSound.play();
                hud.addScore(30);
                explosions.emplace_back(&explosionFrames, it->getPosition().x, it->getPosition().y);
                it = asteroids.erase(it);
            }
			//  Out of bounds
            else if (it->getPosition().y > window.getSize().y) it = asteroids.erase(it);
			// Continue to next asteroid
            else ++it;
        }

        
		// Boss spawning
        if (hud.getScore() >= nextBossScore && !activeBoss) {
            int bossHealth = 250 + (bossCount * 100);
            float bossBulletSpeed = 300.f + (std::min(bossCount, 5) * 30.f);
            activeBoss = new Boss(bossTex, bossHealth, bossBulletSpeed);
        }
		// Update Boss
        if (activeBoss) {
            activeBoss->update(dt, window.getSize(), enemyBullets, bulletTex);
            for (auto it = playerBullets.begin(); it != playerBullets.end();) {
				// Player bullet hits boss
                if (activeBoss->getGlobalBounds().findIntersection(it->getGlobalBounds())) {
                    activeBoss->takeDamage(10);
                    bossHitSound.play();
                    explosions.emplace_back(&explosionFrames, it->getPosition().x, it->getPosition().y);
                    screenShake.shake(4.f, 0.1f);
                    it = playerBullets.erase(it);
					// Check if boss defeated
                    if (!activeBoss->isAlive()) {
                        hud.addScore(100); hud.addEnemyDefeated();
                        explosions.emplace_back(&explosionFrames, activeBoss->getPosition().x, activeBoss->getPosition().y);
                        screenShake.shake(12.5f, 0.5f);
                        powerups.emplace_back(healTex, Powerup::HEAL, activeBoss->getPosition().x, activeBoss->getPosition().y);
                        delete activeBoss; activeBoss = nullptr;
                        bossCount++;
                        nextBossScore += 600;
                        break;
                    }
                } else ++it;
            }
			// Boss vs Player
            if (activeBoss && activeBoss->getGlobalBounds().findIntersection(player->getGlobalBounds())) {
                hud.loseHeart(); screenShake.shake(10.f, 0.2f);
            }
        }


        // Player bullets vs enemies
        for (size_t i = 0; i < playerBullets.size(); i++) {
            playerBullets[i].update(dt);
            bool removed = false;
            for (size_t k = 0; k < enemies.size(); k++) {
				// Bullet hits enemy
                if (playerBullets[i].getGlobalBounds().findIntersection(enemies[k].getGlobalBounds())) {
                    sf::Vector2f enemyPos = enemies[k].getPosition();
                    enemies[k].takeDamage(10);
                    explosions.emplace_back(&explosionFrames, enemyPos.x, enemyPos.y);
                    playerBullets.erase(playerBullets.begin() + i); removed = true;
					// Check if enemy destroyed
                    if (enemies[k].getHp() <= 0) {
                        explosionSound.play();
                        hud.addScore(10); hud.addEnemyDefeated();
						// 20% chance to drop powerup
                        if (rand() % 2 == 0) {
                            int typeId = rand() % 3;
                            auto type = static_cast<Powerup::Type>(typeId);
                            sf::Texture* tex = (type == Powerup::SCORE_BONUS) ? &coinTex : 
                                              (type == Powerup::HEAL) ? &healTex : &boltTex;
                            powerups.emplace_back(*tex, type, enemyPos.x, enemyPos.y);
                        }
						// Remove enemy
                        enemies.erase(enemies.begin() + k);
                        screenShake.shake(4.f, 0.2f);
                    } 
					// Bullet processed, exit enemy loop
                    else {
                        screenShake.shake(4.f, 0.1f);
                    }
                    break;
                }
            }
			// Remove bullet if out of bounds
            if (!removed && playerBullets[i].getGlobalBounds().position.y < 0) {
                playerBullets.erase(playerBullets.begin() + i); i--;
            } else if (removed) i--;
        }

        // Enemy bullets vs player
        for (size_t i = 0; i < enemyBullets.size(); i++) {
            enemyBullets[i].update(dt);
			// Bullet hits player
            if (enemyBullets[i].getGlobalBounds().findIntersection(player->getGlobalBounds())) {
                sf::Vector2f playerPos = player->getPosition();
                enemyBullets.erase(enemyBullets.begin() + i);
                hud.loseHeart();
                explosions.emplace_back(&playerExplosionFrames, playerPos.x, playerPos.y);
				// Check if player is dead
                if (!hud.isAlive()) {
                    if (hud.getScore() > currentHighScore) { currentHighScore = hud.getScore(); saveHighScore(currentHighScore); }
                    gameOverScreen.reset();
                    currentState = GameState::GAME_OVER;
                }
                screenShake.shake(4.f, 0.10f);
                i--;
                continue;
            }
            // Remove bullet if out of bounds
			else if (enemyBullets[i].getGlobalBounds().position.y > window.getSize().y) {
                enemyBullets.erase(enemyBullets.begin() + i); i--;
            }
        }

        // Update explosions
        for (size_t i = 0; i < explosions.size(); i++) {
            explosions[i].update(dt);
            if (explosions[i].isFinished()) {
                explosions.erase(explosions.begin() + i); i--;
            }
        }

        // Powerups
        for (size_t i = 0; i < powerups.size(); i++) {
            powerups[i].update(dt);
			// Check for collection by player
            if (powerups[i].getGlobalBounds().findIntersection(player->getGlobalBounds())) {
                switch (powerups[i].getType()) {
                case Powerup::SCORE_BONUS: hud.addScore(50); hud.showPowerup("+50 SCORE!"); break;
                case Powerup::HEAL: hud.heal(3); hud.showPowerup("+3 HEALTH!"); break;
                case Powerup::TRIPLE_SHOT: player->activateTripleShot(10.f); hud.showPowerup("TRIPLE SHOT!");  break;
                }
                powerups.erase(powerups.begin() + i); i--;
            }
			// Remove if out of bounds
            else if (powerups[i].getGlobalBounds().position.y > window.getSize().y) {
                powerups.erase(powerups.begin() + i); i--;
            }
        }
    }

	// RENDER FUNCTION
    void render() {
        window.clear();
		// game state menu
        if (currentState == GameState::MENU) {
            window.setView(window.getDefaultView());
            menu.render(window);
        }
		// game state options
        else if (currentState == GameState::OPTIONS) {
            window.setView(window.getDefaultView());
            optionsMenu.render(window);
        }
		// game state playing
        else if (currentState == GameState::PLAYING) {
			// Apply screen shake to view
            sf::View view = window.getView();
            view.setCenter({ 600.f + screenShake.getOffset().x, 450.f + screenShake.getOffset().y });
            window.setView(view);

			// Render game objects
            background->render(window);
            stars->render(window);
            for (auto& b : playerBullets) b.render(window);
            for (auto& b : enemyBullets) b.render(window);
            for (auto& p : powerups) p.render(window);
            for (auto& e : explosions) e.render(window);
            for (auto& a : asteroids) a.render(window);
            player->render(window);
            if (activeBoss) activeBoss->render(window);
            for (auto& e : enemies) e.render(window);
            hud.render(window);
            
            pauseMenu.renderIcon(window);
            if (pauseMenu.isPaused()) {
                pauseMenu.renderMenu(window);
            }
        }
		// game state high score
        else if (currentState == GameState::HIGHSCORE) {
            window.setView(window.getDefaultView());
            window.draw(*highScoreSprite);
			// Draw high score text
            sf::Text scoreNum(hud.getFont(), std::to_string(currentHighScore), 100);
            scoreNum.setFillColor(sf::Color::White);
            scoreNum.setOutlineColor(sf::Color::Black);
            scoreNum.setOutlineThickness(4.f);
            sf::FloatRect scoreBounds = scoreNum.getLocalBounds();
            scoreNum.setOrigin({ scoreBounds.size.x / 2.f, scoreBounds.size.y / 2.f });
            scoreNum.setPosition({ 600.f, 450.f });
            window.draw(scoreNum);
            
            sf::Text backText(hud.getFont(), "Press ESC", 30);
            backText.setPosition({ 50.f, 850.f });
            window.draw(backText);
        }
		// game state game over
        else if (currentState == GameState::GAME_OVER) {
            window.setView(window.getDefaultView());
            background->render(window);
            gameOverScreen.render(window);
            hud.render(window);
        }
		// Display the rendered frame
        window.display();
    }
};