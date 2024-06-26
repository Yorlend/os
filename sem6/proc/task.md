# Л/Р proc: анализ собственной программы

Система предоставляет в распоряжение разработчику мощные средства анализа разрабатываемого ПО. У процесса только виртуальное адресное пространство, физические страницы фактически выделяются по прерыванию.

Первое, что для проц. делает система -- создает адресное пространство и выделяет минимальное количество физических страниц. Одной из таких страниц является страница, содержащая точку входа (int main()).

#include <...> не так безвредна, как кажется. Выделяется пространство на каждом

com -- поле структуры task struct, содержит имя исполняемого файла.

|Элемент|Тип|Содержание|
|:-:|:-:|:-:|
|pagemap|64-разр. величина|отображение страниц на физич. страницы или область свопинга; для каждой страницы устанавливается соотв. биты, если установлен 63й бит -- страница в физ. памяти, 62й -- в области своппинга, 61й -- страница является отображаемой (file mapped) или разделяемой (shared) анонимной страницей|
|task|директория|Содержит поддиректории потоков|

```
/proc/<pid>/task (/...(дальше не идём))
```

Информацию из полей необходимо записать в текстовый файл.

```c
#include <stdio.h>
#define BUF_SIZE 0x100

int main(int argc, char *argv[])
{
    char buf[BUF_SIZE];
    int len, i;
    FILE *f;
    f = fopen("/proc/self/environ", "r");
    while ((len = fread(buf, 1, BUFSIZE, f)) > 0)
    {
        for (int i = 0; i < len; ++i)
        {
            if (buf[i] == 0)
            {
                buf[i] = 10; // '\n'
            }
        }
        buf[len] = 0;
        printf("%s", buf);
    }

    fclose(f);
    return 0;
}
```

proc предоставляет информацию не только о конкретных процессах, но и о системе в целом.
