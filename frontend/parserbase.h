// Generated by Bisonc++ V4.05.00 on Thu, 05 Jun 2014 10:35:51 -0700

#ifndef streamParserBase_h_included
#define streamParserBase_h_included

#include <exception>
#include <vector>
#include <iostream>

// $insert debugincludes
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <iomanip>

namespace // anonymous
{
    struct PI__;
}

// $insert namespace-open
namespace stream
{


class ParserBase
{
    public:
// $insert tokens

    // Symbolic tokens:
    enum Tokens__
    {
        SCANNER_ERROR = 257,
        INT,
        REAL,
        ID,
        FOR,
        EACH,
        TAKES,
        EVERY,
        IN,
        EQ,
        NEQ,
        LESS,
        MORE,
        DOTDOT,
    };

// $insert STYPE
typedef int STYPE__;

    private:
        int d_stackIdx__;
        std::vector<size_t>   d_stateStack__;
        std::vector<STYPE__>  d_valueStack__;

    protected:
        enum Return__
        {
            PARSE_ACCEPT__ = 0,   // values used as parse()'s return values
            PARSE_ABORT__  = 1
        };
        enum ErrorRecovery__
        {
            DEFAULT_RECOVERY_MODE__,
            UNEXPECTED_TOKEN__,
        };
        bool        d_debug__;
        size_t      d_nErrors__;
        size_t      d_requiredTokens__;
        size_t      d_acceptedTokens__;
        int         d_token__;
        int         d_nextToken__;
        size_t      d_state__;
        STYPE__    *d_vsp__;
        STYPE__     d_val__;
        STYPE__     d_nextVal__;

        ParserBase();

// $insert debugdecl
        static std::ostringstream s_out__;

        std::string symbol__(int value) const;
        std::string stype__(char const *pre, STYPE__ const &semVal,
                            char const *post = "") const;
        static std::ostream &dflush__(std::ostream &out);
        void ABORT() const;
        void ACCEPT() const;
        void ERROR() const;
        void clearin();
        bool debug() const;
        void pop__(size_t count = 1);
        void push__(size_t nextState);
        void popToken__();
        void pushToken__(int token);
        void reduce__(PI__ const &productionInfo);
        void errorVerbose__();
        size_t top__() const;

    public:
        void setDebug(bool mode);
}; 

inline bool ParserBase::debug() const
{
    return d_debug__;
}

inline void ParserBase::setDebug(bool mode)
{
    d_debug__ = mode;
}

inline void ParserBase::ABORT() const
{
    throw PARSE_ABORT__;
}

inline void ParserBase::ACCEPT() const
{
    throw PARSE_ACCEPT__;
}

inline void ParserBase::ERROR() const
{
    throw UNEXPECTED_TOKEN__;
}


// As a convenience, when including ParserBase.h its symbols are available as
// symbols in the class Parser, too.
#define Parser ParserBase

// $insert namespace-close
}

#endif


