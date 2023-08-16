from collections.abc import Callable, Hashable, Iterable
from collections import OrderedDict, namedtuple
from functools import wraps

def lru_cache(max_items: int) -> Callable:
    """
    Функция создает декоратор, позволяющий кэшировать результаты выполнения обернутой функции по принципу LRU-кэша.
    Размер LRU кэша ограничен количеством max_items. При попытке сохранить новый результат в кэш, в том случае, когда
    размер кэша уже равен max_size, происходит удаление одного из старых элементов, сохраненных в кэше.
    Удаляется тот элемент, к которому обращались давнее всего.
    Также у обернутой функции должен появиться атрибут stats, в котором лежит объект с атрибутами cache_hits и
    cache_misses, подсчитывающие количество успешных и неуспешных использований кэша.
    :param max_items: максимальный размер кэша.
    :return: декоратор, добавляющий LRU-кэширование для обернутой функции.
    """
    stats = namedtuple('stats', ['cache_hits', 'cache_misses'])
    def decorator(f):
        @wraps(f)
        def wrapper(*args,**kwargs):
            str_of_args="{}".format((args,kwargs))
            if (str_of_args in mydict):
                mydict.move_to_end(str_of_args)
                wrapper.stats = wrapper.stats._replace(cache_hits=wrapper.stats.cache_hits+1)
                ret = mydict[str_of_args]
            else:
                wrapper.stats = wrapper.stats._replace(cache_misses=wrapper.stats.cache_misses+1)
                if (len(mydict)==max_items):
                    mydict.popitem(False)
                ret = f(*args,**kwargs)
                mydict[str_of_args]=ret
            return ret
        wrapper.stats = stats(0,0)
        mydict = OrderedDict()
        return wrapper

        
    return decorator