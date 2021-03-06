/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "ir-generator.hpp"
#include "error.hpp"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalAlias.h>
#include LLVM_VERIFIER_HEADER
#include <llvm/Support/raw_os_ostream.h>
#include <algorithm>

using namespace std;

namespace stream {
namespace IR {

generator::generator(const string & module_name, environment &env):
    m_env(env),
    m_module(module_name, llvm::getGlobalContext()),
    m_builder(llvm::getGlobalContext()),
    m_buffer_pool(nullptr),
    m_buffer_pool_size(0)
{
    m_ctx.enter_scope();

    llvm::Type *double_type = llvm::Type::getDoubleTy(llvm_context());
    llvm::Type *int_type = llvm::Type::getInt32Ty(llvm_context());
    vector<llvm::Type*> double2_type = {double_type, double_type};

    llvm::FunctionType *dd_type = llvm::FunctionType::get(double_type, double_type, false);
    llvm::FunctionType *ddd_type = llvm::FunctionType::get(double_type, double2_type, false);
    llvm::FunctionType *ii_type = llvm::FunctionType::get(int_type, int_type, false);
    {
        vector<string> names = {
            "log",
            "log2",
            "log10",
            "exp",
            "exp2",
            "sqrt",
            "sin",
            "cos",
            "tan",
            "asin",
            "acos",
            "atan"
        };
        for (const string & name : names)
        {
            builtin_func_item *item = new builtin_func_item;
            item->add_overload(function_signature({type::real_num},type::real_num),
                               name, dd_type);
            m_ctx.bind(name, context_item_ptr(item));
        }
    }
    {
        builtin_func_item *item = new builtin_func_item;
        item->add_overload(function_signature({type::real_num,type::real_num},type::real_num),
                           "pow", ddd_type);
        item->add_overload(function_signature({type::integer_num,type::integer_num},type::integer_num),
                           "pow", ddd_type);
        m_ctx.bind("pow", context_item_ptr(item));
    }
    for (const string & name : {"ceil", "floor"})
    {
        builtin_func_item *item = new builtin_func_item;
        item->add_overload(function_signature({type::real_num},type::integer_num),
                           name, dd_type);
        m_ctx.bind(name, context_item_ptr(item));
    }
    {
        builtin_func_item *item = new builtin_func_item;
        item->add_overload(function_signature({type::real_num},type::real_num),
                           "fabs", dd_type);
        item->add_overload(function_signature({type::integer_num},type::integer_num),
                           "abs", ii_type);
        m_ctx.bind("abs", context_item_ptr(item));
    }
    {
        builtin_func_item *item = new builtin_func_item;
        item->add_overload(function_signature({type::real_num,type::real_num},type::real_num),
                           "fmax", ddd_type);
        item->add_overload(function_signature({type::integer_num,type::integer_num},type::integer_num),
                           "fmax", ddd_type);
        m_ctx.bind("max", context_item_ptr(item));
    }
}

void generator::generate( const symbol & sym,
                          const vector<type_ptr> & arg_types )
{
    const type_ptr & result_type = sym.source_expression()->semantic_type;

    // Create function

    llvm::Type *i8_ptr_ptr =
            llvm::PointerType::get(llvm::Type::getInt8PtrTy(llvm_context()), 0);


    llvm::FunctionType * func_type =
            llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_context()),
                                    i8_ptr_ptr,
                                    false);

    string name = sym.name.substr(0, sym.name.find(':'));

    llvm::Function *func =
            llvm::Function::Create(func_type,
                                   llvm::Function::ExternalLinkage,
                                   name,
                                   &m_module);


    llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm_context(), "entry", func);
    m_builder.SetInsertPoint(bb);

    // Extract args

    llvm::Value *args = func->arg_begin();

    vector<value_ptr> inputs;
    for (int idx = 0; idx < arg_types.size(); ++idx)
    {
        inputs.push_back(value_for_argument(args, idx, arg_types[idx], true));
    }

    // Extract output destination:

    value_ptr output =
            value_for_argument(args, arg_types.size(), result_type, false);

    // Set up buffer pool:

    llvm::Value *buffer_pool_arg =
            m_builder.CreateGEP(args, get_int32(arg_types.size() + 1));
    llvm::Value *buffer_pool_void_ptr =
            m_builder.CreateLoad(buffer_pool_arg);
    m_buffer_pool =
            m_builder.CreateBitCast(buffer_pool_void_ptr,
                                    llvm::Type::getDoublePtrTy(llvm_context()));
    m_buffer_pool_size = 0;

    // Generate

    context::scope_holder scope(m_ctx);

    context_item_ptr item = item_for_symbol(sym, output);

    if (inputs.size())
    {
        user_func_item *func = dynamic_cast<user_func_item*>(item.get());
        assert(func);
        value_ptr result =
                generate_call(func, inputs, output, m_ctx.root_scope());
    }

    m_builder.CreateRetVoid();

    cout << "buffer pool size = " << m_buffer_pool_size << endl;
}

