#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "engine_interface.h"

namespace Chess
{
    class Engine final : public EngineInterface
    {

    public:

        Engine();

        Engine(const Engine& engine) = default;
        Engine(Engine&& engine)      = default;

        Engine& operator=(const Engine& engine) = default;
        Engine& operator=(Engine&& engine)      = default;

        ~Engine() = default;

    protected:

    };
}

#endif
