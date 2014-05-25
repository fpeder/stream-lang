// Generated by Bisonc++ V4.05.00 on Sun, 25 May 2014 16:04:29 -0700

// $insert class.ih
#include "parser_impl.hpp"

// The FIRST element of SR arrays shown below uses `d_type', defining the
// state's type, and `d_lastIdx' containing the last element's index. If
// d_lastIdx contains the REQ_TOKEN bitflag (see below) then the state needs
// a token: if in this state d_token__ is _UNDETERMINED_, nextToken() will be
// called

// The LAST element of SR arrays uses `d_token' containing the last retrieved
// token to speed up the (linear) seach.  Except for the first element of SR
// arrays, the field `d_action' is used to determine what to do next. If
// positive, it represents the next state (used with SHIFT); if zero, it
// indicates `ACCEPT', if negative, -d_action represents the number of the
// rule to reduce to.

// `lookup()' tries to find d_token__ in the current SR array. If it fails, and
// there is no default reduction UNEXPECTED_TOKEN__ is thrown, which is then
// caught by the error-recovery function.

// The error-recovery function will pop elements off the stack until a state
// having bit flag ERR_ITEM is found. This state has a transition on _error_
// which is applied. In this _error_ state, while the current token is not a
// proper continuation, new tokens are obtained by nextToken(). If such a
// token is found, error recovery is successful and the token is
// handled according to the error state's SR table and parsing continues.
// During error recovery semantic actions are ignored.

// A state flagged with the DEF_RED flag will perform a default
// reduction if no other continuations are available for the current token.

// The ACCEPT STATE never shows a default reduction: when it is reached the
// parser returns ACCEPT(). During the grammar
// analysis phase a default reduction may have been defined, but it is
// removed during the state-definition phase.

// So:
//      s_x[] = 
//      {
//                  [_field_1_]         [_field_2_]
//
// First element:   {state-type,        idx of last element},
// Other elements:  {required token,    action to perform},
//                                      ( < 0: reduce, 
//                                          0: ACCEPT,
//                                        > 0: next state)
// Last element:    {set to d_token__,    action to perform}
//      }

// When the --thread-safe option is specified, all static data are defined as
// const. If --thread-safe is not provided, the state-tables are not defined
// as const, since the lookup() function below will modify them

// $insert debugincludes
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <iomanip>

namespace // anonymous
{
    char const author[] = "Frank B. Brokken (f.b.brokken@rug.nl)";

    enum 
    {
        STACK_EXPANSION = 5     // size to expand the state-stack with when
                                // full
    };

    enum ReservedTokens
    {
        PARSE_ACCEPT     = 0,   // `ACCEPT' TRANSITION
        _UNDETERMINED_   = -2,
        _EOF_            = -1,
        _error_          = 256
    };
    enum StateType       // modify statetype/data.cc when this enum changes
    {
        NORMAL,
        ERR_ITEM,
        REQ_TOKEN,
        ERR_REQ,    // ERR_ITEM | REQ_TOKEN
        DEF_RED,    // state having default reduction
        ERR_DEF,    // ERR_ITEM | DEF_RED
        REQ_DEF,    // REQ_TOKEN | DEF_RED
        ERR_REQ_DEF // ERR_ITEM | REQ_TOKEN | DEF_RED
    };    
    struct PI__     // Production Info
    {
        size_t d_nonTerm; // identification number of this production's
                            // non-terminal 
        size_t d_size;    // number of elements in this production 
    };

    struct SR__     // Shift Reduce info, see its description above
    {
        union
        {
            int _field_1_;      // initializer, allowing initializations 
                                // of the SR s_[] arrays
            int d_type;
            int d_token;
        };
        union
        {
            int _field_2_;

            int d_lastIdx;          // if negative, the state uses SHIFT
            int d_action;           // may be negative (reduce), 
                                    // postive (shift), or 0 (accept)
            size_t d_errorState;    // used with Error states
        };
    };

