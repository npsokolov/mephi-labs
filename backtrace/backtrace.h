#pragma once

// Input:
//   - backtrace - массив, который требуется заполнить адресами возврата из стека
//   - limit - размер массива backtrace
// Output:
//   - количество элементов, записанных в массив
//   - backtrace[0] равен адресу возврата из функции Backtrace
int Backtrace(void* backtrace[], int limit);

// Input:
//   - addr - указатель на инструкцию внутри произвольной функции
// Output:
//   - статически выделенную строку c именем функции, которой принадлежит инструкция
char* AddrToName(void* addr);

void PrintBt();
