
#include "coro/Timer.h"
#include "coro/CoroPool.h"
#include <catch.hpp>

using namespace coro;
using namespace std::chrono_literals;

TEST_CASE("CoroPool::wait", "[CoroPool]") {
	std::vector<uint8_t> result;

	CoroPool pool;

	pool.exec([&]() {
		result.push_back(1);
	});
	pool.exec([&]() {
		result.push_back(2);
	});

	REQUIRE_NOTHROW(pool.waitAll(false));

	REQUIRE(result.size() == 2);
}


TEST_CASE("CoroPool::cancelAll", "[CoroPool]") {
	CoroPool pool;

	pool.exec([] {
		Coro::current()->yield({TokenThrow});
	});
	pool.exec([] {
		Coro::current()->yield({TokenThrow});
	});

	pool.cancelAll();
	REQUIRE_NOTHROW(pool.waitAll(false));
}

TEST_CASE("CoroPool::cancelAll crash", "[CoroPool]") {
	CoroPool pool;
	for (int i = 0; i < 20; i++) {
		pool.exec([] {
			// Nothing
		});
	}
	pool.cancelAll();
	REQUIRE_NOTHROW(pool.waitAll(false));
}

TEST_CASE("WaitOne") {
	int x = 0;
	WaitOne({
		[&] {
			Timer timer;
			timer.waitFor(123s);
			x = 10;
		},
		[&] {
			x = 20;
		}
	});
	REQUIRE(x == 20);
}

TEST_CASE("DO NOT DO IT", "[CoroPool]")
{
	CoroPool inPool;
	CoroPool outPool;
	outPool.exec([&]
	{
		REQUIRE_THROWS(inPool.exec([]
		{
		}));
	});
}
