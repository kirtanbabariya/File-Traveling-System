#include "FileSystem.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

// ── helpers ────────────────────────────────────────────────────────────────

static std::string fmt_time(time_t t) {
    char buf[32];
    struct tm* tm_info = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buf);
}

File* FileSystem::get_file(const std::string& name) {
    auto it = files.find(name);
    if (it == files.end()) return nullptr;
    return it->second;
}

std::vector<FileSystem::FileEntry> FileSystem::all_file_entries() const {
    std::vector<FileEntry> v;
    for (auto& [name, f] : files)
        v.push_back({name, f->last_modified, f->total_versions});
    return v;
}

FileSystem::~FileSystem() {
    for (auto& [name, f] : files) delete f;
}

// ── commands ───────────────────────────────────────────────────────────────

void FileSystem::cmd_create(const std::string& filename) {
    if (files.count(filename)) {
        std::cout << "ERROR: File '" << filename << "' already exists.\n";
        return;
    }
    files[filename] = new File(filename);
    std::cout << "Created file '" << filename << "' (version 0, initial snapshot).\n";
}

void FileSystem::cmd_read(const std::string& filename) {
    File* f = get_file(filename);
    if (!f) { std::cout << "ERROR: File '" << filename << "' not found.\n"; return; }
    std::cout << "[" << filename << " | v" << f->active_version->version_id;
    if (f->active_version->is_snapshot()) std::cout << " (snapshot)";
    std::cout << "]\n";
    if (f->active_version->content.empty())
        std::cout << "(empty)\n";
    else
        std::cout << f->active_version->content << "\n";
}

void FileSystem::cmd_insert(const std::string& filename, const std::string& content) {
    File* f = get_file(filename);
    if (!f) { std::cout << "ERROR: File '" << filename << "' not found.\n"; return; }

    if (f->active_version->is_snapshot()) {
        // Create a new child version
        TreeNode* nv = f->new_version(f->active_version->content + content);
        std::cout << "Inserted into new version v" << nv->version_id
                  << " of '" << filename << "'.\n";
    } else {
        // Modify in place
        f->active_version->content += content;
        f->last_modified = time(nullptr);
        std::cout << "Inserted into current version v"
                  << f->active_version->version_id << " of '" << filename << "' (in-place).\n";
    }
}

void FileSystem::cmd_update(const std::string& filename, const std::string& content) {
    File* f = get_file(filename);
    if (!f) { std::cout << "ERROR: File '" << filename << "' not found.\n"; return; }

    if (f->active_version->is_snapshot()) {
        TreeNode* nv = f->new_version(content);
        std::cout << "Updated to new version v" << nv->version_id
                  << " of '" << filename << "'.\n";
    } else {
        f->active_version->content = content;
        f->last_modified = time(nullptr);
        std::cout << "Updated version v" << f->active_version->version_id
                  << " of '" << filename << "' (in-place).\n";
    }
}

void FileSystem::cmd_snapshot(const std::string& filename, const std::string& message) {
    File* f = get_file(filename);
    if (!f) { std::cout << "ERROR: File '" << filename << "' not found.\n"; return; }

    TreeNode* av = f->active_version;
    if (av->is_snapshot()) {
        std::cout << "ERROR: Version v" << av->version_id << " is already a snapshot.\n";
        return;
    }
    av->message = message;
    av->snapshot_timestamp = time(nullptr);
    f->last_modified = av->snapshot_timestamp;
    std::cout << "Snapshot taken at version v" << av->version_id
              << " of '" << filename << "': \"" << message << "\".\n";
}

void FileSystem::cmd_rollback(const std::string& filename, int version_id, bool has_id) {
    File* f = get_file(filename);
    if (!f) { std::cout << "ERROR: File '" << filename << "' not found.\n"; return; }

    TreeNode* target = nullptr;
    if (has_id) {
        TreeNode** ptr = f->version_map.find(version_id);
        if (!ptr) {
            std::cout << "ERROR: Version " << version_id << " not found in '" << filename << "'.\n";
            return;
        }
        target = *ptr;
    } else {
        target = f->active_version->parent;
        if (!target) {
            std::cout << "ERROR: Version v0 has no parent to roll back to.\n";
            return;
        }
    }
    f->active_version = target;
    std::cout << "Rolled back '" << filename << "' to version v"
              << target->version_id << ".\n";
}

void FileSystem::cmd_history(const std::string& filename) {
    File* f = get_file(filename);
    if (!f) { std::cout << "ERROR: File '" << filename << "' not found.\n"; return; }

    // Collect snapshots via DFS traversal of version tree
    std::vector<TreeNode*> snapshots;
    std::vector<TreeNode*> stack = {f->root};
    while (!stack.empty()) {
        TreeNode* cur = stack.back(); stack.pop_back();
        if (cur->is_snapshot()) snapshots.push_back(cur);
        for (auto* child : cur->children) stack.push_back(child);
    }

    // Sort chronologically by snapshot_timestamp
    std::sort(snapshots.begin(), snapshots.end(),
              [](TreeNode* a, TreeNode* b) {
                  return a->snapshot_timestamp < b->snapshot_timestamp;
              });

    if (snapshots.empty()) {
        std::cout << "No snapshots found for '" << filename << "'.\n";
        return;
    }

    std::cout << "Snapshot history of '" << filename << "':\n";
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(22) << "Timestamp"
              << "Message\n";
    std::cout << std::string(60, '-') << "\n";
    for (auto* s : snapshots) {
        std::cout << std::setw(6)  << ("v" + std::to_string(s->version_id))
                  << std::setw(22) << fmt_time(s->snapshot_timestamp)
                  << s->message << "\n";
    }
}

void FileSystem::cmd_recent_files() {
    auto entries = all_file_entries();
    if (entries.empty()) { std::cout << "No files in the system.\n"; return; }

    Heap<FileEntry> heap([](const FileEntry& a, const FileEntry& b) {
        return a.last_modified > b.last_modified;
    });
    heap.rebuild(entries);

    auto sorted = heap.sorted_copy();
    std::cout << "Recent files (most recently modified first):\n";
    std::cout << std::left << std::setw(25) << "File" << "Last Modified\n";
    std::cout << std::string(50, '-') << "\n";
    for (auto& e : sorted)
        std::cout << std::setw(25) << e.name << fmt_time(e.last_modified) << "\n";
}

void FileSystem::cmd_biggest_trees() {
    auto entries = all_file_entries();
    if (entries.empty()) { std::cout << "No files in the system.\n"; return; }

    Heap<FileEntry> heap([](const FileEntry& a, const FileEntry& b) {
        return a.total_versions > b.total_versions;
    });
    heap.rebuild(entries);

    auto sorted = heap.sorted_copy();
    std::cout << "Biggest version trees (most versions first):\n";
    std::cout << std::left << std::setw(25) << "File" << "Total Versions\n";
    std::cout << std::string(45, '-') << "\n";
    for (auto& e : sorted)
        std::cout << std::setw(25) << e.name << e.total_versions << "\n";
}
