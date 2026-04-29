#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <vector>
#include <deque>

using namespace std;

class platformClass {
public:
    float xPosition;
    float yPosition;
    float scale;

    float topSide;
    float bottomSide;
    float rightSide;
    float leftSide;

    sf::Sprite image;

    platformClass(float xPosition, float yPosition, sf::Sprite& sprite)
        : xPosition(xPosition), yPosition(yPosition), image(sprite) {
		// set scale and position of platform.
        scale = 4;
        image.setPosition(sf::Vector2f(xPosition, yPosition));
        image.scale(sf::Vector2f(scale, scale));

		// calculate sides of platform for collision.
        leftSide = image.getPosition().x;
        rightSide = image.getPosition().x + (image.getLocalBounds().size.x * scale);
        topSide = image.getPosition().y;
        bottomSide = image.getPosition().y + (image.getLocalBounds().size.y * scale);
    }
};

class textClass {
public:
    float xPosition;
    float yPosition;
    sf::Font font;
    sf::Text text;

    textClass(float xPosition, float yPosition, const std::string& fontPath, const std::string& str)
        : xPosition(xPosition), yPosition(yPosition), font(), text(font) {
        font.openFromFile(fontPath);
        text.setString(str);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::Black);
        text.setPosition(sf::Vector2f(xPosition, yPosition));
    }
};

class PlayerClass {
public:
    float xVelocity;
    float yVelocity;
    float clearance;

    bool rightFacing;

    bool onGround;
    bool wasOnGround;
    bool canDoubleJump;

    bool canDash;
    float dashSpeed;

    float scale;

    float speed;
    float gravity;
    float jumpStrength;
    float drag;
    float acceleration;

    float slideSpeed;
    float hasSlid;
    bool isSliding;
    bool wantsToStop;
    float heightDifference;
    float appliedHeightOffset;

    sf::Sprite image;

    PlayerClass(sf::Sprite& sprite)
        : image(sprite) {
        xVelocity = 0;
        yVelocity = 0;
        clearance = 15.0f;

        rightFacing = true;

        onGround = false;
        wasOnGround = false;
        canDoubleJump = false;

        canDash = true;
        dashSpeed = 1000.0f;

        scale = 1;
        image.setScale(sf::Vector2f(scale, scale));

        speed = 200.0f;
        gravity = 900.0f;
        jumpStrength = -400.0f;
        drag = 0.0f;
        acceleration = 0.0f;

        slideSpeed = 400.0f;
        hasSlid = false;
        isSliding = false;
        wantsToStop = false;
        heightDifference = 0.0f;
        appliedHeightOffset = 0.0f;



        sf::Vector2f center = { image.getLocalBounds().size.x / 2.0f, image.getLocalBounds().size.y / 2.0f };
        image.setOrigin(center);
    }

