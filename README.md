## Tiny Heap Allocator
---

```
 _________
< T-alloc >
 ---------
   \
    \
        .--.
       |o_o |
       |:_/ |
      //   \ \
     (|     | )
    /'\_   _/`\
    \___)=(___/
```

```
A tiny heap allocator like malloc() made with C for learning purpose.
---

### Resource followed:
[Danluu](https://danluu.com/malloc-tutorial/)
---

### Features :
- Uses sbrk() to move the break in heap.
- Searches for free spaces before moving the break.
- Uses meta data in a double linked list data structure.
- Splitting of blocks on freeing memory.
- Coalescing consecutive free blocks into one.
---

### Limitations:
- Will fail if multiple processes use this allocation.
- No proper safety checks for going out of bounds.
- User can easily access the meta data if the size is known.
- Uses first fit approach so is not optimised for memory storage.
---

