#pragma once
#include "page.h"
#include <fstream>
#include <mutex>
#include <string>

class DiskManager {
public:
    explicit DiskManager(const std::string& db_file);
    ~DiskManager();

    // Writes exactly PAGE_SIZE bytes from `data` to the page at page_id.
    void WritePage(pageId page_id, const void* data);

    // Reads exactly PAGE_SIZE bytes from the page at page_id into `data`.
    void ReadPage(pageId page_id, void* data);

    // Grows the file by one page and returns the new page's id.
    pageId AllocatePage();

    uint32_t NumPages() const { return num_pages_; }

private:
    std::fstream file_;
    std::string  file_name_;
    pageId num_pages_;
    std::mutex   latch_;   // protects the fstream + num_pages_
};
