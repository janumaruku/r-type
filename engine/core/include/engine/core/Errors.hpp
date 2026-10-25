/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Errors.hpp
*/

#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <stdexcept>

namespace engine::core {

class EngineError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class RessourceError : public EngineError {
public:
    using EngineError::EngineError;
};

class ConfigError : public EngineError {
public:
    using EngineError::EngineError;
};

class NetworkError : public EngineError {
public:
    using EngineError::EngineError;
};


}

#endif