    // $insert staticdata
    
// Productions Info Records:
PI__ const s_productionInfo[] = 
{
     {0, 0}, // not used: reduction values are negative
     {273, 1}, // 1: program ->  func_def_list
     {274, 0}, // 2: func_def_list ->  <empty>
     {274, 1}, // 3: func_def_list ->  func_def
     {274, 3}, // 4: func_def_list (';') ->  func_def ';' func_def_list
     {275, 4}, // 5: func_def (ID) ->  ID func_param_spec '=' func_body
     {276, 0}, // 6: func_param_spec ->  <empty>
     {276, 3}, // 7: func_param_spec ('(') ->  '(' func_param_list ')'
     {278, 1}, // 8: func_param_list (ID) ->  ID
     {278, 3}, // 9: func_param_list (ID) ->  ID ',' func_param_list
     {277, 1}, // 10: func_body ->  expr
     {277, 1}, // 11: func_body ->  for_stmt
     {279, 3}, // 12: expr (EQ) ->  expr EQ expr
     {279, 3}, // 13: expr (NEQ) ->  expr NEQ expr
     {279, 3}, // 14: expr (LESS) ->  expr LESS expr
     {279, 3}, // 15: expr (MORE) ->  expr MORE expr
     {279, 3}, // 16: expr ('+') ->  expr '+' expr
     {279, 3}, // 17: expr ('-') ->  expr '-' expr
     {279, 3}, // 18: expr ('*') ->  expr '*' expr
     {279, 3}, // 19: expr ('/') ->  expr '/' expr
     {279, 3}, // 20: expr ('(') ->  '(' expr ')'
     {279, 1}, // 21: expr ->  atom
     {281, 1}, // 22: atom (INT) ->  INT
     {281, 1}, // 23: atom (REAL) ->  REAL
     {281, 1}, // 24: atom (ID) ->  ID
     {280, 7}, // 25: for_stmt (FOR) ->  FOR '(' for_spec ')' '{' for_body '}'
     {282, 2}, // 26: for_spec ->  for_iter_spec for_domain_spec
     {284, 2}, // 27: for_iter_spec (EACH) ->  EACH ID
     {284, 4}, // 28: for_iter_spec (INT) ->  INT ID for_iter_hop for_iter_offset
     {286, 0}, // 29: for_iter_hop ->  <empty>
     {286, 2}, // 30: for_iter_hop (EVERY) ->  EVERY INT
     {287, 0}, // 31: for_iter_offset ->  <empty>
     {287, 2}, // 32: for_iter_offset (AT) ->  AT INT
     {285, 0}, // 33: for_domain_spec ->  <empty>
     {285, 2}, // 34: for_domain_spec (IN) ->  IN for_domain_list
     {288, 1}, // 35: for_domain_list (ID) ->  ID
     {288, 3}, // 36: for_domain_list (ID) ->  ID ',' for_domain_list
     {283, 1}, // 37: for_body ->  expr
     {283, 1}, // 38: for_body ->  for_stmt
     {289, 1}, // 39: program_$ ->  program
};

// State info and SR__ transitions for each state.


SR__ s_0[] =
{
    { { REQ_DEF}, {  5} },                 
    { {     273}, {  1} }, // program      
    { {     274}, {  2} }, // func_def_list
    { {     275}, {  3} }, // func_def     
    { {     260}, {  4} }, // ID           
    { {       0}, { -2} },                 
};

SR__ s_1[] =
{
    { { REQ_TOKEN}, {            2} }, 
    { {     _EOF_}, { PARSE_ACCEPT} }, 
    { {         0}, {            0} }, 
};

SR__ s_2[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -1} }, 
};

SR__ s_3[] =
{
    { { REQ_DEF}, {  2} },       
    { {      59}, {  5} }, // ';'
    { {       0}, { -3} },       
};

SR__ s_4[] =
{
    { { REQ_DEF}, {  3} },                   
    { {     276}, {  6} }, // func_param_spec
    { {      40}, {  7} }, // '('            
    { {       0}, { -6} },                   
};

SR__ s_5[] =
{
    { { REQ_DEF}, {  4} },                 
    { {     274}, {  8} }, // func_def_list
    { {     275}, {  3} }, // func_def     
    { {     260}, {  4} }, // ID           
    { {       0}, { -2} },                 
};

SR__ s_6[] =
{
    { { REQ_TOKEN}, { 2} },       
    { {        61}, { 9} }, // '='
    { {         0}, { 0} },       
};

