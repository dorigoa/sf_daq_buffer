### BufferBinaryWriter.cpp:63-64
Ignored write() error: if write returns -1, in the comparison of line 64 it is casted to size_t which is unsigned (even if its type was size_t which is signed), and it turns to SIZE_MAX which is always not less than sizeof()...
Possible exception is never raised.

Test code:
```
int main()
{
    ssize_t i = -1;
    std::cout << "i=" << i << std::endl;
    size_t st = 1000000;
    std::cout << "st=" << st << std::endl;
    bool b = (i < st);
    std::cout << "b=" << (b ? "true" : "false") << std::endl; // should print "true" as -1 is < 1000000
    return 0;
}
```

