// CMakeProject2.cpp : Defines the entry point for the application.
//

#define _CRT_SECURE_NO_WARNINGS
#include "CMakeProject2.h"
#include <dpp/dpp.h>
#include <cstdlib> // for std::getenv
#include <ogg/ogg.h>
#include <opus/opusfile.h>

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

int main()
{

	dpp::cluster bot(token);

	bot.on_log(dpp::utility::cout_logger());

	bot.on_slashcommand([](const dpp::slashcommand_t& event) {
		if (event.command.get_command_name() == "ping")
		{
            dpp::embed embed = dpp::embed()
                .set_color(dpp::colors::sti_blue)
                .set_title("Some name")
                .set_url("https://dpp.dev/")
                .set_author("Some name", "https://dpp.dev/", "https://dpp.dev/DPP-Logo.png")
                .set_description("Some description here")
                .set_thumbnail("https://dpp.dev/DPP-Logo.png")
                .add_field(
                    "Regular field title",
                    "Some value here"
                )
                .add_field(
                    "Inline field title",
                    "Some value here",
                    true
                )
                .add_field(
                    "Inline field title",
                    "Some value here",
                    true
                )
                .set_image("https://dpp.dev/DPP-Logo.png")
                .set_footer(
                    dpp::embed_footer()
                    .set_text("Some footer text here")
                    .set_icon("https://dpp.dev/DPP-Logo.png")
                )
                .set_timestamp(time(0));

            /* Create a message with the content as our new embed. */
            dpp::message msg(event.command.channel_id, embed);

            /* Reply to the user with the message, containing our embed. */
            event.reply(msg);
		}

        if (event.command.get_command_name() == "play")
        {
            std::string context = std::get<std::string>(event.get_parameter("song"));

            dpp::voiceconn* voiceContext = event.from->get_voice(event.command.channel.guild_id);

            std::cout << "audio player thread created\n";

            if (!voiceContext || !voiceContext->voiceclient || !voiceContext->voiceclient->is_ready()) {
                event.reply("There was an issue with getting the voice channel. Make sure I'm in a voice channel!");
                return;
            }

            ogg_sync_state oy;
            ogg_stream_state os;
            ogg_page og;
            ogg_packet op;
            OpusHead header;
            char* buffer;

            FILE* fd;

            fd = fopen("C:/Users/Aeroshide/AppData/Roaming/Aeroshide/Jumbo-Josh/test.opus", "r");

            fseek(fd, 0L, SEEK_END);
            size_t sz = ftell(fd);
            rewind(fd);

            ogg_sync_init(&oy);

            int eos = 0;
            int i;

            buffer = ogg_sync_buffer(&oy, sz);
            fread(buffer, 1, sz, fd);

            ogg_sync_wrote(&oy, sz);

            if (ogg_sync_pageout(&oy, &og) != 1) {
                fprintf(stderr, "Does not appear to be ogg stream.\n");
                exit(1);
            }

            ogg_stream_init(&os, ogg_page_serialno(&og));

            if (ogg_stream_pagein(&os, &og) < 0) {
                fprintf(stderr, "Error reading initial page of ogg stream.\n");
                exit(1);
            }

            if (ogg_stream_packetout(&os, &op) != 1) {
                fprintf(stderr, "Error reading header packet of ogg stream.\n");
                exit(1);
            }

            /* We must ensure that the ogg stream actually contains opus data */
            if (!(op.bytes > 8 && !memcmp("OpusHead", op.packet, 8))) {
                fprintf(stderr, "Not an ogg opus stream.\n");
                exit(1);
            }

            /* Parse the header to get stream info */
            int err = opus_head_parse(&header, op.packet, op.bytes);
            if (err) {
                fprintf(stderr, "Not a ogg opus stream\n");
                exit(1);
            }

            /* Now we ensure the encoding is correct for Discord */
            if (header.channel_count != 2 && header.input_sample_rate != 48000) {
                fprintf(stderr, "Wrong encoding for Discord, must be 48000Hz sample rate with 2 channels.\n");
                exit(1);
            }

            /* Now loop though all the pages and send the packets to the vc */
            while (ogg_sync_pageout(&oy, &og) == 1) {
                ogg_stream_init(&os, ogg_page_serialno(&og));

                if (ogg_stream_pagein(&os, &og) < 0) {
                    fprintf(stderr, "Error reading page of Ogg bitstream data.\n");
                    exit(1);
                }

                while (ogg_stream_packetout(&os, &op) != 0) {

                    /* Read remaining headers */
                    if (op.bytes > 8 && !memcmp("OpusHead", op.packet, 8)) {
                        int err = opus_head_parse(&header, op.packet, op.bytes);
                        if (err) {
                            fprintf(stderr, "Not a ogg opus stream\n");
                            exit(1);
                        }

                        if (header.channel_count != 2 && header.input_sample_rate != 48000) {
                            fprintf(stderr, "Wrong encoding for Discord, must be 48000Hz sample rate with 2 channels.\n");
                            exit(1);
                        }

                        continue;
                    }

                    /* Skip the opus tags */
                    if (op.bytes > 8 && !memcmp("OpusTags", op.packet, 8))
                        continue;

                    /* Send the audio */
                    int samples = opus_packet_get_samples_per_frame(op.packet, 48000);

                    voiceContext->voiceclient->send_audio_opus(op.packet, op.bytes, samples / 48);
                }
            }

            /* Cleanup */
            ogg_stream_clear(&os);
            ogg_sync_clear(&oy);

            event.reply("Finished playing the audio file!");

        }

        if (event.command.get_command_name() == "leave")
        {
            event.from->disconnect_voice(event.command.guild_id);
            event.reply("See ya!");
        }

        if (event.command.get_command_name() == "join") {

            /* Get the guild */
            dpp::guild* g = dpp::find_guild(event.command.guild_id);

            /* Attempt to connect to a voice channel, returns false if we fail to connect. */
            if (!g->connect_member_voice(event.command.get_issuing_user().id)) {
                event.reply("You don't seem to be in a voice channel!");
                return;
            }
        }
	});


	bot.on_ready([&bot](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {

			bot.global_command_create(dpp::slashcommand("ping", "Ping the bot!!!", 1164187926888972480));
            bot.global_command_create(dpp::slashcommand("join", "Joins your voice channel.", 1164187926888972480));
            bot.global_command_create(dpp::slashcommand("leave", "Leaves the vc", 1164187926888972480));

            dpp::slashcommand playmusic("play", "Play a selected music", 1164187926888972480);
            playmusic.add_option(
                dpp::command_option(dpp::co_string, "song", "Pick the song")
                .add_choice(dpp::command_option_choice("Everything Goes On", std::string("song_ego")))
                .add_choice(dpp::command_option_choice("Something Comforting", std::string("song_sc")))
                .add_choice(dpp::command_option_choice("Shop", std::string("song_sh")))
            );

            bot.global_command_create(playmusic);
        }
	});

	bot.start(dpp::st_wait);

	return 0;
}