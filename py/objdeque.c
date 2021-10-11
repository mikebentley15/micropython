/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Paul Sokolovsky
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <unistd.h> // for ssize_t
#include <string.h>

#include "py/mpconfig.h"
#if MICROPY_PY_COLLECTIONS_DEQUE

#include "py/runtime.h"

#define DEQUE_IDX(deq, i) ((i) % (deq)->alloc)

/******************************************************************************/
/* deque                                                                      */

typedef struct _mp_obj_deque_t {
    mp_obj_base_t base;
    size_t alloc;
    size_t i_get;
    size_t i_put;
    mp_obj_t *items;
    uint32_t flags;
    #define FLAG_CHECK_OVERFLOW 1
} mp_obj_deque_t;

STATIC mp_obj_t mp_obj_deque_append(mp_obj_t self_in, mp_obj_t arg);
STATIC mp_obj_t deque_getiter(mp_obj_t self_in, mp_obj_iter_buf_t *iter_buf);

STATIC void deque_print(const mp_print_t *print, mp_obj_t o_in, mp_print_kind_t kind) {
    (void)kind;
    mp_obj_deque_t *o = MP_OBJ_TO_PTR(o_in);
    mp_print_str(print, "deque([");
    for (size_t i = o->i_get; i != o->i_put; i = DEQUE_IDX(o, i+1)) {
        if (i != o->i_get) {
            mp_print_str(print, ", ");
        }
        mp_obj_print_helper(print, o->items[i], PRINT_REPR);
    }
    mp_printf(print, "], maxlen=%ld)", o->alloc-1);
}

STATIC void deque_extend_from_iter(mp_obj_t self, mp_obj_t iterable) {
    mp_obj_t iter = mp_getiter(iterable, NULL);
    mp_obj_t item;
    while ((item = mp_iternext(iter)) != MP_OBJ_STOP_ITERATION) {
        mp_obj_deque_append(self, item);
    }
}

STATIC mp_obj_t deque_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 2, 3, false);

    // Protect against -1 leading to zero-length allocation and bad array access
    mp_int_t maxlen = mp_obj_get_int(args[1]);
    if (maxlen < 0) {
        mp_raise_ValueError(NULL);
    }

    mp_obj_deque_t *o = m_new_obj(mp_obj_deque_t);
    o->base.type = type;
    o->alloc = maxlen + 1;
    o->i_get = o->i_put = 0;
    o->items = m_new0(mp_obj_t, o->alloc);
    o->flags = 0;

    if (n_args > 2) {
        o->flags = mp_obj_get_int(args[2]);
    }

    mp_obj_t self = MP_OBJ_FROM_PTR(o);
    deque_extend_from_iter(self, args[0]);
    return self;
}

STATIC inline size_t deque_len(mp_obj_deque_t *self) {
    if (self->i_get > self->i_put) {
        // Note: Order matters here since these are unsigned types.
        //       This is to avoid both overflow and underflow.
        return (self->alloc - self->i_get) + self->i_put;
    } else {
        return self->i_put - self->i_get;
    }
}

STATIC mp_obj_t deque_unary_op(mp_unary_op_t op, mp_obj_t self_in) {
    mp_obj_deque_t *self = MP_OBJ_TO_PTR(self_in);
    switch (op) {
        case MP_UNARY_OP_BOOL:
            return mp_obj_new_bool(self->i_get != self->i_put);
        case MP_UNARY_OP_LEN: {
            return MP_OBJ_NEW_SMALL_INT(deque_len(self));
        }
        #if MICROPY_PY_SYS_GETSIZEOF
        case MP_UNARY_OP_SIZEOF: {
            size_t sz = sizeof(*self) + sizeof(mp_obj_t) * self->alloc;
            return MP_OBJ_NEW_SMALL_INT(sz);
        }
        #endif
        default:
            return MP_OBJ_NULL; // op not supported
    }
}

