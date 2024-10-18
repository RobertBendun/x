import sys

small_numbers = {
    0: [''] * 4,
    1: ['jeden'   ,'jedenaście'    ,'dziesieć'        ,'sto'        ],
    2: ['dwie'     ,'dwanaście'     ,'dwadzieścia'     ,'dwieście'   ],
    3: ['trzy'    ,'trzynaście'    ,'trzydzieści'     ,'trzysta'    ],
    4: ['cztery'  ,'czternaście'   ,'czterdzieści'    ,'czterysta'  ],
    5: ['pieć'    ,'pietnaście'    ,'piećdziesiąt'    ,'piećset'    ],
    6: ['sześć'   ,'szesnaście'    ,'sześćdziesiąt'   ,'sześćset'   ],
    7: ['siedem'  ,'siedemnaście'  ,'siedemdziesiąt'  ,'siedemset'  ],
    8: ['osiem'   ,'osiemnaście'   ,'osiemdziesiąt'   ,'osiemset'   ],
    9: ['dziewięć','dziewiętnaście','dziewięćdziesiąt','dziewięćset'],
}

large_numbers = [
    ['tysiac' ,'tysiace' ,'tysiecy'  ],
    ['milion' ,'miliony' ,'milionow' ],
    ['miliard','miliardy','miliardow'],
    ['bilion' ,'biliony' ,'bilionow' ],
    ['biliard','biliardy','biliardow'],
    ['trylion','tryliony','trylionow'],
]


def number_in_words(n: int) -> str:
    def sep(*args, by = ' '):
        s = ""
        for i, arg in enumerate(args):
            if i != 0 and len(arg):
                s += by
            s += arg
        return s


    assert n >= 0, "Only positive numbers are supported"
    if n == 0:
        return 'zero'

    words = ""

    while n != 0:
        hundreds, tens, units = n % 1000 // 100, n % 100 // 10, n % 10
        if tens == 1 and units > 0:
            between_11_and_19 = units
            tens, units = 0, 0
        else:
            between_11_and_19 = 0

        if units == 1 and hundreds + tens + int(between_11_and_19) == 0:
            k = 0
        elif units in (2,3,4):
            k = 1
        else:
            k = 2

        if hundreds+tens+units+int(between_11_and_19) > 0:
            words = sep(
                small_numbers[hundreds][3],
                small_numbers[tens][2],
                small_numbers[between_11_and_19][1],
                small_numbers[units][0],
                words,
            )
        n //= 1000
    return words.strip()

iterations = int(sys.argv[1]) if len(sys.argv) >= 2 else 30

edges = []

for i in range(20, 40):
    suffix = ""
    if i % 10 in [2, 3, 4]:
        suffix += "y"

    sentence = f"To zdanie ma {number_in_words(i)} liter{suffix}"
    spaces = sum(1 for char in sentence if char.isspace())
    letters = len(sentence) - spaces

    # print(letters, i, letters == i, sentence)
    edges.append((i, letters))

rejects = {int(reject) for reject in sys.argv[2:]}

while True:
    next_edges = []

    for edge in edges:
        if edge[1] not in rejects:
            next_edges.append(edge)
            continue
        rejects.add(edge[0])

    if len(next_edges) == len(edges):
        break
    edges = next_edges

edges = next_edges
print('digraph Numbers_In_Words {')
print('  rankdir="LR";')
print('  label="Liczba liter w zdaniu które mówi, że ma daną liczbę liter";')

for reject in {int(reject) for reject in sys.argv[2:]}:
    print(f"  {reject} [fontcolor=red];")

for edge in edges:
    print("  %d -> %d;" % edge)

print('}')

# TODO Some stats for each level how many nodes are there and how much linkage there is
# Watch out for 4 -> 6 -> 5 -> 4 cycle
