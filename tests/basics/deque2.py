# Tests for deques with "check overflow" flag and other extensions
# wrt to CPython.
try:
    try:
        from ucollections import deque
    except ImportError:
        from collections import deque
except ImportError:
    print("SKIP")
    raise SystemExit


# Initial sequence is not supported
try:
    deque([1, 2, 3], 10)
except ValueError:
    print("ValueError")

# Not even empty list, only empty tuple
try:
    deque([], 10)
except ValueError:
    print("ValueError")

# Only fixed-size deques are supported, so length arg is mandatory
try:
    deque(())
except TypeError:
    print("TypeError")

d = deque((), 2, True)

try:
    d.popleft()
except IndexError:
    print("IndexError")

print(d.append(1))
print(d.popleft())

d.append(2)
print(d.popleft())

d.append(3)
d.append(4)
print(d.popleft(), d.popleft())
try:
    d.popleft()
except IndexError as e:
    print(repr(e))

d.append(5)
d.append(6)
print(len(d))
try:
    d.append(7)
except IndexError as e:
    print(repr(e))
print(len(d))

print(d.popleft(), d.popleft())
print(len(d))
try:
    d.popleft()
except IndexError as e:
    print(repr(e))

d = deque((), 5, True)
d.append(3)
d.append(5)
d.append(7)

# slice indexing with step=1 - slices not supported
try:
    print(d[:])
except TypeError as e:
    print(repr(e))
try:
    print(d[1:])
except TypeError as e:
    print(repr(e))
try:
    print(d[1:3])
except TypeError as e:
    print(repr(e))
try:
    print(d[1:-1])
except TypeError as e:
    print(repr(e))
try:
    print(d[::1])
except TypeError as e:
    print(repr(e))

# slice indexing with step!=1 - slices not supported
try:
    print(d[::2])
except TypeError as e:
    print(repr(e))
try:
    print(d[1::2])
except TypeError as e:
    print(repr(e))
try:
    print(d[1:3:3])
except TypeError as e:
    print(repr(e))
try:
    print(d[-1::-1])
except TypeError as e:
    print(repr(e))
try:
    print(d[::3])
except TypeError as e:
    print(repr(e))