    void update(
        bool playerSpecial, bool playerLeft, bool playerRight, bool fastFall, bool keyUp,
        vector<platformClass>& level, float deltaTime, int playerMode,
        sf::Texture& texJump, sf::Texture& texDouble, sf::Texture& texDash, sf::Texture& texPreSlide, sf::Texture& texSlide) {

        // reset onGround each frame, collide() will set it back if still grounded.
        onGround = false;

        // coyote: if we just walked off a ledge, grant double jump
        if (wasOnGround && !onGround) {
            canDoubleJump = true;
        }

        // ground control
        if (wasOnGround) {
            acceleration = 2000.0f;
            drag = 8.0f;
        }
        // air control
        else {
            acceleration = 600.0f;
            drag = 1.5f;
        }

        // add horizontal acceleration.
        if (playerRight) {
            xVelocity += acceleration * deltaTime;
            rightFacing = true;
            image.setScale(sf::Vector2f(scale, scale));
        }
        if (playerLeft) {
            xVelocity -= acceleration * deltaTime;
            rightFacing = false;
            image.setScale(sf::Vector2f(-scale, scale));
        }

        // apply drag and gravity.
        xVelocity -= xVelocity * drag * deltaTime;
        if (fastFall) {
            gravity = 4000.0f;
        }
        else {
            gravity = 900.0f;
        }
        yVelocity += gravity * deltaTime;

        // player mode switching.
        if (!isSliding) {
            if (playerMode == 1) { setTextureOrigin(texJump); }
            if (playerMode == 2) { setTextureOrigin(texDouble); }
            if (playerMode == 3) { setTextureOrigin(texDash); }
            if (playerMode == 4) { setTextureOrigin(texPreSlide); }
        }

        // exit slide if mode switched away from 4.
        if (playerMode != 4 && isSliding) {
            isSliding = false;
            if (appliedHeightOffset != 0.0f) {
                image.move({ 0.f, -appliedHeightOffset });
                appliedHeightOffset = 0.0f;
            }
            hasSlid = false;
            setTextureOrigin(playerMode == 1 ? texJump : playerMode == 2 ? texDouble : playerMode == 3 ? texDash : texPreSlide);
        }

        // special actions
        // jump
        if (playerSpecial && wasOnGround && !fastFall && playerMode == 1) {
            yVelocity = jumpStrength;
            onGround = false;
            canDoubleJump = true;
        }

        // double jump
        if (playerSpecial && !wasOnGround && canDoubleJump && playerMode == 2) {
            yVelocity = jumpStrength * 1.25f;
            canDoubleJump = false;
        }

        // dash
        if (playerSpecial && canDash && playerMode == 3) {
            canDash = false;
            yVelocity = 0;
            xVelocity = rightFacing ? dashSpeed : -dashSpeed;
        }

        // slide variables
        wantsToStop = (xVelocity <= 0.0f && rightFacing) || (xVelocity >= 0.0f && !rightFacing);
        heightDifference = (50.0f - 35.0f) / 2.0f;

        // start slide
        if (playerMode == 4 && playerSpecial && !isSliding) {
            isSliding = true;
            appliedHeightOffset = heightDifference;
            image.move({ 0.0f, appliedHeightOffset });
            collide(0, appliedHeightOffset, level);
        }

        // sliding logic
        if (isSliding) {
            setTextureOrigin(texSlide);
            xVelocity += (rightFacing ? slideSpeed : -slideSpeed) * 5 * deltaTime;
            drag = 0.0f;
            hasSlid = true;

            // check if there is a block above the player while sliding.
            if (wantsToStop && !blockedAbove(clearance * scale, level)) {
                isSliding = false;
                if (appliedHeightOffset != 0.0f) {
                    image.move({ 0.f, -appliedHeightOffset });
                    appliedHeightOffset = 0.0f;
                }
                setTextureOrigin(texPreSlide);
            }
        } else {
            image.setScale({ rightFacing ? scale : -scale, scale });
            if (hasSlid && appliedHeightOffset != 0.0f) {
                image.move({ 0.f, -appliedHeightOffset });
                appliedHeightOffset = 0.0f;
                hasSlid = false;
            }
        }

        // apply X and Y velocity, then check for collisions.
        image.move(sf::Vector2f(xVelocity * deltaTime, 0));
        collide(xVelocity * deltaTime, 0, level);
        image.move(sf::Vector2f(0, yVelocity * deltaTime));
        collide(0, yVelocity * deltaTime, level);

        // store grounded state for next frame.
        wasOnGround = onGround;
    }

    // collision function, checks for collisions and adjusts position and velocity.
    void collide(float xVelocityDelta, float yVelocityDelta, vector<platformClass>& level) {
        for (auto& platform : level) {

            // calculate player bounds
            sf::Vector2f playerPos = image.getPosition();
            sf::Vector2f halfSize = {
                (image.getLocalBounds().size.x * std::abs(image.getScale().x)) / 2.0f,
                (image.getLocalBounds().size.y * scale) / 2.0f
            };

            // variables for collision check.
            float playerLeft = playerPos.x - halfSize.x;
            float playerRight = playerPos.x + halfSize.x;
            float playerTop = playerPos.y - halfSize.y;
            float playerBottom = playerPos.y + halfSize.y;

            // check for collision with platform.
            if (playerRight > platform.leftSide &&
                playerLeft  < platform.rightSide &&
                playerBottom > platform.topSide &&
                playerTop < platform.bottomSide) {

                // horizontal collision.
                if (xVelocityDelta > 0) {
                    image.setPosition({ platform.leftSide - halfSize.x, playerPos.y });
                }
                else if (xVelocityDelta < 0) {
                    image.setPosition({ platform.rightSide + halfSize.x, playerPos.y });
                }

                // vertical collision.
                playerPos = image.getPosition();
                if (yVelocityDelta > 0) {
                    image.setPosition({ playerPos.x, platform.topSide - halfSize.y });
                    yVelocity = 0;
                    onGround = true;
                    canDoubleJump = true;
                    canDash = true;
                }
                else if (yVelocityDelta < 0) {
                    image.setPosition({ playerPos.x, platform.bottomSide + halfSize.y });
                    yVelocity = 0;
                }
            }
        }
    }

