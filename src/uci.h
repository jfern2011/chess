#ifndef __UCI_PROTOCOL_H__
#define __UCI_PROTOCOL_H__

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "protocol.h"

#include "abort/abort.h"
#include "util/str_util.h"

namespace Chess
{
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

        std::string print() const;

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

        /**
         * What \ref print() returns
         */
        std::string m_uciOutput;
    };

    template <typename T>
    class Option : public OptionBase
    {

    public:

        using updater_t = std::function<bool(const T&)>;

        Option(const std::string& name,
               const std::string& type);

        Option(const Option& option) = default;
        Option(Option&& option)      = default;

        Option& operator=(const Option& option) = default;
        Option& operator=(Option&& option)      = default;

        virtual ~Option() = default;

        bool update( const std::string& value )
            final;

        updater_t m_updater;

    protected:

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

    class Check final : public Option<bool>
    {

    public:

        Check(const std::string& name, bool initValue);

        Check(const Check& check) = default;
        Check(Check&& check)      = default;

        Check& operator=(const Check& check) = default;
        Check& operator=(Check&& check)      = default;

        ~Check() = default;

    private:

        bool m_default;
    };

    class Spin final : public Option<int64>
    {

    public:

        Spin(const std::string& name,
             int64 initValue, int64 min, int64 max);

        Spin(const Spin& spin) = default;
        Spin(Spin&& spin)      = default;

        Spin& operator=(const Spin& spin) = default;
        Spin& operator=(Spin&& spin)      = default;

        ~Spin() = default;

    private:

        int64 m_default;

        int64 m_max;

        int64 m_min;
    };

    class Combo final : public Option< std::string >
    {

    public:

        Combo(const std::string& name,
              const std::string& initValue);

        template <typename... T>
        Combo(const std::string& name,
              const std::string& initValue,
              const T&... values);

        Combo(const Combo& combo) = default;
        Combo(Combo&& combo)      = default;

        Combo& operator=(const Combo& combo) = default;
        Combo& operator=(Combo&& combo)      = default;

        ~Combo() = default;

    private:

        std::string m_default;

        std::vector<std::string>
            m_options;
    };

    template <typename... T>
    Combo::Combo(const std::string& name,
                 const std::string& initValue,
                 const T&... values)
        : Combo(name, values...)
    {
        m_options.push_back(initValue);
        m_default = initValue;

        m_uciOutput = " name "    + m_name +
                      " type "    + m_type +
                      " default " + m_default;

        for (auto& var : m_options)
            m_uciOutput += " var " + var;
    }

    class Button final : public OptionBase
    {

    public:

        using updater_t = std::function<bool()>;

        Button(const std::string& name);

        Button(const Button& button) = default;
        Button(Button&& button)      = default;

        Button& operator=(const Button& button) = default;
        Button& operator=(Button&& button)      = default;

        ~Button() = default;

        bool update( const std::string&  )
            override;

        updater_t m_updater;
    };

    class String final : public Option< std::string >
    {

    public:

        String(const std::string& name,
               const std::string& initValue);

        String(const String& string) = default;
        String(String&& string)      = default;

        String& operator=(const String& string) = default;
        String& operator=(String&& string)      = default;

        ~String() = default;

    private:

        std::string m_default;
    };

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

        bool initOptions();

        bool m_debug;

        std::map<std::string, std::shared_ptr<OptionBase>>
            m_options;
    };
}

#endif
