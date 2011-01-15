/*
suinput - Simple C-API to the Linux uinput-system.
Copyright (C) 2009 Tuomas Räsänen <tuos@codegrove.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SUINPUT_H
#define SUINPUT_H
#include <stdint.h>

#include <linux/input.h>
#include <linux/uinput.h>

int suinput_write(int uinput_fd,
                         uint16_t type, uint16_t code, int32_t value);
/*
  Creates and opens a connection to the event device. Returns an uinput file
  descriptor on success. On error, -1 is returned, and errno is set
  appropriately.
*/
int suinput_open(const char* device_name, const struct input_id* id);

/* 
  Destroys and closes a connection to the event device. Returns 0 on success.
  On error, -1 is returned, and errno is set appropriately.
   
  Behaviour is undefined when passed a file descriptor not returned by
  suinput_open().
*/
int suinput_close(int uinput_fd);

/*
  Sends a relative pointer motion event to the event device. Values increase
  towards right-bottom. Returns 0 on success. On error, -1 is returned, and
  errno is set appropriately.

  Behaviour is undefined when passed a file descriptor not returned by
  suinput_open().
*/
int suinput_move_pointer(int uinput_fd, int32_t x, int32_t y);
int suinput_set_pointer(int uinput_fd, int32_t x, int32_t y);

/*
  Sends a press event to the event device. Event is repeated after
  a short delay until a release event is sent. Returns 0 on success.
  On error, -1 is returned, and errno is set appropriately.

  Behaviour is undefined when passed a file descriptor not returned by
  suinput_open().

  All possible values of `code` are defined in linux/input.h prefixed
  by KEY_ or BTN_.
*/
int suinput_press(int uinput_fd, uint16_t code);

/*
  Sends a release event to the event device. Returns 0 on success.
  On error, -1 is returned, and errno is set appropriately.

  Behaviour is undefined when passed a file descriptor not returned by
  suinput_open().

  All possible values of `code` are defined in linux/input.h prefixed
  by KEY_ or BTN_.
*/
int suinput_release(int uinput_fd, uint16_t code);

/*
  Sends a press and release events to the event device. Returns 0 on
  success. On error, -1 is returned, and errno is set appropriately.

  Behaviour is undefined when passed a file descriptor not returned by
  suinput_open().

  All possible values of `code` are defined in linux/input.h prefixed
  by KEY_ or BTN_.

  This function is provided as a convenience and has effectively the
  same result as calling suinput_press() and suinput_release() sequentially.
*/
int suinput_click(int uinput_fd, uint16_t code);

#endif /* SUINPUT_H */
