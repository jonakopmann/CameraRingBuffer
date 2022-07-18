/* RingBuffer.h
 *
 * Copyright 2022 jona <unknown@domain.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <glib-2.0/glib-object.h>
#include <stdio.h>

G_BEGIN_DECLS

#define RING_TYPE_BUFFER (ring_buffer_get_type())

G_DECLARE_FINAL_TYPE (RingBuffer, ring_buffer, RING, BUFFER, GObject)

struct _RingBuffer
{
  gpointer buffer;
  gpointer head;
  gpointer end;
  gsize capacity;
  gsize itemSize;
  gsize index;
};

RingBuffer* ring_buffer_new (gsize capacity, gsize itemSize);

gboolean ring_buffer_add    (RingBuffer* self, const gpointer item);

void ring_buffer_advance    (RingBuffer* self, gsize count);

gpointer ring_buffer_get    (RingBuffer* self);

void ring_buffer_dispose    (RingBuffer* self);

G_END_DECLS

#endif
