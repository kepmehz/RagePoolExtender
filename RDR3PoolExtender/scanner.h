#pragma once

class scanner
{
private:
	uintptr_t module_address;
	static std::vector<int> convert_pattern_to_byte(const char* pattern);
public:
	void* scan(const char* pattern);

	scanner(const char* module_name);
};