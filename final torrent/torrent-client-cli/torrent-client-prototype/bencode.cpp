#include "bencode.h"


namespace Bencode {
/*
 * В это пространство имен рекомендуется вынести функции для работы с данными в формате bencode.
 * Этот формат используется в .torrent файлах и в протоколе общения с трекером
 */
    size_t StringToSize_t(std::string &str)
    {
        size_t answer;
        std::stringstream to_read(str);
        to_read >> answer;
        return answer;
    }

    std::string GetValue(size_t &start, std::string &data)
    {
        std::string Value;
        std::string length_str;
        if (data[start] == 'i')
        {
            start++;
            while (data[start] != 'e')
            {
                Value += data[start++];
            }
            start++;
        }
        else
        {
            while (data[start] != ':')
            {
                length_str += data[start++];
            }
            size_t length = StringToSize_t(length_str);
            start++;
            Value = data.substr(start, length);
            start += length;
        }
        return Value;
    }

    void Parse(std::string &data, std::map<std::string, std::variant<std::map<std::string, std::string>, std::string, std::list<std::string>, std::list<std::list<std::string>>>> &Dictionary, size_t& Start_of_hash, size_t& End_of_hash)
    {
        for (size_t i = 0; i < data.size(); ++i)
        {
            char key = data[i];
            if (key == 'd')
            {
                i++;
                while (data[i] != 'e')
                {
                    std::string Dict_value = "", Dict_key = "";
                    Dict_key = Bencode::GetValue(i, data);

                    if (Dict_key == "info")
                    {
                        Start_of_hash = i;
                    }

                    if (data[i] == 'd') // dictionary
                    {
                        i++;
                        std::map<std::string, std::string> In_dict;
                        while (data[i] != 'e')
                        {
                            std::string In_dict_value = "", In_dict_key = "";
                            In_dict_key = Bencode::GetValue(i, data);
                            In_dict_value = Bencode::GetValue(i, data);
                            In_dict[In_dict_key] = In_dict_value;
                        }
                        i++;
                        Dictionary[Dict_key] = In_dict;
                    }
                    else if (data[i] == 'l')
                    {
                        i++;
                        if (data[i] == 'l')
                        { // list of lists
                            std::list<std::list<std::string>> In_list_list;
                            while (data[i] != 'e')
                            {
                                i++;
                                std::list<std::string> In_list;
                                while (data[i] != 'e')
                                {
                                    std::string In_list_value = "";
                                    In_list_value = Bencode::GetValue(i, data);
                                    In_list.push_back(In_list_value);
                                }
                                In_list_list.push_back(In_list);
                                i++;
                            }
                            i++;
                            Dictionary[Dict_key] = In_list_list;
                        }
                        else
                        { // lists of strings
                            std::list<std::string> In_list;
                            while (data[i] != 'e')
                            {
                                std::string In_list_value = "";
                                In_list_value = Bencode::GetValue(i, data);
                                In_list.push_back(In_list_value);
                            }
                            i++;
                            Dictionary[Dict_key] = In_list;
                        }
                    }
                    else // string
                    {
                        Dict_value = Bencode::GetValue(i, data);
                        Dictionary[Dict_key] = Dict_value;
                    }

                    if (Dict_key == "info")
                    {
                        End_of_hash = i;
                    }
                }
                i++;
            }
        }
    }

    void Parse(std::string &data, std::map<std::string, std::variant<std::map<std::string, std::string>, std::string, std::list<std::string>, std::list<std::list<std::string>>>> &Dictionary)
    {
        size_t s = 0; // to do the same Parse without info_hash field
        size_t e = 0;
        Parse(data,Dictionary,s,e);
    }

}