SR__ s_7[] =
{
    { { REQ_TOKEN}, {  3} },                   
    { {       278}, { 10} }, // func_param_list
    { {       260}, { 11} }, // ID             
    { {         0}, {  0} },                   
};

SR__ s_8[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -4} }, 
};

SR__ s_9[] =
{
    { { REQ_TOKEN}, { 10} },             
    { {       277}, { 12} }, // func_body
    { {       279}, { 13} }, // expr     
    { {       280}, { 14} }, // for_stmt 
    { {        40}, { 15} }, // '('      
    { {       281}, { 16} }, // atom     
    { {       261}, { 17} }, // FOR      
    { {       258}, { 18} }, // INT      
    { {       259}, { 19} }, // REAL     
    { {       260}, { 20} }, // ID       
    { {         0}, {  0} },             
};

SR__ s_10[] =
{
    { { REQ_TOKEN}, {  2} },       
    { {        41}, { 21} }, // ')'
    { {         0}, {  0} },       
};

SR__ s_11[] =
{
    { { REQ_DEF}, {  2} },       
    { {      44}, { 22} }, // ','
    { {       0}, { -8} },       
};

SR__ s_12[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -5} }, 
};

SR__ s_13[] =
{
    { { REQ_DEF}, {   9} },        
    { {     267}, {  23} }, // EQ  
    { {     268}, {  24} }, // NEQ 
    { {     269}, {  25} }, // LESS
    { {     270}, {  26} }, // MORE
    { {      43}, {  27} }, // '+' 
    { {      45}, {  28} }, // '-' 
    { {      42}, {  29} }, // '*' 
    { {      47}, {  30} }, // '/' 
    { {       0}, { -10} },        
};

SR__ s_14[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -11} }, 
};

SR__ s_15[] =
{
    { { REQ_TOKEN}, {  7} },        
    { {       279}, { 31} }, // expr
    { {        40}, { 15} }, // '(' 
    { {       281}, { 16} }, // atom
    { {       258}, { 18} }, // INT 
    { {       259}, { 19} }, // REAL
    { {       260}, { 20} }, // ID  
    { {         0}, {  0} },        
};

SR__ s_16[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -21} }, 
};

SR__ s_17[] =
{
    { { REQ_TOKEN}, {  2} },       
    { {        40}, { 32} }, // '('
    { {         0}, {  0} },       
};

SR__ s_18[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -22} }, 
};

SR__ s_19[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -23} }, 
};

SR__ s_20[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -24} }, 
};

SR__ s_21[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -7} }, 
};

SR__ s_22[] =
{
    { { REQ_TOKEN}, {  3} },                   
    { {       278}, { 33} }, // func_param_list
    { {       260}, { 11} }, // ID             
    { {         0}, {  0} },                   
};

SR__ s_23[] =
{
    { { REQ_TOKEN}, {  7} },        
    { {       279}, { 34} }, // expr
    { {        40}, { 15} }, // '(' 
    { {       281}, { 16} }, // atom
    { {       258}, { 18} }, // INT 
    { {       259}, { 19} }, // REAL
    { {       260}, { 20} }, // ID  
    { {         0}, {  0} },        
};

SR__ s_24[] =
{
    { { REQ_TOKEN}, {  7} },        
    { {       279}, { 35} }, // expr
    { {        40}, { 15} }, // '(' 
    { {       281}, { 16} }, // atom
    { {       258}, { 18} }, // INT 
    { {       259}, { 19} }, // REAL
    { {       260}, { 20} }, // ID  
    { {         0}, {  0} },        
};

SR__ s_25[] =
{
    { { REQ_TOKEN}, {  7} },        
    { {       279}, { 36} }, // expr
    { {        40}, { 15} }, // '(' 
    { {       281}, { 16} }, // atom
    { {       258}, { 18} }, // INT 
    { {       259}, { 19} }, // REAL
    { {       260}, { 20} }, // ID  
    { {         0}, {  0} },        
};

SR__ s_26[] =
{
    { { REQ_TOKEN}, {  7} },        
    { {       279}, { 37} }, // expr
    { {        40}, { 15} }, // '(' 
    { {       281}, { 16} }, // atom
    { {       258}, { 18} }, // INT 
    { {       259}, { 19} }, // REAL
    { {       260}, { 20} }, // ID  
    { {         0}, {  0} },        
};

