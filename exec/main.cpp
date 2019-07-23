#include <unistd.h>
#include <cstdlib>
#include <memory>
#include <fstream>
#include <iostream>
#include <string>

#include "CommandLine/CommandLine.h"
#include "src/command_interface.h"
#include "src/engine.h"
#include "src/FdInputHandler.h"
#include "src/Position4.h"
#include "src/Search4.h"
#include "src/uci.h"
#include "util/util.h"

#include "abort/abort.h"

bool init_options(CommandLineOptions& options)
{
    AbortIfNot(options.add<std::string>("logfile", "",
        "Log filename, including path (defaults to standard output)"),
            false);

    AbortIfNot(options.add<bool>("help", false,
        "Print this help message"), false);

    return true;
}

bool go(int argc, char** argv)
{
    CommandLineOptions opts;
    AbortIfNot(init_options(opts),
        false);

    CommandLine cmd(opts);
    AbortIfNot(cmd.parse(argc, argv), false);

    bool help = false;
    AbortIfNot(cmd.get("help", help), false);

    if (help)
    {
        opts.print(argv[0]);
        return true;
    }

    std::string logfile;
    AbortIfNot(cmd.get("logfile", logfile), false);

    std::shared_ptr<std::ostream> stream;
    if (logfile.empty())
    {
        stream = std::make_shared<std::ostream>(
            std::cout.rdbuf());
        AbortIfNot(stream, false);
    }
    else
    {
        AbortIf( Util::is_file( logfile ), false,
            "File exists: %s", logfile.c_str());

        auto outfile = std::make_shared<std::ofstream>(
            logfile);
        AbortIfNot( outfile, false);
        AbortIfNot( outfile->is_open() , false );
        stream = outfile;
    }

    std::shared_ptr<Chess::EngineInterface>
        engine(new Chess::Engine());
    AbortIfNot(engine, false);

    engine->m_master =
        std::make_shared<Chess::Position>(
            stream);
    AbortIfNot(engine->m_master, false);

    engine->m_search =
        std::make_shared< Chess::Search4 >();
    AbortIfNot(engine->m_search, false);

    std::shared_ptr< Chess::Protocol >
        protocol(new Chess::UCI());
    AbortIfNot(protocol, false);

    AbortIfNot(protocol->install(engine),
        false);

    auto cmd_interface =
        std::make_shared<Chess::CommandInterface>();
    AbortIfNot(cmd_interface, false);

    AbortIfNot(protocol->init(cmd_interface),
        false);

    // Note: STDIN_FILENO for Linux only

    Chess::FdInputHandler handler(STDIN_FILENO);
    handler.input_signal = std::bind(
        &Chess::CommandInterface::process, *cmd_interface,
            std::placeholders::_1);

    while (true)
    {
        AbortIfNot(handler.poll(1000),
            false);
    }

    return true;
}

int main(int argc, char** argv)
{
    AbortIfNot(go(argc, argv), EXIT_FAILURE);
    return EXIT_SUCCESS;
}