value_ptr generator::value_for_argument( llvm::Value *args, int index,
                                         const type_ptr & arg_type,
                                         bool load_scalar )
{
    llvm::Value *arg_ptr_ptr =
            m_builder.CreateGEP(args, get_int32(index));
    llvm::Value *arg_ptr = m_builder.CreateLoad(arg_ptr_ptr);

    switch(arg_type->get_tag())
    {
    case type::integer_num:
    {
        llvm::Value *val_ptr =
                m_builder.CreateBitCast(arg_ptr, llvm::Type::getInt32PtrTy(llvm_context()));
        if (load_scalar)
        {
            llvm::Value *val =
                    m_builder.CreateLoad(val_ptr);
            return make_shared<scalar_value>(val);
        }
        else
            return make_shared<scalar_value>(val_ptr);
    }
    case type::real_num:
    {
        llvm::Value *val_ptr =
                m_builder.CreateBitCast(arg_ptr, llvm::Type::getDoublePtrTy(llvm_context()));
        if (load_scalar)
        {
            llvm::Value *val =
                    m_builder.CreateLoad(val_ptr);
            return make_shared<scalar_value>(val);
        }
        else
            return make_shared<scalar_value>(val_ptr);
    }
    case type::stream:
    {
        vector<int> & stream_size = arg_type->as<stream>().size;
        llvm::Value *data_ptr =
                m_builder.CreateBitCast(arg_ptr, llvm::Type::getDoublePtrTy(llvm_context()));
        return make_shared<stream_value>(data_ptr, stream_size);
    }
    default:
        assert(false);
    }
    return value_ptr();
}

llvm::Type *generator::llvm_type( const type_ptr & t )
{
    using namespace semantic;

    switch(t->get_tag())
    {
    case type::integer_num:
        return llvm::Type::getInt32Ty(llvm_context());
    case type::real_num:
        return llvm::Type::getDoubleTy(llvm_context());
    case type::stream:
        return llvm::PointerType::get( llvm::Type::getDoubleTy(llvm_context()), 0 );
    }

    // TODO
    assert(false);
    return nullptr;
}

context_item_ptr generator::item_for_symbol( const symbol & sym,
                                             const value_ptr & result_space )
{
    switch(sym.type)
    {
    case symbol::expression:
    {
        context::scope_holder(m_ctx, m_ctx.root_scope());
        value_ptr val = process_block(sym.source_expression(), result_space);
        return make_shared<value_item>(val);
    }
    case symbol::function:
    {
        user_func_item *item = new user_func_item;
        item->f = dynamic_pointer_cast<semantic::function>(sym.source->semantic_type);
        if (!item->f)
            throw error("baaad.");
        assert(item->f);
        return context_item_ptr(item);
    }
    default:
        assert(false);
    }
    return context_item_ptr();
}

value_ptr generator::process_block( const ast::node_ptr & root,
                                    const value_ptr & result_space )
{
    assert(root->type == ast::expression_block);
    ast::list_node *expr_block = root->as_list();
    assert(expr_block->elements.size() == 2);
    const auto & stmts = expr_block->elements[0];
    const auto & expr = expr_block->elements[1];

    if (stmts)
        process_stmt_list( stmts );

    return process_expression( expr, result_space );
}

void generator::process_stmt_list( const ast::node_ptr & root )
{
    assert(root->type == ast::statement_list || root->type == ast::program);
    ast::list_node *stmts = root->as_list();
    for ( const ast::node_ptr & stmt : stmts->elements )
    {
        process_stmt(stmt);
    }
}

void generator::process_stmt( const ast::node_ptr & root )
{
    ast::list_node *stmt = root->as_list();
    const auto & id_node = stmt->elements[0];
    const auto & params_node = stmt->elements[1];
    const auto & expr_node = stmt->elements[2];

    const string & id = id_node->as_leaf<string>()->value;

    context_item_ptr ctx_item;

    if (params_node)
    {
        vector<string> parameters;
        ast::list_node *param_list = params_node->as_list();
        for ( const auto & param : param_list->elements )
        {
            string param_name = param->as_leaf<string>()->value;
            parameters.push_back(param_name);
        }

        user_func_item *item = new user_func_item;
        item->f = dynamic_pointer_cast<function>(root->semantic_type);
        ctx_item = context_item_ptr(item);
    }
    else
    {
        value_ptr result = process_block(expr_node);
        ctx_item = make_shared<value_item>(result);
    }

    m_ctx.bind(id, ctx_item);
}

value_ptr generator::process_expression( const ast::node_ptr & root, const value_ptr & result_space )
{
    value_ptr result;

    switch(root->type)
    {
    case ast::integer_num:
    {
        int i = root->as_leaf<int>()->value;
        result = make_shared<scalar_value>(get_int32(i));
        break;
    }
    case ast::real_num:
    {
        double d = root->as_leaf<double>()->value;
        llvm::Value * v = llvm::ConstantFP::get(llvm_context(), llvm::APFloat(d));
        result = make_shared<scalar_value>(v);
        break;
    }
    case ast::identifier:;
    {
        result = process_identifier(root).first;
        break;
    }
    case ast::call_expression:
        return process_call(root, result_space);
    case ast::negate:
        return process_negate(root, result_space);
    case ast::add:
    case ast::subtract:
    case ast::multiply:
    case ast::divide:
    case ast::raise:
    case ast::lesser:
    case ast::greater:
    case ast::lesser_or_equal:
    case ast::greater_or_equal:
    case ast::equal:
    case ast::not_equal:
        return process_binop(root, result_space);
    case ast::range:
        result = process_range(root);
        break;
    case ast::hash_expression:
        result = process_extent(root);
        break;
    case ast::transpose_expression:
        result = process_transpose(root);
        break;
    case ast::slice_expression:
        result = process_slice(root);
        break;
    case ast::for_expression:
        return process_iteration(root, result_space);
    case ast::reduce_expression:
        return process_reduction(root, result_space);
    default:
        throw source_error("Unsupported expression.", root->line);
    }

    if (result_space)
        generate_store(result_space, result);

    return result;
}