SR__ s_27[] =
{
    { { REQ_TOKEN}, {  7} },        
    { {       279}, { 38} }, // expr
    { {        40}, { 15} }, // '(' 
    { {       281}, { 16} }, // atom
    { {       258}, { 18} }, // INT 
    { {       259}, { 19} }, // REAL
    { {       260}, { 20} }, // ID  
    { {         0}, {  0} },        
};

SR__ s_28[] =
{
    { { REQ_TOKEN}, {  7} },        
    { {       279}, { 39} }, // expr
    { {        40}, { 15} }, // '(' 
    { {       281}, { 16} }, // atom
    { {       258}, { 18} }, // INT 
    { {       259}, { 19} }, // REAL
    { {       260}, { 20} }, // ID  
    { {         0}, {  0} },        
};

SR__ s_29[] =
{
    { { REQ_TOKEN}, {  7} },        
    { {       279}, { 40} }, // expr
    { {        40}, { 15} }, // '(' 
    { {       281}, { 16} }, // atom
    { {       258}, { 18} }, // INT 
    { {       259}, { 19} }, // REAL
    { {       260}, { 20} }, // ID  
    { {         0}, {  0} },        
};

SR__ s_30[] =
{
    { { REQ_TOKEN}, {  7} },        
    { {       279}, { 41} }, // expr
    { {        40}, { 15} }, // '(' 
    { {       281}, { 16} }, // atom
    { {       258}, { 18} }, // INT 
    { {       259}, { 19} }, // REAL
    { {       260}, { 20} }, // ID  
    { {         0}, {  0} },        
};

SR__ s_31[] =
{
    { { REQ_TOKEN}, { 10} },        
    { {        41}, { 42} }, // ')' 
    { {       267}, { 23} }, // EQ  
    { {       268}, { 24} }, // NEQ 
    { {       269}, { 25} }, // LESS
    { {       270}, { 26} }, // MORE
    { {        43}, { 27} }, // '+' 
    { {        45}, { 28} }, // '-' 
    { {        42}, { 29} }, // '*' 
    { {        47}, { 30} }, // '/' 
    { {         0}, {  0} },        
};

SR__ s_32[] =
{
    { { REQ_TOKEN}, {  5} },                 
    { {       282}, { 43} }, // for_spec     
    { {       284}, { 44} }, // for_iter_spec
    { {       262}, { 45} }, // EACH         
    { {       258}, { 46} }, // INT          
    { {         0}, {  0} },                 
};

SR__ s_33[] =
{
    { { DEF_RED}, {  1} }, 
    { {       0}, { -9} }, 
};

SR__ s_34[] =
{
    { { REQ_DEF}, {   5} },       
    { {      43}, {  27} }, // '+'
    { {      45}, {  28} }, // '-'
    { {      42}, {  29} }, // '*'
    { {      47}, {  30} }, // '/'
    { {       0}, { -12} },       
};

SR__ s_35[] =
{
    { { REQ_DEF}, {   5} },       
    { {      43}, {  27} }, // '+'
    { {      45}, {  28} }, // '-'
    { {      42}, {  29} }, // '*'
    { {      47}, {  30} }, // '/'
    { {       0}, { -13} },       
};

SR__ s_36[] =
{
    { { REQ_DEF}, {   5} },       
    { {      43}, {  27} }, // '+'
    { {      45}, {  28} }, // '-'
    { {      42}, {  29} }, // '*'
    { {      47}, {  30} }, // '/'
    { {       0}, { -14} },       
};

SR__ s_37[] =
{
    { { REQ_DEF}, {   5} },       
    { {      43}, {  27} }, // '+'
    { {      45}, {  28} }, // '-'
    { {      42}, {  29} }, // '*'
    { {      47}, {  30} }, // '/'
    { {       0}, { -15} },       
};

SR__ s_38[] =
{
    { { REQ_DEF}, {   3} },       
    { {      42}, {  29} }, // '*'
    { {      47}, {  30} }, // '/'
    { {       0}, { -16} },       
};

SR__ s_39[] =
{
    { { REQ_DEF}, {   3} },       
    { {      42}, {  29} }, // '*'
    { {      47}, {  30} }, // '/'
    { {       0}, { -17} },       
};

