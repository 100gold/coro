
#include <boost/test/unit_test.hpp>
#include "FixedSizedMemoryPool.h"
#include "DynamicMemoryPool.h"
#include "CoroPool.h"


BOOST_AUTO_TEST_SUITE(SuiteMemoryPool)


BOOST_AUTO_TEST_CASE(TestFixedSizedMemoryPool) {
	std::vector<uint8_t> actual, expected = {1, 2, 3, 4};

	FixedSizedMemoryPool pool(2, 1000);

	BOOST_REQUIRE(pool.size() == 2);

	auto a = pool.makeUnique();
	auto b = pool.makeShared();

	actual.push_back(1);

	Fork([&, copy = std::move(b)]() {
		actual.push_back(3);
	});

	actual.push_back(2);
	auto c = pool.makeUnique();
	actual.push_back(4);

	BOOST_REQUIRE(pool.size() == 2);
	BOOST_REQUIRE(actual == expected);
}


BOOST_AUTO_TEST_CASE(TestDynamicMemoryPool) {
	DynamicMemoryPool pool(1000);

	BOOST_REQUIRE(pool.size() == 0);

	auto a = pool.makeUnique();
	auto b = pool.makeShared();

	BOOST_REQUIRE(pool.size() == 2);
}


BOOST_AUTO_TEST_SUITE_END()