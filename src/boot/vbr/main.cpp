#if 0
	  This file is a part of StOS project - a small operating system
	  	made for learning purposes
	  Copyright (C) 2021 Egorov Stanislav, kodirovsshik@mail.ru, kodirovsshik@gmail.com

	  This program is free software: you can redistribute it and/or modify
	  it under the terms of the GNU General Public License as published by
	  the Free Software Foundation, either version 3 of the License, or
	  (at your option) any later version.

	  This program is distributed in the hope that it will be useful,
	  but WITHOUT ANY WARRANTY; without even the implied warranty of
	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	  GNU General Public License for more details.

	  You should have received a copy of the GNU General Public License
	  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#endif



#include <stddef.h>

#include "multiloader.h"
#include "vbr.h"
#include "aux.h"



using request_handler_t = uint32_t (*)(stos_request_header_t*);

static const request_handler_t request_handlers[] =
{
	nullptr,
	rh_get_boot_options,
	rh_boot,
};


uint32_t main(stos_request_header_t* req)
{
	if (req == nullptr)
	{
		native_boot();
	}
	else
	{
		if (req->struct_size < 10)
			return STOS_REQ_INVOKE_ERR_INVALID_REQUEST;
		if (req->request_number == 0 || req->request_number >= countof(request_handlers))
			return STOS_REQ_INVOKE_ERR_INVALID_REQUEST_ID;
		return request_handlers[req->request_number](req);
	}
}
