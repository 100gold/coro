
#include <boost/test/unit_test.hpp>
#include "TcpServer.h"


extern ThreadPool g_threadPool;


BOOST_AUTO_TEST_SUITE(SuiteTcp)


BOOST_AUTO_TEST_CASE(TestTransferData) {
	Buffer test_data({ 0x01, 0x02, 0x03, 0x04 });
	auto endpoint = TcpEndpoint(IPv4Address::from_string("127.0.0.1"), 44442);

	bool serverDone = false, clientDone = false;

	g_threadPool.parallel({
		[&]() {
			TcpServer server(g_threadPool, endpoint);
			TcpSocket socket = server.accept();
			Buffer data(4);
			socket.receiveData(&data);
			BOOST_REQUIRE(data == test_data);
			socket.sendData(&data);
			BOOST_REQUIRE(data.usefulDataSize() == 0);
			serverDone = true;
		},
		[&]() {

			TcpSocket socket(g_threadPool);
			socket.connect(endpoint);
			Buffer data({ 0x01, 0x02, 0x03, 0x04 });
			socket.sendData(&data);
			BOOST_REQUIRE(data.usefulDataSize() == 0);
			socket.receiveData(&data);
			BOOST_REQUIRE(data == test_data);
			clientDone = true;
		}
	}, [](const std::exception& exception) {
		BOOST_REQUIRE(false);
	});

	BOOST_REQUIRE(serverDone && clientDone);
}


BOOST_AUTO_TEST_SUITE_END()
