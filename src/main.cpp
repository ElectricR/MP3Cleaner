#include <iostream>
#include <vector>
#include <ranges>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <fstream>
#include <regex>
#include <experimental/iterator>

#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>

struct SongEntry {
    std::string artist;
    std::string title;
    std::vector<std::string> featuring;
    std::string mod;
    std::string meta_artist;
    std::string meta_title;
    std::string meta_genre;
    std::string meta_album;
    std::string meta_year;
    std::string meta_comment;
};

const std::string CACHE_FILE = "/tmp/MP3CleanerCache";
const std::string MUSIC_DIR = "music";

std::unordered_set<std::string> processed_files;

void check_music_dir() {
    if (!std::filesystem::exists(MUSIC_DIR)) {
        std::cout << "'music' directory is not present, aborting" << std::endl;
        exit(1);
    }
    std::cout << "'music' directory found" << std::endl;
}

void load_cache() {
    std::ifstream cache(CACHE_FILE);
    
    if (cache) {
        std::cout << "Looking up cache..." << std::endl;
        for (std::string line; std::getline(cache, line); ) {
            processed_files.insert(line);
            std::cout << "Found in cache, skipping: " << line << std::endl;
        }
    }
}

int calc_mp3_count() {
    int file_count = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(MUSIC_DIR)) {
        if (entry.is_regular_file() && entry.path().extension() == ".mp3") {
            ++file_count;
        }
    }
    return file_count;
}


void print_template() {
    std::cout << "Result file name:\n\n\n";
    std::cout << "File name:\n\nMetaArtist:\n\nMetaTitle:\n\nMetaAlbum (will be deleted):\n\nMetaYear:\n\nMetaGenre (will be deleted):\n\nMetaComment (will be deleted):\n\n\nArtist:\n\nTitle:\n\nFeaturing:\n\nMod:\n\n" << std::flush;
    std::cout << "\nK?\n\n";
}

std::string get_edited_song_field(int upper_shift) {
    for (int i = 0; i != upper_shift - 1; ++i) {
        std::cout << "\033[F";
    }
    std::cout << "\t\33[2K";
    std::string field;
    std::getline(std::cin, field);
    for (int i = 0; i != upper_shift - 3; ++i) {
        std::cout << "\n";
    }
    std::cout << "\33[2K\t";
    
    return field;
}

void print_result_file_name(SongEntry& song_entry) {
    for (int i = 0; i != 27; ++i) {
        std::cout << "\033[F";
    }
    std::cout << "\33[2K\t";
    std::cout << song_entry.artist;
    if (song_entry.featuring.size()) {
        std::cout << " feat. ";
        std::copy(song_entry.featuring.begin(), song_entry.featuring.end(), std::experimental::make_ostream_joiner(std::cout, ", "));
    }

    std::cout << " - ";
    std::cout << song_entry.title;

    if (song_entry.mod.length()) {
        std::cout << " (";
        std::cout << song_entry.mod;
        std::cout << ")";
    }

    for (int i = 0; i != 27; ++i) {
        std::cout << "\n";
    }
    std::cout << "\t";
}

void print_counter(int file_count) {
    std::cout << "\033[A[" << processed_files.size() + 1 << "/" << file_count << "]" << std::endl;
}


void edit_song_entry(SongEntry& song_entry, auto entry, int file_count) {
    // std::format waiting room
    std::string response;

    for (int i = 0; i != 25; ++i) {
        std::cout << "\033[F";
    }
    std::cout << "\t\33[2K" << entry.path().stem().string() << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_artist << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_title << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_album << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_year << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_genre << std::endl << std::endl;    
    std::string meta_comment_string = song_entry.meta_comment;
    std::ranges::replace(meta_comment_string, '\n', ' ');
    std::ranges::replace(meta_comment_string, '\r', ' ');
    std::cout << "\t\33[2K" << meta_comment_string << std::endl << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.artist << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.title << std::endl << std::endl;    
    std::cout << "\t\33[2K";
    std::copy(song_entry.featuring.begin(), song_entry.featuring.end(), std::experimental::make_ostream_joiner(std::cout, ", "));
    std::cout << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.mod << std::endl;    
    std::cout << "\n\n\t" << std::flush;

    print_counter(file_count);

    while (true) {
        print_result_file_name(song_entry);
    
        std::getline(std::cin, response);

        if (response == "a") {
            song_entry.artist = get_edited_song_field(11);
        } else if (response == "t") {
            song_entry.title = get_edited_song_field(9);
        } else if (response == "f") {
            auto new_featuring = get_edited_song_field(7);
            if (new_featuring.empty()) {
                song_entry.featuring = {};
            } else {
                song_entry.featuring = {new_featuring};
            }
        } else if (response == "m") {
            song_entry.mod = get_edited_song_field(5);
        } else if (response == "q") {
            std::exit(0);
        } else if (response == "") {
            break;
        } else {
            std::cout << "\033[F\33[2K\t";
        }
    }
}


