#pragma once

#ifndef ICE_DATA
#define ICE_DATA

#include <algorithm>
#include <vector>
#include <string>

class ice_data final
{

private:

	ice_data();

public:

	class write final
	{

	public:

		write(unsigned short reserve_size = 32);

	private:

		std::vector<char> data;

	public:

		void set_flag(const char& value);

	public:

		void add_buffer(const char* value, const unsigned short size);

		void add_int8(const char& value);

		void add_int16(const short& value);

		void add_string(const std::string& value);

	public:

		char* get_buffer();

		unsigned short get_buffer_size();
	};

	class read final
	{

	public:

		read(char* data, const int size);

	private:

		std::vector<char> data;

		unsigned short read_position;

	public:

		char get_flag();

	public:

		char get_int8();

		short get_int16();

		std::string get_string();

	public:

		char* get_buffer();

		unsigned short get_buffer_size();
	};
};

#endif
