#pragma once
#include <string>
#include <unordered_map>
#include "File.hpp"
#include "Heap.hpp"

class FileSystem {
private:
    // We use std::unordered_map only as a name->File* registry (not the custom HashMap).
    // The custom HashMap is used inside each File for version_map, as required.
    std::unordered_map<std::string, File*> files;

    File* get_file(const std::string& name);

    // Helper: collect all heapable file entries for RECENT/BIGGEST
    struct FileEntry {
        std::string name;
        time_t last_modified;
        int total_versions;
    };

    std::vector<FileEntry> all_file_entries() const;

public:
    FileSystem() = default;
    ~FileSystem();

    void cmd_create(const std::string& filename);
    void cmd_read(const std::string& filename);
    void cmd_insert(const std::string& filename, const std::string& content);
    void cmd_update(const std::string& filename, const std::string& content);
    void cmd_snapshot(const std::string& filename, const std::string& message);
    void cmd_rollback(const std::string& filename, int version_id, bool has_id);
    void cmd_history(const std::string& filename);
    void cmd_recent_files();
    void cmd_biggest_trees();
};
