#include <fstream>
#include <iostream>
#include <string>
#include <dirent.h>
#include <vector>
#include <SFML/Graphics.hpp>
#include "mdk/Player.h"

//dsym hack1 0001CB60
//dsym hack2 00015BB1
//so.0 hack2 00015BB1
//so.0 hack1 0001CB60

using namespace std;
using namespace MDK_NS;

#define QCALL(noisy) \
{ \
    FILE* tmp = stdout;\
    stdout = tmpfile();\
    (noisy);\
    fclose(stdout);\
    stdout = tmp;\
}

Player player;
sf::RenderWindow window;
sf::RenderTexture texture;
sf::Sprite sprite;
float frame_rate_vid = 0.0;
float frame_time_vid = 0.0;
const float frame_time_display = 1000.0 / 60.0;
float frame_time_counter = 0.0;
std::vector<std::string> files;

void load (const char* file)
{
	player.setNextMedia(nullptr, -1);
	player.setState(State::Stopped);
	player.waitFor(State::Stopped);
	player.setMedia(nullptr);
	player.setVideoSurfaceSize(-1, -1);
	player.setPreloadImmediately(true);
	player.setMedia(file);
	player.setBufferRange(0, 1000, false);
	// while (!(player.mediaStatus() & MediaStatus::Loaded));
	// player.setVideoSurfaceSize(window.getSize().x, window.getSize().y);
	// player.setState(State::Playing);
	player.prepare(0);
	player.setLoop(1000);
	// player.setBackgroundColor(0, 0, 0, -1);
	// while (!(player.mediaStatus() & MediaStatus::Loaded));
	// player.setState(State::Paused);
	player.waitFor(State::Paused);
	auto& codec = player.mediaInfo().video[0].codec;
	player.setVideoSurfaceSize(codec.width, codec.height);
	texture.create(codec.width, codec.height);
	texture.setSmooth(true);
	texture.display();

	frame_rate_vid = player.mediaInfo().video[0].codec.frame_rate + 0.0001;
	frame_time_vid = 1000.0 / player.mediaInfo().video[0].codec.frame_rate;
	frame_time_counter = 0.0;

	sprite.setTexture(texture.getTexture(), true);
	player.setState(State::Playing);
	// player.setPlaybackRate(1.0);
}

int main(int argc, char** argv)
{
	DIR *dir;
	struct dirent *entry;

	dir = opendir("./");
	if (dir != NULL)
	{
		while (entry = readdir(dir))
		{
			if ( entry->d_type == DT_REG )
			{
				std::string fname = entry->d_name;
				if (fname.find("mp4", (fname.length() - 3)) != std::string::npos)
					files.push_back(fname);
			}
		}
	}
	closedir(dir);

 	if (files.size() == 0)
 	{
 		cout << endl << "No MP4 files found in current directory." << endl;
		return EXIT_FAILURE;
	}

	sf::ContextSettings contextSettings;
	contextSettings.depthBits = 24;
	contextSettings.majorVersion = 2;
	contextSettings.minorVersion = 1;

	window.create(sf::VideoMode(1280, 1024), "MDK + SFML RenderTexture", sf::Style::Fullscreen, contextSettings);
	window.setVerticalSyncEnabled(true);
	window.setActive();

	// for (int i = 0; i < argc; ++i)
	// {
	// 	if (strcmp(argv[i], "-c:v") == 0) {
	// 		i++;
	// 		player.setVideoDecoders({ argv[i] });
	// 	}
	// 	else
	// 		player.setVideoDecoders({"FFmpeg"});
	// }

	player.setVideoDecoders({"FFmpeg"});

	int file_idx = 0;
	load(files[0].c_str());

	sf::Font font;
	font.loadFromFile("segoeui.ttf");
	sf::Text text("MDK demo", font, 40);
	text.setPosition(0.f, window.getSize().y - 100.f);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Space)
					player.setState(player.state() == State::Playing ? State::Paused : State::Playing);
				else if (event.key.code == sf::Keyboard::Right)
				{
					file_idx++;
					if ( file_idx > files.size() - 1 ) file_idx = 0;
					load(files[file_idx].c_str());
				}
				else if (event.key.code == sf::Keyboard::Left)
				{
					file_idx--;
					if ( file_idx < 0 ) file_idx = files.size() - 1;
					load(files[file_idx].c_str());
				}
				else if (event.key.code == sf::Keyboard::Escape)
				{
					player.setVideoSurfaceSize(-1, -1);
					return EXIT_SUCCESS;
				}
			}
		}

		// texture.clear(sf::Color::Red);

		text.setString(to_string(texture.getSize().x) + " x " + to_string(texture.getSize().y) + " @ " + to_string(frame_rate_vid).substr(0, std::to_string(frame_rate_vid).find(".") + 3));

		if (frame_time_counter <= 0.0)
		{
			frame_time_counter += frame_time_vid;
			window.setActive(false);
			texture.setActive(true);
			QCALL(player.renderVideo());
			texture.display();
			texture.setActive(false);
			window.setActive(true);
			player.setPlaybackRate(1.0);
			// sprite.setTexture(texture.getTexture());
		}
		// else
		// 	text.setString(std::to_string(frame_time_counter));

		frame_time_counter -= frame_time_display;

		// window.clear(sf::Color::Blue);
		window.clear();
		float scalex = (float)window.getSize().x / (float)texture.getSize().x;
		float scaley = (float)window.getSize().y / (float)texture.getSize().y;
		sprite.setScale(scalex,scaley);
		window.draw(sprite);
		window.draw(text);
		window.display();
	}
	return EXIT_SUCCESS;
}
