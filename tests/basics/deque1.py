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

# indexing on an empty object - out of bounds
try:
    print(d[0])
except IndexError:
    print("IndexError")

d.append(3)
d.append(5)
d.append(7)

# indexing on an object that has elements
print(d[0])
print(d[1])
print(d[2])

# indexing with negative numbers - indexing is not supported
print(d[-1])
print(d[-2])
print(d[-3])

# out of bounds indexing - indexing is not supported
try:
    print(d[3])
except IndexError:
    print("IndexError")
try:
    print(d[-4])
except IndexError:
    print("IndexError")

# iterating
for val in d:
    print(val)
