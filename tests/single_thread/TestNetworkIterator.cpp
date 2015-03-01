
#include <boost/test/unit_test.hpp>
#include "TcpServer.h"
#include "NetworkIterator.h"
#include "CoroUtils.h"


static auto endpoint = TcpEndpoint(IPv4Address::from_string("127.0.0.1"), 44442);
static Buffer test_data({ 0x01, 0x02, 0x03, 0x04 });
static Buffer test_data2({ 0x05, 0x06, 0x07, 0x08 });


BOOST_AUTO_TEST_SUITE(SuiteNetworkIterator)


BOOST_AUTO_TEST_CASE(TestSTL) {
	bool serverDone = false, clientDone = false;

	Parallel({
		[&]() {
			TcpServer server(endpoint);
			TcpSocket socket = server.accept();
			socket.sendData(test_data);
			socket.sendData(test_data2);
			serverDone = true;
		},
		[&]() {
			TcpSocket socket;
			socket.connect(endpoint);

			Buffer buffer(1000);

			BOOST_REQUIRE(std::equal(
				NetworkIterator(socket, buffer),
				NetworkIterator(socket, buffer) + 4,
				test_data.begin()
			));

			BOOST_REQUIRE(std::equal(
				test_data2.begin(),
				test_data2.end(),
				NetworkIterator(socket, buffer) + 4
			));

			BOOST_REQUIRE(buffer.usefulDataSize() == 8);

			clientDone = true;
		}
	}, [](const std::exception& exception) {
		BOOST_REQUIRE(false);
	});

	BOOST_REQUIRE(serverDone && clientDone);
}


BOOST_AUTO_TEST_CASE(TestRangeError) {
	TcpSocket socket;

	Buffer buffer(4);

	BOOST_REQUIRE_NO_THROW(NetworkIterator(socket, buffer) + 0);
	BOOST_REQUIRE_NO_THROW(NetworkIterator(socket, buffer) + 4);
	BOOST_REQUIRE_THROW(NetworkIterator(socket, buffer) + 4 + 1, std::range_error);
	BOOST_REQUIRE_THROW(NetworkIterator(socket, buffer) + 5, std::range_error);
	BOOST_REQUIRE_THROW(NetworkIterator(socket, buffer) + 8, std::range_error);

	buffer.pushBack(test_data.begin(), test_data.end());

	BOOST_REQUIRE_NO_THROW(NetworkIterator(socket, buffer) + 0);
	BOOST_REQUIRE_NO_THROW(NetworkIterator(socket, buffer) + 4);
	BOOST_REQUIRE_THROW(NetworkIterator(socket, buffer) + 4 + 1, std::range_error);
	BOOST_REQUIRE_THROW(NetworkIterator(socket, buffer) + 5, std::range_error);
	BOOST_REQUIRE_THROW(NetworkIterator(socket, buffer) + 8, std::range_error);
}

BOOST_AUTO_TEST_CASE(TestCastToBufferIterator) {
	TcpSocket socket;

	Buffer buffer(4);
	auto it = NetworkIterator(socket, buffer);
	buffer.pushBack(2);
	BOOST_REQUIRE(*(it + 1) == *(buffer.begin() + 1));
	BOOST_REQUIRE(buffer.end() == (it + 2));
}


BOOST_AUTO_TEST_SUITE_END()
