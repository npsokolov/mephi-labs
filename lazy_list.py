from collections.abc import Iterator, MutableSequence, Iterable


"""
В данном задании требуется реализовать класс LazyList, который представляет собой "ленивый" список.
В отличие от обычного списка, который хранит в памяти все свои элементы, ленивый список может постепенно
"подгружать" новые элементы, когда это необходимо. Это дает возможность представить в виде списка бесконечные
последовательности, но при попытке пробежать по всем значениям такого бесконечного списка мы никогда не сможем
дойти до конца.
В классе LazyList необходимо реализовать основные операции, присущие спискам:
- Доступ к элементам (чтение, вставка, удаление) по индексу и слайсу.
- Добавление элемента в конец (append).
- Расширение списка (extend).
- Проверка на вхождение элемента. Если список бесконечный, а искомого элемента в нем нет, то эта операция никогда не завершится.
- Вычисление длины списка. Если список оказался бесконечным, то эта операция никогда не завершится.
- Разворот списка. Если список оказался бесконечным, то эта операция никогда не завершится.
- Итерация по списку
- Сравнение двух списков между собой. Должно работать в том числе сравнение обычного списка и ленивого.
"""


class LazyList(MutableSequence):
    def __init__(self, items: Iterable):
        self.beg = iter(items)
        self.ended = False
        self.lst = list()

    def insert(self, index: int, value) -> None:
        itr = self.beg
        c = len(self.lst)
        if (c<=index):
            while (c<=index):
                try:
                    x = next(itr)
                except StopIteration:
                    break
                else:
                    self.lst.append(x)
                    c+=1
        self.lst[index]=value

    def append(self, value):
        # while (True):
        #     try:
        #         x = next(self.beg)
        #     except StopIteration:
        #         break
        #     else:
        #         self.lst.append(x)
        self.lst.append(value)

    def extend(self, values: Iterable) -> None:
        # while (True):
        #     try:
        #         x = next(self.beg)
        #     except StopIteration:
        #         break
        #     else:
        #         self.lst.append(x)
        self.lst.extend(values)

    def __getitem__(self, index):
        if (isinstance(index,slice)):
            start = index.start
            if (start is None):
                start = 0
            step = index.step
            if (step is None):
                step = 1
            stop = index.stop
            if (stop is None):
                stop = len(self)
            s = range(start,stop,step)
            ans = list()
            for x in s:
                ans.append(self.__getitem__(x))
            return ans
        else:
            itr = self.beg
            c = len(self.lst)
            if (c<=index):
                while (c<=index):
                    try:
                        x = next(itr)
                    except StopIteration:
                        break
                    else:
                        self.lst.append(x)
                        c+=1
            return self.lst.__getitem__(index)

    def __setitem__(self, index, value) -> None:
        self.__delitem__(index)
        if (isinstance(index,slice)):
            start = index.start
            if (start is None):
                start = 0
            for x in value:
                self.lst.insert(start,x)
                start+=1
        else:
            self.lst.insert(index,value)
    def __delitem__(self, index) -> None:
        if (isinstance(index,slice)):
            start = index.start
            if (start is None):
                start = 0
            step = index.step
            if (step is None):
                step = 1
            stop = index.stop
            if (stop is None):
                stop = len(self)
            s = range(((stop-1)//step)*step,start-1,-step)
            for x in s:
                self.__delitem__(x)
        else:
            itr = self.beg
            c = len(self.lst)
            if (c<=index):
                while (c<=index):
                    try:
                        x = next(itr)
                    except StopIteration:
                        break
                    else:
                        self.lst.append(x)
                        c+=1
            self.lst.__delitem__(index)

    def __contains__(self, x) -> bool:
        if (self.lst.__contains__(x)):
            return True
        y = None
        while(y!=x):
            try:
                y = next(self.beg)
            except StopIteration:
                return False
            else:
                self.lst.append(y)
        return True

    def __len__(self) -> int:
        itr = self.beg
        while (True):
            try:
                x = next(itr)
            except StopIteration:
                break
            else:
                self.lst.append(x)
        return len(self.lst)

    def __reversed__(self) -> Iterator:
        len(self)
        return self.lst.__reversed__()

    def __iter__(self) -> Iterator:
        return self.beg

    def __eq__(self, other):
        if (len(other)!=len(self)):
            return False
        else:
            return other==self.lst


def test_lazify_list():
    lst = [1, 2, 3, 4, 5]

    lazy_list = LazyList(lst)

    assert lst == lazy_list
    assert len(lst) == len(lazy_list)
    assert list(reversed(lst)) == list(reversed(lazy_list))
    assert 4 in lazy_list

    lst.append(100)
    lazy_list.append(100)
    assert lst == lazy_list

    lst.extend([1, 2, 3])
    lazy_list.extend([1, 2, 3])
    assert lst == lazy_list

    del lazy_list[1]
    assert lazy_list == [1, 3, 4, 5, 100, 1, 2, 3]
    assert lazy_list[:2] == [1, 3]


def factorials():
    value = 1
    i = 1
    while True:
        yield value
        value *= i
        i += 1


def test_infinite_list():
    all_factorials = LazyList(factorials())
    assert all_factorials[4:7] == [24, 120, 720]
    assert all_factorials[:7] == [1, 1, 2, 6, 24, 120, 720]
    assert all_factorials[7] == 5040
    del all_factorials[6]
    assert all_factorials[6] == 5040
    all_factorials[:6] = [1]
    assert all_factorials[:2] == [1, 5040]
    del all_factorials[:2]
    assert all_factorials[:2] == [40320, 362880]
    all_factorials.append(100500)
    all_factorials.extend([1, 2, 3])
    assert any(factorial > 100000000 for factorial in all_factorials)
    assert not all(factorial < 500000 for factorial in all_factorials)
    assert 1307674368000 in all_factorials


def main():
    test_lazify_list()
    test_infinite_list()


if __name__ == '__main__':
    main()