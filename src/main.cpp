#include <iostream>
#include <vector>
#include <ranges>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <fstream>
#include <regex>
#include <experimental/iterator>

#include <tag.h>
#include <fileref.h>

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

std::unordered_set<std::string> progressed_files;
size_t file_count;

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

void print_counter() {
    std::cout << "\033[A[" << progressed_files.size() + 1 << "/" << file_count << "]" << std::endl;
}

void edit_song_entry(SongEntry& song_entry, auto entry) {
    // std::format waiting room
    std::string responce;

    for (int i = 0; i != 25; ++i) {
        std::cout << "\033[F";
    }
    std::cout << "\t\33[2K" << entry.path().stem().string() << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_artist << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_title << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_album << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_year << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_genre << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.meta_comment << std::endl << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.artist << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.title << std::endl << std::endl;    
    std::cout << "\t\33[2K";
    std::copy(song_entry.featuring.begin(), song_entry.featuring.end(), std::experimental::make_ostream_joiner(std::cout, ", "));
    std::cout << std::endl << std::endl;    
    std::cout << "\t\33[2K" << song_entry.mod << std::endl;    
    std::cout << "\n\n\t" << std::flush;

    print_counter();

    while (true) {
        print_result_file_name(song_entry);
    
        std::getline(std::cin, responce);

        if (responce == "a") {
            song_entry.artist = get_edited_song_field(11);
        } else if (responce == "t") {
            song_entry.title = get_edited_song_field(9);
        } else if (responce == "f") {
            auto new_featuring = get_edited_song_field(7);
            if (new_featuring.empty()) {
                song_entry.featuring = {};
            } else {
                song_entry.featuring = {new_featuring};
            }
        } else if (responce == "m") {
            song_entry.mod = get_edited_song_field(5);
        } else if (responce == "q") {
            std::exit(0);
        } else if (responce == "") {
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
    TagLib::FileRef tag_entry(entry.path().c_str());
    song_entry.meta_artist = tag_entry.tag()->artist().toCString();
    song_entry.meta_title = tag_entry.tag()->title().toCString();
    song_entry.meta_genre = tag_entry.tag()->genre().toCString();
    song_entry.meta_album = tag_entry.tag()->album().toCString();
    song_entry.meta_year = tag_entry.tag()->year();
    song_entry.meta_comment = tag_entry.tag()->comment().toCString();
}

SongEntry load_data(auto entry) {
    SongEntry song_entry {};
    parse_name(song_entry, entry);
    load_metadata(song_entry, entry);
    return song_entry;
}

void load_cache() {
    std::ifstream cache(std::filesystem::current_path()/"MP3CleanerCache");
    
    if (cache) {
        for (std::string line; std::getline(cache, line); ) {
            progressed_files.insert(line);
            std::cout << line << std::endl;
        }
    }
}

std::string apply_entry(SongEntry& song_entry, auto entry) {
    TagLib::FileRef tag_entry(entry.path().c_str());
    std::stringstream name_ss;
    name_ss << song_entry.artist; 
    if (song_entry.featuring.size()) {
        name_ss << " feat. ";
        std::copy(song_entry.featuring.begin(), song_entry.featuring.end(), std::experimental::make_ostream_joiner(name_ss, ", "));
    }

    tag_entry.tag()->setArtist(name_ss.str());

    std::stringstream title_ss;
    title_ss << song_entry.title;
    if (song_entry.mod.size()) {
        title_ss << " (" +song_entry.mod + ")";
    }
    tag_entry.tag()->setTitle(title_ss.str());
    tag_entry.tag()->setComment("");
    tag_entry.tag()->setAlbum("");
    tag_entry.tag()->setGenre("");

    tag_entry.save();
    auto new_entry = entry;
    new_entry.replace_filename(name_ss.str() + " - " + title_ss.str() + ".mp3");
    std::filesystem::rename(entry, new_entry);
    return new_entry.path().stem().string();
}

int main() {
    load_cache();

    std::ofstream cache (std::filesystem::current_path()/"MP3CleanerCache", std::ios_base::app);
    for (const auto& entry : std::filesystem::recursive_directory_iterator("Music")) {
        if (entry.is_regular_file()) {
            ++file_count;
        }
    }

    print_template();
    for (const auto& entry : std::filesystem::recursive_directory_iterator("Music")) {
        if (entry.is_regular_file() && !progressed_files.contains(entry.path().stem().string())) {
            SongEntry song_entry = load_data(entry);
            edit_song_entry(song_entry, entry);
            auto result = apply_entry(song_entry, entry);   
            progressed_files.insert(result);
            cache << result << std::endl;
            cache.flush();
        }
    }

    return 0;
}
