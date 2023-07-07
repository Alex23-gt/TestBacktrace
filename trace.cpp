#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <stdio.h>

//Необходимо вызвать эту функцию для вывода
void backtrace() {
  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  // Unwind frames one by one, going up the frame stack.
  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    printf("0x%lx:", pc);

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      printf(" (%s+0x%lx)\n", sym, offset);
    } else {
      printf(" -- error: unable to obtain symbol name for this frame\n");
    }
  }
}

void foo() {
  backtrace(); // <- backtrace here!
}

void bar() {
  foo();
}

//для компиляции в терминале использовал строку: gcc -o trace -Wall -g trace.cpp -lunwind (при условии нахождения в папке с файлом trace.cpp)
//в результате получаем адрес и имя вызываемых команд
//адрес можно рассшифорвать, используя команду addr2line <адрес> -e trace. В результате получим номер строки кода, на которой была вызвана функция с введенным адресом
//Вывод имен функций получается не совсем понятным и наполнен лишними символами. По рекомендациям я пробовал сделать его более читабельным, используя приведенный ниже закомментированный код. Однако у меня возники проблемы с компиляцией. Не хотела распознаваться операция abi::__cxa_demangle
int main(int argc, char **argv) {
  bar();

  return 0;
}

/*#define UNW_LOCAL_ONLY
#include <cxxabi.h>
#include <libunwind.h>
#include <cstdio>
#include <cstdlib>

void backtrace() {
  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  // Unwind frames one by one, going up the frame stack.
  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    std::printf("0x%lx:", pc);

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      char* nameptr = sym;
      int status;
      char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
      if (status == 0) {
        nameptr = demangled;
      }
      std::printf(" (%s+0x%lx)\n", nameptr, offset);
      std::free(demangled);
    } else {
      std::printf(" -- error: unable to obtain symbol name for this frame\n");
    }
  }
}

namespace ns {

template <typename T, typename U>
void foo(T t, U u) {
  backtrace(); // <-------- backtrace here!
}

}  // namespace ns

template <typename T>
struct Klass {
  T t;
  void bar() {
    ns::foo(t, true);
  }
};

int main(int argc, char** argv) {
  Klass<double> k;
  k.bar();

  return 0;
}*/
