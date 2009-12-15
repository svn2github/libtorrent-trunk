/*

Copyright (c) 2008, Arvid Norberg
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

#include "libtorrent/session.hpp"
#include "libtorrent/kademlia/node.hpp" // for verify_message
#include "libtorrent/bencode.hpp"
#include <iostream>

#include "test.hpp"

using namespace libtorrent;

int dht_port = 48199;

void send_dht_msg(datagram_socket& sock, char const* msg, lazy_entry* reply, char const* t = "10")
{
	entry e;
	e["q"] = msg;
	e["t"] = t;
	e["y"] = "q";
	entry::dictionary_type& a = e["a"].dict();
	a["id"] = "00000000000000000000";
	char msg_buf[1500];
	int size = bencode(msg_buf, e);

	error_code ec;
	sock.send_to(asio::buffer(msg_buf, size)
		, udp::endpoint(address::from_string("127.0.0.1"), dht_port), 0, ec);
	TEST_CHECK(!ec);
	if (ec) std::cout << ec.message() << std::endl;

	static char inbuf[1500];
	udp::endpoint ep;
	size = sock.receive_from(asio::buffer(inbuf, sizeof(inbuf)), ep, 0, ec);
	TEST_CHECK(!ec);
	if (ec) std::cout << ec.message() << std::endl;

	int ret = lazy_bdecode(inbuf, inbuf + size, *reply);
	TEST_CHECK(ret == 0);
}

int test_main()
{
	session ses(fingerprint("LT", 0, 1, 0, 0), std::make_pair(dht_port, 49000));

	// DHT should be running on port 48199 now

	io_service ios;
	error_code ec;
	datagram_socket sock(ios);

	sock.open(udp::v4(), ec);
	TEST_CHECK(!ec);
	if (ec) std::cout << ec.message() << std::endl;

	lazy_entry response;
	lazy_entry const* parsed[5];
	char error_string[200];
	bool ret;

	// ====== ping ======

	send_dht_msg(sock, "ping", &response, "10");

	dht::key_desc_t pong_desc[] = {
		{"y", lazy_entry::string_t, 1, 0},
		{"t", lazy_entry::string_t, 2, 0},
	};

	ret = dht::verify_message(&response, pong_desc, parsed, 2, error_string, sizeof(error_string));
	TEST_CHECK(ret);
	if (ret)
	{
		TEST_CHECK(parsed[0]->string_value() == "r");
		TEST_CHECK(parsed[1]->string_value() == "10");
	}

	// ====== invalid message ======

	send_dht_msg(sock, "find_node", &response, "10");

	dht::key_desc_t err_desc[] = {
		{"y", lazy_entry::string_t, 1, 0},
		{"e", lazy_entry::list_t, 0, 0},
	};

	ret = dht::verify_message(&response, err_desc, parsed, 2, error_string, sizeof(error_string));
	TEST_CHECK(ret);
	if (ret)
	{
		TEST_CHECK(parsed[0]->string_value() == "e");
		TEST_CHECK(parsed[1]->list_size() >= 2);
		if (parsed[1]->list_size() >= 2
			&& parsed[1]->list_at(0)->type() == lazy_entry::int_t
			&& parsed[1]->list_at(1)->type() == lazy_entry::string_t)
		{
			TEST_CHECK(parsed[1]->list_at(1)->string_value() == "missing 'target' key");
		}
		else
		{
			TEST_ERROR("invalid error response");
		}
	}

	return 0;
}

