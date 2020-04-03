/**
 *  \file   data_tables.cc
 *  \author Jason Fernandez
 *  \date   11/02/2019
 */

#include "chess/data_tables.h"

namespace chess {
namespace data_tables {

/**
 * Runtime-intialized tables
 *
 * @{
 */

std::array<std::uint64_t,internal::kAttacksDiagDbSize> bishop_attacks;
std::array<int,internal::kAttacksDiagDbSize>           bishop_mobility;
std::array<std::uint64_t,internal::kAttacksRookDbSize> rook_attacks;
std::array<int,internal::kAttacksRookDbSize>           rook_mobility;

/**
 * @}
 */

namespace internal {

/**
 * Initialize tables that take too long to create as compile-time constants
 */
class Initializer final {
public:
    Initializer();

    Initializer(const Initializer& init)            = default;
    Initializer(Initializer&& init)                 = default;
    Initializer& operator=(const Initializer& init) = default;
    Initializer& operator=(Initializer&& init)      = default;
    ~Initializer()                                  = default;
};

/**
 * Initialize tables not created during compile time
 */
Initializer::Initializer() {
    for (std::uint32_t i = 0; i < internal::kAttacksDiagDbSize; i++) {
        bishop_attacks[i]  = internal::InitAttacksFromDiag(i);
        bishop_mobility[i] = internal::InitMobilityDiag(i);
    }

    for (std::uint32_t i = 0; i < internal::kAttacksRookDbSize; i++) {
        rook_attacks[i]  = internal::InitAttacksFromRook(i);
        rook_mobility[i] = internal::InitMobilityRook(i);
    }
}

/**
 * Static instantiation to initialize tables not created at ccompile-time
 */
Initializer initializer;

}  // namespace internal
}  // namespace data_tables
}  // namespace chess
