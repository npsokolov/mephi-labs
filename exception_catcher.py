import contextlib
from typing import Any
from functools import wraps

"""
В данном задании требуется реализовать декоратор `exception_catcher`.
Если данным декоратором обернуть функцию, то в случае возникновения исключений при вызове обернутой функции исключение
будет перехватываться и отправляться в обработчик `handler`, который передается в качестве параметра при создании декоратора.
Если данным декоратором обернуть класс, то аналогичный перехват исключений будет происходить при вызове любого метода в классе.
Обработчик `handler` принимает три аргумента -- кортеж args, словарь kwargs и пойманное исключение `exception`.
Если обработчик в свою очередь бросит исключение, то оно не будет уже перехватываться и пройдет выше по стеку вызовов.
Если обработчик не бросил исключение, то возвращенное обработчиком значение должно быть возвращено из обернутой функции
или из метода обернутого класса.
"""


def exception_catcher(handler):
    # Замените это на ваш код
    def dummy(obj):
        if (isinstance(obj,type)):
            def func(f,cls):
                @wraps(f)
                def wrapper(*args,**kwargs):
                    try:
                        return f(*args,**kwargs)
                    except Exception as e:
                        return handler((cls,),kwargs,e)
                return wrapper
            #@wraps(obj)
            class WrappedClass:
                def __init__(self,*args,**kwargs):
                    self.dec = obj(*args,**kwargs)
                def __getattribute__(self, s: str) -> Any:
                    try:
                        x = super().__getattribute__(s)
                    except AttributeError:
                        pass
                    else:
                        return x
                    attr = self.dec.__getattribute__(s)
                    if (isinstance(attr,type(self.__init__))):
                        return func(attr,self)
                    else:
                        return attr
            return WrappedClass 
        else:
            @wraps(obj)
            def wrapper(*args,**kwargs):
                try:
                    return obj(*args,**kwargs)
                except Exception as e:
                    return handler(args,kwargs,e)
            return wrapper
    return dummy


# Код, помогающий в отладке


def some_exception_handler(args, kwargs, exception):
    if isinstance(exception, ValueError):
        return args, kwargs, str(exception), 42
    elif isinstance(exception, NotImplementedError):
        return "Lol"
    else:
        raise exception


@exception_catcher(some_exception_handler)
def bad_function(a, b, c):
    if a + b < 10:
        return c
    elif a + b == 15:
        raise NotImplementedError
    elif a + b == 35:
        raise Exception("Bad function")
    else:
        raise ValueError("a + b >= 10")


@exception_catcher(some_exception_handler)
class Foo:
    def __init__(self, a, b, c):
        self.a = a
        self.b = b
        self.c = c

    def oops(self, x):
        if x < self.a:
            raise ValueError("x < a")
        return x - self.a

    def under_construction(self):
        raise NotImplementedError()


class Bar(Foo):
    def under_construction(self):
        return self.a + self.b

    def new_bad_function(self):
        raise NotImplementedError()

    @exception_catcher(some_exception_handler)
    def new_function(self):
        raise ValueError(self.c)


@contextlib.contextmanager
def assert_raises(exception_type):
    try:
        yield
    except exception_type:
        pass
    else:
        assert False


def check_bad_result(bad_result, args, kwargs, exception_message):
    assert isinstance(bad_result, tuple)
    assert len(bad_result) == 4
    assert bad_result[0] == args, bad_result
    assert bad_result[1] == kwargs, bad_result
    assert bad_result[2] == exception_message, bad_result
    assert bad_result[3] == 42, bad_result


def test_function_exception():
    assert bad_function(1, 2, 3) == 3

    check_bad_result(bad_function(5, 5, 5), (5, 5, 5), {}, "a + b >= 10")

    assert bad_function(10, 5, 0) == "Lol"

    with assert_raises(Exception):
        bad_function(20, 15, 1)


def test_class_exception():
    foo = Foo(10, 20, 30)
    assert foo.oops(200) == 190
    check_bad_result(foo.oops(x=5), (foo,), {"x": 5}, "x < a")
    assert foo.under_construction() == "Lol"

    bar = Bar(10, 20, 30)
    check_bad_result(bar.oops(x=5), (bar,), {"x": 5}, "x < a")
    assert bar.under_construction() == 30
    with assert_raises(NotImplementedError):
        bar.new_bad_function()
    check_bad_result(bar.new_function(), (bar,), {}, str(bar.c))


def main():
    test_function_exception()
    test_class_exception()


if __name__ == '__main__':
    main()