mp_obj_t mp_obj_deque_append(mp_obj_t self_in, mp_obj_t arg) {
    mp_obj_deque_t *self = MP_OBJ_TO_PTR(self_in);

    size_t new_i_put = DEQUE_IDX(self, self->i_put + 1);

    if (self->flags & FLAG_CHECK_OVERFLOW && new_i_put == self->i_get) {
        mp_raise_msg(&mp_type_IndexError, MP_ERROR_TEXT("full"));
    }

    self->items[self->i_put] = arg;
    self->i_put = new_i_put;

    if (self->i_get == new_i_put) {
        self->i_get = DEQUE_IDX(self, self->i_get + 1);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(deque_append_obj, mp_obj_deque_append);

STATIC mp_obj_t mp_obj_deque_extend(mp_obj_t self_in, mp_obj_t arg) {
    mp_check_self(mp_obj_is_type(self_in, &mp_type_deque));
    deque_extend_from_iter(self_in, arg);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(deque_extend_obj, mp_obj_deque_extend);

STATIC mp_obj_t deque_popleft(mp_obj_t self_in) {
    mp_obj_deque_t *self = MP_OBJ_TO_PTR(self_in);

    if (self->i_get == self->i_put) {
        mp_raise_msg(&mp_type_IndexError, MP_ERROR_TEXT("empty"));
    }

    mp_obj_t ret = self->items[self->i_get];
    self->items[self->i_get] = MP_OBJ_NULL;

    self->i_get = DEQUE_IDX(self, self->i_get + 1);

    return ret;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(deque_popleft_obj, deque_popleft);

STATIC mp_obj_t deque_clear(mp_obj_t self_in) {
    mp_obj_deque_t *self = MP_OBJ_TO_PTR(self_in);
    self->i_get = self->i_put = 0;
    mp_seq_clear(self->items, 0, self->alloc, sizeof(*self->items));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(deque_clear_obj, deque_clear);

STATIC mp_obj_t deque_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
    if (value != MP_OBJ_SENTINEL) {
        return MP_OBJ_NULL; // unsupported: item deletion & item assignment
    }
    #if MICROPY_PY_BUILTINS_SLICE
    if (mp_obj_is_type(index, &mp_type_slice)) {
        mp_raise_TypeError(MP_ERROR_TEXT("deque doesn't support slices"));
    }
    #endif
    mp_obj_deque_t *self = MP_OBJ_TO_PTR(self_in);
    size_t len = deque_len(self);
    size_t i = mp_get_index(self->base.type, len, index, false);
    return MP_OBJ_FROM_PTR(self->items[DEQUE_IDX(self, self->i_get + i)]);
}

STATIC const mp_rom_map_elem_t deque_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_append), MP_ROM_PTR(&deque_append_obj) },
    { MP_ROM_QSTR(MP_QSTR_extend), MP_ROM_PTR(&deque_extend_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&deque_clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_popleft), MP_ROM_PTR(&deque_popleft_obj) },
};

STATIC MP_DEFINE_CONST_DICT(deque_locals_dict, deque_locals_dict_table);

const mp_obj_type_t mp_type_deque = {
    { &mp_type_type },
    .name = MP_QSTR_deque,
    .print = deque_print,
    .make_new = deque_make_new,
    .unary_op = deque_unary_op,
    .subscr = deque_subscr,
    .getiter = deque_getiter,
    .locals_dict = (mp_obj_dict_t *)&deque_locals_dict,
};

/******************************************************************************/
/* deque iterator                                                             */

typedef struct _mp_obj_deque_it_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    mp_obj_deque_t *deq;
    size_t i;
} mp_obj_deque_it_t;

STATIC mp_obj_t deque_it_iternext(mp_obj_t self_in) {
    mp_obj_deque_it_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->i != self->deq->i_put) {
        mp_obj_t o_out = self->deq->items[self->i];
        self->i = DEQUE_IDX(self->deq, self->i + 1);
        return o_out;
    } else {
        return MP_OBJ_STOP_ITERATION;
    }
}

mp_obj_t deque_getiter(mp_obj_t self_in, mp_obj_iter_buf_t *iter_buf) {
    assert(sizeof(mp_obj_deque_it_t) <= sizeof(mp_obj_iter_buf_t));
    mp_obj_deque_t *self = MP_OBJ_TO_PTR(self_in);
    mp_obj_deque_it_t *o = (mp_obj_deque_it_t *)iter_buf;
    o->base.type = &mp_type_polymorph_iter;
    o->iternext = deque_it_iternext;
    o->deq = self;
    o->i = self->i_get;
    return MP_OBJ_FROM_PTR(o);
}

#endif // MICROPY_PY_COLLECTIONS_DEQUE
