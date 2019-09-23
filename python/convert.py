from math import log

import rtttl

tone = "starwars:d=4,o=5,b=180:8f,8f,8f,2a#.,2f.6,8d#6,8d6,8c6,2a#.6,f.6,8d#6,8d6,8c6,2a#.6,f.6,8d#6,8d6,8d#6,2c6,p,8f,8f,8f,2a#.,2f.6,8d#6,8d6,8c6,2a#.6,f.6,8d#6,8d6,8c6,2a#.6,f.6,8d#6,8d6,8d#6,2c6"

def run():
  tune = rtttl.RTTTL(tone)

  notes = list(tune.notes())

  # find base duration

  min_note = min(dur for freq, dur in notes if dur != 0)

  #print [dur for freq, dur in notes]

  for divisor in xrange(50):
    min_note_i = min_note / (divisor + 1)
    if all((dur / min_note_i).is_integer() for freq, dur in notes):
      break
  else:
    raise ValueError("could not find base duration")
    
  base_duration = min_note_i

  transformed = [(int(round(1000000./freq)) if freq > 0 else 0, int(round(dur/base_duration))) for (freq, dur) in notes]

  print 'const Note NOTES[] PROGMEM = {'
  print ', '.join("{%s, %s}" % (freq, dur) for (freq, dur) in transformed)
  print '};'
  print
  print '#define NUM_NOTES', len(transformed)
  print

  generate_mult(int(base_duration))

def nearPower(y):
  base = int(log(y, 2))
  dist = y - 2**base
  sign = 1
  return dist, base, sign

def generate_mult(dist):
  ops = []
  while dist > 2:
    dist, nearest, sign = nearPower(dist)
    ops.append((nearest, sign))

  print "#define APPLY_DURATION(x) ",

  for (i, (shift, sign)) in enumerate(ops):
    if i > 0:
      print '-' if sign == -1 else '+',
    print "(x << %s)" % shift,

  for _ in xrange(dist):
    print '+ x',

  print

if __name__ == '__main__':
  run()
