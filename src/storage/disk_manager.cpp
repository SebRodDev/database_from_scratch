#include "disk_manager.h"
#include <stdexcept>

DiskManager::DiskManager(const std::string& db_file) : file_name_(db_file) {
    // Open for read+write; create the file if it doesn't exist yet.
    file_.open(file_name_, std::ios::in | std::ios::out | std::ios::binary);
    if (!file_.is_open()) {
        file_.clear();
        file_.open(file_name_, std::ios::out | std::ios::binary); // create
        file_.close();
        file_.open(file_name_, std::ios::in | std::ios::out | std::ios::binary);
    }
    if (!file_.is_open()) {
        throw std::runtime_error("DiskManager: could not open " + file_name_);
    }

    file_.seekg(0, std::ios::end);
    auto size_bytes = file_.tellg();
    num_pages_ = static_cast<pageId>(size_bytes / PAGE_SIZE);
}

DiskManager::~DiskManager() {
    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }
}

void DiskManager::WritePage(pageId page_id, const void* data) {
    std::lock_guard<std::mutex> lock(latch_);
    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
    file_.seekp(offset);
    file_.write(reinterpret_cast<const char*>(data), PAGE_SIZE);
    if (file_.bad()) {
        throw std::runtime_error("DiskManager: write failed for page " + std::to_string(page_id));
    }
    file_.flush(); // fine for now; batching flushes is a later optimization
}

void DiskManager::ReadPage(pageId page_id, void* data) {
    std::lock_guard<std::mutex> lock(latch_);
    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
    file_.seekg(offset);
    file_.read(reinterpret_cast<char*>(data), PAGE_SIZE);
    if (file_.eof()) {
        // Reading a page that was allocated but never written -- zero-fill.
        file_.clear();
        std::memset(data, 0, PAGE_SIZE);
    } else if (file_.bad()) {
        throw std::runtime_error("DiskManager: read failed for page " + std::to_string(page_id));
    }
}

pageId DiskManager::AllocatePage() {
    std::lock_guard<std::mutex> lock(latch_);
    return num_pages_++;
}
