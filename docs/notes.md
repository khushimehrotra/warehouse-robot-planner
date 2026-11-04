# Week 1 Notes

## 1. Why vec_push doubles the capacity (amortized O(1))
A single push is usually O(1), but when the array is full we must allocate
a bigger block and copy all n items, which costs O(n). If we grew by 1 each
time, every push would copy everything, giving O(n^2) total for n pushes.
If we double instead, copies happen only at sizes 4, 8, 16, ..., n, so the
total copying is about 4 + 8 + ... + n < 2n. Spread over n pushes, that is
under 2 copies per push, which is O(1) amortized.

## 2. Heap push and pop are O(log n)
The heap is a complete binary tree stored in an array, so with n items its
height is about log2(n). Push places the item at the bottom and sifts up at
most one swap per level. Pop moves the last item to the root and sifts down
at most one swap per level. So both cost O(height) = O(log n).
Peeking at the minimum is O(1) because it is always at index 0.

## 3. The formula r * cols + c
A 2D grid is stored as one long array, row after row. To reach cell (r, c)
we skip r full rows (each has cols cells) and then move c cells along.
Example: with cols = 8, cell (2, 5) is at index 2*8 + 5 = 21.
The reverse is r = index / cols and c = index % cols. We will use this
to turn grid cells into graph node numbers in Week 3.

## 4. Every malloc needs a free
malloc takes memory from the heap and it stays reserved until we free it.
If we lose the pointer without freeing, that memory is leaked: it cannot be
reused until the program ends. In a long simulation, leaks add up. Rules:
whoever allocates must document who frees, free on every error path, and
set the pointer to NULL after free so it cannot be used by mistake.
AddressSanitizer reports leaks at program exit.

## 5. Library code returns error codes instead of calling exit()
A library function does not know what the caller wants to do on failure:
retry, print a message, use a default, or stop. If it calls exit(), the
whole program dies and the caller loses that choice. It also makes the
function impossible to unit test, because the test program would quit
too. Returning codes (0 ok, negative for errors) lets tests check
invalid inputs, and only main() decides to terminate.