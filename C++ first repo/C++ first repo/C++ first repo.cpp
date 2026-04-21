#include <SFML/Graphics.hpp>
#include <iostream>
#include <SFML/Window/Keyboard.hpp>

using namespace sf;
using namespace std;

class PlayerClass
{
    public:
        PlayerClass()
        {
			cout << "Player created!" << endl;
        }
};

int main()
{
    // variables
    bool playerJump = false, playerLeft = false, playerRight = false;
    
	// create the main window
    RenderWindow window(VideoMode({ 960, 540 }), "C++ first repo", Style::Titlebar | Style::Close);

	// create a circle shape and set its properties
    CircleShape shape(200.f);
    shape.setFillColor(Color::Red);
    shape.setOutlineThickness(10.f);
	shape.setOutlineColor(Color::Green);

	// create a rectangle shape and set its properties
    RectangleShape rectangle(Vector2f(120.f, 50.f));
    rectangle.setFillColor(Color::Yellow);
    rectangle.setPosition(Vector2f(0, 0));
    rectangle.setPosition(Vector2f(20, 20));

	PlayerClass player;

	// start the game loop
    while (window.isOpen())
    {

		// event handling
        while (const std::optional event = window.pollEvent())
        {
			// close window event
            if (event->is<Event::Closed>())
				window.close();

            // input handling
            if (Keyboard::isKeyPressed(Keyboard::Scancode::Up))
                playerJump = true;
            if (Keyboard::isKeyPressed(Keyboard::Scancode::Left))
                playerLeft = true;
            if (Keyboard::isKeyPressed(Keyboard::Scancode::Right))
                playerRight = true;
            if(!(Keyboard::isKeyPressed(Keyboard::Scancode::Up)))
				playerJump = false;
            if (!(Keyboard::isKeyPressed(Keyboard::Scancode::Left)))
                playerLeft = false;
            if (!(Keyboard::isKeyPressed(Keyboard::Scancode::Right)))
                playerRight = false;
        }


        
        // update and display the shapes
        window.clear();
        window.draw(rectangle);
        window.draw(shape);
        window.display();
    }
}
