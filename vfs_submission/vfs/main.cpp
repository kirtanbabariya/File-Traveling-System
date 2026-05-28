#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "FileSystem.hpp"

// Split a line into tokens, respecting quoted strings
static std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    bool in_quotes = false;
    std::string current;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ' ' && !in_quotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

int main() {
    FileSystem fs;
    std::string line;

    std::cout << "Time-Travelling File System (COL106 Assignment)\n";
    std::cout << "Type HELP for command list, EXIT to quit.\n\n";

    while (true) {
        std::cout << "vfs> ";
        if (!std::getline(std::cin, line)) break;  // EOF

        // Trim leading/trailing whitespace
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.erase(line.begin());
        while (!line.empty() && (line.back() == ' ' || line.back() == '\t' || line.back() == '\r')) line.pop_back();

        if (line.empty()) continue;

        auto tokens = tokenize(line);
        if (tokens.empty()) continue;

        const std::string& cmd = tokens[0];

        // ── EXIT ──────────────────────────────────────────────
        if (cmd == "EXIT" || cmd == "exit" || cmd == "quit") {
            std::cout << "Goodbye.\n";
            break;
        }

        // ── HELP ──────────────────────────────────────────────
        else if (cmd == "HELP" || cmd == "help") {
            std::cout <<
                "Commands:\n"
                "  CREATE <filename>\n"
                "  READ <filename>\n"
                "  INSERT <filename> <content>\n"
                "  UPDATE <filename> <content>\n"
                "  SNAPSHOT <filename> <message>\n"
                "  ROLLBACK <filename> [versionID]\n"
                "  HISTORY <filename>\n"
                "  RECENT_FILES\n"
                "  BIGGEST_TREES\n"
                "  EXIT\n"
                "Note: content/message with spaces must be wrapped in double quotes.\n";
        }

        // ── CREATE <filename> ─────────────────────────────────
        else if (cmd == "CREATE") {
            if (tokens.size() < 2) { std::cout << "Usage: CREATE <filename>\n"; continue; }
            fs.cmd_create(tokens[1]);
        }

        // ── READ <filename> ───────────────────────────────────
        else if (cmd == "READ") {
            if (tokens.size() < 2) { std::cout << "Usage: READ <filename>\n"; continue; }
            fs.cmd_read(tokens[1]);
        }

        // ── INSERT <filename> <content> ───────────────────────
        else if (cmd == "INSERT") {
            if (tokens.size() < 3) { std::cout << "Usage: INSERT <filename> <content>\n"; continue; }
            // Rejoin tokens[2..] as content
            std::string content;
            for (size_t i = 2; i < tokens.size(); i++) {
                if (i > 2) content += " ";
                content += tokens[i];
            }
            fs.cmd_insert(tokens[1], content);
        }

        // ── UPDATE <filename> <content> ───────────────────────
        else if (cmd == "UPDATE") {
            if (tokens.size() < 3) { std::cout << "Usage: UPDATE <filename> <content>\n"; continue; }
            std::string content;
            for (size_t i = 2; i < tokens.size(); i++) {
                if (i > 2) content += " ";
                content += tokens[i];
            }
            fs.cmd_update(tokens[1], content);
        }

        // ── SNAPSHOT <filename> <message> ─────────────────────
        else if (cmd == "SNAPSHOT") {
            if (tokens.size() < 3) { std::cout << "Usage: SNAPSHOT <filename> <message>\n"; continue; }
            std::string msg;
            for (size_t i = 2; i < tokens.size(); i++) {
                if (i > 2) msg += " ";
                msg += tokens[i];
            }
            fs.cmd_snapshot(tokens[1], msg);
        }

        // ── ROLLBACK <filename> [versionID] ───────────────────
        else if (cmd == "ROLLBACK") {
            if (tokens.size() < 2) { std::cout << "Usage: ROLLBACK <filename> [versionID]\n"; continue; }
            if (tokens.size() >= 3) {
                try {
                    int vid = std::stoi(tokens[2]);
                    fs.cmd_rollback(tokens[1], vid, true);
                } catch (...) {
                    std::cout << "ERROR: Invalid version ID '" << tokens[2] << "'.\n";
                }
            } else {
                fs.cmd_rollback(tokens[1], -1, false);
            }
        }

        // ── HISTORY <filename> ────────────────────────────────
        else if (cmd == "HISTORY") {
            if (tokens.size() < 2) { std::cout << "Usage: HISTORY <filename>\n"; continue; }
            fs.cmd_history(tokens[1]);
        }

        // ── RECENT_FILES ──────────────────────────────────────
        else if (cmd == "RECENT_FILES" || cmd == "RECENT FILES") {
            fs.cmd_recent_files();
        }

        // ── BIGGEST_TREES ─────────────────────────────────────
        else if (cmd == "BIGGEST_TREES" || cmd == "BIGGEST TREES") {
            fs.cmd_biggest_trees();
        }

        else {
            std::cout << "Unknown command: '" << cmd << "'. Type HELP for list.\n";
        }
    }
    return 0;
}
