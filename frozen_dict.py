from collections.abc import Mapping, Hashable, Sized, Container
from typing import Any
from copy import deepcopy
"""
В данном задании требуется написать класс `FrozenDict`, который представляет собой словарь "ключ-значение",
но в отличие от типа `dict` является неизменяемым, а также является хешируемым.

У типа `list` есть хешируемый неизменяемый аналог `tuple`, у `set`'а есть `frozenset`, а у `dict`'а нет такого аналога,
его вам предстоит создать самостоятельно.
"""


class FrozenDict(Mapping, Hashable, Sized, Container):
    def __init__(self,dct):
        if (isinstance(dct,dict)):
            self.dct = deepcopy(dct)
        else:
            self.dct = dict((k,v) for (k,v) in dct)
        self.hash = 0
        for (k,v) in self.dct.items():
            self.hash += hash((k,v))
    def __len__(self):
        return len(self.dct)
    def __iter__(self):
        return iter(self.dct)
    def __getitem__(self, __key: Any) -> Any:
        return self.dct[__key]
    def __hash__(self):
        return self.hash

# Код, помогающий в отладке


def compare_dicts(d1, d2):
    assert set(d1.keys()) == set(d2.keys())
    assert set(d1.values()) == set(d2.values())
    assert set(d1.items()) == set(d2.items())

    counter = 0
    iterated_keys = set()
    for k in d1:
        assert d1[k] == d2[k]
        iterated_keys.add(k)
        counter += 1

    assert len(d1) == counter
    assert len(d2) == counter
    assert iterated_keys == set(d2) == set(d2.keys())


def test_freeze_dict():
    dct = {1: 2, "a": "b", 5.0: None, None: 42}
    frozendict = FrozenDict(dct)
    compare_dicts(dct, frozendict)

    dct["check"] = True
    try:
        frozendict["check"] = True
        assert False
    except TypeError:
        pass

    assert "check" in dct and dct["check"]
    assert "check" not in frozendict
    try:
        lol = frozendict["check"]
        assert False
    except KeyError:
        pass


def test_init_frozendict_from_generator():
    dct = dict((k, v) for k, v in zip(range(10), range(15, 25)))
    frozendict = FrozenDict((k, v) for k, v in zip(range(10), range(15, 25)))
    compare_dicts(dct, frozendict)


def test_order_does_not_matter():
    some_list = [(2, 10), (3, 20), (1, 30)]
    sorted_list = sorted(some_list)

    d1 = FrozenDict(some_list)
    d2 = FrozenDict(sorted_list)

    assert hash(d1) == hash(d2)
    compare_dicts(d1, d2)


if __name__ == "__main__":
    test_freeze_dict()
    test_init_frozendict_from_generator()
    test_order_does_not_matter()