#pragma once
#include <string>
#define RENDERING_SERVER_ADDR_ENVNAME RENDERING_SERVER_ADDR

template<typename T>
std::string encodePointer(T* ptr)
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(ptr);
    return ss.str();
}

template<typename T>
T* decodePointer(const std::string& encodedString)
{
    std::stringstream ss(encodedString);
    uintptr_t address;
    ss >> std::hex >> address;
    return reinterpret_cast<T*>(address);
}

void setEnvValue(const std::string& key, const std::string& value)
{
    _putenv_s(key.c_str(), value.c_str());
}

std::string getEnvValue(const std::string& key)
{
    const char* value = getenv(key.c_str());
    return std::string(value);
}
