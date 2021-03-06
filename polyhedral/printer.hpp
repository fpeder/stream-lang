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

#ifndef STREAM_POLYHEDRAL_PRINTER_INCLUDED
#define STREAM_POLYHEDRAL_PRINTER_INCLUDED

#include "model.hpp"

#include <iostream>
#include <string>

namespace stream {
namespace polyhedral {

using std::ostream;
using std::string;

class printer
{
public:
    printer();
    void print(const numerical_type t, ostream &);
    void print(const expression *expr, ostream &);
    void print(const statement *stmt, ostream & );
    void indent() { ++level; }
    void unindent() { --level; }
private:
    string indentation() { return string(level * 2, ' '); }
    int level;

};

}
}
#endif // STREAM_POLYHEDRAL_PRINTER_INCLUDED
