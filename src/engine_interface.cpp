#include "engine_interface.h"

namespace Chess
{
    EngineInterface::EngineInterface()
        : m_master(nullptr),
          m_search(nullptr)
    {
    }

    EngineInterface::~EngineInterface()
        = default;
}
