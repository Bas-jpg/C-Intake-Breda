#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

using namespace sf;
using namespace std;

class platformClass
{
    public:
        float xPosition;
        float yPosition;
        float scale;

        float topSide;
		float bottomSide;
		float rightSide;
		float leftSide;

        Sprite image;

        platformClass(float xPosition, float yPosition, Sprite& sprite)
            :xPosition(xPosition), yPosition(yPosition), image(sprite)
        {
            scale = 3;
            image.setPosition(Vector2f(xPosition, yPosition));
			image.scale(Vector2f(scale, scale));
			leftSide = image.getPosition().x;
            rightSide = image.getPosition().x + (image.getLocalBounds().size.x * scale);
            topSide = image.getPosition().y;
            bottomSide = image.getPosition().y + (image.getLocalBounds().size.y * scale);
        }
};

class PlayerClass
{
    public:
        bool onGround;
        bool isColliding;

        bool playerFaceRight;
		bool playerFaceLeft;

		float xPosition;
		float yPosition;

        float xVelocity;
        float yVelocity;

        float topSide;
        float bottomSide;
        float rightSide;
        float leftSide;

        float scale;

		Sprite image;

        PlayerClass(Sprite& sprite)
			:image(sprite)
        {
            playerFaceRight = true;
			playerFaceLeft = false;

			xPosition = 0;
			yPosition = 0;

			xVelocity = 0;
			yVelocity = 0;

            onGround = false;
			isColliding = false;

			scale = 1;
        }

		// update the player's position and velocity based on booleans
        void update(bool playerJump,bool playerLeft, bool playerRight,bool playerFall, platformClass platforms)
        {
            if (playerRight)
            {
                playerFaceRight = true;
				playerFaceLeft = false;
				xVelocity = .5;
            }
            if (playerLeft)
            {
                playerFaceRight = false;
                playerFaceLeft = true;
                xVelocity = -.5;
            }
            if (playerJump)
            {
                yVelocity = -.5;
            }
            if (playerFall)
            {
                yVelocity = .5;
            }
            if (!(playerJump || playerFall))
            {
                yVelocity = 0;
            }
            if (!(playerRight || playerLeft))
            {
				xVelocity = 0;
            }
            if (onGround == true)
            {
                yVelocity = 0;
            }

			image.move(Vector2f(xVelocity, 0));
            collide(xVelocity, 0, platforms);
            image.move(Vector2f(0, yVelocity));
			collide(0, yVelocity, platforms);
        }

        void collide(float xVelocityDelta, float yVelocityDelta, platformClass platforms)
        {
            if (image.getPosition().x + image.getLocalBounds().size.x * scale > platforms.leftSide &&
                image.getPosition().x < platforms.rightSide &&
                image.getPosition().y + image.getLocalBounds().size.y * scale > platforms.topSide &&
                image.getPosition().y < platforms.bottomSide)
            {
                /*image.setPosition(Vector2f(platforms.leftSide - image.getLocalBounds().size.x * scale, image.getPosition().y));*/
				isColliding = true;
            }
            else {
				isColliding = false;
            }
            if (isColliding)
            {
                if (xVelocityDelta > 0)
                {
                    image.setPosition(Vector2f(platforms.leftSide - image.getLocalBounds().size.x * scale, image.getPosition().y));
                }
                if (xVelocityDelta < 0)
                {
                    image.setPosition(Vector2f(platforms.rightSide, image.getPosition().y));
                }
                if (yVelocityDelta > 0)
                {
                    image.setPosition(Vector2f(image.getPosition().x, platforms.topSide - image.getLocalBounds().size.y * scale));
                }
                if (yVelocityDelta < 0)
                {
                    image.setPosition(Vector2f(image.getPosition().x, platforms.bottomSide));
                }
            }

        }

};

int main()
{
    // variables
    bool playerJump = false, playerLeft = false, playerRight = false, playerFall = false;
    
	// create the main window
    RenderWindow window(VideoMode({ 1440, 810 }), "C++ first repo", Style::Titlebar | Style::Close);



	Font arialFont;
    arialFont.openFromFile("../Debug/data/fonts/arial.ttf");

    Text helloText(arialFont);
    helloText.setString("Hello SFML");
    helloText.setCharacterSize(50);

    Texture playerTexture;
    playerTexture.loadFromFile("../Debug/data/IMG/Man1.png");
	Sprite playerSprite(playerTexture);

    PlayerClass player(playerSprite);

    Texture platformSpriteSheet;
	platformSpriteSheet.loadFromFile("../Debug/data/IMG/world_tileset.png");
	Sprite groundSprite1(platformSpriteSheet);

    groundSprite1.setTextureRect(IntRect({ 0, 0 }, {16, 16}));

	platformClass platformObj(100, 100, groundSprite1);

	// start the game loop
    while (window.isOpen())
    {

		// event handling
        while (const std::optional event = window.pollEvent())
        {
			// close window event
            if (event->is<Event::Closed>())
				window.close();
        }

        // input handling
        if (Keyboard::isKeyPressed(Keyboard::Scancode::Up))
            playerJump = true;
        if (Keyboard::isKeyPressed(Keyboard::Scancode::Left))
            playerLeft = true;
        if (Keyboard::isKeyPressed(Keyboard::Scancode::Right))
            playerRight = true;
        if (Keyboard::isKeyPressed(Keyboard::Scancode::Down))
            playerFall = true;
        if (!(Keyboard::isKeyPressed(Keyboard::Scancode::Up)))
            playerJump = false;
        if (!(Keyboard::isKeyPressed(Keyboard::Scancode::Left)))
            playerLeft = false;
        if (!(Keyboard::isKeyPressed(Keyboard::Scancode::Right)))
            playerRight = false;
        if (!(Keyboard::isKeyPressed(Keyboard::Scancode::Down)))
            playerFall = false;

        player.update(playerJump, playerLeft, playerRight, playerFall, platformObj);

        // clear screen
        window.clear(Color::White);

        // draw shapes / sprites
		window.draw(platformObj.image);
		window.draw(player.image);
		/*window.draw(helloText);*/

        // display the new window
        window.display();
    }
}
