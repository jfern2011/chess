/**
 *  \file   engine_interface.h
 *  \author Jason Fernandez
 *  \date   01/08/2023
 */

#ifndef CHESS_ENGINE_INTERFACE_H_
#define CHESS_ENGINE_INTERFACE_H_

#include <string>
#include <vector>

namespace chess {
/**
 * @brief Interface used by a UciProtocol to make requests to the engine
 *
 * @see https://www.shredderchess.com/download.html
 */
class EngineInterface {
public:
    ~EngineInterface() = default;

    virtual void Uci() noexcept = 0;
    virtual void DebugMode(bool enable) noexcept = 0;
    virtual bool IsReady() const noexcept = 0;
    virtual bool SetOption(const std::string& name,
                           const std::vector<std::string>& args) noexcept = 0;
    virtual void UciNewGame() noexcept = 0;
    virtual bool Position (const std::vector<std::string>& args) noexcept = 0;
    virtual void Go() noexcept = 0;
    virtual void Stop() noexcept = 0;
    virtual void PonderHit() noexcept = 0;
};

}  // namespace chess

#endif  // CHESS_ENGINE_INTERFACE_H_
