
#pragma once

#include "Coro/Coro.h"
#include <set>

namespace coro {

/// Класс для иерархического управления корутинами в пределах текущего Strand
class CoroPool {
public:
	/// Блокирует поток выполнения до тех пор, пока не завершатся все дочерние корутины
	~CoroPool();
	/// Запустить новую корутину в текущем Strand
	Coro* exec(std::function<void()> routine);
	/// Дождаться завершения всех дочерних корутин
	void waitAll(bool noThrow);
	/// Отменить все дочерние корутины
	void cancelAll();

private:
	void onCoroDone(Coro* coro);

	std::string token() const;

	Coro* _parentCoro = Coro::current();
	std::set<Coro*> _childCoros;
};

}