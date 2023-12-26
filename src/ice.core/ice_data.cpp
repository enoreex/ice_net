#include "ice_data.h"

ice_data::write::write(unsigned short reserve_size)
{
    if (reserve_size == 0) ++reserve_size;

    this->data = std::vector<char>();
    this->data.reserve(reserve_size);
    this->data.push_back(-1);
}

void ice_data::write::set_flag(const char& value)
{
    data[0] = value;
}

void ice_data::write::add_buffer(const char* value, const unsigned short size)
{
    for (int i = 0; i < size; i++) data.push_back(value[i]);
}

void ice_data::write::add_int8(const char& value)
{
    data.push_back(value & 0xFF);
}

void ice_data::write::add_int16(const short& value)
{
    data.push_back(((value) >> 8) & 0xFF);
    data.push_back(((value) >> 0) & 0xFF);
}

void ice_data::write::add_string(const std::string& value)
{
    int size = static_cast<unsigned short>(value.size());

    add_int16(size);

    for (int i = 0; i < size; i++) data.push_back((value)[i] & 0xFF);
}

char* ice_data::write::get_buffer()
{
    return data.data();
}

unsigned short ice_data::write::get_buffer_size()
{
    return static_cast<unsigned short>(data.size());
}

ice_data::read::read(char* data, const int size)
{
    this->data = std::vector<char>(data, (data + size));
    this->read_position = 0;
}

char ice_data::read::get_flag()
{
    if (data.empty()) return -1;

    return data[0];
}

char ice_data::read::get_int8()
{
    if (read_position + 1 > data.size() - 1) return 0;

    return data[++read_position];
}

short ice_data::read::get_int16()
{
    if (read_position + 2 > data.size() - 1) return 0;

    short result = 0;
    
    result |= static_cast<unsigned char>(data[++read_position]) << 8;
    result |= static_cast<unsigned char>(data[++read_position]) << 0;

    return result;
}

std::string ice_data::read::get_string()
{
    unsigned short size = get_int16();
    
    if (size < 1) return "";

    if (read_position + size > data.size() - 1) return "";

    std::string result(data.begin() + read_position + 1, data.begin() + read_position + 1 + size);

    read_position += size;

    return result;
}

char* ice_data::read::get_buffer()
{
    return data.data();
}

unsigned short ice_data::read::get_buffer_size()
{
    return static_cast<unsigned short>(data.size());
}