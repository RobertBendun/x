import typing

class refzip(typing.Sequence):
    def __init__(self, *args):
        self.lists = args

    def __len__(self):
        return min(map(len, self.lists))

    def __getitem__(self, i):
        return tuple(l[i] for l in self.lists)

    def __setitem__(self, i, tup):
        for j, x in enumerate(tup):
            self.lists[j][i] = x

    def __iter__(self):
        return zip(*self.lists)

    def __contains__(self, x):
        return x in zip(*self.lists)

name = ["Gertruda", "Wiesława", "Anastazja"]
age  = [40, 30, 20]

assert ("Gertruda", 40) in refzip(name, age)
assert len(refzip(name, age)) == 3
assert all(a == b for a, b in zip(iter(refzip(name, age)), iter(zip(name, age))))

def sort(xs: typing.Sequence):
    for i in range(1, len(xs)):
        key = xs[i]
        j = i-1
        while j >= 0 and key < xs[j]:
            xs[j+1] = xs[j]
            j -= 1
        xs[j+1] = key
    return xs

sort(refzip(name, age))
print(name)
print(age)

