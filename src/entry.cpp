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

#include "libtorrent/entry.hpp"

#if defined(_MSC_VER)
namespace std
{
	using ::isprint;
}
#define for if (false) {} else for
#endif

namespace
{
	template <class T>
	void call_destructor(T* o)
	{
		o->~T();
	}
}

void libtorrent::entry::construct(data_type t)
{
	m_type = t;
	switch(m_type)
	{
	case int_t:
		new(data) integer_type;
		break;
	case string_t:
		new(data) string_type;
		break;
	case list_t:
		new(data) list_type;
		break;
	case dictionary_t:
		new (data) dictionary_type;
		break;
	default:
		m_type = undefined_t;
	}
}

void libtorrent::entry::copy(const entry& e)
{
	m_type = e.m_type;
	switch(m_type)
	{
	case int_t:
		new(data) integer_type(e.integer());
		break;
	case string_t:
		new(data) string_type(e.string());
		break;
	case list_t:
		new(data) list_type(e.list());
		break;
	case dictionary_t:
		new (data) dictionary_type(e.dict());
		break;
	default:
		m_type = undefined_t;
	}
}

void libtorrent::entry::destruct()
{
	switch(m_type)
	{
	case int_t:
		call_destructor(reinterpret_cast<integer_type*>(data));
		break;
	case string_t:
		call_destructor(reinterpret_cast<string_type*>(data));
		break;
	case list_t:
		call_destructor(reinterpret_cast<list_type*>(data));
		break;
	case dictionary_t:
		call_destructor(reinterpret_cast<dictionary_type*>(data));
		break;
	default:
		break;
	}
}

void libtorrent::entry::print(std::ostream& os, int indent) const
{
	for (int i = 0; i < indent; ++i) os << " ";
	switch (m_type)
	{
	case int_t:
		os << integer() << "\n";
		break;
	case string_t:
		{
			bool binary_string = false;
			for (std::string::const_iterator i = string().begin(); i != string().end(); ++i)
			{
				if (!std::isprint(static_cast<unsigned char>(*i)))
				{
					binary_string = true;
					break;
				}
			}
			if (binary_string)
			{
				os.unsetf(std::ios_base::dec);
				os.setf(std::ios_base::hex);
				for (std::string::const_iterator i = string().begin(); i != string().end(); ++i)
					os << static_cast<unsigned int>((unsigned char)*i);
				os.unsetf(std::ios_base::hex);
				os.setf(std::ios_base::dec);
				os << "\n";
			}
			else
			{
				os << string() << "\n";
			}
		} break;
	case list_t:
		{
			os << "list\n";
			for (list_type::const_iterator i = list().begin(); i != list().end(); ++i)
			{
				i->print(os, indent+1);
			}
		} break;
	case dictionary_t:
		{
			os << "dictionary\n";
			for (dictionary_type::const_iterator i = dict().begin(); i != dict().end(); ++i)
			{
				for (int j = 0; j < indent+1; ++j) os << " ";
				os << "[" << i->first << "]";
				if (i->second.type() != entry::string_t && i->second.type() != entry::int_t) os << "\n";
				else os << " ";
				i->second.print(os, indent+2);
			}
		} break;
	default:
		os << "<uninitialized>\n";
	}
}