SR__ s_40[] =
{
    { { REQ_DEF}, {   1} }, 
    { {       0}, { -18} }, 
};

SR__ s_41[] =
{
    { { REQ_DEF}, {   1} }, 
    { {       0}, { -19} }, 
};

SR__ s_42[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -20} }, 
};

SR__ s_43[] =
{
    { { REQ_TOKEN}, {  2} },       
    { {        41}, { 47} }, // ')'
    { {         0}, {  0} },       
};

SR__ s_44[] =
{
    { { REQ_DEF}, {   3} },                   
    { {     285}, {  48} }, // for_domain_spec
    { {     265}, {  49} }, // IN             
    { {       0}, { -33} },                   
};

SR__ s_45[] =
{
    { { REQ_TOKEN}, {  2} },      
    { {       260}, { 50} }, // ID
    { {         0}, {  0} },      
};

SR__ s_46[] =
{
    { { REQ_TOKEN}, {  2} },      
    { {       260}, { 51} }, // ID
    { {         0}, {  0} },      
};

SR__ s_47[] =
{
    { { REQ_TOKEN}, {  2} },       
    { {       123}, { 52} }, // '{'
    { {         0}, {  0} },       
};

SR__ s_48[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -26} }, 
};

SR__ s_49[] =
{
    { { REQ_TOKEN}, {  3} },                   
    { {       288}, { 53} }, // for_domain_list
    { {       260}, { 54} }, // ID             
    { {         0}, {  0} },                   
};

SR__ s_50[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -27} }, 
};

SR__ s_51[] =
{
    { { REQ_DEF}, {   3} },                
    { {     286}, {  55} }, // for_iter_hop
    { {     263}, {  56} }, // EVERY       
    { {       0}, { -29} },                
};

SR__ s_52[] =
{
    { { REQ_TOKEN}, { 10} },            
    { {       283}, { 57} }, // for_body
    { {       279}, { 58} }, // expr    
    { {       280}, { 59} }, // for_stmt
    { {        40}, { 15} }, // '('     
    { {       281}, { 16} }, // atom    
    { {       261}, { 17} }, // FOR     
    { {       258}, { 18} }, // INT     
    { {       259}, { 19} }, // REAL    
    { {       260}, { 20} }, // ID      
    { {         0}, {  0} },            
};

SR__ s_53[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -34} }, 
};

SR__ s_54[] =
{
    { { REQ_DEF}, {   2} },       
    { {      44}, {  60} }, // ','
    { {       0}, { -35} },       
};

SR__ s_55[] =
{
    { { REQ_DEF}, {   3} },                   
    { {     287}, {  61} }, // for_iter_offset
    { {     264}, {  62} }, // AT             
    { {       0}, { -31} },                   
};

SR__ s_56[] =
{
    { { REQ_TOKEN}, {  2} },       
    { {       258}, { 63} }, // INT
    { {         0}, {  0} },       
};

SR__ s_57[] =
{
    { { REQ_TOKEN}, {  2} },       
    { {       125}, { 64} }, // '}'
    { {         0}, {  0} },       
};

SR__ s_58[] =
{
    { { REQ_DEF}, {   9} },        
    { {     267}, {  23} }, // EQ  
    { {     268}, {  24} }, // NEQ 
    { {     269}, {  25} }, // LESS
    { {     270}, {  26} }, // MORE
    { {      43}, {  27} }, // '+' 
    { {      45}, {  28} }, // '-' 
    { {      42}, {  29} }, // '*' 
    { {      47}, {  30} }, // '/' 
    { {       0}, { -37} },        
};

SR__ s_59[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -38} }, 
};

SR__ s_60[] =
{
    { { REQ_TOKEN}, {  3} },                   
    { {       288}, { 65} }, // for_domain_list
    { {       260}, { 54} }, // ID             
    { {         0}, {  0} },                   
};

SR__ s_61[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -28} }, 
};

SR__ s_62[] =
{
    { { REQ_TOKEN}, {  2} },       
    { {       258}, { 66} }, // INT
    { {         0}, {  0} },       
};

SR__ s_63[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -30} }, 
};

SR__ s_64[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -25} }, 
};

SR__ s_65[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -36} }, 
};

