
#include "Coro/IoService.h"
#ifdef _MSC_VER
#include "Coro/FiberWindows.h"
#endif
#ifdef __GNUC__
#include "Coro/FiberLinux.h"
#endif

namespace coro {

thread_local IoService* t_ioService = nullptr;

void IoService::set_current(IoService* io_service) {
	t_ioService = io_service;
}

IoService* IoService::current() {
	if (!t_ioService) {
		throw std::runtime_error("IoService::current() is nullptr");
	}
	return t_ioService;
}

void IoService::run() {
	Fiber::initialize();
	t_ioService = this;
	_impl.run();
	t_ioService = nullptr;
	Fiber::deinitialize();
}

}