pair<value_ptr, generator::context::scope_iterator>
generator::process_identifier( const ast::node_ptr & root )
{
    string id = root->as_leaf<string>()->value;
    if (context::item ctx_entry = m_ctx.find(id))
    {
        context_item_ptr ctx_item = ctx_entry.value();
        value_item *val_item = ctx_item->as_value();
        return make_pair(val_item->get_value(), ctx_entry.scope());
    }
    else
    {
        environment::const_iterator it = m_env.find(id);
        if (it != m_env.end())
        {
            context_item_ptr ctx_item = item_for_symbol(it->second);
            m_ctx.root_scope()->emplace(id, ctx_item);

            value_item *val_item = ctx_item->as_value();
            return make_pair(val_item->get_value(), m_ctx.root_scope());
        }
    }
    assert(false);
    throw source_error("Name not in scope.", root->line);
}

value_ptr generator::process_call( const ast::node_ptr & root, const value_ptr & result_space )
{
    assert(root->type == ast::call_expression);

    ast::list_node * call = root->as_list();
    const auto & func_node = call->elements[0];
    const auto & args_node = call->elements[1];

    // Get function

    context_item_ptr ctx_item;
    context::scope_iterator scope;

    string id = func_node->as_leaf<string>()->value;
    if (context::item ctx_entry = m_ctx.find(id))
    {
        ctx_item = ctx_entry.value();
        scope = ctx_entry.scope();
    }
    else
    {
        environment::const_iterator it = m_env.find(id);
        assert(it != m_env.end());
        ctx_item = item_for_symbol(it->second);
        scope = m_ctx.root_scope();
    }

    // Get args

    std::vector<value_ptr> args;
    ast::list_node * arg_list = args_node->as_list();
    for (const auto & arg_node : arg_list->elements)
    {
        args.push_back( process_expression(arg_node) );
    }

    // Process function

    builtin_func_item *builtin_func = dynamic_cast<builtin_func_item*>(ctx_item.get());
    if (builtin_func_item *builtin_func = dynamic_cast<builtin_func_item*>(ctx_item.get()))
    {
        assert(func_node->semantic_type->is(type::builtin_function));
        builtin_function *builtin_func_type =
                static_cast<builtin_function*>(func_node->semantic_type.get());
        function_signature &func_signature = builtin_func_type->signature;

        llvm::Function *func = function_for(builtin_func, func_signature);

        type_ptr result_type = root->semantic_type;

        return generate_call(func, args, result_type, result_space);
    }
    else if (user_func_item *f = dynamic_cast<user_func_item*>(ctx_item.get()))
    {
        return generate_call(f, args, result_space, scope);
    }
    else
    {
        throw std::runtime_error("Unexpected context item type.");
    }
}

value_ptr generator::generate_call( user_func_item *user_func,
                                    const vector<value_ptr> & args,
                                    const value_ptr & result_space,
                                    context::scope_iterator scope )
{
    function & f = *user_func->f;
    assert(f.parameters.size() == args.size());

    context::scope_holder func_scope(m_ctx, scope);
    for (int i = 0; i < args.size(); ++i)
        m_ctx.bind(f.parameters[i],
                   make_shared<value_item>(args[i]));

    return process_block(f.expression(), result_space);
}

value_ptr generator::generate_call( llvm::Function *func,
                                    const vector<value_ptr> & args,
                                    const type_ptr & result_type,
                                    const value_ptr & result_space )
{
    if (result_type->is(type::stream))
    {
        // Allocate result stream

        stream & result_stream_type = result_type->as<stream>();
        stream_value_ptr result_stream =
                dynamic_pointer_cast<abstract_stream_value>(result_space);
        if (!result_stream)
        {
            result_stream = allocate_stream(result_stream_type.size);
        }

        // Generate iteration

        auto scalar_action = [&]( const vector<scalar_value> & index )
        {
            // Reduce args to scalars

            vector<scalar_value> scalar_args;

            for(const value_ptr & arg : args)
            {
                if (abstract_stream_value * s =
                        dynamic_cast<abstract_stream_value*>(arg.get()))
                {
                    llvm::Value *arg_address = s->at(index, m_builder);
                    llvm::Value *arg_value = m_builder.CreateLoad(arg_address);
                    scalar_args.push_back(arg_value);
                }
                else if (range_value * r =
                         dynamic_cast<range_value*>(arg.get()))
                {
                    assert(index.size() == 1);
                    llvm::Value *v = range_at(*r, index[0]);
                    scalar_args.push_back(v);
                }
                else
                {
                    scalar_value *v =
                            dynamic_cast<scalar_value*>(arg.get());
                    assert(v);
                    scalar_args.push_back( v->data() );
                }
            }

            // Call

            scalar_value result_value = generate_call(func, scalar_args,
                                                      result_type->get_tag());

            // Store result

            llvm::Value * result_dst = result_stream->at(index, m_builder);
            m_builder.CreateStore(result_value.data(), result_dst);
        };

        generate_iteration(result_stream_type.size, scalar_action);

        return result_stream;
    }
    else
    {
        vector<scalar_value> scalar_args;
        for (const auto & arg : args)
        {
            scalar_value *scalar_arg = dynamic_cast<scalar_value*>(arg.get());
            assert(scalar_arg);
            scalar_args.push_back(*scalar_arg);
        }
        scalar_value result_value = generate_call(func, scalar_args,
                                                  result_type->get_tag());
        value_ptr result = make_shared<scalar_value>(result_value);
        if (result_space)
        {
            generate_store(result_space, result);
            return result_space;
        }
        else
            return result;
    }
}

