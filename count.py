import time

start = time.time()

a = 1

while a < 1000000000:
#    print(a)
    a += 111

print(a)
duration = time.time() - start
print(duration * 1e3, "ms")
