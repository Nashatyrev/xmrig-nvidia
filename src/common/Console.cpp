/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2016-2018 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "common/Console.h"
#include "interfaces/IConsoleListener.h"


Console::Console(IConsoleListener *listener)
    : m_listener(listener)
{
	m_tty.data = this;
	uv_stream_t* stream;

	switch (uv_guess_handle(0)) {
		case UV_TTY: {
			auto tty = new uv_tty_t();
			tty->data = this;
			uv_tty_init(uv_default_loop(), tty, 0, 1);
			uv_tty_set_mode(tty, UV_TTY_MODE_RAW);
			stream = reinterpret_cast<uv_stream_t*>(tty);
			break;
		}
		case UV_NAMED_PIPE: {
			auto pipe = new uv_pipe_t();
			pipe->data = this;
			uv_pipe_init(uv_default_loop(), pipe, 0);
			uv_pipe_open(pipe, 0);
			stream = reinterpret_cast<uv_stream_t*>(pipe);
			break;
		}
		default:
			printf("Unable to determine input type %d\n", uv_guess_handle(0));
	}

    if (!uv_is_readable(stream)) {
        return;
    }

    uv_read_start(stream, Console::onAllocBuffer, Console::onRead);
}


void Console::onAllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    auto console = static_cast<Console*>(handle->data);
    buf->len  = 1;
    buf->base = console->m_buf;
}


void Console::onRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0) {
        return uv_close(reinterpret_cast<uv_handle_t*>(stream), nullptr);
    }

    if (nread == 1) {
        static_cast<Console*>(stream->data)->m_listener->onConsoleCommand(buf->base[0]);
    }
}
