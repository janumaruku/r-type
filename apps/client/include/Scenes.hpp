/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Scenes.hpp
*/

#ifndef R_TYPE_SCENES_HPP
    #define R_TYPE_SCENES_HPP
    #include <cstdint>
    #include <memory>
    // #include "Actions.hpp"
    // #include "ClientConfig.hpp"
    // #include "ClientSettings.hpp"
    #include "engine/render/include/RenderQueue.hpp"
    

namespace rtype::client {

    struct SceneRequest {
        enum class Kind : std::uint8_t { STAY, CHANGE, QUIT };
        Kind kind = Kind::STAY;
        std::unique_ptr<IScene> next;

        [[nodiscard]] static SceneRequest stay();
        [[nodiscard]] static SceneRequest change(std::unique_ptr<IScene> next);
        [[nodiscard]] static SceneRequest quit();
    };

    class IScene {
    public:
        virtual ~IScene() = default;
        virtual void onEnter(FrameContext &context) = 0;
        virtual void onExit(FrameContext &context) = 0;
        [[nodiscard]] virtual SceneRequest update(FrameContext &context, float dt) = 0;
        virtual void draw(engine::render::RenderQueue &queue, const FrameContext &context) = 0;
    };

    class SceneManager {
    public:
        void change(std::unique_ptr<IScene> next, FrameContext &context);
        [[nodiscard]] bool update(FrameContext &context, float dt);
        void draw(engine::render::RenderQueue &queue, const FrameContext &context);
        [[nodiscard]] IScene *current() noexcept;

    private:
        std::unique_ptr<IScene> _current;
    };

    class MessageScene : public IScene {
    public:
        MessageScene(std::string title, std::string detail, bool canReconnect);
        // the 4 IScene functions
    };

}

#endif