scalar_value generator::generate_call( llvm::Function *func,
                                       const vector<scalar_value> & args,
                                       type::tag result_type_tag )
{
    llvm::FunctionType *func_type = func->getFunctionType();

    assert(func_type->getNumParams() == args.size());
    vector<llvm::Value*> arg_values;
    auto param_it = func_type->param_begin();
    for (const auto & arg : args)
    {
        llvm::Value *arg_value = arg.data();
        llvm::Type *param_type = *param_it;
        if (arg_value->getType() != param_type)
        {
            arg_value = m_builder.CreateSIToFP(arg_value, param_type);
        }
        arg_values.push_back(arg_value);
        ++param_it;
    }

    llvm::Value *result_value = m_builder.CreateCall(func, arg_values);

    llvm::Type *result_type = func_type->getReturnType();
    llvm::Type *int_type = llvm::Type::getInt32Ty(llvm_context());
    if ( result_type_tag == type::integer_num &&
         result_type != int_type )
    {
        result_value = m_builder.CreateFPToSI(result_value, int_type);
    }

    return scalar_value(result_value);
}

llvm::Function *generator::function_for( builtin_func_item *item,
                                         const function_signature &signature )
{
    auto overload_it = item->m_overloads.find(signature);
    assert(overload_it != item->m_overloads.end());
    builtin_func_item::overload & overload = overload_it->second;
    assert(overload.type);
    if (!overload.func)
    {
        overload.func = llvm::Function::Create(overload.type,
                                               llvm::Function::ExternalLinkage,
                                               overload.name,
                                               &m_module);
    }
    return overload.func;
}


value_ptr generator::process_range( const ast::node_ptr & node )
{
    assert(node->type == ast::range);
    const auto & start_node = node->as_list()->elements[0];
    const auto & end_node = node->as_list()->elements[1];

    value_ptr start_val = process_expression(start_node);
    value_ptr end_val = process_expression(end_node);

    scalar_value * scalar_start = dynamic_cast<scalar_value*>(start_val.get());
    scalar_value * scalar_end = dynamic_cast<scalar_value*>(end_val.get());
    assert(scalar_start);
    assert(scalar_end);

    return make_shared<range_value>(scalar_start->data(), scalar_end->data());
}

value_ptr generator::process_negate( const ast::node_ptr & root,
                                     const value_ptr & result_space )
{
    value_ptr val = process_expression( root->as_list()->elements[0] );

    if (scalar_value *scalar = dynamic_cast<scalar_value*>(val.get()))
    {
        llvm::Value *operand = scalar->data();
        llvm::Value *negated;
        if (operand->getType() == llvm::Type::getDoubleTy(llvm_context()))
            negated = m_builder.CreateFNeg(operand);
        else
            negated = m_builder.CreateNeg(operand);
        value_ptr result = make_shared<scalar_value>(negated);
        if (result_space)
            generate_store(result_space, result);
        return make_shared<scalar_value>(negated);
    }

    // Get result size

    vector<int> result_stream_size;
    {
        const type_ptr & result_type = root->semantic_type;
        assert(result_type->is(type::stream));
        stream & result_stream_type = result_type->as<stream>();
        result_stream_size = result_stream_type.size;
    }

    // Allocate result stream

    stream_value_ptr result_stream =
            dynamic_pointer_cast<abstract_stream_value>(result_space);
    if (!result_stream)
    {
        result_stream = allocate_stream(result_stream_size);
    }

    // Iterate and perform operation:

    auto operation = [&]( const vector<scalar_value> & stream_index )
    {
        llvm::Value *operand;

        if (range_value *range = dynamic_cast<range_value*>(val.get()))
        {
            assert(stream_index.size() == 1);
            operand = int_to_real( range_at(*range, stream_index[0]) );
        }
        else
        {
            abstract_stream_value *strm = dynamic_cast<abstract_stream_value*>(val.get());
            assert(strm);
            operand = m_builder.CreateLoad( strm->at(stream_index, m_builder) );
        }

        llvm::Value * result_val = m_builder.CreateFNeg(operand);

        llvm::Value * result_ptr = result_stream->at(stream_index, m_builder);
        m_builder.CreateStore(result_val, result_ptr);
    };

    generate_iteration(result_stream->size(), operation);

    return result_stream;
}

