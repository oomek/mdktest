#include <fstream>
#include <iostream>
#include <string>
#include <dirent.h>
#include <vector>
#include <SFML/Graphics.hpp>
#include "mdk/Player.h"

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
float vid_frame_rate = 0.0;
float vid_frame_time = 0.0;
const float disp_frame_time = 1000.0 / 60.0;
float counter_frame_time = 0.0;
int vid_sample_rate = 0;
std::vector<std::string> files;
sf::Context vid_context;
sf::Mutex mutex;

int frames_ready = 0;
void load (const char* file)
{
	player.setNextMedia(nullptr, -1);
	player.setState(State::Stopped);
	player.waitFor(State::Stopped);
	player.setMedia(nullptr);
	player.setVideoSurfaceSize(-1, -1);
	player.setPreloadImmediately(true);
	player.setMedia(file);
	// player.setBufferRange(0, 1000, false);
	// while (!(player.mediaStatus() & MediaStatus::Loaded));
	// player.setVideoSurfaceSize(window.getSize().x, window.getSize().y);
	// player.setState(State::Playing);
	player.prepare(0);
	player.setLoop(1000);
	// player.setBackgroundColor(0, 0, 0, -1);
	// while (!(player.mediaStatus() & MediaStatus::Loaded));
	// player.setState(State::Paused);
	player.waitFor(State::Paused);
	auto& vid_codec = player.mediaInfo().video[0].codec;
	auto& aud_codec = player.mediaInfo().audio[0].codec;
	player.setVideoSurfaceSize(vid_codec.width, vid_codec.height);
	texture.create(vid_codec.width, vid_codec.height);
	texture.setSmooth(true);
	texture.display();

	// player.setRenderCallback([=](void*){
	// 	frames_ready = 1;
	// 	// sf::Lock lock(mutex);
	// 	// vid_context.setActive(true);
	// 	// texture.setActive(true);
	// 	// // QCALL(player.renderVideo());
	// 	// player.renderVideo();
	// 	// texture.display();
	// 	// texture.setActive(false);
	// 	// vid_context.setActive(false);
	// });

	vid_frame_rate = vid_codec.frame_rate + 0.0001;
	vid_frame_time = 1000.0 / vid_codec.frame_rate;
	counter_frame_time = 0.0;
	vid_sample_rate = aud_codec.sample_rate;

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

	// player.setVideoDecoders({"MMAL", "FFmpeg"});
	player.setVideoDecoders({"FFmpeg"});

	int file_idx = 0;
	load(files[file_idx].c_str());

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

		text.setString(
			"[File: " +
			to_string(file_idx) +
			"] " +
			to_string(texture.getSize().x) +
			" x " +
			to_string(texture.getSize().y) +
			" @ " +
			to_string(vid_frame_rate).substr(0, to_string(vid_frame_rate).find(".") + 3) +
			" Audio:" +
			to_string(vid_sample_rate));

		if (counter_frame_time <= 0.0)
		// if ( frames_ready == 1 )
		{
			counter_frame_time += vid_frame_time;
			// window.setActive(false);
			// sf::Lock lock(mutex);
			vid_context.setActive(true);
			texture.setActive(true);
			// QCALL(player.renderVideo());
			player.renderVideo();
			texture.display();
			texture.setActive(false);
			vid_context.setActive(false);
			// window.setActive(true);

			// player.setPlaybackRate(44100.0/48000.0);
			// sprite.setTexture(texture.getTexture());
			// frames_ready = 0;
		}
		// else
		// 	text.setString(std::to_string(counter_frame_time));

		counter_frame_time -= disp_frame_time;

		// window.clear(sf::Color::Blue);
		// window.setActive(true);
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
