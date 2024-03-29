from collections.abc import Iterable, Generator


def flatten(iterable: Iterable) -> Generator:
    """
    Генератор flatten принимает итерируемый объект iterable и с помощью обхода в глубину отдает все вложенные объекты.
    Для любых итерируемых вложенных объектов, не являющихся строками, нужно делать рекурсивный заход.
    В результате генератор должен пробегать по всем вложенным объектам на любом уровне вложенности.
    """
    for x in iterable:
        if isinstance(x, Iterable) and not (isinstance(x, str)):
            yield from flatten(x)
        else:
            yield x