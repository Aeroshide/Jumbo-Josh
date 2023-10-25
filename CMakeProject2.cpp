// CMakeProject2.cpp : Defines the entry point for the application.
//

#include "CMakeProject2.h"
#include <dpp/dpp.h>

const std::string token = "TVRFMk5ERTROemt5TmpnNE9EazNNalE0TUEuR0RZSmdCLnE3MWNTT1o0MFotSXNfLXo0TUFXa0lsMUNkZHNPWFlFV0xqYXVN"; //ytta

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

        if (event.command.get_command_name() == "leave")
        {
            event.from->disconnect_voice(event.command.guild_id);
            event.reply("See ya!");
        }

        if (event.command.get_command_name() == "join") {

            /* Get the guild */
            dpp::guild* g = dpp::find_guild(event.command.guild_id);

            /* Get the voice channel that the bot is currently in from this server (will return nullptr if we're not in a voice channel!) */
            auto current_vc = event.from->get_voice(event.command.guild_id);

            bool join_vc = true;

            /* Are we in a voice channel? If so, let's see if we're in the right channel. */
            if (current_vc) {
                /* Find the channel id that the user is currently in */
                auto users_vc = g->voice_members.find(event.command.get_issuing_user().id);

                if (users_vc != g->voice_members.end() && current_vc->channel_id == users_vc->second.channel_id) {
                    join_vc = false;

                    /* We are on this voice channel, at this point we can send any audio instantly to vc:
                     * current_vc->send_audio_raw(...)
                     */
                }
                else {
                    /* We are on a different voice channel. We should leave it, then join the new one
                     * by falling through to the join_vc branch below.
                     */
                    event.from->disconnect_voice(event.command.guild_id);

                    join_vc = true;
                }
            }

            /* If we need to join a vc at all, join it here if join_vc == true */
            if (join_vc) {
                /* Attempt to connect to a voice channel, returns false if we fail to connect. */

                /* The user issuing the command is not on any voice channel, we can't do anything */
                if (!g->connect_member_voice(event.command.get_issuing_user().id)) {
                    event.reply("You don't seem to be in a voice channel!");
                    return;
                }

                /* We are now connecting to a vc. Wait for on_voice_ready
                 * event, and then send the audio within that event:
                 *
                 * event.voice_client->send_audio_raw(...);
                 *
                 * NOTE: We can't instantly send audio, as we have to wait for
                 * the connection to the voice server to be established!
                 */

                 /* Tell the user we joined their channel. */
                event.reply("Joined your channel!");
            }
            else {
                event.reply("Don't need to join your channel as i'm already there with you!");
            }
        }
	});


	bot.on_ready([&bot](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			bot.global_command_create(dpp::slashcommand("ping", "Ping the bot!!!", 1164187926888972480));
            bot.global_command_create(dpp::slashcommand("join", "Joins your voice channel.", 1164187926888972480));
            bot.global_command_create(dpp::slashcommand("leave", "Leaves the vc", 1164187926888972480));
        }
	});

	bot.start(dpp::st_wait);

	return 0;
}
