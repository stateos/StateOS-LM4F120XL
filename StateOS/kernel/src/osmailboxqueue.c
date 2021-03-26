/******************************************************************************

    @file    StateOS: osmailboxqueue.c
    @author  Rajmund Szymanski
    @date    26.12.2020
    @brief   This file provides set of functions for StateOS.

 ******************************************************************************

   Copyright (c) 2018-2020 Rajmund Szymanski. All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.

 ******************************************************************************/

#include "inc/osmailboxqueue.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_box_init( box_t *box, size_t size, void *data, size_t bufsize, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(box, 0, sizeof(box_t));

	core_obj_init(&box->obj, res);

	box->limit = (bufsize / size) * size;
	box->size  = size;
	box->data  = data;
}

/* -------------------------------------------------------------------------- */
void box_init( box_t *box, size_t size, void *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(box);
	assert(size);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		priv_box_init(box, size, data, bufsize, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
box_t *box_create( unsigned limit, size_t size )
/* -------------------------------------------------------------------------- */
{
	struct box_T { box_t box; char buf[]; } *tmp;
	box_t *box = NULL;
	size_t bufsize;

	assert_tsk_context();
	assert(limit);
	assert(size);

	sys_lock();
	{
		bufsize = limit * size;
		tmp = malloc(sizeof(struct box_T) + bufsize);
		if (tmp)
			priv_box_init(box = &tmp->box, size, tmp->buf, bufsize, tmp);
	}
	sys_unlock();

	return box;
}

/* -------------------------------------------------------------------------- */
static
void priv_box_reset( box_t *box, int event )
/* -------------------------------------------------------------------------- */
{
	box->count = 0;
	box->head  = 0;
	box->tail  = 0;

	core_all_wakeup(box->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void box_reset( box_t *box )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);

	sys_lock();
	{
		priv_box_reset(box, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void box_destroy( box_t *box )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);

	sys_lock();
	{
		priv_box_reset(box, box->obj.res ? E_DELETED : E_STOPPED);
		core_res_free(&box->obj);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_box_get( box_t *box, char *data )
/* -------------------------------------------------------------------------- */
{
	size_t size = box->size;
	size_t i = box->head;

	while (size--)
		*data++ = box->data[i++];
	box->head = (i < box->limit) ? i : 0;
	box->count -= box->size;
}

/* -------------------------------------------------------------------------- */
static
void priv_box_put( box_t *box, const char *data )
/* -------------------------------------------------------------------------- */
{
	size_t size = box->size;
	size_t i = box->tail;

	while (size--)
		box->data[i++] = *data++;
	box->tail = (i < box->limit) ? i : 0;
	box->count += box->size;
}

/* -------------------------------------------------------------------------- */
static
void priv_box_skip( box_t *box )
/* -------------------------------------------------------------------------- */
{
	size_t i = box->head + box->size;

	box->head = (i < box->limit) ? i : 0;
	box->count -= box->size;
}

/* -------------------------------------------------------------------------- */
static
void priv_box_getUpdate( box_t *box, char *data )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	priv_box_get(box, data);
	tsk = core_one_wakeup(box->obj.queue, E_SUCCESS);
	if (tsk) priv_box_put(box, tsk->tmp.box.data.out);
}

/* -------------------------------------------------------------------------- */
static
void priv_box_putUpdate( box_t *box, const char *data )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	priv_box_put(box, data);
	tsk = core_one_wakeup(box->obj.queue, E_SUCCESS);
	if (tsk) priv_box_get(box, tsk->tmp.box.data.in);
}

/* -------------------------------------------------------------------------- */
static
void priv_box_skipUpdate( box_t *box )
/* -------------------------------------------------------------------------- */
{
	tsk_t *tsk;

	while (box->count == box->limit)
	{
		priv_box_skip(box);
		tsk = core_one_wakeup(box->obj.queue, E_SUCCESS);
		if (tsk) priv_box_put(box, tsk->tmp.box.data.out);
	}
}

/* -------------------------------------------------------------------------- */
static
int priv_box_take( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	if (box->count > 0)
	{
		priv_box_getUpdate(box, data);
		return E_SUCCESS;
	}

	return E_TIMEOUT;
}

/* -------------------------------------------------------------------------- */
int box_take( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_take(box, data);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int box_waitFor( box_t *box, void *data, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_take(box, data);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.box.data.in = data;
			result = core_tsk_waitFor(&box->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int box_waitUntil( box_t *box, void *data, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_take(box, data);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.box.data.in = data;
			result = core_tsk_waitUntil(&box->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_box_give( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	if (box->count < box->limit)
	{
		priv_box_putUpdate(box, data);
		return E_SUCCESS;
	}

	return E_TIMEOUT;
}

/* -------------------------------------------------------------------------- */
int box_give( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_give(box, data);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int box_sendFor( box_t *box, const void *data, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_give(box, data);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.box.data.out = data;
			result = core_tsk_waitFor(&box->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int box_sendUntil( box_t *box, const void *data, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		result = priv_box_give(box, data);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.box.data.out = data;
			result = core_tsk_waitUntil(&box->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
void box_push( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		priv_box_skipUpdate(box);
		priv_box_putUpdate(box, data);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned box_count( box_t *box )
/* -------------------------------------------------------------------------- */
{
	unsigned count;

	assert(box);
	assert(box->obj.res!=RELEASED);

	sys_lock();
	{
		count = box->count / box->size;
	}
	sys_unlock();

	return count;
}

/* -------------------------------------------------------------------------- */
unsigned box_space( box_t *box )
/* -------------------------------------------------------------------------- */
{
	unsigned space;

	assert(box);
	assert(box->obj.res!=RELEASED);

	sys_lock();
	{
		space = (box->limit - box->count) / box->size;
	}
	sys_unlock();

	return space;
}

/* -------------------------------------------------------------------------- */
unsigned box_limit( box_t *box )
/* -------------------------------------------------------------------------- */
{
	unsigned limit;

	assert(box);
	assert(box->obj.res!=RELEASED);

	sys_lock();
	{
		limit = box->limit / box->size;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */

#if OS_ATOMICS

/* -------------------------------------------------------------------------- */
static
void priv_box_getAsync( box_t *box, char *data )
/* -------------------------------------------------------------------------- */
{
	size_t size = box->size;
	size_t i = box->head;

	while (size--)
		*data++ = box->data[i++];
	box->head = (i < box->limit) ? i : 0;
	atomic_fetch_sub((atomic_uint *)&box->count, box->size);
}

/* -------------------------------------------------------------------------- */
int box_takeAsync( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	int result = E_TIMEOUT;

	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		if (atomic_load((atomic_uint *)&box->count) > 0)
		{
			priv_box_getAsync(box, data);
			result = E_SUCCESS;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int box_waitAsync( box_t *box, void *data )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();

	while (box_takeAsync(box, data) != E_SUCCESS)
		core_ctx_switch();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
static
void priv_box_putAsync( box_t *box, const char *data )
/* -------------------------------------------------------------------------- */
{
	size_t size = box->size;
	size_t i = box->tail;

	while (size--)
		box->data[i++] = *data++;
	box->tail = (i < box->limit) ? i : 0;
	atomic_fetch_add((atomic_uint *)&box->count, box->size);
}

/* -------------------------------------------------------------------------- */
int box_giveAsync( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	int result = E_TIMEOUT;

	assert(box);
	assert(box->obj.res!=RELEASED);
	assert(box->data);
	assert(box->limit);
	assert(data);

	sys_lock();
	{
		if (atomic_load((atomic_uint *)&box->count) < box->limit)
		{
			priv_box_putAsync(box, data);
			result = E_SUCCESS;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int box_sendAsync( box_t *box, const void *data )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();

	while (box_giveAsync(box, data) != E_SUCCESS)
		core_ctx_switch();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */

#endif//OS_ATOMICS
