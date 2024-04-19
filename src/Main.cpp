// CMakeProject2.cpp : Defines the entry point for the application.
//

#define _CRT_SECURE_NO_WARNINGS
#include <dpp/dpp.h>
#include <cstdlib> // for std::getenv
#include "Song.h"
#include <filesystem>

#include <fmt/format.h>
#include <iomanip>
#include <sstream>

#include <vector>
#include <fstream>
#include <iostream>
#include <mpg123.h>
#include <out123.h>
#include <future>
#include <chrono>

namespace fs = std::filesystem;

auto programStartTime = std::chrono::steady_clock::now();

// smart
void executePlayback(dpp::voiceconn* v, std::vector<uint8_t> pcmdata)
{
    printf("Init Player Thread!\n");
    /* If the voice channel was invalid, or there is an issue with it, then tell the user. */
    if (!v || !v->voiceclient || !v->voiceclient->is_ready()) {
        printf("Player exception thrown, exiting!\n");
        return;
    }

    printf("Player thread playing!\n");

    /* Stream the already decoded MP3 file. This passes the PCM data to the library to be encoded to OPUS */
    v->voiceclient->send_audio_raw((uint16_t*)pcmdata.data(), pcmdata.size());
}

std::string getOsName()
{
#ifdef _WIN32
    return "Windows 32-bit";
#elif _WIN64
    return "Windows 64-bit";
#elif __APPLE__ || __MACH__
    return "Mac OSX";
#elif __linux__
    return "Linux";
#elif __FreeBSD__
    return "FreeBSD";
#elif __unix || __unix__
    return "Unix";
#else
    return "Other";
#endif
}

std::string GetCpuInfo()
{
    // 4 is essentially hardcoded due to the __cpuid function requirements.
    // NOTE: Results are limited to whatever the sizeof(int) * 4 is...
    std::array<int, 4> integerBuffer = {};
    constexpr size_t sizeofIntegerBuffer = sizeof(int) * integerBuffer.size();

    std::array<char, 64> charBuffer = {};

    // The information you wanna query __cpuid for.
    // https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=vs-2019
    constexpr std::array<int, 3> functionIds = {
        // Manufacturer
        //  EX: "Intel(R) Core(TM"
        0x8000'0002,
        // Model
        //  EX: ") i7-8700K CPU @"
        0x8000'0003,
        // Clockspeed
        //  EX: " 3.70GHz"
        0x8000'0004
    };

    std::string cpu;

    for (int id : functionIds)
    {
        // Get the data for the current ID.
        __cpuid(integerBuffer.data(), id);

        // Copy the raw data from the integer buffer into the character buffer
        std::memcpy(charBuffer.data(), integerBuffer.data(), sizeofIntegerBuffer);

        // Copy that data into a std::string
        cpu += std::string(charBuffer.data());
    }

    return cpu;
}

std::string ws2s(const std::wstring& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
    std::string r(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
    return r;
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string getProgramUptime(std::chrono::steady_clock::time_point programStartTime) {
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - programStartTime).count();
    return "Program Uptime: " + std::to_string(uptime) + " seconds";
}

std::string getSystemInfo(std::chrono::steady_clock::time_point programStartTime) {
    std::string os = getOsName();
    std::string cpu = GetCpuInfo();
    std::string programUptime = getProgramUptime(programStartTime);

    return "Operating System: " + os + "\nCPU: " + cpu + "\n" + programUptime;
}

std::tuple<std::string, int, int> javaHashCode(const std::string& str) {
    int h = 0;
    int wraps = 0;
    std::stringstream debugInfo;
    debugInfo << "Showing hashing steps for " << str << ":\n";
    debugInfo << "String Length: " << str.size() << "\n";
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        int prev_h = h;
        h = 31 * h + c;
        if (prev_h > h) { // if the current hash is less than the previous, it means it has wrapped around
            wraps++;
            debugInfo << c << " = " << (31 * h) << " + " << static_cast<int>(c) << " **(overflow here!)**\n";
        }
        else {
            debugInfo << c << " = " << (31 * h) << " + " << static_cast<int>(c) << "\n";
        }
    }
    return { debugInfo.str(), h, wraps };
}

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



