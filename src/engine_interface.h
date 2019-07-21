#ifndef __ENGINE_INTERFACE_H__
#define __ENGINE_INTERFACE_H__

#include <memory>

#include "Position4.h"
#include "Search4.h"

namespace Chess
{
    class EngineInterface
    {

    public:

        EngineInterface();

        EngineInterface(const EngineInterface& engine) = default;
        EngineInterface(EngineInterface&& engine)      = default;

        EngineInterface& operator=(const EngineInterface& engine) = default;
        EngineInterface& operator=(EngineInterface&& engine)      = default;

        virtual ~EngineInterface() = 0;

        std::shared_ptr<Position>
            m_master;

        std::shared_ptr<Search4 >
            m_search;
    };
}

#endif
