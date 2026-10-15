/*
** EPITECH PROJECT, 2026
** myftp
** File description:
** FactoryTemplate
*/

#ifndef FACTORYTEMPLATE_HPP
#define FACTORYTEMPLATE_HPP

#include <concepts>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace designPattern {
template <typename BaseClass, typename ConcreteClass, typename... Args>
concept HasCreateMethod = requires(Args... args)
{
    {
        ConcreteClass::create(args...)
    } -> std::same_as<std::unique_ptr<BaseClass>>;
};

template <typename BaseClass, typename KeyType, typename... Args>
class FactoryTemplate {
public:
    using CreatorCallable = std::function<std::unique_ptr<BaseClass>(Args...)>;
    using CreatorMap      = std::unordered_map<KeyType,
        std::function<std::unique_ptr<BaseClass>(Args...)>>;

    FactoryTemplate() = default;

    ~FactoryTemplate() = default;

    FactoryTemplate(const FactoryTemplate &) = delete;

    FactoryTemplate(const FactoryTemplate &&) = delete;

    FactoryTemplate &operator=(const FactoryTemplate &) = delete;

    FactoryTemplate &operator=(const FactoryTemplate &&) = delete;

    template <typename ConcreteClass>
        requires HasCreateMethod<BaseClass, ConcreteClass, Args...>
    void registerCreator(KeyType key)
    {
        _creators[key] = &ConcreteClass::create;
    }

    void unregisterCreator(KeyType key)
    {
        _creators.erase(key);
    }

    std::unique_ptr<BaseClass> create(const KeyType &key, Args... args)
    {
        const auto itt = _creators.find(key);

        if (itt != _creators.end())
            return (*itt).second(args...);

        std::stringstream sstream;
        sstream << "No creator found for key: " << key;
        throw std::runtime_error(sstream.str());
    }

    [[nodiscard]] bool hasKey(const KeyType &key) const noexcept
    {
        return _creators.contains(key);
    }

private:
    CreatorMap _creators{};
};
} // namespace designPattern

#endif // ZAPPY_FACTORYTEMPLATE_HPP
