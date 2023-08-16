import logging
from functools import wraps
from typing import Any
from inspect import ismethod
from types import FunctionType
"""
В данном задании требуется написать декоратор, добавляющий логирование в вызов функций.
Должна быть возможность применять этот декоратор как к отдельной функции, так и к классу.
Если декоратор применяется к классу, то логирование должно добавиться к каждому методу в классе, имя которого не
начинается с двух символов `_`.
Также при применении декоратора к классу, логирование должно добавиться только к тем методам, которые определены
непосредственно в декорируемом класса, но не к унаследованным от базовых классов методам.
У декоратора должен быть параметр, в который передается логирующая функция. Чтобы сделать запись в лог, нужно
вызвать эту функцию, передав в нее строку лога с параметрами для форматирования.
"""


def with_logging(log_function):
    """
    Добавить логирование к одной функции или ко всем методам в классе.
    :param log_function: функция, реализующая запись лога.
    """
    def decorator(obj):
        if (isinstance(obj,type)):
            for (name,attr) in obj.__dict__.items():
                if isinstance(attr,type(obj.__init__)) and not name.startswith("__"):
                    setattr(obj,name,decorator(attr))
            return obj
        else:
            @wraps(obj)
            def wrapper(*args,**kwargs):
                x = obj(*args,**kwargs)
                s = "Called " + obj.__name__ + "; args=" + str(args) +"; kwargs="+ str(kwargs)
                log_function(s)
                return x
            return wrapper
    return decorator

logger = logging.getLogger("list_logger")


@with_logging(logger.info)
def foo(a, b):
    return a + b


@with_logging(logger.info)
class Bar:
    def __init__(self):
        self.x = 5

    def get_x(self):
        return self.x

    @property
    def xx(self):
        return self.x * 2


class Foo(Bar):
    def __init__(self):
        super().__init__()
        self.y = 15

    def get_y(self):
        return self.y

    @property
    def yy(self):
        return self.y * 2

    def __len__(self):
        return 0


class Baz:
    def __init__(self):
        self.x = 5

    def get_x(self):
        return self.x

    @property
    def xx(self):
        return self.x * 2


@with_logging(logger.info)
class Fzz(Baz):
    def __init__(self):
        super().__init__()
        self.y = 15

    def get_y(self):
        return self.y

    @property
    def yy(self):
        return self.y * 2

    def __len__(self):
        return 0


def test_wrap_function(log_list: list[str]):
    log_list.clear()
    assert foo(1, 2) == 3
    assert log_list[-1] == "Called foo; args=(1, 2); kwargs={}", log_list

    assert foo(a=1, b=2) == 3
    assert log_list[-1] == "Called foo; args=(); kwargs={'a': 1, 'b': 2}", log_list


def test_wrap_class_without_inheritance(log_list: list[str]):
    log_list.clear()

    a = Bar()
    assert not log_list

    a.x = 50
    assert not log_list
    assert a.get_x() == a.x
    assert len(log_list) == 1
    assert log_list[0] == f"Called get_x; args=({str(a)},); kwargs={{}}", log_list

    assert a.xx == a.x * 2
    assert len(log_list) == 1


def test_wrap_base_class(log_list: list[str]):
    log_list.clear()

    a = Foo()
    assert not log_list

    a.x = 50
    assert not log_list

    assert a.get_x() == a.x
    assert len(log_list) == 1
    assert log_list[0] == f"Called get_x; args=({str(a)},); kwargs={{}}", log_list

    assert a.xx == a.x * 2
    assert len(log_list) == 1

    assert a.get_y() == a.y
    assert len(log_list) == 1

    assert a.yy == a.y * 2
    assert len(log_list) == 1

    assert len(a) == 0
    assert len(log_list) == 1


def test_wrap_inherited_class(log_list: list[str]):
    log_list.clear()

    a = Fzz()
    assert not log_list

    a.x = 50
    assert not log_list

    assert a.get_x() == a.x
    assert not log_list

    assert a.xx == a.x * 2
    assert not log_list

    assert a.get_y() == a.y
    assert len(log_list) == 1
    assert log_list[0] == f"Called get_y; args=({str(a)},); kwargs={{}}", log_list

    assert a.yy == a.y * 2
    assert len(log_list) == 1

    assert len(a) == 0
    assert len(log_list) == 1


def run_tests(log_list: list[str]):
    #test_wrap_function(log_list)
    #test_wrap_class_without_inheritance(log_list)
    test_wrap_base_class(log_list)
    test_wrap_inherited_class(log_list)


class ListHandler(logging.Handler):
    def __init__(self, log_list):
        logging.Handler.__init__(self)
        self.log_list = log_list

    def emit(self, record):
        self.log_list.append(record.msg % record.args)


def main():
    log_list = list()
    logging.basicConfig(handlers=[ListHandler(log_list)], level=logging.INFO)
    run_tests(log_list)


if __name__ == "__main__":
    main()