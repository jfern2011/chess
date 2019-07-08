#include <cstdlib>
#include <ctime>
#include <iostream>
#include <unistd.h>

#include "abort/abort.h"
#include "src/chess4.h"
#include "src/Search4.h"

int main(int argc, char** argv)
{
    Chess::Handle<std::ostream>
            stream(new std::ostream(std::cout.rdbuf()));

    Chess::Handle<Chess::Position>
        pos(new Chess::Position(stream/*, "r6k/6pp/7N/8/8/1Q6/8/7K w - - 0 1"*/));

    // 8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1
    // r2q2rk/pb3p1p/2n4Q/5N2/8/8/PP3PPP/R3R1K1 w - - 0 1

    Chess::Search4::duration_t dur(1000000000);

    Chess::Search4 search;
    AbortIfNot(search.init(pos), EXIT_FAILURE);

    std::clock_t begin = clock();
    search.run(90000, dur);
    std::clock_t end = clock();

    double elapsed = double(end - begin) / CLOCKS_PER_SEC;

    std::cout << "Finished in " << elapsed << " seconds."
        << std::endl;

    return EXIT_SUCCESS;
}
