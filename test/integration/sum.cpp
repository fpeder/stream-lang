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

#include "sum.h"

#include <iostream>

using namespace std;

int main()
{
    sum::buffer buf;
    sum::allocate(&buf);

    double *input = new double[10];
    for (int i = 0; i < 10; ++i)
    {
        input[i] = i;
    }

    sum::initialize(input, &buf);

    double result = *sum::get_output(&buf);

    cout << "result = " << result << endl;

    if (result == 45)
        return 0;
    else
        return 1;
}
