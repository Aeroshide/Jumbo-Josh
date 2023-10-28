// CMakeProject2.cpp : Defines the entry point for the application.
//

#define _CRT_SECURE_NO_WARNINGS
#include "CMakeProject2.h"
#include <dpp/dpp.h>
#include <cstdlib> // for std::getenv

#include <fmt/format.h>
#include <iomanip>
#include <sstream>

#include <vector>
#include <fstream>
#include <iostream>
#include <mpg123.h>
#include <out123.h>

std::string readFileContext(const std::string& fileName)
{
    const char* appData = std::getenv("APPDATA");
    if (!appData) {
        // Handle error: APPDATA environment variable not found.
        return "";
    }
    
    std::string path = std::string(appData) + "/Aeroshide/Jumbo-Josh/";
    std::ifstream file(path + fileName);
    if (!file) {
        // Handle error: file could not be opened.
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}



const std::string token = readFileContext("secret.aeroshide"); //ytta
const int id = 1164187926888972480;

int main() {
    /* This will hold the decoded MP3.
    * The D++ library expects PCM format, which are raw sound
    * data, 2 channel stereo, 16 bit signed 48000Hz.
    */
    std::vector<uint8_t> pcmdata;

    

    /* Setup the bot */
    dpp::cluster bot(readFileContext("secret.aeroshide"));

    bot.on_log(dpp::utility::cout_logger());

    /* The event is fired when someone issues your commands */
    bot.on_slashcommand([&bot, &pcmdata](const dpp::slashcommand_t& event) {
        /* Check which command they ran */
        if (event.command.get_command_name() == "join") {
            /* Get the guild */
            dpp::guild* g = dpp::find_guild(event.command.guild_id);

            /* Attempt to connect to a voice channel, returns false if we fail to connect. */
            if (!g->connect_member_voice(event.command.get_issuing_user().id)) {
                event.reply("You don't seem to be in a voice channel!");
                return;
            }

            /* Tell the user we joined their channel. */
            event.reply("Joined your channel!");
        }
        else if (event.command.get_command_name() == "mp3") {
            /* Get the voice channel the bot is in, in this current guild. */
            dpp::voiceconn* v = event.from->get_voice(event.command.guild_id);

            /* If the voice channel was invalid, or there is an issue with it, then tell the user. */
            if (!v || !v->voiceclient || !v->voiceclient->is_ready()) {
                event.reply("There was an issue with getting the voice channel. Make sure I'm in a voice channel!");
                return;
            }

            /* Stream the already decoded MP3 file. This passes the PCM data to the library to be encoded to OPUS */
            v->voiceclient->send_audio_raw((uint16_t*)pcmdata.data(), pcmdata.size());

            event.reply("Played the mp3 file.");
            //pcmdata.clear();
        }
        else if (event.command.get_command_name() == "append") {
            mpg123_init();

            int err = 0;
            unsigned char* buffer;
            size_t buffer_size, done;
            int channels, encoding;
            long rate;

            /* Note it is important to force the frequency to 48000 for Discord compatibility */
            mpg123_handle* mh = mpg123_new(NULL, &err);
            mpg123_param(mh, MPG123_FORCE_RATE, 48000, 48000.0);

            /* Decode entire file into a vector. You could do this on the fly, but if you do that
            * you may get timing issues if your CPU is busy at the time and you are streaming to
            * a lot of channels/guilds.
            */
            buffer_size = mpg123_outblock(mh);
            buffer = new unsigned char[buffer_size];

            /* Note: In a real world bot, this should have some error logging */
            const char* appData = std::getenv("APPDATA");
            std::string path = std::string(appData) + "\\Aeroshide\\Jumbo-Josh";
            std::string fileName = "\\test.mp3"; // replace with your file name
            std::string fullPath = path + fileName;

            std::cout << "Appending file " << fullPath << "\n";

            mpg123_open(mh, fullPath.c_str());
            mpg123_getformat(mh, &rate, &channels, &encoding);

            unsigned int counter = 0;
            for (int totalBytes = 0; mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK; ) {
                for (size_t i = 0; i < buffer_size; i++) {
                    pcmdata.push_back(buffer[i]);
                }
                counter += buffer_size;
                totalBytes += done;
            }
            delete[] buffer;
            //pcmdata.clear(); // The pcmdata.clear(); should be placed right before you start loading the new song data into pcmdata an implementation of currently_playing would help!
            mpg123_close(mh);
            mpg123_delete(mh);
        }
        else if (event.command.get_command_name() == "download")
        {
            printf("Early Logging of Download thread\n");
            const char* appData = std::getenv("APPDATA");
            std::string path = std::string(appData) + "/Aeroshide/Jumbo-Josh/";
            std::string fileName = "test"; // Remove the ".mp3" extension
            std::string fullPath = path + fileName;

            std::cout << "Download path initialized: " << fullPath;

            std::string context = std::get<std::string>(event.get_parameter("url")); // Replace with the YouTube URL you want to download

            std::cout << "Youtube URL: " << context;
            // construct the command
            std::string command = ".\\youtube-dl.exe -o \"" + fullPath + ".opus\" --verbose --extract-audio --audio-format opus --postprocessor-args \" -ac 2 -ar 48000\" \"" + context + "\"";


            std::cout << "Executing Download Thread with command: " << command;

            // execute the command
            system(command.c_str());

            // Construct the command to convert the downloaded WAV to MP3
            std::string conversionCommand = "ffmpeg -i " + fullPath + ".opus -codec:a libmp3lame -q:a 4 " + fullPath + ".mp3";

            std::cout << "Executing Conversion with command: " << conversionCommand;

            // Execute the command to convert to MP3
            system(conversionCommand.c_str());

            printf("Download and Conversion Completed!\n");

        }
        });

    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            dpp::slashcommand joincommand("join", "Joins your voice channel.", bot.me.id);
            dpp::slashcommand mp3command("mp3", "Plays an mp3 file.", bot.me.id);
            dpp::slashcommand appendCommand("append", "append RIGHT NOW", bot.me.id);

            dpp::slashcommand myCommand("download", "My command description", bot.me.id);
            myCommand.add_option(
                dpp::command_option(
                    dpp::co_string, // This sets the option type to string
                    "url", // This is the name of the option
                    "My option description", // This is the description of the option
                    true
                )
            );


            bot.global_bulk_command_create({ joincommand, mp3command, myCommand, appendCommand});
        }
        });

	bot.start(dpp::st_wait);

	return 0;
}