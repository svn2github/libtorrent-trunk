/*

Copyright (c) 2003, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TORRENT_FILE_HPP_INCLUDED
#define TORRENT_FILE_HPP_INCLUDED

#include <memory>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "libtorrent/error_code.hpp"
#include "libtorrent/size_type.hpp"
#include "libtorrent/config.hpp"

#ifdef TORRENT_WINDOWS
// windows part
#include <windows.h>
#include <winioctl.h>
#else
// posix part
#define _FILE_OFFSET_BITS 64

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
namespace libtorrent
{
	namespace fs = boost::filesystem;

	class TORRENT_EXPORT file: public boost::noncopyable
	{
	public:

		enum
		{
			// when a file is opened with no_buffer
			// file offsets have to be aligned to
			// pages and buffer addresses and sizes
			// have to be page aligned too
#ifdef TORRENT_WINDOWS
			read_only = GENERIC_READ,
			write_only = GENERIC_WRITE,
			read_write = GENERIC_READ | GENERIC_WRITE,
			rw_mask = GENERIC_READ | GENERIC_WRITE,
			no_buffer = 1
#else
			read_only = O_RDONLY,
			write_only = O_WRONLY | O_CREAT,
			read_write = O_RDWR | O_CREAT,
			rw_mask = O_RDONLY | O_WRONLY | O_RDWR | O_CREAT,
#if defined O_DIRECT
			no_buffer = O_DIRECT
#else
			no_buffer = O_SYNC
#endif
#endif
		};

#ifdef TORRENT_WINDOWS
		struct iovec_t
		{
			void* iov_base;
			size_t iov_len;
		};
#else
		typedef iovec iovec_t;
#endif

		file();
		file(fs::path const& p, int m, error_code& ec);
		~file();

		bool open(fs::path const& p, int m, error_code& ec);
		bool is_open() const;
		void close();
		bool set_size(size_type size, error_code& ec);

		size_type writev(size_type file_offset, iovec_t const* bufs, int num_bufs, error_code& ec);
		size_type readv(size_type file_offset, iovec_t const* bufs, int num_bufs, error_code& ec);

		size_type get_size(error_code& ec);

	private:

#ifdef TORRENT_WINDOWS
		HANDLE m_file_handle;
#ifdef TORRENT_USE_WPATH
		std::wstring m_path;
#else
		std::string m_path;
#endif
#else
		int m_fd;
#endif
#if defined TORRENT_WINDOWS || defined TORRENT_DEBUG
		void init_file();
		static int m_page_size;
#endif
		int m_open_mode;
	};

}

#endif // TORRENT_FILE_HPP_INCLUDED

