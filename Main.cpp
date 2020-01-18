#include <SFML/Graphics.hpp>
#include <iostream>
using namespace sf;

//using the picture from https://stackoverflow.com/questions/8507885/shift-hue-of-an-rgb-color
sf::Color getColor(sf::Color startColor) {
	int red = startColor.r;
	int green = startColor.g;
	int blue = startColor.b;

	if (red == 255 && green < 255 && blue == 0) { startColor.g += 1; }
	if (red > 0 && green == 255 && blue == 0) { startColor.r -= 1; }
	if (red == 0 && green == 255 && blue < 255) { startColor.b += 1; }
	if (red == 0 && green > 0 && blue == 255) { startColor.g -= 1; }
	if (red < 255 && green == 0 && blue == 255) { startColor.r += 1; }
	if (red == 255 && green == 0 && blue > 0) { startColor.b -= 1; }
	return startColor;
}

std::vector<sf::Text> setHeader(std::vector<sf::Text> segment_info, sf::Font* font) {
	int font_height = 50;

	for (int i = 0; i < 3; i++) {
		sf::Text text;
		text.setCharacterSize(font_height);
		text.setFont(*font);
		text.setPosition(sf::Vector2f(10 + i * 300, 0));
		if (i == 0) { text.setString("Segment No."); }
		if (i == 1) { text.setString("Radius"); }
		if (i == 2) { text.setString("Velocity"); }

		segment_info.push_back(text);
	}

	return segment_info;
}

std::vector<sf::Text> updateSegmentInfo(std::vector<sf::Text> segment_info, double radius, double angular_velocity, int index, sf::Color color, sf::Font* font) {
	int font_height = 50;

	for (int i = 0; i < 3; i++) {
		sf::Text text;
		text.setCharacterSize(font_height);
		text.setFont(*font);
		text.setFillColor(color);
		text.setPosition(sf::Vector2f(10 + i * 300, index * font_height * 1.1));
		if (i == 0) {
			text.setString("Segment " + std::to_string(index));
		}
		if (i == 1) {
			text.setString(std::to_string((int)radius));
		}
		if (i == 2) {
			text.setString(std::to_string((int)angular_velocity));
		}
		if (segment_info.size() / 3 <= index) { segment_info.push_back(text); }
		else { segment_info.at(index * 3 + i) = text; }
	}

	return segment_info;
}

int getRightmostLocation(std::vector<sf::Text> segment_info) {
	sf::Text text = segment_info.at(2);
	sf::FloatRect rect = text.getGlobalBounds();
	return rect.left + rect.width;
}

double averageAbsoluteVelocity(std::vector<double> angular_velocities) {
	double averageAbsoluteVelocity = 0;
	for (int i = 0; i < angular_velocities.size(); i++)
		averageAbsoluteVelocity += abs(angular_velocities.at(i));
	return averageAbsoluteVelocity / angular_velocities.size();
}

class Segment {
public:
	sf::Vertex line[2];
	double radius;
	sf::Vector2f center;
	double angular_velocity;
	double angle;
	Segment(sf::Vector2f input_center, double input_radius, double input_angular_velocity) {
		center = input_center;
		radius = input_radius;
		angular_velocity = input_angular_velocity;
		angle = 0;
		line[0].position = input_center;
		line[1].position.x = input_center.x + input_radius;
		line[1].position.y = input_center.y;
		line[0].color = sf::Color::Blue;
		line[1].color = sf::Color::Blue;
	}
	void rotate(Segment parent_segment, int steps, double velocity_multiplier, double radius_multiplier) {
		center = sf::Vector2f(parent_segment.line[1].position.x, parent_segment.line[1].position.y);
		line[0].position = center;
		angle += angular_velocity / steps * velocity_multiplier;
		line[1].position.x = center.x + radius * radius_multiplier * cos(angle * 3.14159265 / 180);
		line[1].position.y = center.y + radius * radius_multiplier * sin(angle * 3.14159265 / 180);
	}
	void rotate(int steps, double velocity_multiplier, double radius_multiplier) {
		line[0].position = center;
		angle += angular_velocity / steps * velocity_multiplier;
		line[1].position.x = center.x + radius * radius_multiplier * cos(angle * 3.14159265 / 180);
		line[1].position.y = center.y + radius * radius_multiplier * sin(angle * 3.14159265 / 180);
	}
	void setColor(sf::Color color) {
		line[0].color = color;
		line[1].color = color;
	}
	void changeCenter(sf::Vector2i prev_clicked_mouse_location, sf::Vector2i clicked_mouse_location) {
		if (prev_clicked_mouse_location != Vector2i(-1, -1) && clicked_mouse_location != Vector2i(-1, -1)) {
			center.x = center.x - clicked_mouse_location.x + prev_clicked_mouse_location.x;
			center.y = center.y - clicked_mouse_location.y + prev_clicked_mouse_location.y;
		}
	}
};