value_ptr generator::process_binop( const ast::node_ptr & root,
                                    const value_ptr & result_space  )
{
    ast::list_node *expr = root->as_list();
    const ast::node_ptr & lhs_node = expr->elements[0];
    const ast::node_ptr & rhs_node = expr->elements[1];
    value_ptr lhs = process_expression(expr->elements[0]);
    value_ptr rhs = process_expression(expr->elements[1]);

    if (root->type == ast::raise)
    {
        type::tag l = lhs_node->semantic_type->get_tag();
        type::tag r = rhs_node->semantic_type->get_tag();
        function_signature signature;
        if( (l == type::integer_num && r == type::integer_num) ||
            (l == type::range && r == type::range) )
            signature = function_signature({type::integer_num, type::integer_num},
                                           type::integer_num);
        else
            signature = function_signature({type::real_num, type::real_num},
                                           type::real_num);

        builtin_func_item *ctx_item =
                static_cast<builtin_func_item*>(m_ctx.root_scope()->at("pow").get());
        assert(ctx_item);
        llvm::Function *f = function_for(ctx_item, signature);
        assert(f);
        return generate_call(f, {lhs,rhs}, root->semantic_type, result_space);
    }

    llvm::Type *fp_type = llvm::Type::getDoubleTy(llvm_context());
    llvm::Type *int_type = llvm::Type::getInt32Ty(llvm_context());
    if ( typeid(*lhs) == typeid(scalar_value) &&
         typeid(*rhs) == typeid(scalar_value) )
    {
        llvm::Value *lhs_ir = static_cast<scalar_value&>(*lhs).data();
        llvm::Value *rhs_ir = static_cast<scalar_value&>(*rhs).data();
        llvm::Value *result_ir;

        bool fp_op = lhs_ir->getType() == fp_type || rhs_ir->getType() == fp_type;
        if (fp_op)
        {
            if (lhs_ir->getType() != fp_type)
                lhs_ir = m_builder.CreateSIToFP(lhs_ir, fp_type);
            if (rhs_ir->getType() != fp_type)
                rhs_ir = m_builder.CreateSIToFP(rhs_ir, fp_type);
            switch(root->type)
            {
            case ast::add:
                result_ir = m_builder.CreateFAdd(lhs_ir, rhs_ir); break;
            case ast::subtract:
                result_ir = m_builder.CreateFSub(lhs_ir, rhs_ir); break;
            case ast::multiply:
                result_ir = m_builder.CreateFMul(lhs_ir, rhs_ir); break;
            case ast::divide:
                result_ir = m_builder.CreateFDiv(lhs_ir, rhs_ir); break;
            default:
                assert(false);
            }
        }
        else
        {
            switch(root->type)
            {
            case ast::add:
                result_ir = m_builder.CreateAdd(lhs_ir, rhs_ir); break;
            case ast::subtract:
                result_ir = m_builder.CreateSub(lhs_ir, rhs_ir); break;
            case ast::multiply:
                result_ir = m_builder.CreateMul(lhs_ir, rhs_ir); break;
            case ast::divide:
                result_ir = m_builder.CreateSDiv(lhs_ir, rhs_ir); break;
            default:
                assert(false);
            }
        }

        value_ptr result = make_shared<scalar_value>(result_ir);
        if (result_space)
            generate_store(result_space, result);
        return result;
    }
    else
    {
        // Get result size

        vector<int> result_stream_size;
        {
            const type_ptr & result_type = root->semantic_type;
            assert(result_type->is(type::stream));
            stream & result_stream_type = result_type->as<stream>();
            result_stream_size = result_stream_type.size;
        }

        // Allocate result stream

        stream_value_ptr result_stream =
                dynamic_pointer_cast<abstract_stream_value>(result_space);
        if (!result_stream)
        {
            result_stream = allocate_stream(result_stream_size);
        }

        // Promote integer scalars to floats

        auto scalar_to_float = [&]( const value_ptr & val ) -> value_ptr
        {
            scalar_value *scalar = dynamic_cast<scalar_value*>(val.get());
            if (scalar && scalar->data()->getType() != fp_type)
            {
                llvm::Value *converted = m_builder.CreateSIToFP(scalar->data(), fp_type);
                return make_shared<scalar_value>(converted);
            }
            return val;
        };

        lhs = scalar_to_float(lhs);
        rhs = scalar_to_float(rhs);

        // Operation loop

        auto operation = [&]( const vector<scalar_value> & stream_index )
        {
            llvm::Value *lhs_val, *rhs_val;

            auto get_operand = [&]( const value_ptr & val ) -> llvm::Value*
            {
                llvm::Value *operand;
                if(scalar_value *scalar = dynamic_cast<scalar_value*>(val.get()))
                {
                    operand = scalar->data();
                    assert(operand->getType() == fp_type);
                }
                else if (range_value *range = dynamic_cast<range_value*>(val.get()))
                {
                    assert(stream_index.size() == 1);
                    operand = int_to_real( range_at(*range, stream_index[0]) );
                }
                else
                {
                    abstract_stream_value *strm = dynamic_cast<abstract_stream_value*>(val.get());
                    assert(strm);
                    operand = m_builder.CreateLoad( strm->at(stream_index, m_builder) );
                }
                return operand;
            };

            lhs_val = get_operand(lhs);
            rhs_val = get_operand(rhs);

            llvm::Value * result_val;

            switch(root->type)
            {
            case ast::add:
                result_val = m_builder.CreateFAdd(lhs_val, rhs_val); break;
            case ast::subtract:
                result_val = m_builder.CreateFSub(lhs_val, rhs_val); break;
            case ast::multiply:
                result_val = m_builder.CreateFMul(lhs_val, rhs_val); break;
            case ast::divide:
                result_val = m_builder.CreateFDiv(lhs_val, rhs_val); break;
            default:
                assert(false);
            }

            llvm::Value * result_ptr = result_stream->at(stream_index, m_builder);
            m_builder.CreateStore(result_val, result_ptr);
        };

        generate_iteration(result_stream->size(), operation);

        return result_stream;
    }
}

value_ptr generator::process_extent( const ast::node_ptr & node )
{
    assert(node->type == ast::hash_expression);
    ast::list_node *list = node->as_list();
    const auto & object_node = list->elements[0];
    const auto & dim_node = list->elements[1];

    value_ptr object = process_expression(object_node);
    abstract_stream_value *object_stream = dynamic_cast<abstract_stream_value*>(object.get());
    assert(object_stream);

    int dim = 0;
    if (dim_node)
    {
        const type_ptr & dim_type = dim_node->semantic_type;
        assert(dim_type);
        assert(dim_type->is(type::integer_num));
        integer_num & dim_int = dim_type->as<integer_num>();
        assert(dim_int.is_constant());
        dim = dim_int.constant_value() - 1;
    }

    assert(dim >= 0 && dim < object_stream->dimensions());
    int stream_size = object_stream->size(dim);

    return make_shared<scalar_value>(get_int32(stream_size));
}

