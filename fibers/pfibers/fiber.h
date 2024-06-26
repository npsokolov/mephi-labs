#pragma once

// Инициализирует библиотеку
// Функция должна быть вызвана до первгого вызова FiberSpawn
void FiberInit();

// Создаёт новый fiber и помещает его в конец очереди исполнения
//
// Input:
//   - func, data - аналогично аргументам pthread_create
void FiberSpawn(void (*func)(void*), void* data);

// Останавливает текущий fiber, помещает его в конце очереди исполнения
// Запускает первый fiber из очереди исполнения
void FiberYield();

// Возвращает 1, если текущий fiber единственный, иначе 0
int FiberTryJoin();
