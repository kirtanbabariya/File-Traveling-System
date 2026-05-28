#pragma once
#include <string>
#include <vector>
#include <ctime>
#include "HashMap.hpp"

// Represents one version of a file
struct TreeNode {
    int version_id;
    std::string content;
    std::string message;          // Non-empty only if this is a snapshot
    time_t created_timestamp;
    time_t snapshot_timestamp;    // 0 if not a snapshot
    TreeNode* parent;
    std::vector<TreeNode*> children;

    bool is_snapshot() const { return snapshot_timestamp != 0; }

    TreeNode(int id, const std::string& content, TreeNode* parent)
        : version_id(id), content(content), message(""),
          created_timestamp(time(nullptr)), snapshot_timestamp(0),
          parent(parent) {}
};

// Represents a managed file with its version tree
struct File {
    std::string filename;
    TreeNode* root;
    TreeNode* active_version;
    int total_versions;
    time_t last_modified;
    HashMap<int, TreeNode*> version_map;

    File(const std::string& name)
        : filename(name), total_versions(0), last_modified(time(nullptr)),
          version_map(128) {
        // Create root version (ID 0) as an initial snapshot
        root = new TreeNode(0, "", nullptr);
        root->message = "Initial version";
        root->snapshot_timestamp = root->created_timestamp;
        active_version = root;
        version_map.insert(0, root);
        total_versions = 1;
    }

    ~File() { delete_tree(root); }

    void delete_tree(TreeNode* node) {
        if (!node) return;
        for (auto* child : node->children)
            delete_tree(child);
        delete node;
    }

    // Create a new child version of the current active version
    TreeNode* new_version(const std::string& content) {
        int new_id = total_versions;
        TreeNode* node = new TreeNode(new_id, content, active_version);
        active_version->children.push_back(node);
        version_map.insert(new_id, node);
        total_versions++;
        last_modified = time(nullptr);
        active_version = node;
        return node;
    }
};