    // set origin of the player to the center for sprite updates.
    void setTextureOrigin(const sf::Texture& tex) {
        image.setTexture(tex, true);
        sf::Vector2f center = {
            image.getLocalBounds().size.x / 2.0f,
            image.getLocalBounds().size.y / 2.0f
        };
        image.setOrigin(center);
    }

    // check if there's a block above the player within a clearance & return true or false.
    bool blockedAbove(float clearance, vector<platformClass>& level) {
        sf::Vector2f pos = image.getPosition();
        sf::Vector2f halfSize = {
            (image.getLocalBounds().size.x * std::abs(image.getScale().x)) / 2.0f,
            (image.getLocalBounds().size.y * scale) / 2.0f
        };

        // variables for clearance check.
        float playerLeft = pos.x - halfSize.x;
        float playerRight = pos.x + halfSize.x;
        float playerTop = pos.y - halfSize.y - clearance;
        float playerBottom = pos.y - halfSize.y;

        // check for block above player within clearance.
        for (auto& platform : level) {
            if (playerRight > platform.leftSide &&
                playerLeft  < platform.rightSide &&
                playerBottom > platform.topSide &&
                playerTop < platform.bottomSide) {
                return true;
            }
        }
        return false;
    }
};

int main() {
    // variables
    bool keySpecial = false, keyLeft = false, keyRight = false, keyDown = false, keyUp = false;
    bool keySpecialLastFrame = false, keySpecialCurrent = false;
    int playerMode = 1;

    float playerSpawnPointY;
    float playerSpawnPointX;

    int checkpoint = 0;

    float sunScale = 7.5f;

    int platformPixelSize = 16, platformScale = 4, platformPixelScale = platformPixelSize * platformScale;

	// create window.
    sf::RenderWindow window(sf::VideoMode({ 1440, 810 }), "C++ first repo", sf::Style::None);

    // load player textures.
    sf::Texture texJump, texDouble, texDash, texPreSlide, texSlide;
    if (!texJump.loadFromFile("../Debug/data/IMG/JumpPlayer.png")) {
        std::cerr << "Failed to load JumpPlayer.png" << std::endl;
        return -1;
    }
    if (!texDouble.loadFromFile("../Debug/data/IMG/DoubleJumpPlayer.png")) {
        std::cerr << "Failed to load DoubleJumpPlayer.png" << std::endl;
        return -1;
    }
    if (!texDash.loadFromFile("../Debug/data/IMG/DashPlayer.png")) {
        std::cerr << "Failed to load DashPlayer.png" << std::endl;
        return -1;
    }
    if (!texPreSlide.loadFromFile("../Debug/data/IMG/PreSlidePlayer.png")) {
        std::cerr << "Failed to load PreSlidePlayer.png" << std::endl;
        return -1;
    }
    if (!texSlide.loadFromFile("../Debug/data/IMG/SlidePlayer.png")) {
        std::cerr << "Failed to load SlidePlayer.png" << std::endl;
        return -1;
    }
    sf::Sprite playerSprite(texJump);
    PlayerClass player(playerSprite);

    // load sun sprite.
	sf::Texture texSun;
    if (!texSun.loadFromFile("../Debug/data/IMG/sun.png")) {
        std::cerr << "Failed to load sun.png" << std::endl;
        return -1;
    }
	sf::Sprite sunSprite(texSun);
	sunSprite.setScale(sf::Vector2f(sunScale, sunScale));

    // load world sprite
    sf::Texture platformSpriteSheet;
    if (!platformSpriteSheet.loadFromFile("../Debug/data/IMG/world_tileset.png")) {
        std::cerr << "Failed to load world_tileset.png" << std::endl;
        return -1;
    }
    sf::Sprite grassSprite1(platformSpriteSheet);
    sf::Sprite groundSprite1(platformSpriteSheet);
    sf::Sprite cloudSprite1(platformSpriteSheet);

    // camera and delta time.
    sf::View view(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1440.0f, 810.0f));
    sf::Clock gameClock;

    // get player spawn point.
    playerSpawnPointX = player.image.getPosition().x;
    playerSpawnPointY = player.image.getPosition().y;

    // level
	// tutorial text.
    vector<unique_ptr<textClass>> levelText;
    levelText.push_back(make_unique<textClass>(platformPixelScale * 0.0f, platformPixelScale * 3.0f,"../Debug/data/fonts/arial.ttf","use the <- and -> keys to move."));
    levelText.push_back(make_unique<textClass>(platformPixelScale * 0.0f, platformPixelScale * 4.0f,"../Debug/data/fonts/arial.ttf","use Z, X, C and V to transform into different characters."));
    levelText.push_back(make_unique<textClass>(platformPixelScale * 8.0f, platformPixelScale * 3.0f,"../Debug/data/fonts/arial.ttf","(after pressing Z), press SPACE to jump."));
    levelText.push_back(make_unique<textClass>(platformPixelScale * 13.0f, platformPixelScale * 0.0f,"../Debug/data/fonts/arial.ttf","while midair, transform with X and double jump."));
    levelText.push_back(make_unique<textClass>(platformPixelScale * 19.0f, platformPixelScale * -2.0f,"../Debug/data/fonts/arial.ttf","hold -> and dash"));
    levelText.push_back(make_unique<textClass>(platformPixelScale * 30.0f, platformPixelScale * -2.0f,"../Debug/data/fonts/arial.ttf","hold -> and slide"));
    levelText.push_back(make_unique<textClass>(platformPixelScale * 38.5f, platformPixelScale * -10.0f,"../Debug/data/fonts/arial.ttf","while midair hold -> and slide"));
    levelText.push_back(make_unique<textClass>(platformPixelScale * 38.5f, platformPixelScale * -9.5f,"../Debug/data/fonts/arial.ttf","midair while sliding transform and double jump"));
    
    // set texture rects for world sprites.
    grassSprite1.setTextureRect(sf::IntRect({ 0, 0 }, { 16, 16 }));
    groundSprite1.setTextureRect(sf::IntRect({ 0, 16 }, { 16, 16 }));
    cloudSprite1.setTextureRect(sf::IntRect({ 96, 32 }, { 16, 16 }));

    vector<platformClass> level;
    // tutorial
    // first wall
    level.push_back(platformClass(platformPixelScale * -2.0f, platformPixelScale * 1.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * -2.0f, platformPixelScale * 0.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * -2.0f, platformPixelScale * -1.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * -2.0f, platformPixelScale * -2.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * -2.0f, platformPixelScale * -3.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * -2.0f, platformPixelScale * -4.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * -2.0f, platformPixelScale * -5.0f, grassSprite1));
    // first floor
    level.push_back(platformClass(platformPixelScale * -1.0f, platformPixelScale * 1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 0.0f, platformPixelScale * 1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 1.0f, platformPixelScale * 1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 2.0f, platformPixelScale * 1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 3.0f, platformPixelScale * 1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 4.0f, platformPixelScale * 1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 5.0f, platformPixelScale * 1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 6.0f, platformPixelScale * 1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 7.0f, platformPixelScale * 1.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * 8.0f, platformPixelScale * 1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 9.0f, platformPixelScale * 1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 10.0f, platformPixelScale * 1.0f, groundSprite1));
    // jump tutorial
    level.push_back(platformClass(platformPixelScale * 7.0f, platformPixelScale * 0.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 10.0f, platformPixelScale * 0.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * 10.0f, platformPixelScale * -1.0f, grassSprite1));
    // second floor
    level.push_back(platformClass(platformPixelScale * 11.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 12.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 13.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 14.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 15.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 16.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 17.0f, platformPixelScale * -1.0f, groundSprite1));
    // double jump tutorial
    level.push_back(platformClass(platformPixelScale * 17.0f, platformPixelScale * -2.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * 17.0f, platformPixelScale * -3.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * 17.0f, platformPixelScale * -4.0f, grassSprite1));
    // third floor
    level.push_back(platformClass(platformPixelScale * 18.0f, platformPixelScale * -4.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 19.0f, platformPixelScale * -4.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 20.0f, platformPixelScale * -4.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 20.0f, platformPixelScale * -3.0f, groundSprite1));
    // dash tutorial
    level.push_back(platformClass(platformPixelScale * 20.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 20.0f, platformPixelScale * -7.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 20.0f, platformPixelScale * -8.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 21.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 22.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 23.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 24.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 25.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 25.0f, platformPixelScale * -7.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 25.0f, platformPixelScale * -8.0f, cloudSprite1));
    // fourth floor
    level.push_back(platformClass(platformPixelScale * 25.0f, platformPixelScale * -3.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 25.0f, platformPixelScale * -2.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * 26.0f, platformPixelScale * -3.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 27.0f, platformPixelScale * -3.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 28.0f, platformPixelScale * -3.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 29.0f, platformPixelScale * -3.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 30.0f, platformPixelScale * -3.0f, grassSprite1));
    // slide tutorial
    level.push_back(platformClass(platformPixelScale * 30.0f, platformPixelScale * -4.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 30.0f, platformPixelScale * -5.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 30.0f, platformPixelScale * -6.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 31.0f, platformPixelScale * -4.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 32.0f, platformPixelScale * -4.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 32.0f, platformPixelScale * -5.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 32.0f, platformPixelScale * -6.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 31.0f, platformPixelScale * -3.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 32.0f, platformPixelScale * -3.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 33.0f, platformPixelScale * -3.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 33.0f, platformPixelScale * -2.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * 33.0f, platformPixelScale * -1.0f, groundSprite1));
    // fourth floor
    level.push_back(platformClass(platformPixelScale * 34.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 35.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 36.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 37.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 38.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 39.0f, platformPixelScale * -1.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 40.0f, platformPixelScale * -1.0f, grassSprite1));
    // cloud to block skip
    level.push_back(platformClass(platformPixelScale * 35.0f, platformPixelScale * -5.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 36.0f, platformPixelScale * -5.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 37.0f, platformPixelScale * -5.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 37.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 37.0f, platformPixelScale * -7.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 37.0f, platformPixelScale * -8.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 37.0f, platformPixelScale * -9.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 37.0f, platformPixelScale * -10.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 37.0f, platformPixelScale * -11.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 37.0f, platformPixelScale * -12.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 35.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 35.0f, platformPixelScale * -7.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 35.0f, platformPixelScale * -8.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 35.0f, platformPixelScale * -9.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 35.0f, platformPixelScale * -10.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 35.0f, platformPixelScale * -11.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 35.0f, platformPixelScale * -12.0f, cloudSprite1));

    // level
    level.push_back(platformClass(platformPixelScale * 40.0f, platformPixelScale * 0.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * 46.0f, platformPixelScale * -2.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 46.0f, platformPixelScale * -1.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * 47.0f, platformPixelScale * -2.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 48.0f, platformPixelScale * -2.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 43.0f, platformPixelScale * -5.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 42.0f, platformPixelScale * -5.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 41.0f, platformPixelScale * -5.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 42.0f, platformPixelScale * -7.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 42.0f, platformPixelScale * -8.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 41.0f, platformPixelScale * -8.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 42.0f, platformPixelScale * -9.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 42.0f, platformPixelScale * -10.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 42.0f, platformPixelScale * -11.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 42.0f, platformPixelScale * -12.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 64.0f, platformPixelScale * -5.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 64.0f, platformPixelScale * -4.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * 65.0f, platformPixelScale * -5.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 66.0f, platformPixelScale * -5.0f, groundSprite1));
    level.push_back(platformClass(platformPixelScale * 66.0f, platformPixelScale * -6.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 67.0f, platformPixelScale * -6.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 68.0f, platformPixelScale * -6.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 69.0f, platformPixelScale * -6.0f, grassSprite1));
    level.push_back(platformClass(platformPixelScale * 70.0f, platformPixelScale * -6.0f, grassSprite1));

    // part 2 of the level
    level.push_back(platformClass(platformPixelScale * 74.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 78.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 82.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 74.0f, platformPixelScale * -8.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 78.0f, platformPixelScale * -8.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 82.0f, platformPixelScale * -8.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 74.0f, platformPixelScale * -9.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 78.0f, platformPixelScale * -9.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 82.0f, platformPixelScale * -9.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 74.0f, platformPixelScale * -10.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 78.0f, platformPixelScale * -10.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 82.0f, platformPixelScale * -10.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 83.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 87.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 87.0f, platformPixelScale * -5.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 87.0f, platformPixelScale * -4.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 87.0f, platformPixelScale * -7.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 87.0f, platformPixelScale * -8.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 87.0f, platformPixelScale * -9.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 87.0f, platformPixelScale * -10.75f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 89.0f, platformPixelScale * -6.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 89.0f, platformPixelScale * -7.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 89.0f, platformPixelScale * -8.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 89.0f, platformPixelScale * -9.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 89.0f, platformPixelScale * -10.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 89.0f, platformPixelScale * -11.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 88.0f, platformPixelScale * -4.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 89.0f, platformPixelScale * -4.0f, cloudSprite1));
    level.push_back(platformClass(platformPixelScale * 90.0f, platformPixelScale * -4.0f, cloudSprite1));


    // game loop
    while (window.isOpen()) {

        // event handling
        while (const optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape))
                window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::R)) {
                player.image.setPosition(sf::Vector2f(playerSpawnPointX, playerSpawnPointY));
                player.xVelocity = 0;
            }
        }

        // input handling
        if (!player.isSliding) {
            keySpecialCurrent = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space);
        }
        keySpecial = keySpecialCurrent && !keySpecialLastFrame;
        keySpecialLastFrame = keySpecialCurrent;

        keyUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up);
        keyRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right);
        keyLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left);
        keyDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down);

        if (!player.blockedAbove(player.clearance * player.scale, level) || !player.isSliding)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Z)) playerMode = 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::X)) playerMode = 2;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::C)) playerMode = 3;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::V)) playerMode = 4;
        }

        // delta time
        float deltaTime = gameClock.restart().asSeconds();

        // update player with inputs.
        player.update(keySpecial, keyLeft, keyRight, keyDown, keyUp, level, deltaTime, playerMode, texJump, texDouble, texDash, texPreSlide, texSlide);

        // set camera to follow the player position.
        view.setCenter(sf::Vector2f(
            player.image.getPosition().x + player.image.getLocalBounds().size.x * player.scale / 2.0f,
            player.image.getPosition().y + player.image.getLocalBounds().size.y * player.scale / 2.0f));
        window.setView(view);

		// reset when falling out of the world.
        if (player.image.getPosition().y > 2000.0f) {
            player.image.setPosition(sf::Vector2f(playerSpawnPointX, playerSpawnPointY));
            player.xVelocity = 0;
        }

        // checkpoint triggers
        if (player.image.getPosition().x >= 2137.0f) {
			checkpoint = 1;
            if (checkpoint == 1)
            {
                playerSpawnPointX = 2137.0f;
                playerSpawnPointY = -217.0f;
            }
		}

        // clear / draw / display.
        window.clear(sf::Color::White);
        window.draw(player.image);
		window.draw(sunSprite);
        sunSprite.setPosition(sf::Vector2f(
            player.image.getPosition().x - sunSprite.getLocalBounds().size.x * sunScale / 2.0f,
            player.image.getPosition().y - sunSprite.getLocalBounds().size.y * sunScale / 2.0f - 375.0f));
        for (auto& platform : level) {
            window.draw(platform.image);
        }
        for (auto& text : levelText) {
            window.draw(text->text);
        }
        window.display();
    }
}

// to do:
// - level af maken