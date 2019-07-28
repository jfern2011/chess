#ifndef __UCI_PROTOCOL_H__
#define __UCI_PROTOCOL_H__

#include <functional>

#include "protocol.h"

#include "abort/abort.h"
#include "util/str_util.h"

namespace Chess
{
    class UCI final : public Protocol
    {

    public:

        explicit UCI(std::shared_ptr< std::ostream >
                     stream);

        UCI(const UCI& uci) = default;
        UCI(UCI&& uci)      = default;

        UCI& operator=(const UCI& uci) = default;
        UCI& operator=(UCI&& uci)      = default;

        ~UCI() = default;

        bool cmd_debug     (const std::string& enable);
        bool cmd_isready   (const std::string& );
        bool cmd_position  (const std::string& args);
        bool cmd_quit      (const std::string& );
        bool cmd_stop      (const std::string& );
        bool cmd_uci       (const std::string& );
        bool cmd_ucinewgame(const std::string& );

        bool init(std::shared_ptr<CommandInterface>
                  cmd) override;

    private:

        bool m_debug;
    };

    class OptionBase
    {

    public:

        OptionBase(const std::string& name,
                   const std::string& type);

        OptionBase(const OptionBase& option) = default;
        OptionBase(OptionBase&& option)      = default;

        OptionBase& operator=(const OptionBase& option) = default;
        OptionBase& operator=(OptionBase&& option)      = default;

        ~OptionBase() = default;

        std::string name() const;
        std::string type() const;

        virtual bool update( const std::string& value )
            = 0;

    protected:

        /**
         * The name of this option
         */
        std::string m_name;

        /**
         * The option's UCI type
         */
        std::string m_type;
    };

    template <typename T>
    class Option final : public OptionBase
    {

    public:

        using updater_t = std::function<bool(const T&)>;

        Option(const std::string& name,
               const std::string& type);

        Option(const Option& option) = default;
        Option(Option&& option)      = default;

        Option& operator=(const Option& option) = default;
        Option& operator=(Option&& option)      = default;

        ~Option() = default;

        bool update( const std::string& value )
            override;

        updater_t m_updater;

    private:

        T m_value;
    };

    template <typename T>
    Option<T>::Option(const std::string& name,
                      const std::string& type)
        : OptionBase(name, type)
    {
    }

    template <typename T>
    bool Option<T>::update(const std::string& value)
    {
        AbortIfNot(m_updater, false);
        
        AbortIfNot(Util::from_string<T>(
            value, m_value) , false);

        AbortIfNot(m_updater( m_value ),
            false);

        return true;
    }
}

#endif
