#include "coro/Acceptor.h"
#include "coro/CoroPool.h"
#include "coro_extra/HttpProtocol.h"
#include "coro_extra/TcpSocket.h"
#include <catch.hpp>

using namespace asio::ip;
using namespace coro;

typedef StreamIterator<TcpSocket> SocketStreamIterator;

TEST_CASE("httpprotocol") {
	auto endpoint = tcp::endpoint(address_v4::from_string("104.23.131.81"), 80);
	auto localendpoint = tcp::endpoint(address_v4::from_string("127.0.0.1"), 16787);

	CoroPool pool;
	bool chunkedDone = false;
	bool postDone = false;

	pool.exec([&] {
		TcpSocket socket;
		socket.connect(endpoint);

		HttpProtocol protocol;
		Buffer inbuf;
		Buffer outbuf;
		Buffer body;
		HttpHeaders headers;
		headers.insert({"User-Agent", "test util"});

		protocol.writeGET("http://www.animenewsnetwork.com/encyclopedia/anime.php?id=18671", headers, outbuf);
		socket.write(outbuf);

		inbuf.reserve(128*1024);
		body.reserve(128*1024);
		protocol.readResponse(socket.iterator(inbuf), socket.iterator(), body);
		REQUIRE(protocol.response().code == 200);

		chunkedDone = true;
	});

	pool.exec([&] {
		Acceptor<tcp> acceptor(localendpoint);
		TcpSocket socket = acceptor.accept();

		Buffer requestBuffer;
		socket.readSome(&requestBuffer);

		Buffer responseBuffer;
		responseBuffer.pushBack("HTTP/1.1 200 OK\r\nServer: A\r\nContent-Length: 17\r\nContent-type: test\r\n\r\nTheResponseString");
		socket.write(responseBuffer);
	});

	pool.exec([&] {
		TcpSocket socket;
		socket.connect(localendpoint);

		HttpProtocol protocol;
		Buffer outbuf;

		Buffer postdata;
		postdata.pushBack("var=1&var2=2");

		protocol.writePOST("http://127.0.0.1:16787/path", HttpHeaders(), postdata.usefulDataSize(), outbuf);
		socket.write(outbuf);
		socket.write(postdata);

		Buffer inbuf;
		inbuf.reserve(128*1024);
		Buffer body;
		body.reserve(128*1024);

		protocol.readResponse(socket.iterator(inbuf), socket.iterator(), body);
		REQUIRE(protocol.response().code == 200);
		REQUIRE(std::string(body.begin(), body.end()) == "TheResponseString");

		postDone = true;
	});

	REQUIRE_NOTHROW(pool.waitAll(false));
	REQUIRE(chunkedDone);
	REQUIRE(postDone);
}

