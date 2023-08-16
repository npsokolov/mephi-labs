class EnhancedList(list):
    """
    Улучшенный list.
    Данный класс является наследником класса list и добавляет к нему несколько новых атрибутов.

    - first -- позволяет получать и задавать значение первого элемента списка.
    - last -- позволяет получать и задавать значение последнего элемента списка.
    - size -- позволяет получать и задавать длину списка:
        - если новая длина больше старой, то дополнить список значениями None;
        - если новая длина меньше старой, то удалять значения из конца списка.
    """
    @property
    def first(self):
        return self.__getitem__(0)
    
    @first.setter
    def first(self,value):
        self.__setitem__(0,value)
    
    @property
    def last(self):
        return self.__getitem__(-1)
    
    @last.setter
    def last(self,value):
        self.__setitem__(-1,value)
        
    @property
    def size(self):
        return self.__len__()
    
    @size.setter
    def size(self,new_size:int):
        if (self.__len__()>new_size):
            for _ in range(self.__len__()-new_size):
                self.pop()
        else:
            self.extend([None]*(new_size-self.__len__()))
