#include "end_point.h"

end_point::end_point(std::string address_str, unsigned short port)
{
	set_address(address_str);
	set_port(port);
}

end_point::end_point(unsigned long address, unsigned short port)
{
	set_address(address);
	set_port(port);
}

void end_point::set_address(unsigned long address)
{
	this->address = address;
}

void end_point::set_port(unsigned short port)
{
	this->port = port;
}

bool end_point::set_address(std::string address_str)
{
	unsigned long octet_1, octet_2, octet_3, octet_4;

	bool result = (sscanf_s(address_str.c_str(), "%u.%u.%u.%u", &octet_1, &octet_2, &octet_3, &octet_4) == 4);

	if (result == false) return false;

	this->address =
		(static_cast<unsigned long>(octet_1) << 24) |
		(static_cast<unsigned long>(octet_2) << 16) |
		(static_cast<unsigned long>(octet_3) << 8) |
		(static_cast<unsigned long>(octet_4) << 0);

	return true;
}

bool end_point::set_port(std::string port_str)
{
	this->port = std::stoi(port_str);

	return true;
}

unsigned long end_point::get_address() const
{
	return address;
}

unsigned short end_point::get_port() const
{
	return port;
}

std::string end_point::get_address_str() const
{
	std::string result = std::string();

	result += std::to_string((address >> 24) & 0xFF) + '.';
	result += std::to_string((address >> 16) & 0xFF) + '.';
	result += std::to_string((address >> 8) & 0xFF) + '.';
	result += std::to_string((address >> 0) & 0xFF);

	return result;
}

std::string end_point::get_port_str() const
{
	std::string result = std::string();

	result = std::to_string(port);

	return result;
}

void end_point::reset()
{
	set_address(0);
	set_port(0);
}

bool end_point::operator==(const end_point& other)
{
	return (other.address == this->address && other.port == this->port);
}

std::size_t end_point::operator()(const end_point& ep) const
{
	return ((static_cast<std::size_t>(address)) << 32) | ((static_cast<std::size_t>(port)) << 16);
}
