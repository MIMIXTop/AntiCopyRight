#include "Lemmatizer.hpp"
#include <Util/util.hpp>

#include <fstream>
#include <algorithm>
#include <QDebug>

#ifndef MYSTEM_EXECUTABLE
    # error "MYSTEM_EXECUTABLE не задан!"
#endif

namespace {
    #ifdef WIN32
        #define PATH_STOP_WORDS ..\\"Utils\\StopWords\\russianStopWords.txt"
    #else
        #define PATH_STOP_WORDS "../Utils/StopWords/stopWords.txt"
    #endif

    std::unordered_set<std::string> getStopWords(const std::string &path = PATH_STOP_WORDS) {
        std::unordered_set<std::string> stopWords = {};

        if (std::ifstream file(path); file.is_open()) {
            std::string line;
            while (file >> line) {
                stopWords.insert(line);
            }
            file.close();
        } else {
            qInfo() << "Could not open file";
        }

        return stopWords;
    }

}

Lemmatizer::Lemmatizer() : stopWords(getStopWords()) {}

std::vector<std::string> Lemmatizer::getLemmas(const std::string &text) {
    std::vector<std::string> lemmas;
    std::string command = "echo \"" + text + "\" | " + MYSTEM_EXECUTABLE + " -l -n" ;
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        qInfo() << "Could not open pipe";
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);

        if (!line.empty()) {
            lemmas.push_back(util::StringWorker::getFirstLemma(line));
        }
    }

    pclose(pipe);
    return lemmas;
}
