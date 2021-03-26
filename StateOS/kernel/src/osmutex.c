/******************************************************************************

    @file    StateOS: osmutex.c
    @author  Rajmund Szymanski
    @date    03.07.2020
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

#include "inc/osmutex.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_mtx_init( mtx_t *mtx, unsigned mode, unsigned prio, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(mtx, 0, sizeof(mtx_t));

	core_obj_init(&mtx->obj, res);

	mtx->mode = mode;
	mtx->prio = prio;
}

/* -------------------------------------------------------------------------- */
void mtx_init( mtx_t *mtx, unsigned mode, unsigned prio )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(mtx);
	assert((mtx->mode & ~mtxMASK) == 0);
	assert((mtx->mode &  mtxTypeMASK) != mtxTypeMASK);
	assert((mtx->mode &  mtxPrioMASK) != mtxPrioMASK);

	sys_lock();
	{
		priv_mtx_init(mtx, mode, prio, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
mtx_t *mtx_create( unsigned mode, unsigned prio )
/* -------------------------------------------------------------------------- */
{
	mtx_t *mtx;

	assert_tsk_context();

	sys_lock();
	{
		mtx = malloc(sizeof(mtx_t));
		if (mtx)
			priv_mtx_init(mtx, mode, prio, mtx);
	}
	sys_unlock();

	return mtx;
}

/* -------------------------------------------------------------------------- */
void mtx_reset( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(mtx);
	assert(mtx->obj.res!=RELEASED);

	sys_lock();
	{
		core_mtx_reset(mtx, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void mtx_destroy( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(mtx);
	assert(mtx->obj.res!=RELEASED);

	sys_lock();
	{
		core_mtx_reset(mtx, mtx->obj.res ? E_DELETED : E_STOPPED);
		core_res_free(&mtx->obj);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void mtx_setPrio( mtx_t *mtx, unsigned prio )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(mtx);
	assert(mtx->obj.res!=RELEASED);

	sys_lock();
	{
		mtx->prio = prio;
		if ((mtx->mode & mtxPrioMASK) == mtxPrioProtect)
			while (mtx->obj.queue && mtx->obj.queue->prio > prio)
				core_one_wakeup(mtx->obj.queue, E_FAILURE);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned mtx_getPrio( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	unsigned prio;

	assert_tsk_context();
	assert(mtx);
	assert(mtx->obj.res!=RELEASED);

	sys_lock();
	{
		prio = mtx->prio;
	}
	sys_unlock();

	return prio;
}

/* -------------------------------------------------------------------------- */
static
int priv_mtx_take( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	if ((mtx->mode & mtxPrioMASK) == mtxPrioProtect && mtx->prio < System.cur->prio)
		return E_FAILURE;

	if (mtx->owner == NULL)
	{
		assert(mtx->count == 0);

		core_mtx_link(mtx, System.cur);

		if ((mtx->mode & mtxInconsistent))
		{
			mtx->mode &= ~mtxInconsistent;
			return OWNERDEAD;
		}

		return E_SUCCESS;
	}

	if ((mtx->mode & mtxTypeMASK) == mtxNormal || mtx->owner != System.cur)
		return E_TIMEOUT;

	if ((mtx->mode & mtxTypeMASK) == mtxRecursive && mtx->count < MTX_LIMIT)
	{
		mtx->count++;
		return E_SUCCESS;
	}

	return E_FAILURE;
}

/* -------------------------------------------------------------------------- */
int mtx_take( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(mtx);
	assert(mtx->obj.res!=RELEASED);
	assert((mtx->mode & ~mtxMASK) == 0);
	assert((mtx->mode &  mtxTypeMASK) != mtxTypeMASK);
	assert((mtx->mode &  mtxPrioMASK) != mtxPrioMASK);

	sys_lock();
	{
		result = priv_mtx_take(mtx);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int mtx_waitFor( mtx_t *mtx, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(mtx);
	assert(mtx->obj.res!=RELEASED);
	assert((mtx->mode & ~mtxMASK) == 0);
	assert((mtx->mode &  mtxTypeMASK) != mtxTypeMASK);
	assert((mtx->mode &  mtxPrioMASK) != mtxPrioMASK);

	sys_lock();
	{
		result = priv_mtx_take(mtx);
		if (result == E_TIMEOUT)
		{
			if ((mtx->mode & mtxPrioMASK) != mtxPrioNone && mtx->owner->prio < System.cur->prio)
				core_tsk_prio(mtx->owner, System.cur->prio);

			System.cur->mtx.tree = mtx;
			result = core_tsk_waitFor(&mtx->obj.queue, delay);
			System.cur->mtx.tree = NULL;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int mtx_waitUntil( mtx_t *mtx, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(mtx);
	assert(mtx->obj.res!=RELEASED);
	assert((mtx->mode & ~mtxMASK) == 0);
	assert((mtx->mode &  mtxTypeMASK) != mtxTypeMASK);
	assert((mtx->mode &  mtxPrioMASK) != mtxPrioMASK);

	sys_lock();
	{
		result = priv_mtx_take(mtx);
		if (result == E_TIMEOUT)
		{
			if ((mtx->mode & mtxPrioMASK) != mtxPrioNone && mtx->owner->prio < System.cur->prio)
				core_tsk_prio(mtx->owner, System.cur->prio);

			System.cur->mtx.tree = mtx;
			result = core_tsk_waitUntil(&mtx->obj.queue, time);
			System.cur->mtx.tree = NULL;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_mtx_give( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	if ((mtx->mode & (mtxTypeMASK + mtxRobust)) == mtxNormal || mtx->owner == System.cur)
	{
		if (mtx->count > 0)
		{
			mtx->count--;
			return E_SUCCESS;
		}

		core_mtx_transferLock(mtx, E_SUCCESS);
		return E_SUCCESS;
	}

	return E_FAILURE;
}

/* -------------------------------------------------------------------------- */
int mtx_give( mtx_t *mtx )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(mtx);
	assert(mtx->obj.res!=RELEASED);
	assert((mtx->mode & ~mtxMASK) == 0);
	assert((mtx->mode &  mtxTypeMASK) != mtxTypeMASK);
	assert((mtx->mode &  mtxPrioMASK) != mtxPrioMASK);

	sys_lock();
	{
		result = priv_mtx_give(mtx);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
