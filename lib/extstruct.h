/*--------------------------------------------------------------------
  (C) Copyright 2006-2011 Barcelona Supercomputing Center 
                          Centro Nacional de Supercomputacion
  
  This file is part of Mercurium C/C++ source-to-source compiler.
  
  See AUTHORS file in the top level directory for information 
  regarding developers and contributors.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  
  Mercurium C/C++ source-to-source compiler is distributed in the hope
  that it will be useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the GNU Lesser General Public License for more
  details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with Mercurium C/C++ source-to-source compiler; if
  not, write to the Free Software Foundation, Inc., 675 Mass Ave,
  Cambridge, MA 02139, USA.
--------------------------------------------------------------------*/



#ifndef EXTSTRUCT_H
#define EXTSTRUCT_H

#include <stdlib.h>
#include "red_black_tree.h"

#ifdef WIN32_BUILD
  #ifdef LIBEXTSTRUCT_DLL_EXPORT
    #define LIBEXTSTRUCT_EXTERN extern __declspec(dllexport)
  #else
    #define LIBEXTSTRUCT_EXTERN extern __declspec(dllimport)
  #endif
#else
  #define LIBEXTSTRUCT_EXTERN extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct extensible_struct_tag
{
    rb_red_blk_tree* hash;
};

typedef struct extensible_struct_tag extensible_struct_t;

// Extensible struct operations
LIBEXTSTRUCT_EXTERN void extensible_struct_init(extensible_struct_t** extensible_struct);
LIBEXTSTRUCT_EXTERN void extensible_struct_set_field(extensible_struct_t* extensible_struct, 
        const char* field_name, void *data);

LIBEXTSTRUCT_EXTERN void* extensible_struct_get_field(extensible_struct_t* extensible_struct, 
        const char* field_name);

LIBEXTSTRUCT_EXTERN void extensible_struct_get_all_data(extensible_struct_t* extensible_struct,
        int *num_fields,
        const char ***keys,
        void ***data);

#ifdef __cplusplus
}
#endif

#endif // EXTSTRUCT_H