SR__ s_66[] =
{
    { { DEF_RED}, {   1} }, 
    { {       0}, { -32} }, 
};


// State array:
SR__ *s_state[] =
{
  s_0,  s_1,  s_2,  s_3,  s_4,  s_5,  s_6,  s_7,  s_8,  s_9,
  s_10,  s_11,  s_12,  s_13,  s_14,  s_15,  s_16,  s_17,  s_18,  s_19,
  s_20,  s_21,  s_22,  s_23,  s_24,  s_25,  s_26,  s_27,  s_28,  s_29,
  s_30,  s_31,  s_32,  s_33,  s_34,  s_35,  s_36,  s_37,  s_38,  s_39,
  s_40,  s_41,  s_42,  s_43,  s_44,  s_45,  s_46,  s_47,  s_48,  s_49,
  s_50,  s_51,  s_52,  s_53,  s_54,  s_55,  s_56,  s_57,  s_58,  s_59,
  s_60,  s_61,  s_62,  s_63,  s_64,  s_65,  s_66,
};

typedef std::unordered_map<int, char const *> SMap;
typedef SMap::value_type SMapVal;

SMapVal s_symArr[] =
{
    SMapVal(-2, "_UNDETERMINED_"),  // predefined symbols
    SMapVal(-1, "_EOF_"),
    SMapVal(256, "_error_"),

    SMapVal(257, "SCANNER_ERROR"),
    SMapVal(258, "INT"),
    SMapVal(259, "REAL"),
    SMapVal(260, "ID"),
    SMapVal(261, "FOR"),
    SMapVal(262, "EACH"),
    SMapVal(263, "EVERY"),
    SMapVal(264, "AT"),
    SMapVal(265, "IN"),
    SMapVal(266, "COMPARE"),
    SMapVal(267, "EQ"),
    SMapVal(268, "NEQ"),
    SMapVal(269, "LESS"),
    SMapVal(270, "MORE"),
    SMapVal(271, "MATH1"),
    SMapVal(272, "MATH2"),
    SMapVal(273, "program"),
    SMapVal(274, "func_def_list"),
    SMapVal(275, "func_def"),
    SMapVal(276, "func_param_spec"),
    SMapVal(277, "func_body"),
    SMapVal(278, "func_param_list"),
    SMapVal(279, "expr"),
    SMapVal(280, "for_stmt"),
    SMapVal(281, "atom"),
    SMapVal(282, "for_spec"),
    SMapVal(283, "for_body"),
    SMapVal(284, "for_iter_spec"),
    SMapVal(285, "for_domain_spec"),
    SMapVal(286, "for_iter_hop"),
    SMapVal(287, "for_iter_offset"),
    SMapVal(288, "for_domain_list"),
    SMapVal(289, "program_$"),
};

SMap s_symbol
(
    s_symArr, s_symArr + sizeof(s_symArr) / sizeof(SMapVal)
);

} // anonymous namespace ends