std::tuple<std::wstring, std::wstring> renameMp3File(const std::string& folderPath) {
    std::tuple<std::wstring, std::wstring> renamedFile;
    try {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.path().extension() == L".mp3") {
                std::wstring oldFileName = entry.path().filename();
                std::wstring newFileName = L"test.mp3";
                fs::path newFilePath = entry.path().parent_path() / newFileName;
                fs::rename(entry.path(), newFilePath);
                std::wcout << L"Renamed " << oldFileName << L" to " << newFileName << std::endl;
                renamedFile = std::make_tuple(oldFileName, newFileName);
                break; // Exit the loop after processing the first .mp3 file
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::wcerr << L"Filesystem error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::wcerr << L"General error: " << e.what() << std::endl;
    }
    return renamedFile;
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
        if (event.command.get_command_name() == "play")
        {


            printf("Joining vc.\n");
            auto shit = event.command.get_issuing_user();
            event.reply("Hold on, im downloading and encoding the song on my end. it will play shortly " + shit.username + "!");

            
            std::string context = std::get<std::string>(event.get_parameter("url")); // Replace with the YouTube URL you want to download

            Song daSong = getSongMeta(context);

            printf("Early Logging of Download thread\n");
            const char* appData = std::getenv("APPDATA");
            std::string path = std::string(appData) + "/Aeroshide/Jumbo-Josh/";
            std::string fileName = "test"; // Remove the ".mp3" extension
            std::string fullPath = path + fileName;

            std::cout << "Download path initialized: " << fullPath;

            std::cout << "Song platform is: " << daSong.platform << std::endl;
            
            if (daSong.platform == Song::YOUTUBE)
            {
                std::cout << "Youtube URL: " << context;
                // construct the command
                std::string command = ".\\youtube-dl.exe -o \"" + fullPath + ".opus\" --verbose --extract-audio --audio-format opus --postprocessor-args \" -ac 2 -ar 48000\" \"" + context + "\"";


                std::cout << "Executing Download Thread with command: " << command;

                // execute the command
                system(command.c_str());

                // Construct the command to convert the downloaded WAV to MP3
                std::string conversionCommand = "ffmpeg -i \"" + fullPath + ".opus\" -codec:a libmp3lame -q:a 4 -ar 48000 \"" + fullPath + ".mp3\"";

                std::cout << "Executing Conversion with command: " << conversionCommand;

                // Execute the command to convert to MP3
                system(conversionCommand.c_str());

                printf("Download and Conversion Completed!\n");
            }
            else if (daSong.platform == Song::SPOTIFY)
            {
                std::cout << "Spotify URL: " << context << std::endl;
                std::string command = ".\\spotdl " + context +" --output \"" + path + "\"";
                std::cout << "Executing Download Thread with command: " << command;
                system(command.c_str());
                auto [oldName, newName] = renameMp3File(path);
                bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_game, ws2s(oldName.substr(0, 42))));
                printf("Download and Conversion Completed!\n");
            }


            printf("Append data to memory started.\n");

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

            

            std::string mp3Path = fullPath + ".mp3";
            std::cout << "Appending file " << mp3Path << "\n";

            mpg123_open(mh, mp3Path.c_str());

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

            std::cout << "Finished setup, awaiting for client to be ready (file cleanup point)\n";

            /* Attempt to connect to a voice channel, returns false if we fail to connect. */
            dpp::guild* g = dpp::find_guild(event.command.guild_id);

            if (!g->connect_member_voice(event.command.get_issuing_user().id)) {
                event.reply("You don't seem to be in a voice channel!");
                return;
            }

            

            // playback
            std::thread([event, shit, pcmdata, fullPath] {
                std::this_thread::sleep_for(std::chrono::milliseconds(2500));


                try {
                    std::string mp3File = fullPath + ".mp3";
                    std::string opusFile = fullPath + ".opus";

                    if (!std::filesystem::exists(mp3File)) {
                        std::cout << "File " << mp3File << " does not exist.\n";
                    }
                    else if (!std::filesystem::remove(mp3File)) {
                        std::cout << "Failed to delete " << mp3File << ".\n";
                    }

                    if (!std::filesystem::exists(opusFile)) {
                        std::cout << "File " << opusFile << " does not exist.\n";
                    }
                    else if (!std::filesystem::remove(opusFile)) {
                        std::cout << "Failed to delete " << opusFile << ".\n";
                    }

                    if (std::filesystem::exists(mp3File) || std::filesystem::exists(opusFile)) {
                        printf("IMPORTANT: cleanup fault, next queue will fail!\n");
                    }
                    else {
                        printf("IMPORTANT: cleanup complete\n");
                    }
                }
                catch (const std::filesystem::filesystem_error& err) {
                    std::cout << "filesystem error: " << err.what() << '\n';
                }

                dpp::voiceconn* v = event.from->get_voice(event.command.guild_id);
                executePlayback(v, pcmdata);


                }).detach();

            // before you say anything, yes i know this leaks memory, yes i know im bad, and yes you're smart so congrats
            pcmdata.clear();





            



            




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
        else if (event.command.get_command_name() == "hash") {
            std::string replye = std::get<std::string>(event.get_parameter("string"));
            auto [debugInfo, hashed, wraps] = javaHashCode(replye);

            // Convert hashed to a string
            std::string hashedStr = std::to_string(hashed);
            std::string wrapsStr = std::to_string(wraps);

            event.reply(debugInfo + "\n" + "Final Hash Value:: " + hashedStr + "\n" + "Integer Limit Wraps: " + wrapsStr);
        }
        else if (event.command.get_command_name() == "hostinfo")
        {
            event.reply(getSystemInfo(programStartTime));
        }
    });

    bot.on_ready([&bot](const dpp::ready_t& event) {

        bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, "standby, use /play!"));

        if (dpp::run_once<struct register_bot_commands>()) {

            dpp::slashcommand mp3command("mp3", "Plays an mp3 file.", bot.me.id);
            dpp::slashcommand myCommand("play", "Play a Song (Supports Youtube and Spotify)", bot.me.id);
            dpp::slashcommand hostInfoCommand("hostinfo", "Display host info", bot.me.id);
            dpp::slashcommand hashTest("hash", "Technical stuff", bot.me.id);
            myCommand.add_option(
                dpp::command_option(
                    dpp::co_string, // This sets the option type to string
                    "url", // This is the name of the option
                    "The URL of the yt vidfeo", // This is the description of the option
                    true
                )
            );
            hashTest.add_option(
                dpp::command_option(
                    dpp::co_string, // This sets the option type to string
                    "string", // This is the name of the option
                    "Enter the string to be hashed", // This is the description of the option
                    true
                )
            );


            bot.global_bulk_command_create({ myCommand, mp3command, hashTest, hostInfoCommand });
        }
        });

	bot.start(dpp::st_wait);

	return 0;
}