value_ptr generator::process_transpose( const ast::node_ptr & root )
{
    assert(root->type == ast::transpose_expression);
    ast::list_node *root_list = root->as_list();
    const auto & object_node = root_list->elements[0];
    const auto & dims_node = root_list->elements[1];

    value_ptr src_object = process_expression(object_node);

    stream_value_ptr src_stream =
            dynamic_pointer_cast<abstract_stream_value>(src_object);
    assert(src_stream);

    vector<int> map;

    ast::list_node *dims = dims_node->as_list();
    for ( const auto & dim_node : dims->elements )
    {
        map.push_back( dim_node->as_leaf<int>()->value - 1 );
    }

    return make_shared<transpose_value>(src_stream, map);
}

value_ptr generator::process_slice( const ast::node_ptr & root )
{
    assert(root->type == ast::slice_expression);
    const auto & object_node = root->as_list()->elements[0];
    const auto & ranges_node = root->as_list()->elements[1];

    value_ptr object = process_expression(object_node);

    stream_value_ptr src_stream = dynamic_pointer_cast<abstract_stream_value>(object);
    assert(src_stream);

#if 0
    cout << "-- slice: source stream size: ";
    for (int i = 0; i < src_stream->dimensions(); ++i)
        cout << src_stream->size(i) << " ";
    cout << endl;
#endif

    ast::list_node *range_list = ranges_node->as_list();

    int range_count = range_list->elements.size();

    vector<scalar_value> slice_offset;
    vector<int> slice_size;
    slice_offset.reserve(range_count);
    slice_size.reserve(range_count);

    int dim = 0;
    for( const auto & range_node : range_list->elements )
    {
        const type_ptr & range_type = range_node->semantic_type;
        switch(range_type->get_tag())
        {
        case type::integer_num:
        {
            value_ptr range = process_expression(range_node);
            scalar_value *scalar = dynamic_cast<scalar_value*>(range.get());
            assert(scalar);
            llvm::Value *normalized_index =
                    m_builder.CreateSub( scalar->data(), get_int32(1) );
            slice_offset.emplace_back(normalized_index);
            slice_size.push_back(1);
            break;
        }
        case type::range:
        {
            range & r = range_type->as<range>();
            assert(r.is_constant());
            int start = r.const_start();
            int size = r.const_size();
            assert(start >= 1);
            assert(size >= 1);
            llvm::Value *index = get_int32(start - 1);
            slice_offset.emplace_back(index);
            slice_size.push_back(size);
            break;
        }
        default:
            assert(false);
        }
    }

    value_ptr slice = slice_stream(src_stream, slice_offset, slice_size);
    if (scalar_value *scalar = dynamic_cast<scalar_value*>(slice.get()))
    {
        llvm::Value *slice_value = m_builder.CreateLoad( scalar->data() );
        slice = make_shared<scalar_value>(slice_value);
    }
    return slice;
}

value_ptr generator::process_iteration( const ast::node_ptr & node,
                                        const value_ptr & result_space )
{
    assert(node->type == ast::for_expression);
    const auto & iterators_node = node->as_list()->elements[0];
    const auto & body_node = node->as_list()->elements[1];

    type_ptr iteration_type = body_node->semantic_type;

    stream_value_ptr result;
    if (result_space)
    {
        result =  dynamic_pointer_cast<abstract_stream_value>(result_space);
        assert(result);
    }
    else
    {
        vector<int> & result_size = node->semantic_type->as<stream>().size;
        result = allocate_stream(result_size);
    }

    vector<stream_value_ptr> domains;

    for (const auto & iterator_node : iterators_node->as_list()->elements)
    {
        semantic::iterator & iter =
                iterator_node->semantic_type->as<semantic::iterator>();
        value_ptr domain = process_expression(iter.domain);
        stream_value_ptr stream_domain;
        if (stream_domain = dynamic_pointer_cast<abstract_stream_value>(domain))
            domains.push_back(stream_domain);
        else
            throw error("Unsupported iteration domain type.");
    }

    auto body = [&]( const scalar_value & index )
    {
        // Set up scope.

        context::scope_holder iteration_scope(m_ctx);

        for (int idx = 0; idx < domains.size(); ++idx)
        {
            const auto & iterator_node = iterators_node->as_list()->elements[idx];
            semantic::iterator & iter =
                    iterator_node->semantic_type->as<semantic::iterator>();

            if (iter.id.empty())
                continue;

            scalar_value iterator_index = index;
            if (iter.hop != 1)
            {
                iterator_index =
                        m_builder.CreateMul( iterator_index.data(), get_uint32(iter.hop) );
            }

            value_ptr domain_slice = slice_stream(domains[idx], {iterator_index}, {iter.size});
            {
                scalar_value * scalar = dynamic_cast<scalar_value*>(domain_slice.get());
                if(scalar)
                {
                    llvm::Value *v = m_builder.CreateLoad(scalar->data());
                    domain_slice = make_shared<scalar_value>(v);
                }
            }

            m_ctx.bind(iter.id, make_shared<value_item>(domain_slice));
        }

        // Generate body

        value_ptr body_result_space = slice_stream(result, {index});
        process_block( body_node, body_result_space );
    };

    generate_iteration(0, result->size(0), body);

    return result;
}

