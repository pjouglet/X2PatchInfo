#include <iostream>
#include <filesystem>

void readFile(std::string filename) {
    bool isKom = false;
    if (filename.find_last_of(".kom") == filename.length() - 1)
        isKom = true;
    std::cout << "Reading: " << filename << std::endl;
}

void readDirectory(std::string path) {
    for (auto& p : std::filesystem::directory_iterator(path)) {
        std::string test = p.path().string();
        if (p.is_directory()) {
            readDirectory(test);
            continue;
        }
        readFile(test);
    }
}

int main() {

    readDirectory("./");
    return 0;
}
