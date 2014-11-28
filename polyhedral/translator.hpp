#ifndef STREAM_POLYHEDRAL_TRANSLATOR_INCLUDED
#define STREAM_POLYHEDRAL_TRANSLATOR_INCLUDED

#include "model.hpp"
#include "../frontend/ast.hpp"
#include "../frontend/types.hpp"
#include "../frontend/context.hpp"
#include "../frontend/environment.hpp"
#include "../utility/matrix.hpp"

#include <deque>
#include <vector>
#include <stack>

namespace stream {
namespace polyhedral {

using std::deque;
using std::vector;
using std::stack;
using utility::matrix;

class translator
{
public:
    translator(const semantic::environment &);

    void translate(const semantic::function & func,
                   const vector<semantic::type_ptr> & args);

    vector<statement*> & statements() { return m_statements; }

private:
    class symbol
    {
    public:
        symbol(expression *expr): source(expr) {}
        expression * source;
    };

    class stream_view : public expression
    {
    public:
        statement *target;
        mapping pattern;
        int current_iteration;
    };

    typedef stream::context<string,symbol> context;

    const semantic::environment &m_env;
    context m_context;
    vector<int> m_domain;
    vector<statement*> m_statements;


    void do_statement_list(const ast::node_ptr &node);
    expression * do_statement(const ast::node_ptr &node);
    expression * do_block(const ast::node_ptr &node);
    expression * do_expression(const ast::node_ptr &node);
    expression * do_identifier(const ast::node_ptr &node);
    expression * do_call(const ast::node_ptr &node);
    expression * do_unary_op(const ast::node_ptr &node);
    expression * do_binary_op(const ast::node_ptr &node);
    expression * do_transpose(const ast::node_ptr &node);
    expression * do_slicing(const  ast::node_ptr &node);
    expression * do_mapping(const  ast::node_ptr &node);
    expression * do_reduction(const  ast::node_ptr &node);

    int current_dimension() const
    {
        return m_domain.size();
    }

    expression * translate_input(const semantic::type_ptr & type, int index);

    mapping access(stream_view *source);

    stream_access * complete_access( stream_view * );
    statement * make_statement( expression *, const vector<int> & domain );
    stream_view * make_current_view( statement * );

    expression * update_accesses(expression *, const mapping & map);
};

}
}

#endif // STREAM_POLYHEDRAL_TRANSLATOR_INCLUDED