value_ptr generator::process_reduction( const ast::node_ptr & node, const value_ptr & result_space )
{
    assert(node->type == ast::reduce_expression);
    const auto & id1_node = node->as_list()->elements[0];
    const auto & id2_node = node->as_list()->elements[1];
    const auto & domain_node = node->as_list()->elements[2];
    const auto & body_node = node->as_list()->elements[3];

    string id1 = id1_node->as_leaf<string>()->value;
    string id2 = id2_node->as_leaf<string>()->value;

    type_ptr reduction_type = body_node->semantic_type;
    assert(reduction_type->is(type::real_num));

    shared_ptr<scalar_value> result;
    if (result_space)
    {
        result = dynamic_pointer_cast<scalar_value>(result_space);
        assert(result);
    }
    else
    {
        llvm::Value *result_ptr =
                m_builder.CreateAlloca(llvm::Type::getDoubleTy(llvm_context()));
        result = make_shared<scalar_value>(result_ptr);
    }

    value_ptr domain_val = process_expression(domain_node);
    stream_value_ptr domain_stream =
            dynamic_pointer_cast<abstract_stream_value>(domain_val);
    assert(domain_stream);
    assert(domain_stream->dimensions() == 1);
    assert(domain_stream->size(0) > 0);

    m_builder.CreateStore( m_builder.CreateLoad(domain_stream->data()),
                           result->data() );

    auto body = [&]( const scalar_value & index )
    {
        llvm::Value *item1_val =
                m_builder.CreateLoad( result->data() );
        llvm::Value *item2_val =
                m_builder.CreateLoad( domain_stream->at({index}, m_builder) );

        context::scope_holder iteration_scope(m_ctx);
        m_ctx.bind(id1, make_shared<value_item>(item1_val));
        m_ctx.bind(id2, make_shared<value_item>(item2_val));

        process_block(body_node, result);
    };

    generate_iteration(1, domain_stream->size(0), body);

    return make_shared<scalar_value>( m_builder.CreateLoad(result->data()) );
}

void generator::generate_iteration(const scalar_value & from,
                                    const scalar_value & to,
                                    std::function<void(const scalar_value &)> action )
{
    llvm::Function *parent = m_builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *before_block = m_builder.GetInsertBlock();
    llvm::BasicBlock *cond_block = llvm::BasicBlock::Create(llvm_context(), "condition", parent);
    llvm::BasicBlock *action_block = llvm::BasicBlock::Create(llvm_context(), "action", parent);
    llvm::BasicBlock *after_block = llvm::BasicBlock::Create(llvm_context(), "afterloop");

    llvm::Value *start_index = from.data();
    llvm::Value *end_index = to.data();

    // BEFORE:

    m_builder.CreateBr(cond_block);

    // CONDITION

    m_builder.SetInsertPoint(cond_block);

    // create index
    llvm::PHINode *index = m_builder.CreatePHI(start_index->getType(), 2);
    index->addIncoming(start_index, before_block);

    // create condition
    llvm::Value *condition = m_builder.CreateICmpNE(index, end_index);
    m_builder.CreateCondBr(condition, action_block, after_block);

    // ACTION

    m_builder.SetInsertPoint(action_block);

    // invoke action functor
    action(index);

    // increment index
    llvm::Value *index_increment =
            generate_sign( m_builder.CreateSub(end_index, start_index) );

    llvm::Value *next_index_value =
            m_builder.CreateAdd(index, index_increment);
    index->addIncoming(next_index_value, m_builder.GetInsertBlock());

    m_builder.CreateBr(cond_block);

    // AFTER

    parent->getBasicBlockList().push_back(after_block);

    m_builder.SetInsertPoint(after_block);
}

void generator::generate_iteration( int from, int to,
                                    std::function<void(const scalar_value &)> action )
{
    generate_iteration(get_int32(from), get_int32(to), action);
}

void generator::generate_iteration( const vector<int> range,
                                    std::function<void(const vector<scalar_value> &)> final_action,
                                    const vector<scalar_value> & stream_index )
{
    if (stream_index.size() < range.size())
    {
        int dim = stream_index.size();
        llvm::Value *start_index = get_int32(0);
        llvm::Value *end_index = get_int32(range[dim]);

        auto action = [&](const scalar_value & index)
        {
            vector<scalar_value> composed_index = stream_index;
            composed_index.push_back(index);
            generate_iteration(range, final_action, composed_index);
        };

        generate_iteration(start_index, end_index, action);
    }
    else
    {
        final_action(stream_index);
    }
}

void generator::generate_store( const value_ptr & dst, const value_ptr & src )
{
    if (scalar_value *src_scalar = dynamic_cast<scalar_value*>(src.get()))
    {
        scalar_value *dst_scalar = dynamic_cast<scalar_value*>(dst.get());
        assert(dst_scalar);
        m_builder.CreateStore( src_scalar->data(), dst_scalar->data() );
    }
    else if(abstract_stream_value *src_stream = dynamic_cast<abstract_stream_value*>(src.get()))
    {
        abstract_stream_value *dst_stream = dynamic_cast<abstract_stream_value*>(dst.get());

        auto copy_action = [&]( const vector<scalar_value> & index )
        {
            llvm::Value *src_val_ptr = src_stream->at(index, m_builder);
            llvm::Value *dst_val_ptr = dst_stream->at(index, m_builder);
            m_builder.CreateStore( m_builder.CreateLoad(src_val_ptr), dst_val_ptr );
        };

        generate_iteration(src_stream->size(), copy_action);
    }
    else if (range_value *src_range = dynamic_cast<range_value*>(src.get()))
    {
        abstract_stream_value *dst_stream = dynamic_cast<abstract_stream_value*>(dst.get());
        assert(dst_stream);
        assert(dst_stream->dimensions() == 1);

        auto action = [&]( const scalar_value & index )
        {
            llvm::Value *dst = dst_stream->at({index}, m_builder);
            llvm::Value *val = int_to_real( range_at(*src_range, index) );
            m_builder.CreateStore( val, dst );
        };

        generate_iteration(0, dst_stream->size(0), action);
    }
}

