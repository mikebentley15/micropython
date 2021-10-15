try:
    try:
        from ucollections import deque
    except ImportError:
        from collections import deque
except ImportError:
    print("SKIP")
    raise SystemExit


d = deque((), 2)
print(len(d))
print(bool(d))

try:
    d.popleft()
except IndexError:
    print("IndexError")

print(d.append(1))
print(len(d))
print(bool(d))
print(d.popleft())
print(len(d))

d.append(2)
print(d.popleft())

d.append(3)
d.append(4)
print(len(d))
print(d.popleft(), d.popleft())
try:
    d.popleft()
except IndexError:
    print("IndexError")

d.append(5)
d.append(6)
d.append(7)
print(len(d))
print(d.popleft(), d.popleft())
print(len(d))
try:
    d.popleft()
except IndexError:
    print("IndexError")

# Case where get index wraps around when appending to full deque
d = deque((), 2)
d.append(1)
d.append(2)
d.append(3)
d.append(4)
d.append(5)
print(d.popleft(), d.popleft())

# Negative maxlen is not allowed
try:
    deque((), -1)
except ValueError:
    print("ValueError")

# Unsupported unary op
try:
    ~d
except TypeError:
    print("TypeError")

d = deque((), 5)

# Indexing on an empty object - out of bounds
try:
    print(d[0])
except IndexError:
    print("IndexError")

d.append(3)
d.append(5)
d.append(7)

# Indexing on an object that has elements
print(d[0])
print(d[1])
print(d[2])

# Indexing with negative numbers - indexing is not supported
print(d[-1])
print(d[-2])
print(d[-3])

# Out of bounds indexing - indexing is not supported
try:
    print(d[3])
except IndexError:
    print("IndexError")
try:
    print(d[-4])
except IndexError:
    print("IndexError")

# Iterating
for val in d:
    print(val)

# Initialize with empty iterables
print(deque(list(), 5))                # empty list
print(deque(tuple(), 6))               # empty tuple
print(deque(set(), 7))                 # empty set
print(deque(dict(), 8))                # empty dict
print(deque((x for x in tuple()), 9))  # empty generator
print(deque(deque(tuple(), 3), 9))     # other dequeue

# Initialize with non-empty iterables
print(deque([1], 5))                         # list
print(deque((1,2), 6))                       # tuple
print(deque((x for x in [1,2,3,4,5,6]), 9))  # generator
print(deque(d, 9))                           # other dequeue
# Unordered, so order them for testing
print(sorted(deque({1, 2, 3}, 7)))           # set
print(sorted(deque({'a': 1, 'b': 2}, 8)))    # dict

# Initialize with iterable having more items than maximum size
print(deque([1], 0))                         # list
print(deque((1,2), 1))                       # tuple
print(deque((x for x in [1,2,3,4,5,6]), 3))  # generator
print(deque(d, len(d)-1))                    # other dequeue
