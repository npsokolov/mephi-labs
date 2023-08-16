#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>

namespace Bencode {

/*
 * В это пространство имен рекомендуется вынести функции для работы с данными в формате bencode.
 * Этот формат используется в .torrent файлах и в протоколе общения с трекером
 */

    size_t StringToSize_t(std::string &str);

    std::string GetValue(size_t &start, std::string &data);

    void Parse(std::string &data, std::map<std::string, std::variant<std::map<std::string, std::string>, std::string, std::list<std::string>, std::list<std::list<std::string>>>> &Dictionary, size_t& Start_of_hash, size_t& End_of_hash);

    void Parse(std::string &data, std::map<std::string, std::variant<std::map<std::string, std::string>, std::string, std::list<std::string>, std::list<std::list<std::string>>>> &Dictionary);


}
