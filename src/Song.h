#pragma once

#include <string>
#include <algorithm> // For std::transform

struct Song {
    std::string id;
    std::string link;

    enum Platform {
        SPOTIFY,
        YOUTUBE,
        NONE // Added to handle cases where the platform is neither Spotify nor YouTube
    };

    Platform platform;

    // Utility function to convert a string to lowercase
    static std::string toLower(const std::string& input) {
        std::string output = input;
        std::transform(output.begin(), output.end(), output.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return output;
    }

    // Function to determine the platform based on the link
    static Platform detectPlatform(const std::string& link) {
        std::string lowerLink = toLower(link);
        if (lowerLink.find("youtu") != std::string::npos) {
            return YOUTUBE;
        }
        else if (lowerLink.find("spotify") != std::string::npos) {
            return SPOTIFY;
        }
        else {
            return NONE;
        }
    }
};

Song getSongMeta(const std::string& link) {
    Song song;

    song.link = link;
    song.platform = Song::detectPlatform(link);

    // Assuming the 'id' should be extracted or set here as well
    // This is a placeholder for whatever logic you need to set the 'id'
    // For example, extracting it from the link or setting it based on other criteria
    song.id = "placeholderk";

    return song;
}