value_ptr generator::slice_stream( const stream_value_ptr &stream,
                                   const vector<scalar_value> & offset,
                                   const vector<int> & size )
{
    bool all_sizes_one = true;
    for(int i : size)
    {
        if (i != 1)
        {
            all_sizes_one = false;
            break;
        }
    }

    if (offset.size() < stream->dimensions() || !all_sizes_one)
    {
        vector<int> slice_size = size;
        if (slice_size.empty())
            slice_size = vector<int>(offset.size(), 1);
        return make_shared<slice_value>(stream, offset, slice_size);
    }
    else
    {
        return make_shared<scalar_value>(stream->at(offset, m_builder));
    }
}

stream_value_ptr generator::allocate_stream( const vector<int> & stream_size )
{
    int alloc_count = 1;
    for (int s : stream_size)
        alloc_count *= s;

    llvm::Value *buffer =
            m_builder.CreateAlloca( llvm::Type::getDoubleTy(llvm_context()),
                                    get_int32(alloc_count) );

    return make_shared<stream_value>(buffer, stream_size);

#if 0
    size_t alloc_bytes = alloc_count * 8;

    llvm::Value *offset =
            llvm::ConstantInt::get(llvm_context(),
                                   llvm::APInt(sizeof(alloc_bytes),
                                               m_buffer_pool_size));
    llvm::Value *buffer =
            m_builder.CreateGEP(m_buffer_pool, offset);

    m_buffer_pool_size += alloc_bytes;

    return make_shared<stream_value>(buffer, stream_size);
#endif
}

llvm::Value *generator::generate_sign(llvm::Value * val)
{
#if 0
    llvm::Type *type = val->getType();
    assert(type->isIntegerTy());
    llvm::IntegerType *int_type = static_cast<llvm::IntegerType*>(type);
    uint64_t sign_mask = int_type->getSignBit();
    llvm::Value *sign_bit = m_builder.CreateAnd(val, sign_mask);
    llvm::Value *signed_one = m_builder.CreateOr(sign_bit, 1);
    return signed_one;
#endif

    llvm::Value *is_positive = m_builder.CreateICmpSGE(val, get_int32(0));
    llvm::Value *is_negative = m_builder.CreateICmpSLT(val, get_int32(0));
    return m_builder.CreateSub(m_builder.CreateZExt(is_positive,val->getType()),
                               m_builder.CreateZExt(is_negative,val->getType()));
}

llvm::Value *generator::range_at(const range_value &r, const scalar_value & index)
{
    llvm::Value *sign = generate_sign( m_builder.CreateSub(r.end(), r.start()) );
    llvm::Value *progress = m_builder.CreateMul( sign, index.data() );
    llvm::Value *value = m_builder.CreateAdd( r.start(), progress );
    return value;
}

llvm::Value *stream_value::at( const vector<scalar_value> & index,
                                   llvm::IRBuilder<> & builder )
{
    assert(index.size() == m_index_coeffs.size() + 1);

    llvm::Value * flat_index = index.back().data();

    int c = m_index_coeffs.size();
    while(c--)
    {
        llvm::Value *idx_val = index[c].data();

        llvm::Value *coeff_val =
                llvm::ConstantInt::get(builder.getContext(),
                                       llvm::APInt(32,m_index_coeffs[c],true));

        llvm::Value *partial_val =
                builder.CreateMul( idx_val, coeff_val );

        flat_index = builder.CreateAdd(flat_index, partial_val);
    }

    return llvm::GetElementPtrInst::Create(m_data,
                                           llvm::ArrayRef<llvm::Value*>(&flat_index,1),
                                           "", builder.GetInsertBlock());
}

llvm::Value *slice_value::at( const vector<scalar_value> & index,
                                  llvm::IRBuilder<> & builder )
{
    assert(index.size() == m_size.size());
    vector<scalar_value> source_index;
    source_index.reserve(m_preoffset.size() + m_size.size());
    for(const scalar_value & preoffset : m_preoffset)
    {
        source_index.push_back(preoffset);
    }
    int dim = 0;
    while (dim < m_offset.size())
    {
        llvm::Value *index_val =
                builder.CreateAdd(index[dim].data(),
                                  m_offset[dim].data());
        source_index.emplace_back(index_val);
        ++dim;
    }
    while (dim < m_size.size())
    {
        source_index.push_back(index[dim]);
        ++dim;
    }
    return m_source->at( source_index, builder );
}

bool generator::verify()
{
    string verifier_msg;
    bool failure = llvm::verifyModule(m_module, llvm::ReturnStatusAction,
                                      &verifier_msg);
    if (failure)
    {
        cerr << "ERROR: Failed to verify generated IR code:" << endl;
        cerr << verifier_msg;
    }
    return !failure;
}

void generator::output( std::ostream & out )
{
    llvm::raw_os_ostream llvm_ostream(out);
    m_module.print( llvm_ostream, nullptr );
}

}
}