// $insert namespace-open
namespace stream
{

// If the parsing function call uses arguments, then provide an overloaded
// function.  The code below doesn't rely on parameters, so no arguments are
// required.  Furthermore, parse uses a function try block to allow us to do
// ACCEPT and ABORT from anywhere, even from within members called by actions,
// simply throwing the appropriate exceptions.

ParserBase::ParserBase()
:
    d_stackIdx__(-1),
    // $insert debuginit
    d_debug__(false),
    d_nErrors__(0),
    // $insert requiredtokens
    d_requiredTokens__(0),
    d_acceptedTokens__(d_requiredTokens__),
    d_token__(_UNDETERMINED_),
    d_nextToken__(_UNDETERMINED_)
{}

// $insert debugfunctions
std::string ParserBase::symbol__(int value) const
{
    using namespace std;
    ostringstream ostr;
    SMap::const_iterator it = s_symbol.find(value);
    if (it != s_symbol.end())
        ostr << "`" << it->second << "'";
    else if (isprint(value))
        ostr << "`" << static_cast<char>(value) << "' (" << value << ")";
    else
        ostr << "'\\x" << setfill('0') << hex << setw(2) << value << "'";
    return ostr.str();
}


void Parser::print__()
{
// $insert print

    enum { _UNDETERMINED_ = -2 };

    std::cout << "Token: " << symbol__(d_token__) <<
                ", text: `";
        if (d_token__ == _UNDETERMINED_)
            std::cout << "'\n";
        else
            std::cout << d_scanner.matched() << "'\n";
}

void ParserBase::clearin()
{
    d_token__ = d_nextToken__ = _UNDETERMINED_;
}

void ParserBase::push__(size_t state)
{
    if (static_cast<size_t>(d_stackIdx__ + 1) == d_stateStack__.size())
    {
        size_t newSize = d_stackIdx__ + STACK_EXPANSION;
        d_stateStack__.resize(newSize);
        d_valueStack__.resize(newSize);
    }
    ++d_stackIdx__;
    d_stateStack__[d_stackIdx__] = d_state__ = state;
    *(d_vsp__ = &d_valueStack__[d_stackIdx__]) = d_val__;
}

void ParserBase::popToken__()
{
    d_token__ = d_nextToken__;

    d_val__ = d_nextVal__;
    d_nextVal__ = STYPE__();

    d_nextToken__ = _UNDETERMINED_;
}
     
void ParserBase::pushToken__(int token)
{
    d_nextToken__ = d_token__;
    d_nextVal__ = d_val__;
    d_token__ = token;
}
     
void ParserBase::pop__(size_t count)
{
    if (d_stackIdx__ < static_cast<int>(count))
    {
        ABORT();
    }

    d_stackIdx__ -= count;
    d_state__ = d_stateStack__[d_stackIdx__];
    d_vsp__ = &d_valueStack__[d_stackIdx__];
}

inline size_t ParserBase::top__() const
{
    return d_stateStack__[d_stackIdx__];
}

void Parser::executeAction(int production)
try
{
    if (d_token__ != _UNDETERMINED_)
        pushToken__(d_token__);     // save an already available token

                                    // save default non-nested block $$
    if (int size = s_productionInfo[production].d_size)
        d_val__ = d_vsp__[1 - size];

    switch (production)
    {
        // $insert actioncases
        
    }
}
catch (std::exception const &exc)
{
    exceptionHandler__(exc);
}

inline void ParserBase::reduce__(PI__ const &pi)
{
    d_token__ = pi.d_nonTerm;
    pop__(pi.d_size);

}

// If d_token__ is _UNDETERMINED_ then if d_nextToken__ is _UNDETERMINED_ another
// token is obtained from lex(). Then d_nextToken__ is assigned to d_token__.
void Parser::nextToken()
{
    if (d_token__ != _UNDETERMINED_)        // no need for a token: got one
        return;                             // already

    if (d_nextToken__ != _UNDETERMINED_)
    {
        popToken__();                       // consume pending token
    }
    else
    {
        ++d_acceptedTokens__;               // accept another token (see
                                            // errorRecover())
        d_token__ = lex();
        if (d_token__ <= 0)
            d_token__ = _EOF_;
    }
    print();
}

// if the final transition is negative, then we should reduce by the rule
// given by its positive value. Note that the `recovery' parameter is only
// used with the --debug option
int Parser::lookup(bool recovery)
{
    // $insert threading
    SR__ *sr = s_state[d_state__];        // get the appropriate state-table
    int lastIdx = sr->d_lastIdx;        // sentinel-index in the SR__ array

    SR__ *lastElementPtr = sr + lastIdx;
    SR__ *elementPtr = sr + 1;            // start the search at s_xx[1]

    lastElementPtr->d_token = d_token__;// set search-token

    while (elementPtr->d_token != d_token__)
        ++elementPtr;

    if (elementPtr == lastElementPtr)   // reached the last element
    {
        if (elementPtr->d_action < 0)   // default reduction
        {
            return elementPtr->d_action;                
        }

        // No default reduction, so token not found, so error.
        throw UNEXPECTED_TOKEN__;
    }

    // not at the last element: inspect the nature of the action
    // (< 0: reduce, 0: ACCEPT, > 0: shift)

    int action = elementPtr->d_action;


    return action;
}

