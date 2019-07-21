#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits> 
#include <unistd.h>

#include "abort/abort.h"
#include "src/chess4.h"
#include "src/Search4.h"

#include "src/command_interface.h"
class Blah
{
public:

    bool func(const std::string& str)
    {
        std::cout << "str = '" << str << "'" << std::endl;
        return true;
    }
};

int main(int argc, char** argv)
{
    Chess::Handle<std::ostream>
            stream(new std::ostream(std::cout.rdbuf()));

    Chess::Handle<Chess::Position>
        pos(new Chess::Position(stream, "r6k/5p1p/p1pp2r1/1pb5/4P3/2NP1P2/PPP3PK/4RR2 b - - 0 1"));

    // q1r4k/6pp/7N/8/8/1Q6/6PP/7K w - - 0 1
    // 8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1
    // r2q2rk/pb3p1p/2n4Q/5N2/8/8/PP3PPP/R3R1K1 w - - 0 1

    Chess::Search4::duration_t dur = std::chrono::seconds(2);

    Blah b;
    Chess::CommandInterface cmd;
    cmd.install("blah", std::bind(&Blah::func, &b, std::placeholders::_1));
    cmd.process("blAh bluh bleh");

    Chess::Search4 search;
    AbortIfNot(search.init(pos), EXIT_FAILURE);
    search.setNumberOfLines(4);

    std::clock_t begin = clock();
    const Chess::int16 score = search.run(90000, dur,
                                          std::numeric_limits<Chess::uint64>::max(),
                                          false);
    std::clock_t end = clock();

    double elapsed = double(end - begin) / CLOCKS_PER_SEC;

    auto stats = search.get_stats();

    std::cout << "Score   = " << score << "\n";
    std::cout << "Nodes   = " << stats.node_count  << "\n";
    std::cout << "Lnodes  = " << stats.lnode_count << "\n";
    std::cout << "Quiesce = " << stats.qnode_count << "\n";

    std::cout << "Finished in " << elapsed << " seconds."
        << std::endl;

    return EXIT_SUCCESS;
}