int main()
{
	std::vector<Segment> segments;
	std::vector<double> radii;
	std::vector<double> angular_velocities;
	std::vector<sf::Vertex> path; //the path drawn by the moving segments
	std::vector<sf::Text> segment_info;

	double acceleration = 1.1;
	double velocity_multiplier = 1;
	double radius_multiplier = 1;
	bool show_info = false;
	bool rainbow_path = false;
	bool rainbow_path_key_pressed_previously = false;
	int max_path_length = 50000;
	Vector2i mouse_location;
	Vector2i clicked_mouse_location;
	Vector2i prev_clicked_mouse_location;
	prev_clicked_mouse_location = Vector2i(-1, -1);

	sf::Font font;
	font.loadFromFile("Aller_BdIt.ttf");

	radii.push_back(100); radii.push_back(80);
	angular_velocities.push_back(1); angular_velocities.push_back(-2);

	Event event;
	RenderWindow window(VideoMode(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height), "Spirograph");
	window.setFramerateLimit(120);

	//fill the vector of line segments
	sf::Vector2f center = sf::Vector2f(sf::VideoMode::getDesktopMode().width / 2, sf::VideoMode::getDesktopMode().height / 2);
	for (int i = 0; i < radii.size(); i++) {
		Segment segment = Segment(center, radii.at(i), angular_velocities.at(i));
		center.x = center.x + radii.at(i);
		segments.push_back(segment);
	}

	//fill the vector of segment information
	segment_info = setHeader(segment_info, &font);
	for (int i = 0; i < radii.size(); i++) {
		segment_info = updateSegmentInfo(segment_info, radii.at(i), angular_velocities.at(i), i + 1, sf::Color::White, &font);
	}

	while (window.isOpen()) {
		sf::Color color(255, 0, 0);
		int highlighted_segment = -1;

		mouse_location = Mouse::getPosition(window);

		//decide whether or not to show segment_info, depending on the mouse location
		if (mouse_location.x < 10) { show_info = true; }
		if (mouse_location.x > getRightmostLocation(segment_info)) { show_info = false; }

		//highlight and unhighlight the text in segment_info according to the mouse location
		for (int i = 0; i < segment_info.size(); i++) {
			if (i % 3 == 0 || i == 1 || i == 2) { segment_info.at(i).setFillColor(sf::Color::White); continue; }
			sf::Text text = segment_info.at(i);
			sf::FloatRect rect = segment_info.at(i).getGlobalBounds();
			if (mouse_location.x >= rect.left && mouse_location.x <= rect.left + rect.width
				&& mouse_location.y >= rect.top && mouse_location.y <= rect.top + rect.height) {
				highlighted_segment = i;
				segment_info.at(i).setFillColor(sf::Color::Red);
				segment_info.at(i - i % 3).setFillColor(sf::Color::Red);

				if (i % 3 == 1) { segment_info.at(1).setFillColor(sf::Color::Red); }
				if (i % 3 == 2) { segment_info.at(2).setFillColor(sf::Color::Red); }
			}
			else {
				segment_info.at(i).setFillColor(sf::Color::White);
			}
		}

		//perform actions according to the user's input
		if (window.pollEvent(event)) {
			//close the window
			if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Q) || Keyboard::isKeyPressed(Keyboard::Escape)) { window.close(); }
			//increase or decrease a specific segment's radius/velocity
			if (highlighted_segment != -1) {
				int index = (highlighted_segment / 3) - 1;
				double change = 0;
				if (Keyboard::isKeyPressed(Keyboard::Up)) { change = 1; }
				if (Keyboard::isKeyPressed(Keyboard::Down)) { change = -1; }
				if (highlighted_segment % 3 == 1) {
					segments.at(index).radius = segments.at(index).radius + change;
					radii.at(index) = radii.at(index) + change;
					segment_info.at(highlighted_segment).setString(std::to_string((int)radii.at(index)));
				}
				if (highlighted_segment % 3 == 2) {
					segments.at(index).angular_velocity = segments.at(index).angular_velocity + change;
					angular_velocities.at(index) = angular_velocities.at(index) + change;
					segment_info.at(highlighted_segment).setString(std::to_string((int)angular_velocities.at(index)));
				}
			}
			//speed things up
			if (Keyboard::isKeyPressed(Keyboard::Up) && !Keyboard::isKeyPressed(Keyboard::LControl) && !Keyboard::isKeyPressed(Keyboard::RControl) && highlighted_segment == -1) {
				velocity_multiplier *= acceleration;
			}
			//slow things down
			if (Keyboard::isKeyPressed(Keyboard::Down) && !Keyboard::isKeyPressed(Keyboard::LControl) && !Keyboard::isKeyPressed(Keyboard::RControl) && highlighted_segment == -1) {
				velocity_multiplier /= acceleration;
			}
			//toggle rainbow path
			if (Keyboard::isKeyPressed(Keyboard::C)) { rainbow_path_key_pressed_previously = true; }
			if (!Keyboard::isKeyPressed(Keyboard::C) && rainbow_path_key_pressed_previously) { rainbow_path = !rainbow_path; }
			//clear the path
			if (Keyboard::isKeyPressed(Keyboard::Space)) {
				path.clear();
				path.push_back(segments.at(segments.size() - 1).line[1]);
			}
			//subtract a line segment
			if (Keyboard::isKeyPressed(Keyboard::Down) && (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl))) {
				if (segments.size() > 1) {
					segments.pop_back();
					radii.pop_back();
					angular_velocities.pop_back();
					segment_info.pop_back();
					segment_info.pop_back();
					segment_info.pop_back();
				}
			}
			//add another line segment
			if (Keyboard::isKeyPressed(Keyboard::Up) && (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl))) {
				radii.push_back(100);
				angular_velocities.push_back(0);
				Segment segment = Segment(segments.at(segments.size() - 1).line[1].position, 100, 0);
				segments.push_back(segment);
				segment_info = updateSegmentInfo(segment_info, 100, 0, segments.size(), sf::Color::White, &font);
			}
			//detect the mouse shifting when left-click is pressed
			if (Mouse::isButtonPressed(Mouse::Left)) { clicked_mouse_location = Mouse::getPosition(window); }
			else { clicked_mouse_location = Vector2i(-1, -1); }
			//zoom in or out
			if (event.type == sf::Event::MouseWheelMoved) {
				if (event.mouseWheel.delta == -1) { radius_multiplier /= acceleration; }
				if (event.mouseWheel.delta == 1) { radius_multiplier *= acceleration; }
			}
			if (!Keyboard::isKeyPressed(Keyboard::C)) { rainbow_path_key_pressed_previously = false; }
		}


		window.clear(Color::Black);

		//calculate the path's new points and move the segments
		int steps = averageAbsoluteVelocity(angular_velocities) * 10 * velocity_multiplier;
		Segment first_segment = segments.at(0);
		first_segment.changeCenter(clicked_mouse_location, prev_clicked_mouse_location);

		//move the entire path
		if (prev_clicked_mouse_location != Vector2i(-1, -1) && clicked_mouse_location != Vector2i(-1, -1)) {
			for (int i = 0; i < path.size(); i++) {
				sf::Vertex path_segment = path.at(i);
				path_segment.position.x = path_segment.position.x + clicked_mouse_location.x - prev_clicked_mouse_location.x;
				path_segment.position.y = path_segment.position.y + clicked_mouse_location.y - prev_clicked_mouse_location.y;
				path.at(i) = path_segment;
			}
		}
		
		//rescale the entire path
		if (event.type == sf::Event::MouseWheelMoved) {
			double factor;
			if (event.mouseWheel.delta < 0) { factor = 1 / acceleration; }
			if (event.mouseWheel.delta > 0) { factor = acceleration; }
			sf::Vertex center = segments.at(0).line[0];
			double x_center_difference = center.position.x * factor - center.position.x;
			double y_center_difference = center.position.y * factor - center.position.y;
			for (int i = 0; i < path.size(); i++) {
				sf::Vertex vertex = path.at(i);
				
				int x_difference = vertex.position.x - center.position.x;
				int y_difference = center.position.y - vertex.position.y;
				vertex.position.x = vertex.position.x * factor - x_center_difference;
				vertex.position.y = vertex.position.y * factor - y_center_difference;
				path.at(i) = vertex;
			}
		}
		
		//rotate the segments
		segments.at(0) = first_segment;
		if (steps <= 0) { steps = 1; }
		for (int i = 0; i < steps; i++) {
			Segment prev_segment = segments.at(0);
			prev_segment.rotate(steps, velocity_multiplier, radius_multiplier);
			segments.at(0) = prev_segment;

			for (int i = 1; i < segments.size(); i++) {
				Segment segment = segments.at(i);
				segment.rotate(prev_segment, steps, velocity_multiplier, radius_multiplier);
				segments.at(i) = segment;
				prev_segment = segment;
			}
			path.push_back(segments.at(segments.size() - 1).line[1]);
		}
		prev_clicked_mouse_location = clicked_mouse_location;

		//draw the segments
		for (int i = 0; i < segments.size(); i++) {
			window.draw(segments.at(i).line, 2, sf::Lines);
		}

		//resize the path if it's too large
		if (path.size() > max_path_length) {
			path.erase(path.begin(), path.begin() + path.size() - max_path_length);
			path.shrink_to_fit();
		}
		//draw the path
		for (int i = 0; i < path.size() - 1; i++) {
			sf::Vertex line[2];
			line[0] = path.at(i);
			line[1] = path.at(i + 1);
			line[0].color = getColor(color);
			color = getColor(color);
			line[1].color = getColor(color);

			if (!rainbow_path) {
				line[0].color = sf::Color(255, 255, 255);
				line[1].color = sf::Color(255, 255, 255);
			}

			window.draw(line, 2, sf::Lines);
			color = getColor(color);
			
		}

		//display the text
		if (show_info) {
			for (int i = 0; i < segment_info.size(); i++) {
				window.draw(segment_info.at(i));
			}
		}

		window.display();
	}
	return 0;
}