    // When an error has occurred, pop elements off the stack until the top
    // state has an error-item. If none is found, the default recovery
    // mode (which is to abort) is activated. 
    //
    // If EOF is encountered without being appropriate for the current state,
    // then the error recovery will fall back to the default recovery mode.
    // (i.e., parsing terminates)
void Parser::errorRecovery()
try
{
    if (d_acceptedTokens__ >= d_requiredTokens__)// only generate an error-
    {                                           // message if enough tokens 
        ++d_nErrors__;                          // were accepted. Otherwise
        error("Syntax error");                  // simply skip input

    }


    // get the error state
    while (not (s_state[top__()][0].d_type & ERR_ITEM))
    {
        pop__();
    }

    // In the error state, lookup a token allowing us to proceed.
    // Continuation may be possible following multiple reductions,
    // but eventuall a shift will be used, requiring the retrieval of
    // a terminal token. If a retrieved token doesn't match, the catch below 
    // will ensure the next token is requested in the while(true) block
    // implemented below:

    int lastToken = d_token__;                  // give the unexpected token a
                                                // chance to be processed
                                                // again.

    pushToken__(_error_);                       // specify _error_ as next token
    push__(lookup(true));                       // push the error state

    d_token__ = lastToken;                      // reactivate the unexpected
                                                // token (we're now in an
                                                // ERROR state).

    bool gotToken = true;                       // the next token is a terminal

    while (true)
    {
        try
        {
            if (s_state[d_state__]->d_type & REQ_TOKEN)
            {
                gotToken = d_token__ == _UNDETERMINED_;
                nextToken();                    // obtain next token
            }
            
            int action = lookup(true);

            if (action > 0)                 // push a new state
            {
                push__(action);
                popToken__();

                if (gotToken)
                {

                    d_acceptedTokens__ = 0;
                    return;
                }
            }
            else if (action < 0)
            {
                // no actions executed on recovery but save an already 
                // available token:
                if (d_token__ != _UNDETERMINED_)
                    pushToken__(d_token__);
 
                                            // next token is the rule's LHS
                reduce__(s_productionInfo[-action]); 
            }
            else
                ABORT();                    // abort when accepting during
                                            // error recovery
        }
        catch (...)
        {
            if (d_token__ == _EOF_)
                ABORT();                    // saw inappropriate _EOF_
                      
            popToken__();                   // failing token now skipped
        }
    }
}
catch (ErrorRecovery__)       // This is: DEFAULT_RECOVERY_MODE
{
    ABORT();
}

    // The parsing algorithm:
    // Initially, state 0 is pushed on the stack, and d_token__ as well as
    // d_nextToken__ are initialized to _UNDETERMINED_. 
    //
    // Then, in an eternal loop:
    //
    //  1. If a state does not have REQ_TOKEN no token is assigned to
    //     d_token__. If the state has REQ_TOKEN, nextToken() is called to
    //      determine d_nextToken__ and d_token__ is set to
    //     d_nextToken__. nextToken() will not call lex() unless d_nextToken__ is 
    //     _UNDETERMINED_. 
    //
    //  2. lookup() is called: 
    //     d_token__ is stored in the final element's d_token field of the
    //     state's SR_ array. 
    //
    //  3. The current token is looked up in the state's SR_ array
    //
    //  4. Depending on the result of the lookup() function the next state is
    //     shifted on the parser's stack, a reduction by some rule is applied,
    //     or the parsing function returns ACCEPT(). When a reduction is
    //     called for, any action that may have been defined for that
    //     reduction is executed.
    //
    //  5. An error occurs if d_token__ is not found, and the state has no
    //     default reduction. Error handling was described at the top of this
    //     file.

int Parser::parse()
try 
{
    push__(0);                              // initial state
    clearin();                              // clear the tokens.

    while (true)
    {
        try
        {
            if (s_state[d_state__]->d_type & REQ_TOKEN)
                nextToken();                // obtain next token


            int action = lookup(false);     // lookup d_token__ in d_state__

            if (action > 0)                 // SHIFT: push a new state
            {
                push__(action);
                popToken__();               // token processed
            }
            else if (action < 0)            // REDUCE: execute and pop.
            {
                executeAction(-action);
                                            // next token is the rule's LHS
                reduce__(s_productionInfo[-action]); 
            }
            else 
                ACCEPT();
        }
        catch (ErrorRecovery__)
        {
            errorRecovery();
        }
    }
}
catch (Return__ retValue)
{
    return retValue;
}


// $insert namespace-close
}