std::vector<std::string> extract_featuring(const std::smatch& match, unsigned place) {
    const static std::regex featuring_regex(R"(((\b[fF]eat\. |\b[fF]t\. |\bwith |, )(.+?) ?(?=\b[fF]eat\. |\b[fF]t\. |\bwith |, |$))+?)");
    std::vector<std::string> featuring;

    if (match[place].length()) {
        std::string match_str = match[place].str();
        auto it = std::sregex_iterator(match_str.begin(), match_str.end(), featuring_regex);
        for (; it != std::sregex_iterator{}; ++it) {
            auto it_str = (*it)[3].str();
            if (it_str.length()) featuring.push_back(std::move(it_str));
        }
    }
    return featuring;
}


void parse_name(SongEntry& song_entry, auto entry) {
    const static std::regex base_song_regex(R"(^(.+?) ?((\b[fF]eat\. |\b[fF]t\. |, )(.+))?( (?:–|-) )(.+?)( ?\((.+)\))?$)");
    const static std::regex featuring_detection(R"(^([fF]eat\. |[fF]t\. |with |, ).+)");

    std::string entry_name = entry.path().stem().string();

    std::vector<std::string> featuring;
    std::string song_mod;
    std::smatch match;

    if (std::regex_match(entry_name, match, base_song_regex)) {
        featuring = extract_featuring(match, 2);
        if (match[8].length() && !std::regex_match(match[8].str(), featuring_detection)) {
            song_mod = match[8].str();
        }
        else {
            std::ranges::copy(extract_featuring(match, 8), std::back_inserter(featuring));
        }
    }

    song_entry.artist = match[1];
    song_entry.title = match[6];
    song_entry.mod = std::move(song_mod);
    song_entry.featuring = std::move(featuring);
}


void load_metadata(SongEntry& song_entry, auto entry) {
    TagLib::MPEG::File file_entry(entry.path().c_str());
    TagLib::ID3v2::Tag &tag_entry = *file_entry.ID3v2Tag();
    song_entry.meta_artist = std::string(tag_entry.artist().data(TagLib::String::Type::UTF8).data(), tag_entry.artist().data(TagLib::String::Type::UTF8).size());
    song_entry.meta_title = std::string(tag_entry.title().data(TagLib::String::Type::UTF8).data(), tag_entry.title().data(TagLib::String::Type::UTF8).size());
    song_entry.meta_genre = std::string(tag_entry.genre().data(TagLib::String::Type::UTF8).data(), tag_entry.genre().data(TagLib::String::Type::UTF8).size());
    song_entry.meta_album = std::string(tag_entry.album().data(TagLib::String::Type::UTF8).data(), tag_entry.album().data(TagLib::String::Type::UTF8).size());
    song_entry.meta_comment = std::string(tag_entry.comment().data(TagLib::String::Type::UTF8).data(), tag_entry.comment().data(TagLib::String::Type::UTF8).size());
    song_entry.meta_year = tag_entry.year();
}


SongEntry load_data(auto entry) {
    SongEntry song_entry {};
    parse_name(song_entry, entry);
    load_metadata(song_entry, entry);
    return song_entry;
}


std::string apply_entry(SongEntry& song_entry, auto entry) {
    TagLib::FileRef file_entry(entry.path().c_str());
    TagLib::Tag &tag_entry = *file_entry.tag();

    std::stringstream name_ss;
    name_ss << song_entry.artist; 
    if (song_entry.featuring.size()) {
        name_ss << " feat. ";
        std::copy(song_entry.featuring.begin(), song_entry.featuring.end(), std::experimental::make_ostream_joiner(name_ss, ", "));
    }

    tag_entry.setArtist(TagLib::String(name_ss.str(), TagLib::String::Type::UTF8));

    std::stringstream title_ss;
    title_ss << song_entry.title;
    if (song_entry.mod.size()) {
        title_ss << " (" +song_entry.mod + ")";
    }

    tag_entry.setTitle(TagLib::String(title_ss.str(), TagLib::String::Type::UTF8));
    tag_entry.setComment("");
    tag_entry.setAlbum("");
    tag_entry.setGenre("");

    file_entry.save();
    auto new_entry = entry;
    new_entry.replace_filename(name_ss.str() + " - " + title_ss.str() + ".mp3");
    std::filesystem::rename(entry, new_entry);
    return new_entry.path().stem().string();
}

void run_cleaner(int file_count) {
    std::ofstream cache (CACHE_FILE, std::ios_base::app);
    print_template();
    for (const auto& entry : std::filesystem::recursive_directory_iterator(MUSIC_DIR)) {
        if (entry.is_regular_file() && !processed_files.contains(entry.path().stem().string())) {
            SongEntry song_entry = load_data(entry);
            edit_song_entry(song_entry, entry, file_count);
            auto result = apply_entry(song_entry, entry);   
            processed_files.insert(result);
            cache << result << std::endl;
            cache.flush();
        }
    }
}

int main() {
    check_music_dir();
    load_cache();
    int file_count = calc_mp3_count();
    run_cleaner(file_count);
    return 0;
}
